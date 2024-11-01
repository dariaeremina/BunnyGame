#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

const float BUNNY_SPEED = 2.0f;
const float FOX_SPEED = 2.0f;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const char* GAME_OVER_TEXT = "Game Over!";

Rectangle bunny = {10, 490, 100, 100};
Rectangle fox = {10, 10, 100, 100};
bool gameOver = false;

void restart()
{
	bunny.x = 10;
	bunny.y = 490;
	fox.x = 10;
	fox.y = 10;
	gameOver = false;
}

int main()
{	
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bunny Game");
	SetTargetFPS(60);

	restart();

	while (!WindowShouldClose())
	{
		if (!gameOver)
		{
			Vector2 bunnyMoveVector = {0, 0};
			// Move the bunny
			if (IsKeyDown(KEY_RIGHT)) bunnyMoveVector.x += BUNNY_SPEED;
			else if (IsKeyDown(KEY_LEFT)) bunnyMoveVector.x -= BUNNY_SPEED;

			if (IsKeyDown(KEY_UP)) bunnyMoveVector.y -= BUNNY_SPEED;
			else if (IsKeyDown(KEY_DOWN)) bunnyMoveVector.y += BUNNY_SPEED;

			// Make movement be always the same speed (even on diagonal movement), otherwise will move faster when on diagonal
			bunnyMoveVector = Vector2Scale(Vector2Normalize(bunnyMoveVector), BUNNY_SPEED);
			bunny.x += bunnyMoveVector.x;
			bunny.y += bunnyMoveVector.y;

			bunny.x = Clamp(bunny.x, 0, SCREEN_WIDTH-bunny.width);
			bunny.y = Clamp(bunny.y, 0, SCREEN_HEIGHT-bunny.height);
		}


		//Move the fox
		// Calculate the vector from the fox to the bunny, normalize it and multiply by the fox speed
		Vector2 bunnyVector = {bunny.x+bunny.width/2, bunny.y+bunny.height/2};
		Vector2 foxVector = {fox.x+fox.width/2, fox.y+fox.height/2};
		Vector2 foxDirection = Vector2Subtract(bunnyVector, foxVector);
		Vector2 foxMovement = Vector2Scale(Vector2Normalize(foxDirection), FOX_SPEED);
		fox.x += foxMovement.x;
		fox.y += foxMovement.y;

		// Collision detection
		if (CheckCollisionRecs(bunny, fox))
		{
			gameOver = true;
		}

		// Restart
		if (IsKeyPressed(KEY_R))
		{
			restart();
		}

		// Draw
		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawRectangleRec(bunny, DARKGRAY);
		DrawRectangleRec(fox, ORANGE);
		if (gameOver)
		{
			int gameOverWidth = MeasureText(GAME_OVER_TEXT, 70);
			int restartWidth = MeasureText("Press R to restart", 50);
			DrawText(GAME_OVER_TEXT, SCREEN_WIDTH/2-gameOverWidth/2, SCREEN_HEIGHT/2-50, 70, RED);
			DrawText("Press R to restart", SCREEN_WIDTH/2-restartWidth/2, SCREEN_HEIGHT/2+30, 50, RED);
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}




