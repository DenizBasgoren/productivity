
#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>

Font font;

#define GAMES_ON_SCREEN 10

// row is 0-indexed
static void drawGameName( char *name, int row) {

	char text[] = "0:";
	text[0] += row;
	DrawTextEx(font, text, (Vector2){10, 70+30*row}, 30, 1, WHITE);
	DrawTextEx(font, name, (Vector2){50, 70+30*row}, 30, 1, WHITE);	
}

int cursor = 0; // offset
char *gameBuf = 0;
char *games[GAMES_ON_SCREEN];
int gamesLen = 0;
int currentGameNo = 0;

void populateNextTenGames(void) {

	currentGameNo += gamesLen;
	gamesLen = 0;

	if (cursor == 4096) return;

	for (int i = 0; i<10; i++) {
		while(gameBuf[cursor] == '\0') {
			cursor++;
			if (cursor == 4096) return;
		}

		games[gamesLen++] = gameBuf+cursor;
		
		while(gameBuf[cursor] != '\0') {
			cursor++;
			if (cursor == 4096) return;
		}
		

	}

}

void populatePrevTenGames(void) {

	if (gamesLen > 0) {
		cursor = games[0]-gameBuf-1;
		gamesLen = 0;
	}

	if (cursor < 0) {
		cursor = 0;
		populateNextTenGames();
		return;
	}
	else if (cursor == 4096) {
		cursor--;
	}

	for (int i = 0; i<10; i++, currentGameNo--) {
		while (gameBuf[cursor] == '\0') {
			cursor--;
			if (cursor == 0) {
				populateNextTenGames();
				return;
			}
		}
		while (gameBuf[cursor] != '\0') {
			cursor--;
			if (cursor == 0) {
				populateNextTenGames();
				return;
			}
		}
	}

	populateNextTenGames();
	return;
}


int askGameNumber(char *gamesStartingAddress) {

	gameBuf = gamesStartingAddress;

	populateNextTenGames();


	// SetTraceLogLevel(LOG_ERROR);
	InitWindow(400, 450, "Korsan_papp");
	SetTargetFPS(60);
	font = LoadFont("/usr/share/fonts/droid/DroidSerif-Regular.ttf");

	unsigned int key = -1;
	bool validKeyWasSubmitted = false;
	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground((Color){30,30,30,255});

		DrawTextEx(font, "Press 0-9 to select a game.", (Vector2){10, 20}, 30, 1, WHITE);

		for (int i = 0; i<gamesLen; i++) {
			drawGameName(games[i], i);
		}

		DrawTextEx(font, "<", (Vector2){10, 425}, 30, 1, WHITE);
		DrawTextEx(font, ">", (Vector2){375, 425}, 30, 1, WHITE);


		if (key = GetKeyPressed() ) {
			// printf("debug keypressed: %d\n", key-KEY_ZERO);

			if (key == KEY_RIGHT) {
				populateNextTenGames();
				continue;
			}
			else if (key == KEY_LEFT) {
				populatePrevTenGames();
				continue;
			}

			// debug thing
			// else if (key == KEY_UP) {
			// 	printf("cursor:%d \n", cursor);
			// 	printf("gamesLen:%d \n", gamesLen);
			// 	printf("curGameNo:%d \n", currentGameNo);
			// 	continue;
			// }

			if (key-KEY_ZERO < gamesLen) {
				validKeyWasSubmitted = true;
				break;
			}
		}

		EndDrawing();
	}
	CloseWindow();

	int result;

	if (!validKeyWasSubmitted) {
		result = -1;
	}
	else {
		result = currentGameNo+key-KEY_ZERO;
	}

	// cleanup
	cursor = 0;
	gamesLen = 0;
	currentGameNo = 0;
	return result;
}




__attribute__((weak)) int main(void) {
	askGameNumber(0);
}
