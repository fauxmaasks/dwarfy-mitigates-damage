#include "raylib.h"
#include <malloc.h>
#include <wchar.h>
// 1920 x 1080
#define SCREEN_WIDTH 1900
#define SCREEN_HEIGHT 1000
#define GRID_WIDTH 1200
#define GRID_HEIGHT 900
#define X_OFFSET 10
#define Y_OFFSET 10

// MAP is gonna have a bunch of info?
// creature ID from the list of creatures ocuping the spot
// corpse ID, lying on the ground
// 
// terrain ID - floor, wall, rocks

// so map could be array of structs
//
typedef unsigned int uint;

typedef struct Tile {
  // 0 idx means it has nothing
  uint entity_idx;
  uint item_idx;
  uint terrain_id;
}Tile;

typedef struct Map {
  uint width;
  uint height;
  Tile* cells;
}Map;

typedef struct Entity {
  // uint img_src_idx;
  // uint corpse_img_src_idx;
  char character;
  int HP;
}Entity;

typedef struct Item {
  uint img_src_idx;

}Item;

typedef struct Item_arr {
  uint cap;
  uint size;
  Item* items;
}Item_arr;

typedef struct Entity_arr {
  uint cap;
  uint size;
  Entity* items;
}Entity_arr;

void init_map(Map* map, uint width, uint height)
{
  map->width = width;
  map->height = height;
  Tile* cells = (Tile*)malloc(sizeof(Tile)*width*height);
  for(uint j = 0; j < height; j++){
    for(uint i = 0; i < width; i++){
      cells[j*width + i] = (Tile){0};
    } 
  } 
  map->cells = cells;
  return;
}

int main() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "dwarfy, the protector");
  // init game
  uint cols = 20;
  uint rows = 20;
  float cell_width = (float)GRID_WIDTH/(float)cols;
  float cell_height = (float)GRID_HEIGHT/(float)rows;
  Map map;
  Entity_arr entities = {0};
  entities.cap = 2;
  entities.size = 2;
  entities.items = (Entity*)malloc(sizeof(Entity)*entities.cap);

  entities.items[1] = (Entity){.character = '@', .HP = 5};

  init_map(&map, cols, rows);
  // add "player"
  map.cells[0] = (Tile){.entity_idx=1, .item_idx=0, .terrain_id=0};

  
  Image floor_img = LoadImage("floor.png");
  ImageResize(&floor_img, cell_width, cell_height);
  Texture2D floor = LoadTextureFromImage(floor_img);
  UnloadImage(floor_img);

  SetTargetFPS(30);
  float font_size = 80;
  int up_increment = 0;
  while(!WindowShouldClose()){

    if (IsKeyPressed(KEY_LEFT)) font_size -= 1;  // Reduce fontSize
    if (IsKeyPressed(KEY_RIGHT)) font_size += 1;  // Increase fontSize

    BeginDrawing();
      ClearBackground(GRAY);
      for(uint y = 0; y < rows; y++){
       for(uint x = 0; x < cols; x++){
          Tile tile = map.cells[y*cols + x];
          char c; 
          if(tile.entity_idx != 0 && tile.entity_idx < entities.size){
            Entity en = entities.items[tile.entity_idx];
            c = en.character;
          } else {
            c = '.';
          }
          // DrawText(TextFormat("%c", c), cell_width*x, cell_height*y, font_size, WHITE);
          // if((y*cols + x) % 2 == 0) {
          DrawTexture(floor, cell_width*x, cell_height*y , WHITE);
          // }
          // DrawTexture(floor, 100,100, WHITE);
          // DrawText(TextFormat("%c", c), cell_width*x + X_OFFSET, cell_height*y + Y_OFFSET, font_size, WHITE);
          // break;
        } 
      } 
      // DrawFPS(5, 5);
    EndDrawing();

  }
  UnloadTexture(floor);

  CloseWindow();                                     

}
