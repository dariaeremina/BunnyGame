#include "C:\\raylib\\raylib\\src\\raylib.h"
#include "C:\\raylib\\raylib\\src\\raymath.h"
#include <stdio.h>

const int FPS = 60;

const float BUNNY_SPEED = 3.0f;
const float FOX_SPEED = 4.0f;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const float SEASON_DURATION = 100.0f;
const float SEASON_CHANGE_SPEED = 1.0f;
const float COLOR_CHANGE_DELAY = 00.0f;

Texture texWinter;
Texture texSummer;
Shader shader;
int texWinterLoc;
int dividerLoc;

Rectangle bunny;
Rectangle lake;
int numFoxes;
Rectangle foxes[5];
Rectangle traps[3];
bool pause = true;
bool gameOver;
bool win;
int score;
float seasonX;
int counter;
bool isWinter;
bool isBunnyWinter;

int year;
int health;

void respawnFox(int index) {
    Rectangle * fox = &foxes[index];
    if (GetRandomValue(0, 1) == 0)
    {
        // Top and bottom
        fox->x = GetRandomValue(-fox->width, SCREEN_WIDTH);
        fox->y = GetRandomValue(0, 1) ? -fox->height : SCREEN_HEIGHT;
    }
    else
    {
        // Left and right
        fox->y = GetRandomValue(-fox->height, SCREEN_HEIGHT);
        fox->x = GetRandomValue(0, 1) ? -fox->width : SCREEN_WIDTH;
    }
}

void setupObstacles() {
    // Trap year
    if (year % 2 == 0)
    {
        traps[0] = (Rectangle){SCREEN_WIDTH*0.55, SCREEN_HEIGHT*0.3, 30, 30};
        traps[1] = (Rectangle){SCREEN_WIDTH*0.3, SCREEN_HEIGHT*0.7, 30, 30};
        traps[2] = (Rectangle){SCREEN_WIDTH*0.7, SCREEN_HEIGHT*0.6, 30, 30};

        // Place lake outside the screen instead of removing it (to avoid extra if statements)
        lake = (Rectangle){SCREEN_WIDTH-100, SCREEN_HEIGHT-100, 0, 0};

        numFoxes++;
    }
    // Lake year
    else
    {
        lake = (Rectangle){SCREEN_WIDTH/2-50, SCREEN_HEIGHT/2-50, 100, 100};

        // Place traps outside the screen instead of removing it (to avoid extra if statements)
        traps[0] = (Rectangle){SCREEN_WIDTH-100, SCREEN_HEIGHT-100, 0, 0};
        traps[1] = (Rectangle){SCREEN_WIDTH-100, SCREEN_HEIGHT-100, 0, 0};
        traps[2] = (Rectangle){SCREEN_WIDTH-100, SCREEN_HEIGHT-100, 0, 0};
    }
}

void restart()
{
	year = 0;
	bunny = (Rectangle){SCREEN_WIDTH/2-10, SCREEN_HEIGHT/2-10, 20, 20};
	foxes[0] = (Rectangle){0, 0, 20, 20};
	foxes[1] = (Rectangle){0, 0, 20, 20};
	foxes[2] = (Rectangle){0, 0, 20, 20};
	foxes[3] = (Rectangle){0, 0, 20, 20};
	foxes[4] = (Rectangle){0, 0, 20, 20};
	respawnFox(0);
	respawnFox(1);
	respawnFox(2);
	respawnFox(3);
	respawnFox(4);
    numFoxes = 0;
	gameOver = false;
    score = 0;
	seasonX = SCREEN_WIDTH;
	counter = 0;
	isWinter = true;
	isBunnyWinter = true;
	health = 300;
    setupObstacles();
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

void drawTextCentered(const char* text, int x, int y, int fontSize, Color color) {
    int width = MeasureText(text, fontSize);
    DrawText(text, x-width/2, y, fontSize, color);
}

void drawInstructions(){
	ClearBackground(GRAY);
	drawTextCentered("Instructions", SCREEN_WIDTH/2, 40, 30, BLACK);
	drawTextCentered("(Press P to hide/show)", SCREEN_WIDTH/2, 75, 20, BLACK);

	Rectangle bunnyPic = (Rectangle){20, 115, 20, 20};
	DrawRectangleRec(bunnyPic, WHITE);
	DrawText("Bunny (that's you)", bunnyPic.x+bunnyPic.width+10, bunnyPic.y, 20, WHITE);
	Rectangle foxPic = (Rectangle){20, bunnyPic.y+bunnyPic.height+10, 20, 20};

	DrawRectangleRec(foxPic, ORANGE);
	DrawText("Fox (your enemy)", foxPic.x+foxPic.width+10, foxPic.y, 20, ORANGE);
	drawTextCentered("Your fur changes color with the season", SCREEN_WIDTH/2, foxPic.y+foxPic.height+30, 20, BLACK);

	Rectangle summerBunnyPic = (Rectangle){20, foxPic.y+foxPic.height+60, 20, 20};
	DrawRectangleRec(summerBunnyPic, BROWN);
	DrawText("Bunny in summer", summerBunnyPic.x+summerBunnyPic.width+10, summerBunnyPic.y, 20, BROWN);

	drawTextCentered("Foxes don't see you when you're hidden", SCREEN_WIDTH/2, summerBunnyPic.y+summerBunnyPic.height+10, 20, BLACK);

	Rectangle trapPic = (Rectangle){20, summerBunnyPic.y+summerBunnyPic.height+60, 20, 20};
	DrawRectangleRec(trapPic, RED);
	DrawText("Trap", trapPic.x+trapPic.width+10, trapPic.y, 20, RED);

	drawTextCentered("Lure foxes into the traps", SCREEN_WIDTH/2, trapPic.y+trapPic.height+10, 20, BLACK);

	Rectangle lakePic = (Rectangle){20, trapPic.y+trapPic.height+40, 20, 20};
	DrawRectangleRec(lakePic, BLUE);
	DrawText("Lake", lakePic.x+lakePic.width+10, lakePic.y, 20, BLUE);

	drawTextCentered("Foxes freeze when the lake freezes in winter", SCREEN_WIDTH/2, lakePic.y+lakePic.height+10, 20, BLACK);
}

void UpdateDrawFrame()
{
    if (!gameOver && !pause)
    {
        // Keep seasonX unchanged while the season timer is running
        // once season duration is over, season advances

        counter++;
        
        if (counter > COLOR_CHANGE_DELAY)
        {
            isBunnyWinter = isWinter;
        }

        bool lakeWasLiquid = !isRectangleInWinter(lake);
        if (counter > SEASON_DURATION)
        {
            // Advance the season
            seasonX -= SEASON_CHANGE_SPEED;
            if (seasonX <= -70)
            {
                counter = 0;
                seasonX = SCREEN_WIDTH+70;
                isWinter = !isWinter;
                if (isWinter)
                {
                    year++;
                    if (year == 10)
                    {
                        win = true;
                        gameOver = true;
                    }

                    setupObstacles();
                }					
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

        for (int i = 0; i < numFoxes; i++)
        {
            Rectangle* fox = &foxes[i];

            //Move the fox
            // Calculate the vector from the fox to the bunny, normalize it and multiply by the fox speed
            Vector2 bunnyVector = {bunny.x+bunny.width/2, bunny.y+bunny.height/2};
            Vector2 foxVector = {fox->x+fox->width/2, fox->y+fox->height/2};
            Vector2 foxDirection = Vector2Subtract(bunnyVector, foxVector);
            Vector2 foxMovement = Vector2Scale(Vector2Normalize(foxDirection), FOX_SPEED * scaleSpeedWhenInLake(*fox));
            // Foxs moves only when the bunny is in the season of it's color
            bool isBunnySafe = isBunnyWinter == isRectangleInWinter(bunny);
            if (!isBunnySafe)
            {
                fox->x += foxMovement.x;
                fox->y += foxMovement.y;
            }
        }


        // Collision detection

        // Being attacked by foxes or by traps
        bool beingAttacked = false;
        for (int i = 0; i < numFoxes; i++)
        {
            Rectangle* fox = &foxes[i];
            if (CheckCollisionRecs(bunny, *fox))
            {
                beingAttacked = true;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            if (CheckCollisionRecs(bunny, traps[i]))
            {
                beingAttacked = true;
            }
        }
        if (beingAttacked)
        {
            health--;
            if (health <= 0)
            {
                gameOver = true;
            }
        }

        // Foxes touching traps
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < numFoxes; j++)
            {
                Rectangle* fox = &foxes[j];
                if (CheckCollisionRecs(*fox, traps[i]))
                {
                    score += 1;
                    respawnFox(j);
                }
            }
        }

        // Foxes freeze in lake
        for (int i = 0; i < numFoxes; i++)
        {
            Rectangle* fox = &foxes[i];
            if (CheckCollisionRecs(*fox, lake) && lakeWasLiquid && isRectangleInWinter(lake))
            {
                score += 1;
                respawnFox(i);
            }
        }
    }		

    if (!pause && gameOver)
    {
        // Restart
        if (IsKeyPressed(KEY_R))
        {
            restart();
        }
    }
    

    // Pause
    if (!gameOver)
    {
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
            printf("Paused\n");
        }
    }
    

    // Draw
    
    BeginDrawing();
        if (pause)
        {
            // Draw instructions
            drawInstructions();
        }
        else
        {
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
            for (int i = 0; i < 3; i++)
            {
                DrawRectangleRec(traps[i], RED);
            }			
            DrawRectangleRec(lake, isRectangleInWinter(lake) ? WHITE : BLUE);
            DrawRectangleRec(bunny, isBunnyWinter ? WHITE : BROWN);
            for (int i = 0; i < numFoxes; i++)
            {
                DrawRectangleRec(foxes[i], ORANGE);
            }

            DrawLineEx((struct Vector2){seasonX,0}, (struct Vector2){seasonX,SCREEN_HEIGHT}, 4, BLACK);
            
            if (gameOver)
            {
                int textX = SCREEN_WIDTH/2;
                int textY = 180;
                Color textColor = RED;

                if (win) {
                    textColor = BLUE;
                    drawTextCentered("You died of old age.", textX, textY, 70, textColor);
                }
                else {
                    drawTextCentered("Game Over!", textX, textY, 70, textColor);
                }
                textY += 90;
                drawTextCentered(TextFormat("You survived %i years", year), textX, textY, 35, textColor);
                textY += 40;
                drawTextCentered(TextFormat("And killed %i foxes", score), textX, textY, 35, textColor);
                textY += 70;
                drawTextCentered("Press R to restart", textX, textY, 50, textColor);
                
            }
            else
            {
                DrawText(TextFormat("Foxes trapped: %i", score), 10, 10, 30, RED);
                DrawText(TextFormat("Year: %i", year), 10, 40, 30, RED);
                DrawText(TextFormat("Health: %i", health), SCREEN_WIDTH-200, 40, 30, BLUE);
            }
        }
        

    EndDrawing();
}


int main()
{	
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bunny Game");

	// Textures and shader
	// At the moment textures are just white and green colors.
	// Later we might want to change the shader so that it applies color multiplication to a single texture.
	
	Image imWinter = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 220, 220, 255, 255 });
    texWinter = LoadTextureFromImage(imWinter);
    UnloadImage(imWinter);

    Image imSummer = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 97, 173, 53, 255 });
    texSummer = LoadTextureFromImage(imSummer);
    UnloadImage(imSummer);

	shader = LoadShader(0, "color_mix.fs");
	// devider and texture 1 (from the shader, to be set later)
    texWinterLoc = GetShaderLocation(shader, "texture1");
    dividerLoc = GetShaderLocation(shader, "divider");
	

	restart();

	SetTargetFPS(FPS);
	while (!WindowShouldClose())
	{
        UpdateDrawFrame();
	}
    UnloadShader(shader);
	CloseWindow();
	return 0;
}

/*
Introduction
(Press S to skip)
Bunny (that's you)
Fox (your enemy)
Your fur changes color with the seasons
The fox doesn't see you when you're hidden
Lure the fox into traps
Lure the fox into the lake
The fox will freeze with the lake in winter
*/
