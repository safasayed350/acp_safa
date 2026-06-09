#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <conio.h>
#include <windows.h>

/* ══════════════════ layout constants ═══════════════════════════════
   Total terminal width  = COLS + 2 (border) + MENU_W = 57+2+20 = 79
   Total terminal height = 1(title)+1(top)+ROWS+1(bot)+1(status)+2(prompt) = 24
   ═══════════════════════════════════════════════════════════════════ */
#define ROWS        18      /* canvas drawable rows                  */
#define COLS        57      /* canvas drawable cols                  */
#define MAX_OBJECTS 50

#define MENU_W      20      /* menu panel width (including border)   */
#define MENU_X      (COLS+2)/* menu starts right after canvas border */

/* Screen row positions */
#define ROW_TITLE   0
#define ROW_TOP     1       /* top border of canvas                  */
#define ROW_CANVAS  2       /* first drawable canvas row on screen   */
#define ROW_BOT     (ROW_CANVAS + ROWS)     /* = 20                  */
#define ROW_STATUS  (ROW_BOT + 1)           /* = 21                  */
#define ROW_PROMPT  (ROW_STATUS + 1)        /* = 22                  */

/* ══════════════════ colors ══════════════════════════════════════════ */
#define COL_DEFAULT (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define COL_CYAN    (FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#define COL_YELLOW  (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#define COL_GREEN   (FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#define COL_RED     (FOREGROUND_RED|FOREGROUND_INTENSITY)
#define COL_TITLE   (BACKGROUND_BLUE|BACKGROUND_INTENSITY|\
                     FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#define COL_MENU_HD (BACKGROUND_GREEN|\
                     FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY)

/* ══════════════════ shape descriptor ═══════════════════════════════ */
typedef struct {
    int  active;
    int  type;      /* 0=circle 1=rect 2=line 3=triangle */
    char ch;        /* '*' or '_'                         */
    int cx, cy, r;                          /* circle    */
    int rx1, ry1, rx2, ry2;                 /* rectangle */
    int lx1, ly1, lx2, ly2;                 /* line      */
    int tx1, ty1, tx2, ty2, tx3, ty3;       /* triangle  */
} Shape;

/* ══════════════════ globals ═════════════════════════════════════════ */
static char  C[ROWS][COLS];
static Shape S[MAX_OBJECTS];
static int   SC = 0;
static HANDLE hOut;

/* ══════════════════ console helpers ═════════════════════════════════ */
static void set_color(WORD a) { SetConsoleTextAttribute(hOut, a); }

static void goto_xy(int col, int row) {
    COORD c = { (SHORT)col, (SHORT)row };
    SetConsoleCursorPosition(hOut, c);
}

static void hide_cursor(void) {
    CONSOLE_CURSOR_INFO ci = {1, FALSE};
    SetConsoleCursorInfo(hOut, &ci);
}
static void show_cursor(void) {
    CONSOLE_CURSOR_INFO ci = {100, TRUE};
    SetConsoleCursorInfo(hOut, &ci);
}

/* Clear a single screen row to spaces */
static void clear_row(int row, int width) {
    goto_xy(0, row);
    for (int i = 0; i < width; i++) putchar(' ');
}

/* ══════════════════ canvas primitives ═══════════════════════════════ */
static void pset(int r, int c, char ch) {
    if (r>=0 && r<ROWS && c>=0 && c<COLS) C[r][c]=ch;
}

static void bline(int r1,int c1,int r2,int c2,char ch){
    int dr=abs(r2-r1),dc=abs(c2-c1);
    int sr=r1<r2?1:-1, sc=c1<c2?1:-1, err=dr-dc;
    for(;;){
        pset(r1,c1,ch);
        if(r1==r2&&c1==c2) break;
        int e2=2*err;
        if(e2>-dc){err-=dc;r1+=sr;}
        if(e2< dr){err+=dr;c1+=sc;}
    }
}

static void bcircle(int cy,int cx,int rad,char ch){
    if(rad<=0){pset(cy,cx,ch);return;}
    int x=0,y=rad,d=1-rad;
#define P8(px,py)\
    pset(cy+(py),cx+(px),ch);pset(cy-(py),cx+(px),ch);\
    pset(cy+(py),cx-(px),ch);pset(cy-(py),cx-(px),ch);\
    pset(cy+(px),cx+(py),ch);pset(cy-(px),cx+(py),ch);\
    pset(cy+(px),cx-(py),ch);pset(cy-(px),cx-(py),ch)
    P8(x,y);
    while(x<y){
        if(d<0)d+=2*x+3; else{d+=2*(x-y)+5;y--;}
        x++;P8(x,y);
    }
#undef P8
}

static void brect(int r1,int c1,int r2,int c2,char ch){
    for(int c=c1;c<=c2;c++){pset(r1,c,ch);pset(r2,c,ch);}
    for(int r=r1;r<=r2;r++){pset(r,c1,ch);pset(r,c2,ch);}
}

/* ══════════════════ redraw shapes ═══════════════════════════════════ */
static void redraw(void){
    memset(C,' ',sizeof C);
    for(int i=0;i<SC;i++){
        Shape *s=&S[i];
        if(!s->active) continue;
        switch(s->type){
        case 0: bcircle(s->cy,s->cx,s->r,s->ch); break;
        case 1: brect(s->ry1,s->rx1,s->ry2,s->rx2,s->ch); break;
        case 2: bline(s->ly1,s->lx1,s->ly2,s->lx2,s->ch); break;
        case 3:
            bline(s->ty1,s->tx1,s->ty2,s->tx2,s->ch);
            bline(s->ty2,s->tx2,s->ty3,s->tx3,s->ch);
            bline(s->ty3,s->tx3,s->ty1,s->tx1,s->ch);
            break;
        }
    }
}

/* ══════════════════ static UI drawing ══════════════════════════════
   Called once (and after cls). Never scrolls.
   ═══════════════════════════════════════════════════════════════════ */
static void draw_static_ui(void) {
    /* ── title bar ── */
    set_color(COL_TITLE);
    goto_xy(0, ROW_TITLE);
    printf(" 2D GRAPHICS EDITOR  [cols:0-%d  rows:0-%d]  ",COLS-1,ROWS-1);

    /* ── canvas border ── */
    set_color(COL_GREEN);
    /* top edge */
    goto_xy(0, ROW_TOP);
    putchar('+');
    for(int c=0;c<COLS;c++) putchar('-');
    putchar('+');
    /* side edges */
    for(int r=0;r<ROWS;r++){
        goto_xy(0,      ROW_CANVAS+r); putchar('|');
        goto_xy(COLS+1, ROW_CANVAS+r); putchar('|');
    }
    /* bottom edge */
    goto_xy(0, ROW_BOT);
    putchar('+');
    for(int c=0;c<COLS;c++) putchar('-');
    putchar('+');

    /* ── menu panel ── */
    set_color(COL_MENU_HD);
    goto_xy(MENU_X, ROW_TOP);      printf("+---- MENU ---+");
    set_color(COL_YELLOW);
    goto_xy(MENU_X, ROW_TOP+ 1);   printf("| 1 Circle    |");
    goto_xy(MENU_X, ROW_TOP+ 2);   printf("| 2 Rect      |");
    goto_xy(MENU_X, ROW_TOP+ 3);   printf("| 3 Line      |");
    goto_xy(MENU_X, ROW_TOP+ 4);   printf("| 4 Triangle  |");
    goto_xy(MENU_X, ROW_TOP+ 5);   printf("|-------------|");
    goto_xy(MENU_X, ROW_TOP+ 6);   printf("| 5 Delete    |");
    goto_xy(MENU_X, ROW_TOP+ 7);   printf("| 6 Modify    |");
    goto_xy(MENU_X, ROW_TOP+ 8);   printf("|-------------|");
    goto_xy(MENU_X, ROW_TOP+ 9);   printf("| 7 List      |");
    goto_xy(MENU_X, ROW_TOP+10);   printf("| 8 Clear     |");
    goto_xy(MENU_X, ROW_TOP+11);   printf("|-------------|");
    goto_xy(MENU_X, ROW_TOP+12);   printf("| q Quit      |");
    goto_xy(MENU_X, ROW_TOP+13);   printf("+-------------+");
    /* object count display area */
    goto_xy(MENU_X, ROW_TOP+15);
    set_color(COL_DEFAULT);
    printf("Objs: %d/%d   ", SC, MAX_OBJECTS);

    set_color(COL_DEFAULT);
}

/* ── paint canvas cells (called after every action) ── */
static void paint_canvas(void) {
    redraw();
    for(int r=0;r<ROWS;r++){
        goto_xy(1, ROW_CANVAS+r);
        for(int c=0;c<COLS;c++){
            char ch=C[r][c];
            if     (ch=='*') set_color(COL_CYAN);
            else if(ch=='_') set_color(COL_YELLOW);
            else             set_color(COL_DEFAULT);
            putchar(ch);
        }
    }
    /* refresh object count */
    set_color(COL_DEFAULT);
    goto_xy(MENU_X, ROW_TOP+15);
    printf("Objs: %d/%d   ", SC, MAX_OBJECTS);
    set_color(COL_DEFAULT);
}

/* ── status line (row 21) ── */
static void show_status(const char *msg) {
    clear_row(ROW_STATUS, 79);
    set_color(COL_RED);
    goto_xy(0, ROW_STATUS);
    printf(">> %-75s", msg);
    set_color(COL_DEFAULT);
}

/* ── prompt area (rows 22-23): clear then print label ── */
static void clear_prompt_area(void) {
    clear_row(ROW_PROMPT,   79);
    clear_row(ROW_PROMPT+1, 79);
}

/* ══════════════════ input helpers ═══════════════════════════════════ */
static int ask_int(const char *label) {
    clear_prompt_area();
    show_cursor();
    set_color(COL_GREEN);
    goto_xy(0, ROW_PROMPT);
    printf("%-40s: ", label);
    set_color(COL_DEFAULT);
    int v=0;
    scanf("%d",&v);
    hide_cursor();
    return v;
}

static char ask_char(void) {
    clear_prompt_area();
    show_cursor();
    set_color(COL_GREEN);
    goto_xy(0, ROW_PROMPT);
    printf("Fill char (* or _): ");
    set_color(COL_DEFAULT);
    char buf[8]={0};
    scanf("%7s",buf);
    hide_cursor();
    return (buf[0]=='_') ? '_' : '*';
}

/* ══════════════════ object management ═══════════════════════════════ */
static int alloc_shape(void){
    if(SC<MAX_OBJECTS) return SC++;
    for(int i=0;i<MAX_OBJECTS;i++) if(!S[i].active) return i;
    return -1;
}

static void add_circle(void){
    int idx=alloc_shape();
    if(idx<0){show_status("Max objects reached!");return;}
    int cx=ask_int("Centre col (0-56)");
    int cy=ask_int("Centre row (0-17)");
    int r =ask_int("Radius");
    char ch=ask_char();
    Shape *s=&S[idx]; memset(s,0,sizeof*s);
    s->active=1;s->type=0;s->cx=cx;s->cy=cy;s->r=r;s->ch=ch;
    show_status("Circle added.");
}

static void add_rectangle(void){
    int idx=alloc_shape();
    if(idx<0){show_status("Max objects reached!");return;}
    int c1=ask_int("Top-left  col");
    int r1=ask_int("Top-left  row");
    int c2=ask_int("Bot-right col");
    int r2=ask_int("Bot-right row");
    if(r1>r2){int t=r1;r1=r2;r2=t;}
    if(c1>c2){int t=c1;c1=c2;c2=t;}
    char ch=ask_char();
    Shape *s=&S[idx]; memset(s,0,sizeof*s);
    s->active=1;s->type=1;s->rx1=c1;s->ry1=r1;s->rx2=c2;s->ry2=r2;s->ch=ch;
    show_status("Rectangle added.");
}

static void add_line(void){
    int idx=alloc_shape();
    if(idx<0){show_status("Max objects reached!");return;}
    int c1=ask_int("Start col");
    int r1=ask_int("Start row");
    int c2=ask_int("End   col");
    int r2=ask_int("End   row");
    char ch=ask_char();
    Shape *s=&S[idx]; memset(s,0,sizeof*s);
    s->active=1;s->type=2;s->lx1=c1;s->ly1=r1;s->lx2=c2;s->ly2=r2;s->ch=ch;
    show_status("Line added.");
}

static void add_triangle(void){
    int idx=alloc_shape();
    if(idx<0){show_status("Max objects reached!");return;}
    int c1=ask_int("Vertex 1 col"); int r1=ask_int("Vertex 1 row");
    int c2=ask_int("Vertex 2 col"); int r2=ask_int("Vertex 2 row");
    int c3=ask_int("Vertex 3 col"); int r3=ask_int("Vertex 3 row");
    char ch=ask_char();
    Shape *s=&S[idx]; memset(s,0,sizeof*s);
    s->active=1;s->type=3;
    s->tx1=c1;s->ty1=r1;s->tx2=c2;s->ty2=r2;s->tx3=c3;s->ty3=r3;s->ch=ch;
    show_status("Triangle added.");
}

/* list_objects: shown inside the menu panel area (rows 14-18) */
static void list_objects(void){
    static const char *nm[]={"Circ","Rect","Line","Tri "};
    /* clear listing area inside menu */
    for(int r=ROW_TOP+14;r<=ROW_TOP+18;r++){
        goto_xy(MENU_X, r);
        printf("%-15s", "");
    }
    int row=ROW_TOP+14, shown=0;
    set_color(COL_CYAN);
    for(int i=0;i<SC && row<=ROW_TOP+18;i++){
        if(!S[i].active) continue;
        goto_xy(MENU_X, row++);
        printf("[%2d]%s'%c'", i, nm[S[i].type], S[i].ch);
        shown++;
    }
    if(!shown){
        goto_xy(MENU_X, ROW_TOP+14);
        printf("(empty) ");
    }
    set_color(COL_DEFAULT);
    show_status("Objects listed in menu panel. Press any key...");
    _getch();
}

static void delete_object(void){
    list_objects();
    int id=ask_int("Index to delete (-1=cancel)");
    if(id<0||id>=SC){show_status("Cancelled.");return;}
    S[id].active=0;
    show_status("Object deleted.");
}

static void modify_object(void){
    list_objects();
    int id=ask_int("Index to modify (-1=cancel)");
    if(id<0||id>=SC||!S[id].active){show_status("Invalid index.");return;}
    Shape *s=&S[id];
    switch(s->type){
    case 0:
        s->cx=ask_int("New centre col");
        s->cy=ask_int("New centre row");
        s->r =ask_int("New radius");
        s->ch=ask_char(); break;
    case 1:
        s->rx1=ask_int("New TL col"); s->ry1=ask_int("New TL row");
        s->rx2=ask_int("New BR col"); s->ry2=ask_int("New BR row");
        s->ch =ask_char(); break;
    case 2:
        s->lx1=ask_int("New start col"); s->ly1=ask_int("New start row");
        s->lx2=ask_int("New end   col"); s->ly2=ask_int("New end   row");
        s->ch =ask_char(); break;
    case 3:
        s->tx1=ask_int("New V1 col"); s->ty1=ask_int("New V1 row");
        s->tx2=ask_int("New V2 col"); s->ty2=ask_int("New V2 row");
        s->tx3=ask_int("New V3 col"); s->ty3=ask_int("New V3 row");
        s->ch =ask_char(); break;
    }
    show_status("Object modified.");
}

static void clear_all(void){
    for(int i=0;i<SC;i++) S[i].active=0;
    show_status("Canvas cleared.");
}

/* ══════════════════ main ════════════════════════════════════════════ */
int main(void){
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Force console to exactly 80x25 */
    COORD bufSz = {80, 25};
    SetConsoleScreenBufferSize(hOut, bufSz);
    SMALL_RECT win = {0, 0, 79, 24};
    SetConsoleWindowInfo(hOut, TRUE, &win);

    hide_cursor();
    system("cls");
    draw_static_ui();
    paint_canvas();
    show_status("Ready. Press 1-8 to draw, q to quit.");

    for(;;){
        /* wait for keypress with cursor blinking in menu column */
        goto_xy(MENU_X+1, ROW_TOP+14);
        set_color(COL_GREEN);
        printf("Key> ");
        set_color(COL_DEFAULT);

        int key=_getch();

        /* redraw static frame in case prompt input scrolled things */
        system("cls");
        draw_static_ui();

        switch(key){
        case '1': add_circle();    break;
        case '2': add_rectangle(); break;
        case '3': add_line();      break;
        case '4': add_triangle();  break;
        case '5': delete_object(); break;
        case '6': modify_object(); break;
        case '7': list_objects();  break;
        case '8': clear_all();     break;
        case 'q': case 'Q':
            system("cls");
            show_cursor();
            set_color(COL_DEFAULT);
            goto_xy(0,0);
            printf("Goodbye!\n");
            return 0;
        default:
            show_status("Unknown key. Press 1-8 or q.");
            break;
        }

        paint_canvas();
    }
}
