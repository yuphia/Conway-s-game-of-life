#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#define coordsCheck(x, y)\
        field->field [coordinatesToCellNum (*field, x, y)].curr == '1'

#define ROUNDUP(x, s) (((x)+(s)) % (s))

#define CELL(x,y) field->field[field->xSize*ROUNDUP(y, field->ySize) + ROUNDUP(x, field->xSize)].curr


#define getX_Coordinate(f, i) ((i) % (f)->xSize)
#define getY_Coordinate(f, i) ((i) / (f)->xSize)

struct _coordinates
    {
        int x;
        int y;
    };

struct _cell
    {
        char curr;
        char next;
    };

struct _field 
    {
        int xSize;
        int ySize;

        struct _cell* field;
    };

void printField (struct _field* field, WINDOW* win);
void field_bZero (struct _field * field);

int simpleEvolve (struct _field* field,int cellNumber);
int evolution (struct _field * field, WINDOW* win);
int evolve (struct _field* field);

int isCellAlive (struct _field *field, int cellNumber);

int coordinatesToCellNum (struct _field field, int x, int y);

int waitForStart (struct _field* field, WINDOW* win);

void loadArrayToField (struct _field * field, chtype* arr, WINDOW* win);
void translatePrintToField (struct _field* field, chtype* arr, int lineAmount, WINDOW* win);

int main()
{
    initscr();

    struct _field field = {188/2, 40, calloc ((188/2)*(40), sizeof(struct _cell))};

    int height, width, start_y, start_x;
    height = 40;
    width = 188; 
    start_y = 10;
    start_x = 0;

    start_color();

    mvprintw (3, 78, "Conway's game of life\n");

    init_pair (1, COLOR_BLACK, COLOR_WHITE);

    WINDOW* win = newwin (height, width, start_y, start_x);

    wbkgd (win, COLOR_PAIR(1));
    
    keypad (stdscr, TRUE);
    keypad (win, TRUE);

    refresh();

    curs_set (0);

    box (win, 0, 0);
    wrefresh (win);

    field_bZero (&field);
    printField (&field, win);

    waitForStart (&field, win);

    timeout (100);
    getch();

    int isFinished = 0;             // !!!!!!!!!!!!!!!!!!!!!!  надо переписать детекцию в углах экрана

    int i = 0; 
    while (isFinished == 0)
    {
        isFinished = evolution (&field, win);
        
        timeout (100);
        char c = getch();
        if (c == ' ')
            waitForStart (&field, win);
        
        i++;
    }

    mvprintw (0, 0, "lifecycles = %d", i);

    mvprintw (4, 70, "Game finished\n");
    refresh ();

    waitForStart (&field, win);

    free (field.field);

    endwin();

    return 0;
}

void printField (struct _field* field, WINDOW* win)
{
    wclear (win);
    char tildaChar = '~';
    char ca = '<', cb = '>';
    for (int i = 0; i < field->xSize*field->ySize; i++)
    {
        if (field->field[i].curr == '1')
        {
            int x= getX_Coordinate(field, i);
            int y = getY_Coordinate(field, i);
            if (y%2 != 0 && x == 0)
            {
                mvwprintw (win, y, x, "%c", cb);
                mvwprintw (win, y, 2*field->xSize - 1, "%c", ca);
            }    
            else if (y%2 == 0)
            {    
                mvwprintw (win, y, 2*x, "%c", ca);
                mvwprintw (win, y, 2*x + 1, "%c", cb);
            }
            else
            {
                mvwprintw (win, y, 2*x - 1, "%c", ca);
                mvwprintw (win, y, 2*x, "%c", cb);              
            }


        }
    }    
    box (win, 0, (int)tildaChar);
    wrefresh (win);
}


void field_bZero (struct _field * field)
{
    for (int i = 0; i < field->xSize * field->ySize; i++)
    {
        field->field[i].curr = ' ';
        field->field[i].next = ' ';
    }
/*
    field->field [field->xSize + 1].curr = '1';
    field->field [field->xSize + 3].curr = '1';
    field->field [field->xSize*2 + 1].curr = '1';
    field->field [field->xSize + 0].curr = '1';
    field->field [field->xSize*2 + 3].curr = '1';
    field->field [field->xSize*3 + 2].curr = '1';
    field->field [field->xSize*3 + 0].curr = '1';*/
}

int evolution (struct _field * field, WINDOW* win)
{
    int isFinished = 0;

    for (int cellNumber = 0; cellNumber < field->xSize * field->ySize; cellNumber++)
    {

        simpleEvolve (field, cellNumber);
    }

    isFinished = evolve (field);
    printField (field, win);

    return isFinished;
}

int evolve (struct _field* field)
{
    int countAliveCells = 0;
    for (int cellNum = 0; cellNum < field->xSize * field->ySize; cellNum++)
    {
        if (field->field[cellNum].next == '1')
            countAliveCells++;

        field->field[cellNum].curr = field->field[cellNum].next;
        field->field[cellNum].next = ' '; 
    }
    return (countAliveCells != 0) ? 0 : 1;
}

int simpleEvolve (struct _field* field, int cellNumber)
{   
    int counter = 0;
    int x = getX_Coordinate(field, cellNumber);
    int y = getY_Coordinate(field, cellNumber);
    int alive = 0;

    for (int i = 0; i < 2; i++)
    {
        if (CELL(x+i-(y%2), y-1)  == '1') counter++;
        if (CELL(x+i-(y%2), y+1)  == '1') counter++;
    }
    if(CELL(x-1, y) == '1') counter++;
    if(CELL(x+1, y) == '1') counter++;
    
    if ( field->field[cellNumber].curr == '1') {
        if (counter > 3 || counter < 1) alive= 0;
        else alive = 1;
    } else {
        if ( counter == 3 ) alive = 1;
        else alive = 0;
    }
    if (alive) 
        field->field[cellNumber].next = '1';
    else    
        field->field[cellNumber].next = ' ';

    return counter;
}


int coordinatesToCellNum (struct _field field, int x, int y)
{
    return x + y*field.xSize;
}

int waitForStart (struct _field *field, WINDOW* win)
{
    printf("\033[?1003h\n");
    mousemask(ALL_MOUSE_EVENTS, NULL);
    char ca = '<', cb='>';

    chtype* readingArray = calloc (2*field->xSize * field->ySize, sizeof (chtype));

    while (1)
    {
        //wtimeout (win, 100000);
        int c = getch();
        MEVENT event;

        if (c == KEY_MOUSE)
        {
            int isEventOk = getmouse(&event);
            if (isEventOk == OK && (event.bstate & BUTTON1_CLICKED || event.bstate & BUTTON1_PRESSED))
            {
                if (event.y%2 == 0)
                {                  
                    if (event.x%2 == 0)  
                    {
                        mvwprintw (win, event.y - 10, event.x, "%c", ca);
                        mvwprintw (win, event.y - 10, event.x + 1, "%c", cb);
                        wrefresh (win);
                    }
                    else
                    {
                        mvwprintw (win, event.y - 10, event.x - 1, "%c", ca);
                        mvwprintw (win, event.y - 10, event.x, "%c", cb);
                        wrefresh (win);
                    }
                }
                else
                {
                    if (event.x == 0 || event.x == (2*field->xSize)-1)
                    {
                        mvwprintw (win, event.y - 10, 0, "%c", cb);
                        mvwprintw (win, event.y - 10, 2*field->xSize - 1, "%c", ca);
                        wrefresh (win);
                    }
                    else if (event.x%2 == 0)
                    {
                        mvwprintw (win, event.y - 10, event.x - 1, "%c", ca);
                        mvwprintw (win, event.y - 10, event.x, "%c", cb);
                        wrefresh (win);                       
                    }
                    else
                    {
                        mvwprintw (win, event.y - 10, event.x, "%c", ca);
                        mvwprintw (win, event.y - 10, event.x + 1, "%c", cb);
                        wrefresh (win); 
                    }
                }
            }                        
        }

        else if (c == ' ')
        {                
                loadArrayToField (field, readingArray, win);
                printf("\033[?1003l\n");
                free (readingArray);
                return 0;        

        }    
    }
}

void loadArrayToField (struct _field * field, chtype* arr, WINDOW* win)
{
    for (int j = 0; j < field->ySize*field->xSize; j += 2*field->xSize)
    {
        mvwinchstr (win, j/(2*field->xSize), 0, arr + j); 
        refresh();
    }

    translatePrintToField (field, arr, field->ySize, win);
}

void translatePrintToField (struct _field* field, chtype* arr, int lineAmount, WINDOW* win)
{
    for (int line = 0; line < lineAmount; line++)
    {
            if (line%2 == 0)
            for (int i = 0; i < field->xSize * 2; i+=2)
            {
                //mvprintw (4, 10*i, "i = %d, i/2 = %d", );
                if ((char)arr [i + 2*line*field->xSize] == '<')
                    field->field[i/2 + line*field->xSize].curr = '1';    
                else    
                    field->field[i/2 + line*field->xSize].curr = ' ';
            }
        else
        {
            if ((char)arr [2*line*field->xSize] == '<')
                field->field [line*field->xSize].curr = '1';
            else 
                field->field [line*field->xSize].curr = ' ';
            for (int i = 1; i < field->xSize * 2; i+=2)
            {
                if ((char)arr [i + 2*line*field->xSize] == '<')
                {
                    mvprintw (5, 1, "1");
                    field->field[i/2 + line*field->xSize].curr = '1';    
                    refresh ();
                }
                else    
                    field->field[i/2 + line*field->xSize].curr = ' ';
            }
        }
        //mvprintw (1, 0, "line = %d", line);
    }

    printField (field, win);
    wrefresh (win);
    //for (int i = 0; i < field->xSize; i++)
    //    mvprintw (2, i, "%d", field->field[i + field->xSize].curr == ' ');
}

