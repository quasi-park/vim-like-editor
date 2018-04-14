#include "shell.h"

//Prototypes
void enableRawMode();
void disableRawMode();
//int readline(char *buf, struct history_list *hlist, char *env[], int envlen);
int determinearrow();

//Global Vars
struct termios orig_termios;
int HISTCOUNTER;
int mode, cursorpos;
struct char_line head, * curpos;
struct lines top, * curline;

void base_init(){
    top.fp = top.bp = &top;
    top.len = 0;
    (top.head).bp = (top.head).fp = &(top.head);
    curline = &top;
    curpos = &(curline->head);
    return;
}

void setup_screen(){
    printf("\x1b[2J");
    //DO extra setup here
    printf("\x1b[0;0H");
}

void add_new_line(struct lines *tgt, struct char_line * src){
    struct lines * new_line;

    if((new_line = (struct lines *)malloc(sizeof( struct lines))) == 0){
        fprintf(stderr, "cannot make new line anymore");
        exit(-1);
    }
    //Create new line and add it to the list
    (tgt->fp)->bp = new_line;
    new_line->fp = tgt->fp;
    tgt->fp = new_line;
    new_line->bp = tgt;


    //add new line to the list
    if(src != &(tgt->head)){
        (src->bp)->fp = &(tgt->head);
        (new_line->head).bp = (tgt->head).bp;
        (tgt->head).bp = src->bp;
        (new_line->head).fp = src;
    }else{
        (new_line->head).fp = (new_line->head).bp = &(new_line->head);
    }
    curline = new_line;
    curpos = (curline->head).fp;
    return;
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) < 0){
        perror("tcsetattr");
        exit(1);
    }
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) < 0) {
      perror("tcgetattr");
      exit(1);
    }
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO| ICANON | IEXTEN | ISIG);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
      perror("tcsetattr");
      exit(1);
    }
}

void norm_mode(){
    char c, cmd[BUFLEN];
    int i;
    while(1){
        c = getchar();
        switch(c){
            case ':':
              printf("%c", c);
              for(i = 0; i < BUFLEN;i++){
                cmd[i] = getchar();
                if(cmd[i] == '\n' || cmd[i] == EOF){
                    break;
                }
                printf("%c", cmd[i]);
              }
              cmd[i] = 0;
              break;
            default:
              break;

        }
    }
//do norm mode things
}

void ins_mode(){
    int refresh;
    char c;
    struct char_line *buf, *cbuf;

    refresh = 0;
    cursorpos = 0;

    for(;;){
        c = getchar();
        switch(c){
            case '\n':
                add_new_line(curline, curpos);
                printf("\x1b[1B");
                refresh = 1;
                break;
            case 127:
            case CTRL_KEY('H'):
                if(curpos == &(curline->head)){
                    printf("\b");
                    break;
                }
                buf = curpos;
                (buf->bp)->fp = curpos->fp;
                (buf->fp)->bp = curpos->bp;
                curpos = curpos->bp;
                buf->bp = buf->fp = 0;
                free(buf);
                buf = 0;
                refresh = 1;
                (curline->len)--;
                break;
            case ESC:
            case CTRL_KEY('Q'):
                c = getchar();
                if(c == '['){
                    determinearrow();
                }else if(c == ESC){
                    mode = NORM_MODE;
                    return;
                }else{
                    fprintf(stderr, "ESC command not found\n\r");
                }
                break;
            default:
                if((buf = (struct char_line *)malloc(sizeof(struct char_line))) == 0){
                    fprintf(stderr, "could not allocate anymore memory space!\n");
                    exit(-1);
                }
                buf->c = c;
                buf->fp = curpos->fp;
                curpos->fp = buf;
                buf->bp = curpos;
                curpos = buf;
                refresh = 1;
                cursorpos++;
                (curline->len)++;
                break;
        }
        if(refresh){
            printf("\x1b[?25l");
            printf("\x1b[2K\r");
            for(cbuf = (curline->head).fp; cbuf != &(curline->head); cbuf = cbuf->fp){
                printf("%c", cbuf->c);
            }
            printf("\x1b[?25h");
            printf("\x1b[%dD", cursorpos);
            refresh = 0;
        }
    }
}

void command_check(char *str){
   return;
}

/*
int readline(char *buf, struct history_list *hlist, char *env[], int envlen){
    int i;
    int cursorpos = 0;
    int refresh = 1;
    char c;
    struct bufchars bufheadc, *stringbuf, *ibuf, *cursorbuf;
    bufheadc.fp = bufheadc.bp = &bufheadc;
    bufheadc.len = 0;
    struct history_list *curhislist;
    curhislist = hlist;
    cursorbuf = &bufheadc;
    HISTCOUNTER = 0;

    for(i = 0; i < BUFLEN; i++){
        buf[i] = '\0';
    }

    while(1){
        refresh = 1;
        c = getchar();
        switch(c){
            case CTRL_KEY('M'):
                //Enter key
                for(ibuf = bufheadc.fp, i = 0; ibuf != &bufheadc && i < BUFLEN; ibuf = ibuf->fp, i++){
                    buf[i] = ibuf->c;
                }
                if(i < BUFLEN){
                    buf[i] = '\n';
                }else{
                    buf[BUFLEN - 1] = '\n';
                }
                for(ibuf = bufheadc.fp; bufheadc.fp != &bufheadc; ibuf = bufheadc.fp){
                    bufheadc.fp = ibuf->fp;
                    (ibuf->fp)->bp = &bufheadc;
                    (bufheadc.len)--;
                    if(cursorpos != 0){
                        cursorpos--;
                    }
                    free(ibuf);
                }
                return 1;
                break;
            case 9:
            case 11:
                break;
            case '\x1b':
                //Escape Sequence ex)Arrows
                c = getchar();
                if(c == '['){
                    refresh = determinearrow(&bufheadc, &cursorpos, &curhislist, hlist, &cursorbuf);
                }
                break;
            case CTRL_KEY('p'):
                //Get Prev history
                break;
            case CTRL_KEY('q'):
            case CTRL_KEY('c'):
                //Quit
                exit(1);
            case 127:
            case CTRL_KEY('H'):
                //Delete
                if(bufheadc.len > 0 && cursorpos > 0){
                    bufheadc.len--;
                    cursorpos--;
                    (cursorbuf->bp)->bp->fp = cursorbuf;
                    cursorbuf->bp = (cursorbuf->bp)->bp;
                }else{
                    printf("\a");
                }
                break;
            default:
                if(iscntrl(c)){
                    break;
                }
                stringbuf = (struct bufchars*)malloc(sizeof(struct bufchars));
                if(stringbuf == NULL){
                    fprintf(stderr, "Could not allocate new character.\n");
                    exit(1);
                }
                stringbuf->c = c;
                (cursorbuf->bp)->fp = stringbuf;
                stringbuf->bp = cursorbuf->bp;
                cursorbuf->bp = stringbuf;
                stringbuf->fp = cursorbuf;
                (bufheadc.len)++;
                cursorpos++;
                break;
        }
        if(refresh){
            printf("\x1b[?25l");
            printf("\x1b[2K");
            printf("\rmysh$");
            for(ibuf = bufheadc.fp; ibuf != &bufheadc; ibuf = ibuf->fp){
                printf("%c", ibuf->c);
            }
            printf("\x1b[?25h");
            if(bufheadc.len != cursorpos && cursorpos >= 0){
               printf("\x1b[%dD", bufheadc.len - cursorpos);
            }
        }
    }
}
*/

int determinearrow(){
    char c;

    c = getchar();
    switch(c){
        case ARROWUP:
            printf("\x1b[1A");
            return 0;
        case ARROWDOWN:
            printf("\x1b[1B");
            return 0;
        case ARROWLEFT:
            if(cursorpos > 0){
                cursorpos--;
                curpos = curpos->bp;
                printf("\x1b[1D");
            }else{
                printf("\a");
            }
            return 0;
        case ARROWRIGHT:
            if(cursorpos < curline->len){
                cursorpos++;
                curpos = curpos->fp;
                printf("\x1b[1C");
            }else{
                printf("\a");
            }
            return 0;
        default:
            return 1;
            break;
    }
}
