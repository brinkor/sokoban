#ifndef MYHEADER
#define MYHEADER

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480


static const int LEGEND_SIZE_Y = 36;
static const int BOARD_POSITION_X = 4;
static const int BOARD_POSITION_Y = LEGEND_SIZE_Y + 8;
static const int BOARD_SIZE_X = SCREEN_WIDTH - 8;
static const int BOARD_SIZE_Y = SCREEN_HEIGHT - 12 - LEGEND_SIZE_Y;
static const int MOVE_SPEED = 100;
static const int FIELD_SIZE = 40;
static const int PLAYER_SIZE = FIELD_SIZE;
static const int HORIZONTAL = -1;
static const int VERTICAL = 1;
static const int SPACE_SIZE = 4;
static const int EMPTY_FIELD = ' ';
static const int BOX_FIELD = '#';
static const int WALL_FIELD = '.';
static const int DESTINATION_FIELD = 'o';
static const int PLAYER_FIELD = 'P';
static const int AXIS_X = 1;
static const int AXIS_Y = 0;
static const int STARTING_BOARD_NUMBER = 1;
static const int OPTIONS_AVAILABLE = 3;
static const int CHOOSE_BOARD = 1;
static const int ADD_BOARD = 2;
static const int IN_A_ROW = 3;
static const int MAX_NAME_OF_BOARD_LENGTH = 100;

extern "C"
{
#include <SDL.h>
}

typedef enum
{
	EMPTY, WALL, BOX, DESTINATION, PLAYER, PLADEST, BOXDEST
}fieldType_t;

typedef struct
{
	fieldType_t type = EMPTY;
}field_t;

typedef struct
{
	field_t **field;
	int playerPosX;
	int playerPosY;
	int size = 9;
	//int number = 1;
	int destAmount = 0;
}board_t;

typedef struct
{
	int t1, t2, quit = 0, frames = 0, rc, previousX, previousY, menuSelect = CHOOSE_BOARD, inmenu = 0, alreadyTyped = 0, currentBoardNumber = STARTING_BOARD_NUMBER;
	double delta, worldTime = 0, fpsTimer = 0, fps = 0;
	bool newGame = true, endOfGame = false, pause = false, select = false, manyGamesInARow = false;
	char boardName[MAX_NAME_OF_BOARD_LENGTH];
	char typedIn[MAX_NAME_OF_BOARD_LENGTH];
}settings_t;

typedef struct
{
	SDL_Surface *screen = NULL, *charset = NULL, *box = NULL, *destination = NULL, *playerUp = NULL, *playerDown = NULL, *playerLeft = NULL, *playerRight = NULL, *player = NULL, *gameOver = NULL;
}bmps_t;

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y);
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);
void drawBlock(SDL_Surface *screen, int x, int y); //draw one standard wall and add it to board in x,y position
void drawEmptyField(SDL_Surface *screen, int color, int x, int y); //draw one empty field and add it to board in x,y position
void boardInit(board_t *board); //initialaze board (array), only after getBoardSize
void readBoardFromFile(board_t *board, settings_t *settings); //read board from .txt file ('P' - player starting position, ' ' - empty field, '#' - box, 'o' - destination point, '.' - wall
void drawBoard(bmps_t *bmps, int board_outline_color, int board_fill_color, board_t *board); // draw board below legend
void freeMemory(board_t *board, bmps_t *bmps); // free all memory (bmps + board array)
void freeBoardMemory(board_t *board); // free only board array
void movePlayer(board_t *board, settings_t *settings, int axis); // checking if player can be moved to another field, or if he can push box
void drawFieldImage(SDL_Surface *screen, SDL_Surface *image, int x, int y); // draw field of FIELD_SIZE size on the board (e.g. destination point, box)
bool loadAllBMPs(SDL_Renderer *renderer, SDL_Texture *scrtex, SDL_Window *window, bmps_t *bmps); // all bmps are loaded here
void startNewGame(board_t *board, settings_t *settings); // reading board size, reading board from file, initializing board, clearing arrays and resetting player starting posision to default
bool checkGameEnd(board_t *board); // checking if all the boxes are on the destination points
void showGameEnd(board_t *board, bmps_t *bmps); // shows Game Over graphic
int setWindow(bmps_t *bmps, SDL_Window *window, SDL_Renderer *renderer, int *rc); // setting window options, if something went wrong returns 1, else returns 0
void showPauseMenu(bmps_t *bmps, settings_t *settings, int fillColor, int outlineColor); // showing pause menu on the screeen instead of board
void getBoardSize(board_t *board, char *boardName); // checking file and counting characters and new lines, sets boardSize variable
void getEvent(SDL_Event *event, settings_t *settings, board_t *board, bmps_t *bmps); // handling of events (if there were any)
void showAddingNewBoard(bmps_t *bmps, settings_t *settings, int fillColor, int outlineColor); // modifing menu screen that it shows you field where you can type name for new board
void showBoardsAvailable(bmps_t *bmps, settings_t *settings, int fillColor, int outlineColor); // insist that menu shows all boards available (reading from boardList.txt file)
int countBoards(); // return number of boards (reding it form boardList.txt, couting new lines)
void getBoardName(settings_t *settings, int number); // setting boardName variable, number in the number of board from boardsAvailableMenu (counting form 1, as in menu)
void addNewBoard(char *newBoardName); // writing new line to boardList.txt

#endif // !MYHEADER
