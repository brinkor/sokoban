#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "myHeader.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char **argv) {
	settings_t settings; // all settings variables
	char text[128];
	bmps_t bmps; // all bmp surfaces
	SDL_Event event;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	board_t board; // player position and board array

	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	//setting up window
	settings.rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (setWindow(&bmps, window, renderer, &settings.rc) == 1)
		return 1;

	// loading images
	if (loadAllBMPs(renderer, scrtex, window, &bmps) == 1)
		return 1;

	int czarny = SDL_MapRGB(bmps.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(bmps.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(bmps.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(bmps.screen->format, 0x11, 0x11, 0xCC);
	int grey = SDL_MapRGB(bmps.screen->format, 0x93, 0x93, 0x93);

	settings.t1 = SDL_GetTicks();

	//board initialization and reading from file
	getBoardName(&settings, STARTING_BOARD_NUMBER);
	getBoardSize(&board, settings.boardName);
	boardInit(&board);
	readBoardFromFile(&board, &settings);

	settings.previousX = board.playerPosX;
	settings.previousY = board.playerPosY;

	while(!settings.quit) {

		//running pause menu
		if (settings.pause == true)
		{
			showPauseMenu(&bmps, &settings, czarny, czerwony);
			settings.t1 = SDL_GetTicks();
		}
		else
		{

			//spawdzanie czy nastapil koniec gry
			if (checkGameEnd(&board) == true)
				settings.endOfGame = true;

			settings.t2 = SDL_GetTicks();

			// here t2-t1 is the time in milliseconds since
			// the last screen was drawn
			// delta is the same time in seconds
			settings.delta = (settings.t2 - settings.t1) * 0.001;
			settings.t1 = settings.t2;
			settings.worldTime += settings.delta;


			//uruchamianie nowej gry
			if (settings.newGame == true)
			{
				startNewGame(&board, &settings);
				settings.newGame = false;
			}

			// moving player
			if (settings.previousX != board.playerPosX)
				movePlayer(&board, &settings, AXIS_X);
			else if (settings.previousY != board.playerPosY)
				movePlayer(&board, &settings, AXIS_Y);
			board.playerPosX = settings.previousX;
			board.playerPosY = settings.previousY;

			//board drawing
			SDL_FillRect(bmps.screen, NULL, czarny);
			drawBoard(&bmps, czerwony, grey, &board);

			//show info if game is over
			if (settings.endOfGame == true)
			{
				showGameEnd(&board, &bmps);
				//loading next board if manyGameInARow opiton is set
				if (settings.manyGamesInARow == true)
				{
					if (settings.currentBoardNumber < countBoards())
						settings.currentBoardNumber++;
					else
						settings.currentBoardNumber = STARTING_BOARD_NUMBER;
					startNewGame(&board, &settings);
				}
				else
					settings.quit = 1;
			}

			settings.fpsTimer += settings.delta;
			if (settings.fpsTimer > 0.5) {
				settings.fps = settings.frames * 2;
				settings.frames = 0;
				settings.fpsTimer -= 0.5;
			};

			// tekst informacyjny / info text
			DrawRectangle(bmps.screen, 4, 4, SCREEN_WIDTH - 8, LEGEND_SIZE_Y, czerwony, niebieski);
			//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
			sprintf(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", settings.worldTime, settings.fps);
			DrawString(bmps.screen, bmps.screen->w / 2 - strlen(text) * 8 / 2, 10, text, bmps.charset);
			//	      "Esc - exit, \030 - faster, \031 - slower"
			sprintf(text, "Esc - exit, n - start over, SPACE - pause, ENTER - select");
			DrawString(bmps.screen, bmps.screen->w / 2 - strlen(text) * 8 / 2, 26, text, bmps.charset);
		}

			SDL_UpdateTexture(scrtex, NULL, bmps.screen->pixels, bmps.screen->pitch);
			//		SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			if (settings.endOfGame == true)
			{
				SDL_Delay(2000);
				settings.endOfGame = false;
			}
		// obs≥uga zdarzeÒ (o ile jakieú zasz≥y) / handling of events (if there were any)
		while (SDL_PollEvent(&event))
			getEvent(&event, &settings,&board,&bmps);
		settings.frames++;
	};

	// zwolnienie powierzchni / freeing all surfaces
	freeMemory(&board, &bmps);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};

	void getEvent(SDL_Event *event, settings_t *settings, board_t *board, bmps_t *bmps)
	{
		switch (event->type) {
		case SDL_KEYDOWN:
			if (event->key.keysym.sym == SDLK_ESCAPE) settings->quit = 1; // EXIT
			else if (event->key.keysym.sym == SDLK_n && settings->pause == false) settings->newGame = true; // start new game
			else if (event->key.keysym.sym == SDLK_UP) // try to go up, or go up in menu
			{
				if (settings->pause == false)
				{
					board->playerPosY--;
					bmps->player = bmps->playerUp;
				}
				else if (settings->inmenu == 0 && settings->menuSelect > 1)
					settings->menuSelect--;

			}
			else if (event->key.keysym.sym == SDLK_DOWN) // go down if possible or go to lower option in menu
			{
				if (settings->pause == false)
				{
					board->playerPosY++;
					bmps->player = bmps->playerDown;
				}
				else if (settings->inmenu == 0 && settings->menuSelect < OPTIONS_AVAILABLE)
					settings->menuSelect++;
			}
			else if (event->key.keysym.sym == SDLK_RIGHT) // if possible go right
			{
				if (settings->pause == false)
				{
					board->playerPosX++;
					bmps->player = bmps->playerRight;
				}
			}
			else if (event->key.keysym.sym == SDLK_LEFT) // try to go left
			{
				if (settings->pause == false)
				{
					board->playerPosX--;
					bmps->player = bmps->playerLeft;
				}
			}
			else if (settings->inmenu == ADD_BOARD && event->key.keysym.sym == SDLK_RETURN) // ENTER after typing new board name == adding new board to the list
			{
				settings->alreadyTyped = 0;
				settings->inmenu = 0;
				settings->pause = 0;
				addNewBoard(settings->typedIn);
			}
			else if (settings->menuSelect == IN_A_ROW && event->key.keysym.sym == SDLK_RETURN) // change third (many games in a row) option
			{
				if (settings->manyGamesInARow == false)
					settings->manyGamesInARow = true;
				else
					settings->manyGamesInARow = false;
			}
			else if (event->key.keysym.sym == SDLK_RETURN && settings->pause == true) // select option if menu is on
			{
				settings->select = true;
			}
			else if (event->key.keysym.sym == SDLK_SPACE) // pause, unpause
			{
				if (settings->pause == false)
				{
					settings->pause = true;
				}
				else
				{
					settings->inmenu = 0;
					settings->pause = false;
				}
			}
			else if (settings->inmenu == CHOOSE_BOARD && (event->key.keysym.sym >= SDLK_1 && event->key.keysym.sym <= SDLK_9)) // press the number key 1-9 to choose the board
			{
				settings->inmenu = 0; //turn off submenu
				settings->pause = 0; //turn off pause
				settings->newGame = true; //start new game
				settings->currentBoardNumber = event->key.keysym.sym - SDLK_0;
				/*
				getBoardName(settings, settings->currentBoardNumber);
				getBoardSize(board, settings->boardName);
				boardInit(board);
				readBoardFromFile(board, settings);
				*/
			}
			else if (settings->inmenu == ADD_BOARD && ((event->key.keysym.sym >= SDLK_1 && event->key.keysym.sym <= SDLK_9) || //typing board name (1-9 , a-z, . )
				(event->key.keysym.sym >= SDLK_a && event->key.keysym.sym <= SDLK_z) || event->key.keysym.sym == SDLK_PERIOD))
			{
				settings->typedIn[settings->alreadyTyped++] = event->key.keysym.sym; 
				settings->typedIn[settings->alreadyTyped] = '\0';
			}
			else if (settings->inmenu == ADD_BOARD && event->key.keysym.sym == SDLK_BACKSPACE) 
				settings->typedIn[--settings->alreadyTyped] = '\0';
			break;
		case SDL_KEYUP:
			settings->select = false;
			break;
		case SDL_QUIT:
			settings->quit = 1;
			break;
		};

	}

	void getBoardSize(board_t *board, char *boardName)
	{
		//board->number = boardNumber;
		int sizeX;
		int sizeY = 0;
		bool endOfFile = false;
		char filename[20];
		char sign;
		FILE *fileBoard = NULL;
		//sprintf(filename, "board%d.txt", board->number);
		sprintf(filename, "%s.txt", boardName);
		fileBoard = fopen(filename, "r");
		if (fileBoard != NULL)
		{
			while (endOfFile == false)
			{
				sizeX = 0;
				sizeY++; // counting new lines
				while (true) // counting sings in one line
				{
					sign = fgetc(fileBoard);
					if (sign == EOF)
					{
						endOfFile = true;
						break;
					}
					if (sign == '\n')
						break;
					sizeX++;
				}
			}
			fclose(fileBoard);
		}
		if (sizeY > sizeX) // setting bigger one to size variable
			board->size = sizeY;
		else
			board->size = sizeX;
	}

	void showPauseMenu(bmps_t *bmps, settings_t *settings, int fillColor, int outlineColor)
	{
		DrawRectangle(bmps->screen, BOARD_POSITION_X, BOARD_POSITION_Y, BOARD_SIZE_X, BOARD_SIZE_Y, outlineColor, fillColor);
		if (settings->inmenu == 0)
		{
			char option[] = { "Choose board" }; // first opiton
			if (settings->menuSelect == CHOOSE_BOARD) // highlight if is selected
				DrawRectangle(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 - 4 * strlen(option), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 - 5,
					8 * strlen(option), FIELD_SIZE / 2, outlineColor, fillColor);
			DrawString(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 - 4 * strlen(option), BOARD_POSITION_Y + BOARD_SIZE_Y / 2, option, bmps->charset);
			char option2[] = { "Add new board" }; // secound one
			if (settings->menuSelect == ADD_BOARD) // highlight if is selected
				DrawRectangle(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 - 4 * strlen(option2), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 + FIELD_SIZE - 5,
					8 * strlen(option2), FIELD_SIZE / 2, outlineColor, fillColor);
			DrawString(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 - 4 * strlen(option2), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 + FIELD_SIZE, option2, bmps->charset);
			char option3[] = { "Play many games in a row" }; //third
			if (settings->menuSelect == IN_A_ROW) // highlight if is selected
				DrawRectangle(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 - 4 * strlen(option3), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 + 2* FIELD_SIZE - 5,
					8 * strlen(option3), FIELD_SIZE / 2, outlineColor, fillColor);
			DrawString(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 - 4 * strlen(option3), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 + 2* FIELD_SIZE, option3, bmps->charset);
			if (settings->manyGamesInARow == true) // draw YES if the option is selected or NO if the option isn't
				DrawString(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 + FIELD_SIZE + 4 * strlen(option3), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 + 2 * FIELD_SIZE, "YES", bmps->charset);
			else
				DrawString(bmps->screen, BOARD_POSITION_X + BOARD_SIZE_X / 2 + FIELD_SIZE + 4 * strlen(option3), BOARD_POSITION_Y + BOARD_SIZE_Y / 2 + 2 * FIELD_SIZE, "NO", bmps->charset);
		}
        if (settings->select == true || settings->inmenu != 0) // show submenu
        {
			if (settings->menuSelect == CHOOSE_BOARD || settings->inmenu == CHOOSE_BOARD) // from first option
			{
				settings->inmenu = CHOOSE_BOARD;
				showBoardsAvailable(bmps, settings, fillColor, outlineColor);
			}
			if (settings->menuSelect == ADD_BOARD || settings->inmenu == ADD_BOARD) // sectound one
			{
				settings->inmenu = ADD_BOARD;
				showAddingNewBoard(bmps, settings, fillColor, outlineColor);
			}
        }
	}

	void addNewBoard(char *newBoardName)
	{
		FILE *fileBoard = NULL;
		char nameToWrite[MAX_NAME_OF_BOARD_LENGTH + 1];
		fileBoard = fopen("boardList.txt", "a");
		if (fileBoard != NULL)
		{
			sprintf(nameToWrite, "\n%s", newBoardName); // create new line and add there new board name
			fputs(nameToWrite, fileBoard);
			fclose(fileBoard);
		}
	}

	void showAddingNewBoard(bmps_t *bmps, settings_t *settings, int fillColor, int outlineColor)
	{
		// draw info
		DrawString(bmps->screen, BOARD_POSITION_X + 4, BOARD_POSITION_Y + 10, "Type name of the file containing board you want to add.", bmps->charset);
		DrawString(bmps->screen, BOARD_POSITION_X + 4, BOARD_POSITION_Y + FIELD_SIZE, "You can use only small letters, numbers and periods.", bmps->charset);
		DrawString(bmps->screen, BOARD_POSITION_X + 4, BOARD_POSITION_Y + 2 * FIELD_SIZE, "Name: ", bmps->charset);
		if (settings->alreadyTyped > 0)
		{
			DrawString(bmps->screen, BOARD_POSITION_X + 4 + sizeof("Name: ")/sizeof('a') * 8, BOARD_POSITION_Y + 2 * FIELD_SIZE, settings->typedIn, bmps->charset); // draw what you typed
		}
	}

    void showBoardsAvailable(bmps_t *bmps, settings_t *settings, int fillColor, int outlineColor)
    {
		int boardAmount = countBoards();
		for (int i = 0; i < boardAmount; i++)
		{
			getBoardName(settings, i + 1);
			char number[3];
			itoa(i + 1, number, 10);
			DrawString(bmps->screen, BOARD_POSITION_X + 4, BOARD_POSITION_Y + FIELD_SIZE * (i + 1), number, bmps->charset); // draw number of the board
			DrawString(bmps->screen, BOARD_POSITION_X + FIELD_SIZE, BOARD_POSITION_Y + FIELD_SIZE * (i + 1), settings->boardName, bmps->charset); // draw board name
		}
		char inf[] = { "Press the key with board number you want to play" };
		DrawString(bmps->screen, BOARD_POSITION_X + 4, BOARD_POSITION_Y + BOARD_SIZE_Y - 10, inf, bmps->charset);
    }

	void getBoardName(settings_t *settings, int number)
	{
		char sign;
		FILE *fileBoard = NULL;
		fileBoard = fopen("boardList.txt", "r");
		int currentNumber = 1;
		int nameLength = 0;
		if (fileBoard != NULL)
		{
			while (currentNumber != number) // read till coursor is in the right place 
			{
				sign = fgetc(fileBoard);
				if (sign == EOF)
				{
					break;
				}
				if (sign == '\n')
					currentNumber++;
			}
			sign = fgetc(fileBoard);
			while (sign != '\n' && sign != EOF) // read whole line and write it to boardName variable
			{
				settings->boardName[nameLength++] = sign;
				sign = fgetc(fileBoard);
			}
			settings->boardName[nameLength] = '\0';
			fclose(fileBoard);
		}
	}

    int countBoards()
    {
		char sign;
		int amount = 1;
		FILE *fileBoard = NULL;
		fileBoard = fopen("boardList.txt", "r");
		if (fileBoard != NULL)
		{
			while (true)
			{
                sign = fgetc(fileBoard);
                if (sign == EOF)
                {
                    break;
                }
                if (sign == '\n')
                    amount++; // number of new lines
			}
        fclose(fileBoard);
		}
		return amount;
    }

	int setWindow(bmps_t *bmps, SDL_Window *window, SDL_Renderer *renderer, int *rc)
	{

		// tryb pe≥noekranowy / fullscreen mode
		//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
		//	                                 &window, &renderer);
		if (*rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			return 1;
		};

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


		bmps->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		// wy≥πczenie widocznoúci kursora myszy
		SDL_ShowCursor(SDL_DISABLE);
		return 0;

	}

	void showGameEnd(board_t *board, bmps_t *bmps)
	{
		DrawSurface(bmps->screen, bmps->gameOver, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2); //GAME OVER in the center of a screen
	}

	bool checkGameEnd(board_t *board)
	{
		int boxdestAmount = 0;
		for (int i = 0; i < board->size; i++)
		{
			for (int j = 0; j < board->size; j++)
			{
				if (board->field[i][j].type == BOXDEST)
					boxdestAmount++;
			}
		}
		if (boxdestAmount == board->destAmount) // if all boxes are on right (destination) places
			return true;
		else
			return false;
	}

	void startNewGame(board_t *board, settings_t *settings)
	{
		freeBoardMemory(board);
		getBoardName(settings, settings->currentBoardNumber);
		getBoardSize(board, settings->boardName);
		boardInit(board);
		readBoardFromFile(board, settings);
		// repositioning player
		settings->previousX = board->playerPosX;
		settings->previousY = board->playerPosY;
		//time to zero
		settings->worldTime = 0;
	}

	void movePlayer(board_t *board, settings_t *settings, int axis)
	{
		int *x = &board->playerPosX;
		int *y = &board->playerPosY;
		int addX, addY;
		if (axis == AXIS_X)
		{
			addX = *x - settings->previousX;
			addY = 0;
		}
		else
		{
			addX = 0;
			addY = *y - settings->previousY;
		}

		if (board->field[*y][*x].type == EMPTY) // if destination field is empty
		{
			board->field[*y][*x].type = PLAYER;
			if (board->field[settings->previousY][settings->previousX].type == PLADEST)
				board->field[settings->previousY][settings->previousX].type = DESTINATION;
			else
				board->field[settings->previousY][settings->previousX].type = EMPTY;
			settings->previousX = *x;
			settings->previousY = *y;
		}
		else if (board->field[*y][*x].type == DESTINATION) // if destination field is destination field
		{
			board->field[*y][*x].type = PLADEST;
			if (board->field[settings->previousY][settings->previousX].type == PLADEST)
				board->field[settings->previousY][settings->previousX].type = DESTINATION;
			else
				board->field[settings->previousY][settings->previousX].type = EMPTY;
			settings->previousX = *x;
			settings->previousY = *y;
		}
		if ((board->field[*y][*x].type == BOX || board->field[*y][*x].type == BOXDEST) &&
			(board->field[*y + addY][*x + addX].type == EMPTY || board->field[*y + addY][*x + addX].type == DESTINATION)) // if destination field is BOX
		{
			if (board->field[*y][*x].type == BOXDEST)
				board->field[*y][*x].type = PLADEST;
			else
				board->field[*y][*x].type = PLAYER;
			if (board->field[*y + addY][*x + addX].type == DESTINATION)
				board->field[*y + addY][*x + addX].type = BOXDEST;
			else
				board->field[*y + addY][*x + addX].type = BOX;
			if (board->field[settings->previousY][settings->previousX].type == PLADEST)
				board->field[settings->previousY][settings->previousX].type = DESTINATION;
			else
				board->field[settings->previousY][settings->previousX].type = EMPTY;
			settings->previousX = *x;
			settings->previousY = *y;
		}
	}


	void boardInit(board_t *board)
	{
		//dynamiczna alokacja tablicy o podanym rozmiarze
		board->size;
		board->field = (field_t**)malloc(board->size * sizeof(field_t*));
		for (int i = 0; i < board->size; i++)
		{
			(board->field)[i] = (field_t*)malloc(board->size * sizeof(field_t));
		}
	}

	void freeMemory(board_t *board, bmps_t *bmps)
	{
		//free all surface, bmps
		SDL_FreeSurface(bmps->playerDown);
		SDL_FreeSurface(bmps->playerUp);
		SDL_FreeSurface(bmps->playerLeft);
		SDL_FreeSurface(bmps->playerRight);
		SDL_FreeSurface(bmps->box);
		SDL_FreeSurface(bmps->destination);
		SDL_FreeSurface(bmps->gameOver);
		SDL_FreeSurface(bmps->charset);
		SDL_FreeSurface(bmps->screen);
		freeBoardMemory(board);
	}

	void freeBoardMemory(board_t *board)
	{
		for (int i = 0; i < board->size; i++)
		{
			free((board->field)[i]);
		}
		free(board->field);
	}

	void readBoardFromFile(board_t *board, settings_t *settings)
	{
		//wczytuje plansze z gotowego pliku, jesli dostepny
		char filename[20];
		char sign;
		bool endOfFile = false;
		int j, i = 0;
		board->destAmount = 0;
		FILE *fileBoard = NULL;
		//sprintf(filename, "board%d.txt", board->number);
		sprintf(filename, "%s.txt", settings->boardName);
		fileBoard = fopen(filename, "r");
		if (fileBoard != NULL)
		{
			while (endOfFile == false)
			{
				j = 0;
				while (true)
				{
					sign = fgetc(fileBoard);
					if (sign == EOF)
					{
						endOfFile = true;
						break;
					}
					if (sign == '\n')
					{
						break;
					}
					if (sign == EMPTY_FIELD)
						board->field[i][j].type = EMPTY;
					else if (sign == BOX_FIELD)
						board->field[i][j].type = BOX;
					else if (sign == WALL_FIELD)
						board->field[i][j].type = WALL;
					else if (sign == DESTINATION_FIELD)
					{
						board->field[i][j].type = DESTINATION;
						board->destAmount++;
					}
					else if (sign == PLAYER_FIELD)
					{
						board->field[i][j].type = PLAYER;
						board->playerPosX = j;
						board->playerPosY = i;
					}
					j++;
				}
				i++;
			}
			fclose(fileBoard);
		}
	}

	void drawBoard(bmps_t *bmps, int board_outline_color, int board_fill_color, board_t *board)
	{
		DrawRectangle(bmps->screen, BOARD_POSITION_X, BOARD_POSITION_Y, BOARD_SIZE_X, BOARD_SIZE_Y, board_outline_color, board_fill_color);
		for (int i = 0; i < board->size; i++)
		{
			for (int j = 0; j < board->size; j++)
			{
				switch (board->field[i][j].type)
				{
				case EMPTY:
					drawEmptyField(bmps->screen, board_fill_color, j + 1, i + 1);
					break;
				case WALL:
					drawBlock(bmps->screen, j + 1, i + 1);
					break;
				case BOX:
					drawFieldImage(bmps->screen, bmps->box, j + 1, i + 1);
					break;
				case DESTINATION:
					drawFieldImage(bmps->screen, bmps->destination, j + 1, i + 1);
					break;
				case PLAYER:
					drawFieldImage(bmps->screen, bmps->player, j + 1, i + 1);
					break;
				case PLADEST: // player on destination field
					drawFieldImage(bmps->screen, bmps->destination, j + 1, i + 1);
					drawFieldImage(bmps->screen, bmps->player, j + 1, i + 1);
					break;
				case BOXDEST: // box on destination field
					drawFieldImage(bmps->screen, bmps->box, j + 1, i + 1);
					break;
				}
			}
		}
	}


	void drawEmptyField(SDL_Surface *screen, int color, int x, int y)
	{
		DrawRectangle(screen, BOARD_POSITION_X + SPACE_SIZE + x * FIELD_SIZE, BOARD_POSITION_Y + SPACE_SIZE + y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE, color, color);
	}

	void drawFieldImage(SDL_Surface *screen, SDL_Surface *image, int x, int y)
	{
		DrawSurface(screen, image, BOARD_POSITION_X + SPACE_SIZE - FIELD_SIZE / 2 + (x + 1) * FIELD_SIZE, BOARD_POSITION_Y + (y + 1) * FIELD_SIZE + SPACE_SIZE - FIELD_SIZE / 2);
	}

	void drawBlock(SDL_Surface *screen, int x, int y) //simple block
	{
		int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
		int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
		DrawRectangle(screen, BOARD_POSITION_X + SPACE_SIZE + x * FIELD_SIZE, BOARD_POSITION_Y + SPACE_SIZE + y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE, zielony, niebieski);
	}


	// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
	// charset to bitmapa 128x128 zawierajπca znaki
	// draw a text txt on surface screen, starting from the point (x, y)
	// charset is a 128x128 bitmap containing character images
	void DrawString(SDL_Surface *screen, int x, int y, const char *text,
		SDL_Surface *charset) {
		int px, py, c;
		SDL_Rect s, d;
		s.w = 8;
		s.h = 8;
		d.w = 8;
		d.h = 8;
		while (*text) {
			c = *text & 255;
			px = (c % 16) * 8;
			py = (c / 16) * 8;
			s.x = px;
			s.y = py;
			d.x = x;
			d.y = y;
			SDL_BlitSurface(charset, &s, screen, &d);
			x += 8;
			text++;
		};
	};


	// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
	// (x, y) to punkt úrodka obrazka sprite na ekranie
	// draw a surface sprite on a surface screen in point (x, y)
	// (x, y) is the center of sprite on screen
	void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
		SDL_Rect dest;
		dest.x = x - sprite->w / 2;
		dest.y = y - sprite->h / 2;
		dest.w = sprite->w;
		dest.h = sprite->h;
		SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


	// rysowanie pojedynczego pixela
	// draw a single pixel
	void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
		int bpp = surface->format->BytesPerPixel;
		Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
		*(Uint32 *)p = color;
	};


	// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1)
	// bπdü poziomie (gdy dx = 1, dy = 0)
	// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
	void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
		for (int i = 0; i < l; i++) {
			DrawPixel(screen, x, y, color);
			x += dx;
			y += dy;
		};
	};


	// rysowanie prostokπta o d≥ugoúci bokÛw l i k
	// draw a rectangle of size l by k
	void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
		Uint32 outlineColor, Uint32 fillColor) {
		int i;
		DrawLine(screen, x, y, k, 0, 1, outlineColor);
		DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
		DrawLine(screen, x, y, l, 1, 0, outlineColor);
		DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
		for (i = y + 1; i < y + k - 1; i++)
			DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

	bool loadAllBMPs(SDL_Renderer *renderer, SDL_Texture *scrtex, SDL_Window *window, bmps_t *bmps)
	{
		bool wrong = false;
		bmps->charset = SDL_LoadBMP("./cs8x8.bmp");
		if (bmps->charset == NULL) {
			printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		SDL_SetColorKey(bmps->charset, true, 0x000000);

		bmps->box = SDL_LoadBMP("./box.bmp");
		if (bmps->box == NULL) {
			printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		bmps->destination = SDL_LoadBMP("./dest.bmp");
		if (bmps->destination == NULL) {
			printf("SDL_LoadBMP(dest.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		bmps->gameOver = SDL_LoadBMP("./gameOver.bmp");
		if (bmps->gameOver == NULL) {
			printf("SDL_LoadBMP(gameOver.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		bmps->playerUp = SDL_LoadBMP("./playerUp.bmp");
		if (bmps->playerUp == NULL) {
			printf("SDL_LoadBMP(playerUp.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		SDL_SetColorKey(bmps->playerUp, SDL_TRUE, 0);
		bmps->playerDown = SDL_LoadBMP("./playerDown.bmp");
		if (bmps->playerDown == NULL) {
			printf("SDL_LoadBMP(playerDown.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		SDL_SetColorKey(bmps->playerDown, SDL_TRUE, 0);
		bmps->playerRight = SDL_LoadBMP("./playerRight.bmp");
		if (bmps->playerRight == NULL) {
			printf("SDL_LoadBMP(playerRight.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		SDL_SetColorKey(bmps->playerRight, SDL_TRUE, 0);
		bmps->playerLeft = SDL_LoadBMP("./playerLeft.bmp");
		if (bmps->playerLeft == NULL) {
			printf("SDL_LoadBMP(playerLeft.bmp) error: %s\n", SDL_GetError());
			wrong = true;
		};
		SDL_SetColorKey(bmps->playerLeft, SDL_TRUE, 0);

		bmps->player = bmps->playerUp;

		if (wrong == true)
		{
			SDL_FreeSurface(bmps->charset);
			SDL_FreeSurface(bmps->screen);
			SDL_FreeSurface(bmps->box);
			SDL_FreeSurface(bmps->destination);
			SDL_FreeSurface(bmps->gameOver);
			SDL_FreeSurface(bmps->playerUp);
			SDL_FreeSurface(bmps->playerDown);
			SDL_FreeSurface(bmps->playerRight);
			SDL_FreeSurface(bmps->playerLeft);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		}
		else
			return 0;
	}
