#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#define coordsCheck(x, y)\
        field->field [coordinatesToCellNum (*field, x, y)].curr == '1'
 

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

void printField (struct _field field, WINDOW* win);
void field_bZero (struct _field * field);
void getNeighbourCount (struct _field * field, int cellNumber);

struct _coordinates * getCellCoordinates (struct _field * field, int cellNumber);
int isCoordBorder (struct _field * field, struct _coordinates* coordinates);

int simpleEvolve (struct _field* field, struct _coordinates coords, int cellNumber, int isBorder);
int evolution (struct _field * field, WINDOW* win);
int evolve (struct _field* field);

int isCellAlive (struct _field *field, int cellNumber);

int checkForLeftTop (struct _field *field);
int checkForRightTop (struct _field *field);
int checkForRightBot (struct _field *field);
int checkForLeftBot (struct _field *field);

int checkTopBorder (struct _field *field, struct _coordinates coords);
int checkBotBorder (struct _field *field, struct _coordinates coords);
int checkLeftBorder (struct _field *field, struct _coordinates coords);
int checkRightBorder (struct _field *field, struct _coordinates coords);

int coordinatesToCellNum (struct _field field, int x, int y);

int waitForStart (struct _field* field, WINDOW* win);

void loadArrayToField (struct _field * field, chtype* arr, WINDOW* win);

int main()
{
    initscr();

    struct _field field = {190, 30, calloc (190*30, sizeof(struct _cell))};

    int height, width, start_y, start_x;
    height = 30;
    width = 190; 
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
    printField (field, win);

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

void printField (struct _field field, WINDOW* win)
{
    wclear (win);
    char tildaChar = '~';
    char cellChar = '#';
    for (int i = 0; i < field.xSize*field.ySize; i++)
    {
        if (field.field[i].curr == '1')
        {
            struct _coordinates* coord = getCellCoordinates (&field, i);
            mvwprintw (win, coord->y, coord->x, "%c", cellChar);
            box (win, 0, (int)tildaChar);
            wrefresh (win);

            free (coord);
        }
    }     
}


void field_bZero (struct _field * field)
{
    for (int i = 0; i < field->xSize * field->ySize; i++)
    {
        field->field[i].curr = ' ';
        field->field[i].next = ' ';
    }

    field->field [field->xSize + 3].curr = '1';
    field->field [field->xSize*2 + 1].curr = '1';
    field->field [field->xSize*2 + 3].curr = '1';
    field->field [field->xSize*3 + 3].curr = '1';
    field->field [field->xSize*3 + 2].curr = '1';
}

int evolution (struct _field * field, WINDOW* win)
{
    int isFinished = 0;

    for (int cellNumber = 0; cellNumber < field->xSize * field->ySize; cellNumber++)
    {
        struct _coordinates* coord = getCellCoordinates (field, cellNumber);

        simpleEvolve (field, *coord, cellNumber, isCoordBorder (field, coord));
        free (coord);
    }

    isFinished = evolve (field);
    printField (*field, win);

    return isFinished;
}

struct _coordinates * getCellCoordinates (struct _field * field, int cellNumber)
{
    struct _coordinates* cellCoordinates = calloc (1, sizeof (struct _coordinates));

    cellCoordinates->x = cellNumber % field->xSize;
    cellCoordinates->y = cellNumber / field->xSize;
    
    return cellCoordinates;
}

int isCoordBorder (struct _field * field, struct _coordinates* coordinates)
{
    if (coordinates->x == 0 || coordinates->x == field->xSize-1 || coordinates->y == 0 || coordinates->y == field->ySize-1)
        return 1;
    else
        return 0;
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

int simpleEvolve (struct _field* field, struct _coordinates coords, int cellNumber, int isBorder)
{   
    int counter = 0;

    if (!isBorder)
    {
        for (int i = -1; i < 2; i++)
        {
            if (field->field [cellNumber - field->xSize + i].curr == '1')
                counter++;

            if (field->field [cellNumber + field->xSize + i].curr == '1')
                counter++;

            if (field->field [cellNumber + i].curr == '1' && i != 0)
                counter++;
        }    
    }
    else    
    {
        if (coords.x == 0 && coords.y == 0)        
            counter += checkForLeftTop (field);

        if (coords.x == field->xSize-1 && coords.y == 0)
            counter += checkForRightTop (field);

        if (coords.x == 0 && coords.y == field->ySize-1)
            counter += checkForLeftBot (field);

        if (coords.x == field->xSize-1 && coords.y == field->ySize-1)
            counter += checkForRightBot (field); 

        if (coords.x == 0 && coords.y != 0 && coords.y != field->ySize-1)
            counter += checkLeftBorder (field, coords);

        if (coords.x != 0 && coords.y == 0 && coords.x != field->xSize-1)
            counter += checkTopBorder (field, coords);

        if (coords.x == field->xSize-1 && coords.y != 0 && coords.y != field->ySize-1)
            counter += checkRightBorder (field, coords);

        if (coords.x != 0 && coords.y == field->ySize-1 && coords.x != field->xSize-1)
            counter += checkBotBorder (field, coords);
    }

    if (counter == 3 || ((counter == 2 || counter == 3) && field->field[cellNumber].curr == '1'))
        field->field[cellNumber].next = '1';
    else    
        field->field[cellNumber].next = ' ';

    return counter;
}


int checkForLeftTop (struct _field *field)
{
    int innerCounter = 0;

    if (isCellAlive (field, 1)) innerCounter++;
    if (isCellAlive (field, field->xSize)) innerCounter++;
    if (isCellAlive (field, field->xSize + 1)) innerCounter++;           

    return innerCounter;
}

int checkForRightTop (struct _field *field)
{
    int innerCounter = 0;

    if (isCellAlive (field, field->xSize-1))   innerCounter++;
    if (isCellAlive (field, 2*field->xSize-1)) innerCounter++;
    if (isCellAlive (field, 2*field->xSize-2)) innerCounter++;           

    return innerCounter;
}

int checkForLeftBot (struct _field *field)
{
    int xSize = field->xSize;
    int ySize = field->ySize;
    int innerCounter = 0;

    if (isCellAlive (field, xSize*(ySize-1) + 1)) innerCounter++;
    if (isCellAlive (field, xSize*(ySize-2) + 1)) innerCounter++;
    if (isCellAlive (field, xSize*(ySize-2)))     innerCounter++;           

    return innerCounter;
}

int checkForRightBot (struct _field *field)
{
    int xSize = field->xSize;
    int ySize = field->ySize;
    int innerCounter = 0;

    if (isCellAlive (field, xSize*(ySize   ) - 1)) innerCounter++;
    if (isCellAlive (field, xSize*(ySize-1 ) - 1)) innerCounter++;
    if (isCellAlive (field, xSize*(ySize-1)))      innerCounter++;           

    return innerCounter;
}

int checkTopBorder (struct _field *field, struct _coordinates coords)
{
    int innerCounter = 0;

    for (int i = -1; i < 2; i++)
    {   
        if (coordsCheck (coords.x + i, coords.y+1))
            innerCounter++;
        
        if (coordsCheck (coords.x + i, coords.y) && i != 0)
            innerCounter++;

        if (coordsCheck (coords.x + i, field->ySize-1))
            innerCounter++;
    }

    return innerCounter;
}

int checkBotBorder (struct _field *field, struct _coordinates coords)
{
    int innerCounter = 0;

    for (int i = -1; i < 2; i++)
    {   
        if (coordsCheck (coords.x + i, 0))
            innerCounter++;

        if (coordsCheck (coords.x + i, coords.y-1))
            innerCounter++;
        
        if (coordsCheck (coords.x + i, coords.y) && i != 0)
            innerCounter++;
    }

    return innerCounter;
}

int checkLeftBorder (struct _field *field, struct _coordinates coords)
{
    int innerCounter = 0;

    for (int i = 0; i < 2; i++)
    {   
        if (coordsCheck (coords.x + i, coords.y-1))
            innerCounter++;
 
        if (coordsCheck (coords.x + i, coords.y+1))
            innerCounter++;
        
        if (coordsCheck (coords.x + i, coords.y) && i != 0)
            innerCounter++;
    }

    if (coordsCheck (field->xSize-1, coords.y))
        innerCounter++;
    
    if (coordsCheck (field->xSize-1, coords.y + 1))
        innerCounter++;

    if (coordsCheck (field->xSize-1, coords.y - 1))
        innerCounter++;

    return innerCounter;
}

int checkRightBorder (struct _field *field, struct _coordinates coords)
{
    int innerCounter = 0;

    for (int i = -1; i < 1; i++)
    {   
        if (coordsCheck (coords.x + i, coords.y-1))
            innerCounter++;
 
        if (coordsCheck (coords.x + i, coords.y + 1))
            innerCounter++;
        
        if (coordsCheck (coords.x + i, coords.y) && i != 0)
            innerCounter++;
    }

    if (coordsCheck (0, coords.y))
        innerCounter++;
    
    if (coordsCheck (0, coords.y + 1))
        innerCounter++;

    if (coordsCheck (0, coords.y - 1))
        innerCounter++;

    return innerCounter;
}

int isCellAlive (struct _field *field, int cellNumber)
{
    if (field->field[cellNumber].curr == '1')
        return 1;

    return 0;
}

int coordinatesToCellNum (struct _field field, int x, int y)
{
    return x + y*field.xSize;
}

int waitForStart (struct _field *field, WINDOW* win)
{
    printf("\033[?1003h\n");
    mousemask(ALL_MOUSE_EVENTS, NULL);
    char cellChar = '#';

    chtype* readingArray = calloc (field->xSize * field->ySize, sizeof (chtype));

    while (1)
    {
        //wtimeout (win, 100000);
        int c = getch();
        MEVENT event;

        if (c == KEY_MOUSE)
        {
            int isEventOk = getmouse(&event);
            if (isEventOk == OK && event.bstate & (BUTTON1_CLICKED || event.bstate & BUTTON1_PRESSED))
            {
                mvwprintw (win, event.y - 10, event.x, "%c", cellChar);
                wrefresh (win);
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
    for (int j = 0; j < field->ySize*field->xSize; j += field->xSize)
    {
        mvwinchstr (win, j/field->xSize, 0, arr + j);        
        refresh();
    }

    int i = 0;
    for (i = 0; (char)arr[i] != '\0'; i++)
    {   
        if((char)arr[i] != '#')
            field->field[i].curr = ' ';
        else
            field->field[i].curr = '1';

        mvprintw (2, i, "%c", field->field[i].curr);        
    }
}
