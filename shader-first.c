/*******************************************************************************************
*
*   raylib [shaders] example - ascii rendering
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Maicon Santana (@maiconpintoabreu) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Maicon Santana (@maiconpintoabreu)
*
********************************************************************************************/

#include "raylib.h"
#include <string.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900

int main(void)
{
    float RATIO_X = (float)SCREEN_WIDTH/3;
    float RATIO_Y = (float)SCREEN_HEIGHT/3;
    // Initialization
    //--------------------------------------------------------------------------------------
    char ray_path[124] = "/home/unix-uname/game-engines/raylib/";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib [shaders] example - ascii rendering");

    // Texture to test static drawing
    char* fawkes_path = "examples/shaders/resources/guy-fawkes.png";
    strcat(ray_path, fawkes_path);
    Image fawkes_img = LoadImage(ray_path);
    ImageResize(&fawkes_img, RATIO_X, RATIO_Y);
    Texture2D fawkes = LoadTextureFromImage(fawkes_img);
    // Texture to test moving drawing
    // Texture2D raysan = LoadTexture("examples/shaders/resources/raysan.png");

    // Load shader to be used on postprocessing

    char ray_path_2[124] = "/home/unix-uname/game-engines/raylib/";
    char* shader_path = "examples/shaders/resources/shaders/glsl%i/ascii.fs";
    strcat(ray_path_2, shader_path);
    Shader shader = LoadShader(0, TextFormat(ray_path_2, GLSL_VERSION));

    // These locations are used to send data to the GPU
    int resolutionLoc = GetShaderLocation(shader, "resolution");
    int fontSizeLoc = GetShaderLocation(shader, "fontSize");

    // Set the character size for the ASCII effect
    // Fontsize should be 9 or more
    float fontSize = 9.0f;

    // Send the updated values to the shader
    float resolution[2] = { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
    SetShaderValue(shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);

    Vector2 circlePos = (Vector2){40.0f, (float)SCREEN_HEIGHT*0.5f};
    float circleSpeed = 1.0f;

    // RenderTexture to apply the postprocessing later
    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    // Rectangle rec = {.x = SCREEN_WIDTH/2, .y = SCREEN_HEIGHT/2, .width = SCREEN_WIDTH/4, .height = SCREEN_HEIGHT/4};
    Rectangle rec0 = {.x = 100, .y = 100, .width = (float)RATIO_X, .height = (float)RATIO_Y};
    Rectangle rec1 = {.x = 150, .y = 150, .width = (float)RATIO_X*1.2, .height = (float)RATIO_Y*1.2};
    Rectangle rec = {.x = 0, .y = 0, .width = (float)RATIO_X, .height = (float)RATIO_Y};
    NPatchInfo patch = {.source=rec, .left=0, .top=0, .right=50, .bottom=50, .layout=NPATCH_THREE_PATCH_HORIZONTAL};
    Vector2 vec = {.x=0, .y=0};
    float rotation = 0;
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        circlePos.x += circleSpeed;
        if ((circlePos.x > 200.0f) || (circlePos.x < 40.0f)) circleSpeed *= -1; // Revert speed

        if (IsKeyPressed(KEY_LEFT) && (fontSize > 4.0)) fontSize -= 1;  // Reduce fontSize
        if (IsKeyPressed(KEY_RIGHT) && (fontSize < 35.0)) fontSize += 1;  // Increase fontSize

        // Set fontsize for the shader
        SetShaderValue(shader, fontSizeLoc, &fontSize, SHADER_UNIFORM_FLOAT);

        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(target);
            ClearBackground(WHITE);
            // RLAPI void DrawTextureNPatch(Texture2D texture,  nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint); // Draws a texture (or part of it) that stretches or shrinks nicely
            
            if(((int)GetTime() % 2) > 0){
                DrawTextureNPatch(fawkes, patch, rec0, vec, 10.0f, WHITE);
            } else {
                DrawTextureNPatch(fawkes, patch, rec1, vec, -10.0f, WHITE);
            }
        EndTextureMode();
        
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginShaderMode(shader);
                // Draw the scene texture (that we rendered earlier) to the screen
                // The shader will process every pixel of this texture
                DrawTextureRec(target.texture, 
                    (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, 
                    (Vector2){ 0, 0 }, WHITE);
            EndShaderMode();

            DrawRectangle(0, 0, SCREEN_WIDTH, 40, BLACK);
            DrawText(TextFormat("Ascii effect - FontSize:%2.0f - [Left] -1 [Right] +1 ", fontSize), 120, 10, 20, LIGHTGRAY);
            DrawFPS(10, 10);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);    // Unload render texture

    UnloadShader(shader);           // Unload shader
    UnloadTexture(fawkes);        // Unload texture

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
