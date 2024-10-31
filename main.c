#include "raylib.h"

int main() {
	  const int screenWidth = 800;
	    const int screenHeight = 600;
	      InitWindow(screenWidth, screenHeight, "Bunny Game");
	        SetTargetFPS(60);
		  while (!WindowShouldClose()) {
			      BeginDrawing();
			          ClearBackground(RAYWHITE);
				      DrawText("The Bunny Game by Patrick Traynor and Daria Jerjomina", 20, 20, 20, BLACK);
				          EndDrawing();
					    }
		    CloseWindow();
		      return 0;
}
