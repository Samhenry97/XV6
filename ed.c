#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

static int const MAX_LINE_LENGTH = 128;

//available modes for the editor
enum Mode {
    normal,
    insert
};
typedef enum Mode Mode; //so we can use 'Mode' as a typename instead of 'enum Mode'

//doubly-linked list
struct node {
    char *line;
    struct node *next;
    struct node *prev;
};

//editor state
struct ed_state {
    struct node *ll; //pointer to the beginning of our linked-list buffer
    Mode mode;
    int line_num;
    int edited;     // true if edited since the last save
    int lineNums;   // true if you want line numbers
    struct node *clipboard; // node of the line that is coppied
};
struct ed_state *ed; //global editor state


//free the memory pointed to by ed's linked list buffer
void free_mem() {
    struct node *free_buf1 = ed->ll;
    struct node *free_buf2 = free_buf1->next;
    free(free_buf1);
    while (free_buf2) {
        free_buf1 = free_buf2;
        free_buf2 = free_buf1->next;
        free(free_buf1);
    }
}

//get a user's command
int getcmd(char *buf, int nbuf)
{
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

// saves the file
void save() {
    // TODO: save the file
}

//frees memory and exits the program.
void quit() {
    free_mem();     //free buffer memory -- TODO: are we ever going to call this anywhere else? maybe we can puth the code here
    free(ed);       //free the editor state 'object'
    exit(0);
}

//process a user command in normal mode
void process_nm_command(char *cmd) {
    if (!strcmp(cmd, "s")) {        // save
        save();
    }
    else if (!strcmp(cmd, "q")) {   // quit
        if (!ed->edited)
            quit();
    }
    else if (!strcmp(cmd, "sq")) {  // save & quit
        save();
        quit();
    }
    else if (!strcmp(cmd, "wq")) {  // quit w/o saving
        quit();
    }
}

// process a user command in insert mode
void process_im_command(char *cmd) {

    if (!strcmp(cmd, "c")) { // TODO: this won't work because you have to have a line number given after c
        // TODO: copy the line given to a clipboard variable (copy the node?)
    }
    else if (!strcmp(cmd, "p")) { // TODO: this won't work becouse you have to have a line number given after p
        // TODO: paste the clipboard contents before the line given
    }
    else {
        // TODO: create a new line at the current location
    }
}


//print the current editor state
void print_state() {
    char* msg = "";

    if (ed->mode == insert)
        msg = "insert";
    else if (ed->mode == normal)
        msg = "normal";
    else
        msg = "something else";

    printf(1, "\nCurrent mode: %s\n", msg);
    printf(1, "Current line number: %d\n", ed->line_num);
    printf(1, "Buffer pointer: %d\n\n", ed->ll);
}

// prints a help message
void print_help() {
    printf(1, "==================================================== ed help:\n"
            "|  General Commands:\n"
            "|      .   switch to normal mode\n"
            "|      i   switch to insert mode\n"
            "|      ln  toggle line numbers\n"
            "|      h   list ed commands\n"
            "|\n"
            "|  Normal Mode------------\n"
            "|      q   quit\n"
            "|      s   save\n"
            "|      sq  save and quit\n"
            "|      wq  quit w/o saving\n"
            "|\n"
            "|  Insert Mode------------\n"
            "|      c ##    copy line number\n"
            "|      p ##    paste before line number\n"
            "|      {ENTER} write new line at current positionk\n"
            "====================================================\n"
    );
}

// prints the file to the console
void print_file() {

    // TODO: logic for how much to print
    
    int ln = 1;
    struct node *buffer = ed->ll;
    if (buffer->line) {
        if (ed->lineNums) { // if line numbers is turned on
            printf(1, "   %d  %s", ln, buffer->line);
            ++ln;
        }
        else {
            printf(1, "%s", buffer->line);
        }
    }
    while (buffer->next) {
        buffer = buffer->next;
        if (buffer->line) {
            if (ed->lineNums) { // if line numbers is turned on
                char* spaces = (ln < 10) ? "   " // TODO - can I put a tab character instead?
                             : (ln < 100) ? "  "
                             : (ln < 1000) ? " "
                             : "" ; // handle line number spacing
                printf(1, "%s%d  %s", spaces, ln, buffer->line);
                ++ln;
            }
            else {
                printf(1, "%s", buffer->line);
            }
        }
    }
}


int main(int argc, char *argv[])
{
    int fp;
    struct node *buf = malloc(sizeof(*buf));
    memset(buf, 0, sizeof(buf));

    //set beginning editor state
    ed = malloc(sizeof(*ed));
    ed->line_num = 0;
    ed->lineNums = 1;
    ed->ll = buf; //save a ptr to the beginning of the list

    if (argc > 1) {
        //open file and read it into the buffer
        char *filename = argv[1];

        fp = open(filename, O_RDONLY);
        if (fp < 0) {
            printf(1, "There was an error opening file %s\n", filename);
        }
        else {
            char c;
            int i, cc, eof=0;

            while(!eof) {
                //read one character at a time until we hit the end of the line
                char *linebuf = malloc(MAX_LINE_LENGTH);
                memset(linebuf, 0, sizeof(linebuf));
                for (i=0; i+1<MAX_LINE_LENGTH; i++) {
                    cc = read(fp, &c, 1);
                    if (cc < 1) {
                        eof = 1;
                        break;
                    }
                    linebuf[i] = c;
                    if (c == '\n' || c == '\r') {
                        break;
                    }
                }

                //null-terminate the string and store in the node
                linebuf[i+1] = '\0';
                buf->line = linebuf;

                //create a new node and set pointers
                struct node *newbuf = malloc(sizeof(*newbuf));
                buf->next = newbuf;
                newbuf->prev = buf;

                //make buf point to the new node 
                buf = newbuf;
            }
        }
        close(fp);
    }

    print_file();

    //loop waiting for user input
    char *command = malloc(MAX_LINE_LENGTH);
    memset(command, 0, sizeof(command));

    while (1) {
        print_state(); // DEBUGGING

        //read from command line
        getcmd(command, MAX_LINE_LENGTH);
        command[strlen(command)-1] = 0;  // chop \n

        // handle mode switching or ...
        if (!strcmp(command, ".")) 
            ed->mode = normal;
        else if (!strcmp(command, "i"))
            ed->mode = insert;

        // ... do general settings or ...
        else if (!strcmp(command, "h"))
            print_help();
        else if (!strcmp(command, "ln")) {
            ed->lineNums = (ed->lineNums) ? 0 : 1 ;
            print_file();
        }
        // ... process user command
        else if (ed->mode == normal)
            process_nm_command(command);
        else if (ed->mode == insert)
            process_im_command(command);

    } //end user input loop
    
    //no exit() because that will be called when user enters 'q'
}


///////////////////////////////// COMMANDS

/*

    General----------------
        .   switch to normal mode
        i   switch to insert mode
        ln  toggle line numbers
        h   list ed commands

    Normal Mode------------
        q   quit
        s   save
        sq  save and quit
        wq  quit w/o saving

    Insert Mode------------
        c ##    copy line number
        p ##    paste before line number
                write new line at current position

*/