#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

const float BUNNY_SPEED = 2.0f;
const float FOX_SPEED = 3.0f;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const char* GAME_OVER_TEXT = "Game Over!";

const float SEASON_DURATION = 100.0f;
const float SEASON_CHANGE_SPEED = 1.0f;
const float COLOR_CHANGE_DELAY = 30.0f;

Rectangle bunny;
Rectangle fox;
Rectangle trap;
Rectangle lake;
bool gameOver;
int score;
float seasonX;
int counter;
bool isWinter;
bool isBunnyWinter;

void restart()
{
	bunny = (Rectangle){10, 490, 20, 20};
	fox = (Rectangle){0, 0, 20, 20};
	respawnFox();
	trap = (Rectangle){SCREEN_WIDTH/2-100, SCREEN_HEIGHT/2-100, 30, 30};
	lake = (Rectangle){SCREEN_WIDTH/2-50, SCREEN_HEIGHT/2-50, 100, 100};
	gameOver = false;
    score = 0;
	seasonX = SCREEN_WIDTH;
	counter = 0;
	isWinter = true;
	isBunnyWinter = true;
}

void respawnFox() {
    if (GetRandomValue(0, 1) == 0)
    {
        // Top and bottom
        fox.x = GetRandomValue(-fox.width, SCREEN_WIDTH);
        fox.y = GetRandomValue(0, 1) ? -fox.height : SCREEN_HEIGHT;
    }
    else
    {
        // Left and right
        fox.y = GetRandomValue(-fox.height, SCREEN_HEIGHT);
        fox.x = GetRandomValue(0, 1) ? -fox.width : SCREEN_WIDTH;
    }
}

bool isRectangleInWinter(Rectangle r) {
    bool isObjectInWinter = isWinter;
    if (r.x + r.width/2 > seasonX)
    {
        isObjectInWinter = !isObjectInWinter;
    }
    return isObjectInWinter;
}

float scaleSpeedWhenInLake(Rectangle r) {
    if (CheckCollisionRecs(r, lake))
    {
        return isRectangleInWinter(lake) ? 2.0 : 0.2;
    }
    return 1.0;
}



int main()
{	
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bunny Game");
	SetTargetFPS(60);

	// Textures and shader
	// At the moment textures are just white and green colors.
	// Later we might want to change the shader so that it applies color multiplication to a single texture.
	
	Image imWinter = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 220, 220, 255, 255 });
    Texture texWinter = LoadTextureFromImage(imWinter);
    UnloadImage(imWinter);

    Image imSummer = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 255, 0, 255 });
    Texture texSummer = LoadTextureFromImage(imSummer);
    UnloadImage(imSummer);

	Shader shader = LoadShader(0, "color_mix.fs");
	// devider and texture 1 (from the shader, to be set later)
    int texWinterLoc = GetShaderLocation(shader, "texture1");
    int dividerLoc = GetShaderLocation(shader, "divider");
	

	restart();
	
	while (!WindowShouldClose())
	{
		if (!gameOver)
		{
			// Keep seasonX unchanged while the season timer is running
			// once season duration is over, season advances

			counter++;
			
			if (counter > COLOR_CHANGE_DELAY)
			{
				isBunnyWinter = isWinter;
			}

            bool foxWasSwimming = CheckCollisionRecs(fox, lake) && !isRectangleInWinter(lake);
			if (counter > SEASON_DURATION)
			{
				// Advance the season
				seasonX -= SEASON_CHANGE_SPEED;
				if (seasonX <= 0)
				{
					counter = 0;
					seasonX = SCREEN_WIDTH;
					isWinter = !isWinter;					
				}							
			}
			float dividerValue = Remap(seasonX, 0, SCREEN_WIDTH, 0, 1);	
			SetShaderValue(shader, dividerLoc, &dividerValue, SHADER_UNIFORM_FLOAT);


			// Move the bunny
			Vector2 bunnyMoveVector = {0, 0};
			if (IsKeyDown(KEY_RIGHT)) bunnyMoveVector.x += BUNNY_SPEED;
			else if (IsKeyDown(KEY_LEFT)) bunnyMoveVector.x -= BUNNY_SPEED;

			if (IsKeyDown(KEY_UP)) bunnyMoveVector.y -= BUNNY_SPEED;
			else if (IsKeyDown(KEY_DOWN)) bunnyMoveVector.y += BUNNY_SPEED;

			// Make movement be always the same speed (even on diagonal movement), otherwise will move faster when on diagonal
			bunnyMoveVector = Vector2Scale(Vector2Normalize(bunnyMoveVector), BUNNY_SPEED * scaleSpeedWhenInLake(bunny));
			bunny.x += bunnyMoveVector.x;
			bunny.y += bunnyMoveVector.y;

			bunny.x = Clamp(bunny.x, 0, SCREEN_WIDTH-bunny.width);
			bunny.y = Clamp(bunny.y, 0, SCREEN_HEIGHT-bunny.height);

            //Move the fox
            // Calculate the vector from the fox to the bunny, normalize it and multiply by the fox speed
            Vector2 bunnyVector = {bunny.x+bunny.width/2, bunny.y+bunny.height/2};
            Vector2 foxVector = {fox.x+fox.width/2, fox.y+fox.height/2};
            Vector2 foxDirection = Vector2Subtract(bunnyVector, foxVector);
            Vector2 foxMovement = Vector2Scale(Vector2Normalize(foxDirection), FOX_SPEED * scaleSpeedWhenInLake(fox));
            // Foxs moves only when the bunny is in the season of it's color
            bool isBunnySafe = isBunnyWinter == isRectangleInWinter(bunny);
            if (!isBunnySafe)
            {
                fox.x += foxMovement.x;
                fox.y += foxMovement.y;
            }
		

            // Collision detection
            if (CheckCollisionRecs(bunny, fox))
            {
                gameOver = true;
            }
            if (CheckCollisionRecs(bunny, trap))
            {
                gameOver = true;
            }

            if (CheckCollisionRecs(fox, trap))
            {
                //score += 1;
                respawnFox();
            }

            // Fox freezes in lake
            if (foxWasSwimming && CheckCollisionRecs(fox, lake) && isRectangleInWinter(lake))
            {
                score += 1;
                respawnFox();
            }
		}		

		// Restart
		if (IsKeyPressed(KEY_R))
		{
			restart();
		}

		// Draw
		BeginDrawing();
			// Draw the season
			BeginShaderMode(shader);
			Texture* texLeft;
			Texture* texRight;
			if (isWinter)
			{
				texLeft = &texWinter;
				texRight = &texSummer;
			}
			else
			{
				texLeft = &texSummer;
				texRight = &texWinter;
			}
				
			SetShaderValueTexture(shader, texWinterLoc, *texRight);
			DrawTexture(*texLeft, 0, 0, WHITE);
            EndShaderMode();

			// Draw characters
			DrawRectangleRec(trap, RED);
			DrawRectangleRec(lake, isRectangleInWinter(lake) ? WHITE : BLUE);
			DrawRectangleRec(bunny, isBunnyWinter ? WHITE : BROWN);
			DrawRectangleRec(fox, ORANGE);

			DrawLineEx((struct Vector2){seasonX,0}, (struct Vector2){seasonX,SCREEN_HEIGHT}, 4, BLACK);
			
			if (gameOver)
			{
				int gameOverWidth = MeasureText(GAME_OVER_TEXT, 70);
				int restartWidth = MeasureText("Press R to restart", 50);
				DrawText(GAME_OVER_TEXT, SCREEN_WIDTH/2-gameOverWidth/2, SCREEN_HEIGHT/2-50, 70, RED);
				DrawText("Press R to restart", SCREEN_WIDTH/2-restartWidth/2, SCREEN_HEIGHT/2+30, 50, RED);
			}

            DrawText(TextFormat("Foxes trapped: %i", score), 10, 10, 30, RED);

		EndDrawing();
	}
	CloseWindow();
	return 0;
}

