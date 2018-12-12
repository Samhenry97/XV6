// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define IOCTL_CLEAR 7
#define IOCTL_MASK 42
#define SCREEN_HEIGHT 23
#define LINE_LENGTH 80

static int masks[4] = { 0, 0, 0, 0 };
enum AnsiState { ESC, LBRAC, VAL1, SEP, VAL2 } ansiState = ESC;
enum Color { BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE } bg = BLACK, fg = WHITE;
int val1, val2;
int cursorLine, cursorCol;
int saveCursorLine = 0, saveCursorCol = 0;

static void consputc(int, int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int device, int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i], device);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(int device, char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c, device);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(device, *argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(device, *argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s, device);
      break;
    case '%':
      consputc('%', device);
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%', device);
      consputc(c, device);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  cprintf(ALL_DEVS, "cpu with apicid %d: panic: ", cpu->apicid);
  cprintf(ALL_DEVS, s);
  cprintf(ALL_DEVS, "\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(ALL_DEVS, " %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgamovecursor(int line, int col) {
  // Bounds Checking
  if(line < 0) { line = 0; }
  else if(line > SCREEN_HEIGHT) { line = SCREEN_HEIGHT; }
  if(col < 0) { col = 0; }
  else if(col > LINE_LENGTH) { col = LINE_LENGTH; }
  
  int pos = line * LINE_LENGTH + col;
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos >> 8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | (bg << 12) | (fg << 8);
  
  cursorLine = line;
  cursorCol = col;
}

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos % 80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c & 0xff) | (bg << 12) | (fg << 8);

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0x0000 | (bg << 12) | (fg << 8), sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | (bg << 12) | (fg << 8);
  
  cursorLine = pos / LINE_LENGTH;
  cursorCol = pos % LINE_LENGTH;
}

static void cgaclear() {
  for (int i = 0; i < (SCREEN_HEIGHT * 2); ++i) { // erase twice the window height
    cgaputc('\n');
  }

  for (int i = 0; i < (SCREEN_HEIGHT * LINE_LENGTH); ++i) { // then put the cursor back up one window's height (so it's at the top)
    cgaputc(BACKSPACE);
  }
  
  cgamovecursor(0, 0);
}

static void
cgaclearline() {
  int pos = cursorLine * LINE_LENGTH;
  memset(crt+pos+cursorCol, 0x0000 | (bg << 12) | (fg << 8), sizeof(crt[0])*(LINE_LENGTH - cursorCol));
}

// Helper function to determine if the mask should output '*'
int isprint(int c) {
  return (c > 0x1f && c != 0x7f && c != '\n');
}

int isdigit(int c) {
  return ('0' <= c && c <= '9');
}

int convertAnsiToColor(int ansi) {
  switch(ansi) {
    case 30: case 40: return BLACK;
    case 31: case 41: return RED;
    case 32: case 42: return GREEN;
    case 33: case 43: return YELLOW;
    case 34: case 44: return BLUE;
    case 35: case 45: return MAGENTA;
    case 36: case 46: return CYAN;
    case 37: case 47: return WHITE;
    default: return -1;
  }
}

void changeGraphicsMode(int value) {
  if(value >= 30 && value <= 37) {
    fg = convertAnsiToColor(value);
  } else if(value >= 40 && value <= 47) {
    bg = convertAnsiToColor(value);
  } else if(value == 0) {
    fg = WHITE;
    bg = BLACK;
  }
}

int escapeSequence(int c) {
  // Ansi Escape Sequence State Machine 
  int endOfSequence = 0;
  switch(ansiState) {
    case ESC:
      if(c == '\33') { ansiState = LBRAC; }
      break;
    case LBRAC:
      if(c == '[') {
        val1 = 0;
        ansiState = VAL1;
      } else {
        ansiState = ESC;
      }
      break;
    case VAL1:
      if(isdigit(c)) {
        val1 = 10 * val1 + (c - '0');
      } else if(c == ';') {
        ansiState = SEP;
      } else if(c == 'K') {
        cgaclearline();
        ansiState = ESC;
        endOfSequence = 1;
      } else if(c == 'm') {
        changeGraphicsMode(val1);
        ansiState = ESC;
        endOfSequence = 1;
      } else {
        switch(c) {
          case 'A': cgamovecursor(cursorLine - val1, cursorCol); break;
          case 'B': cgamovecursor(cursorLine + val1, cursorCol); break;
          case 'C': cgamovecursor(cursorLine, cursorCol + val1); break;
          case 'D': cgamovecursor(cursorLine, cursorCol - val1); break;
          case 's': saveCursorLine = cursorLine; saveCursorCol = cursorCol; break;
          case 'u': cgamovecursor(saveCursorLine, saveCursorCol); break;
          case 'J': if(val1 == 2) { cgaclear(); } break;
          default: break;
        }
        ansiState = ESC;
        endOfSequence = 1;
      }
      break;
    case SEP:
      if(isdigit(c)) {
        val2 = c - '0';
        ansiState = VAL2;
      } else {
        ansiState = ESC;
      }
      break;
    case VAL2:
      if(isdigit(c)) {
        val2 = 10 * val2 + (c - '0');
      } else if(c == 'H' || c == 'f') {
        cgamovecursor(val1, val2);
        ansiState = ESC;
        endOfSequence = 1;
      } else if(c == 'm') {
        changeGraphicsMode(val1);
        changeGraphicsMode(val2);
        
        ansiState = ESC;
        endOfSequence = 1;
      } else if(c == ';') {
        if(val1 != -1) { changeGraphicsMode(val1); val1 = -1; }
        changeGraphicsMode(val2);
        ansiState = SEP;
      } else {
        ansiState = ESC;
      }
      break;
    default:
      break;
  }
  
  return !(ansiState == ESC && !endOfSequence);
}

void
consputc(int c, int device)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(device == ALL_DEVS) {
    for(int i = 0; i < uartgetnumdevices(); i++) {
      if(c == BACKSPACE) {
        uartputc('\b', i); uartputc(' ', i); uartputc('\b', i);
      } else {
        uartputc((masks[i] && isprint(c)) ? '*' : c, i);
      }
    }
  } else {
    if(c == BACKSPACE) {
      uartputc('\b', device); uartputc(' ', device); uartputc('\b', device);
    } else {
      uartputc((masks[device] && isprint(c)) ? '*' : c, device);
    }
  }
  
  if(device == 0 || device == ALL_DEVS) {
    if(device == 0 && escapeSequence(c)) { return; }
    
    if(c == BACKSPACE) {
      cgaputc(c);
    } else {
      cgaputc((masks[0] && isprint(c)) ? '*' : c);
    }
  }
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input[4]; // One for each teletype

#define C(x)  ((x)-'@')  // Control-x

void
consoleintr(int (*getc)(int), int device)
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc(device)) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'):  // Kill line.
      while(input[device].e != input[device].w &&
            input[device].buf[(input[device].e-1) % INPUT_BUF] != '\n'){
        input[device].e--;
        consputc(BACKSPACE, device);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input[device].e != input[device].w){
        input[device].e--;
        consputc(BACKSPACE, device);
      }
      break;
    default:
      if(c != 0 && input[device].e-input[device].r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input[device].buf[input[device].e++ % INPUT_BUF] = c;
        consputc(c, device);
        if(c == '\n' || c == C('D') || input[device].e == input[device].r+INPUT_BUF){
          input[device].w = input[device].e;
          wakeup(&input[device].r);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  } 
}

void consoleioctl(struct inode *ip, int key, int value) {
  if(key == IOCTL_CLEAR) {
    // if it's in a UART, just send <ESC>c
    uartputc(27, ip->minor); // <ESC>
    uartputc(99, ip->minor); // 'c'
    
    // if it's in a CGA window (which only TTY0 will be), do some funky stuff
    if (ip->minor == 0) {
      cgaclear();
    }
  } else if(key == IOCTL_MASK) {
      masks[ip->minor] = value;
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input[ip->minor].r == input[ip->minor].w){
      if(proc->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input[ip->minor].r, &cons.lock);
    }
    c = input[ip->minor].buf[input[ip->minor].r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input[ip->minor].r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff, ip->minor);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].ioctl = consoleioctl;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}
