#include "shell.h"

extern void enableRawMode();
extern void base_init();
extern void setup_screen();
extern void ins_mode();

struct termios orig_termios;


int main(int argc, char const *argv[]) {
    enableRawMode();
    base_init();
    setup_screen();

    if(argc > 1){
        //open file here
    }
    ins_mode();
    //start loop here
    return 0;
}
