// Intel 8250 serial port (UART).

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define COM1    0x3f8
#define COM2    0x2f8
#define COM3    0x3e8
#define COM4    0x2e8

static int uart;    // is there a uart?

int getSerialFromDevice(int device) {
  switch(device) {
    case 0: return COM1; break;
    case 1: return COM2; break;
    case 2: return COM3; break;
    case 3: return COM4; break;
    default: return -1;
  }
}

int getDeviceFromSerial(int serial) {
  switch(serial) {
    case COM1: return 0; break;
    case COM2: return 1; break;
    case COM3: return 2; break;
    case COM4: return 3; break;
    default: return -1;
  }
}

int uartgetnumdevices(void) {
  return uart;
}

void uartinitdevice(int com, int irq) {
  int device = getDeviceFromSerial(com);
  char *p;

  // Turn off the FIFO
  outb(com+2, 0);

  // 9600 baud, 8 data bits, 1 stop bit, parity off.
  outb(com+3, 0x80);    // Unlock divisor
  outb(com+0, 115200/9600);
  outb(com+1, 0);
  outb(com+3, 0x03);    // Lock divisor, 8 data bits.
  outb(com+4, 0);
  outb(com+1, 0x01);    // Enable receive interrupts.

  // If status is 0xFF, no serial port.
  if(inb(com+5) == 0xFF)
    return;
  uart++;

  // Acknowledge pre-existing interrupt conditions;
  // enable interrupts.
  inb(com+2);
  inb(com+0);
  picenable(irq);
  ioapicenable(irq, 0);

  // Announce that we're here.
  for(p="xv6...\n"; *p; p++)
    uartputc(*p, device);
}

// Does the uart device have input available?
int uartready(int device) {
  if(device >= uart) { return 0; }  // Safety net - return false if the device doesn't exist
  int serial = getSerialFromDevice(device);
  
  return (inb(serial+5) & 0x01);
}

// Initilize all of the serial ports
void uartinit(void) {
  uartinitdevice(COM1, IRQ_COM1);
  uartinitdevice(COM2, IRQ_COM2);
  uartinitdevice(COM3, IRQ_COM1);
  uartinitdevice(COM4, IRQ_COM2);
}

void
uartputc(int c, int device) {
  int serial = getSerialFromDevice(device);
  int i;

  if(!uart)
    return;
  for(i = 0; i < 128 && !(inb(serial+5) & 0x20); i++)
    microdelay(10);
  outb(serial, c);
}

static int
uartgetc(int device) {
  int serial = getSerialFromDevice(device);

  if(!uart)
    return -1;
  if(!(inb(serial+5) & 0x01))
    return -1;
  return inb(serial);
}

void
uartintr(int device) {
  consoleintr(uartgetc, device);
}
