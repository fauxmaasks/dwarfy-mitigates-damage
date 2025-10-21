#include "raylib.h"
#include <malloc.h>
// #include <wchar.h>
#define ROGUE_H_IMPLEMENTATION
#include "rogue.h"
#include <time.h>
#include <stdlib.h>
// 1920 x 1080
#define SCREEN_WIDTH 1900
#define SCREEN_HEIGHT 1000
#define GRID_WIDTH 1200
#define GRID_HEIGHT 900
#define X_OFFSET 100
#define Y_OFFSET 20
#define COLS 10
#define ROWS 10

#define NORMAL_MODE 0
#define TARGETING_MODE 1

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

uint mode = NORMAL_MODE;

uint coord_to_idx(Vec2 c)
{
  return c.y*COLS + c.x;
}


bool is_inside_bounds(Map map, Vec2 new_coord) 
{
  return new_coord.x >= 0 &&
         new_coord.y >= 0 &&
         new_coord.x < map.width &&
         new_coord.y < map.height;
}

bool is_valid_to_move(Map map, Vec2 new_coord) 
{
  return map.cells[coord_to_idx(new_coord)].entity_idx == 0 &&
         map.cells[coord_to_idx(new_coord)].terrain_id != 1;
}

void make_quadrants(Vec2** qs, int* size, Map map, int x_splits, int y_splits)
{
  *size = x_splits*y_splits;
  Vec2* new_q = (Vec2*)malloc(sizeof(Vec2)*(*size));
  int i = 0;
  for(int x = 0; x < x_splits; x++){
    for(int y = 0; y < y_splits; y++){
      new_q[i] = (Vec2){.x = x*map.width/x_splits, .y = y*map.height/y_splits};
      i++;
    }
  }
  *qs = new_q;
}
void map_generator_random(Map* map)
{
  srand(time(NULL));
  int pillar_min_size = 1;
  int pillar_max_size = 1;

  Vec2* qs;
  int size;
  int x_split = 2;
  int y_split = 2;
  int q_width = map->width/x_split;
  int q_height = map->height/y_split;
  make_quadrants(&qs, &size, *map, x_split, y_split);
  int max_pillars_num = 150;
  int count = 0;
  do {
    int pillar_size = (rand() % (pillar_max_size - pillar_min_size + 1)) + pillar_min_size;

    Vec2 q = qs[rand() % size];
    int pillar_x = (rand() % (q_width - pillar_size - 1)) + q.x + 1;
    int pillar_y = (rand() % (q_height - pillar_size - 1)) + q.y + 1;
    
    for(int y = pillar_y; y < pillar_y + pillar_size; y++){
      for(int x = pillar_x; x < pillar_x + pillar_size; x++){
        Vec2 c = (Vec2){.x=x, .y=y};
        if(is_inside_bounds(*map, c)){
          map->cells[coord_to_idx(c)].terrain_id = 1;
        }
      }
    }
    count++;
    
  } while(count < max_pillars_num);
  free(qs);
}

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

bool move(Map* map, Vec2* entity_coord, int dir_x, int dir_y) 
{
  Vec2 new_coord = (Vec2){.x = entity_coord->x+dir_x, .y = entity_coord->y+dir_y};
  if(is_inside_bounds(*map, new_coord) && is_valid_to_move(*map, new_coord)) {
    map->cells[coord_to_idx(new_coord)].entity_idx = map->cells[coord_to_idx(*entity_coord)].entity_idx;
    map->cells[coord_to_idx(*entity_coord)].entity_idx = 0;
    entity_coord->x = new_coord.x;
    entity_coord->y = new_coord.y;
    return true;
  }
  return false;
}

void swap_in_sequence(Map* map, Vec2 origin_coord, Vec2* sequence, int size){
  Tile t1 = {0}; 
  Tile t2 = {0}; 
  bool is_first_pass = true;
  for(int i = 0; i <= size; i++){
    Vec2 current_coord = {origin_coord.x + sequence[i%size].x, origin_coord.y + sequence[i%size].y};
    if(is_first_pass){
      t1.terrain_id = map->cells[coord_to_idx(current_coord)].entity_idx;
      t1.item_idx   = map->cells[coord_to_idx(current_coord)].item_idx;
      t1.terrain_id = map->cells[coord_to_idx(current_coord)].terrain_id;
      is_first_pass = false;
    } else {
      t2.terrain_id = map->cells[coord_to_idx(current_coord)].entity_idx;
      t2.item_idx   = map->cells[coord_to_idx(current_coord)].item_idx;
      t2.terrain_id = map->cells[coord_to_idx(current_coord)].terrain_id;
      map->cells[coord_to_idx(current_coord)].entity_idx = t1.entity_idx;
      map->cells[coord_to_idx(current_coord)].item_idx   = t1.item_idx;
      map->cells[coord_to_idx(current_coord)].terrain_id = t1.terrain_id;
      t1.terrain_id = t2.entity_idx;
      t1.item_idx   = t2.item_idx;
      t1.terrain_id = t2.terrain_id;
    }
  }
}
bool rotate_terrain(Map* map, Vec2 origin_coord, Vec2 rotation_dir)
{
  // for now size is fixed 2x2, 3x3 is interesting too
  int size = 3;
  int x_dir = origin_coord.x < rotation_dir.x ? 1 : -1;
  // int y_dir = origin_coord.y < rotation_dir.y ? -1 : 1;

  bool is_clockwise = x_dir == 1;

  if(origin_coord.x > (COLS - size) || origin_coord.y > (ROWS - size)){
    return false;
  }
  if(size == 2){
    static const int seq_size = 4;
    if(is_clockwise){
      Vec2 sequence[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
      swap_in_sequence(map, origin_coord, sequence, seq_size);
    } else {
      Vec2 sequence[4] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
      swap_in_sequence(map, origin_coord, sequence, seq_size);
    }
  } else if(size == 3){
      const int seq_size = 8;
      if(is_clockwise){
        Vec2 sequence[8] = {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}};
        swap_in_sequence(map, origin_coord, sequence, seq_size);
      } else {
        Vec2 sequence[8] = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 1}, {2, 0}, {1, 0}};
        swap_in_sequence(map, origin_coord, sequence, seq_size);
      }
  }
  return true;
}

bool move_terrain(Map* map, Vec2 origin_coord, Vec2 target_coord) 
{
  printf("moving terrain: from(%d,%d) to(%d,%d)\n", origin_coord.x, origin_coord.y, target_coord.x, target_coord.y);
  // Vec2_Arr tiles = {0};
  // tiles.cap = 100;
  // tiles.items = (Vec2*)malloc(sizeof(
  // rg_bresenham_line(origin_coord, target_coord, &tiles);
  if(is_inside_bounds(*map, target_coord) && is_valid_to_move(*map, target_coord)) {

    map->cells[coord_to_idx(target_coord)].terrain_id = map->cells[coord_to_idx(origin_coord)].terrain_id;
    map->cells[coord_to_idx(origin_coord)].terrain_id = 0;
    origin_coord.x = target_coord.x;
    origin_coord.y = target_coord.y;
    return true;
  }
  return false;
}

bool is_mouse_inside_grid(Vector2 mouse_coord) 
{
  return mouse_coord.x > X_OFFSET && 
         mouse_coord.x < GRID_WIDTH + X_OFFSET &&
         mouse_coord.y > Y_OFFSET && 
         mouse_coord.y < GRID_HEIGHT + Y_OFFSET;

}

Vec2 get_tile_coord_under_pixel_coord(Vector2 pixel_coord)
{
  float cell_width = (float)GRID_WIDTH/(float)COLS;
  float cell_height = (float)GRID_HEIGHT/(float)ROWS;
  float x_on_grid = (int)((pixel_coord.x - X_OFFSET)/cell_width);
  float y_on_grid = (int)((pixel_coord.y - Y_OFFSET)/cell_height);
  return (Vec2){.x=x_on_grid, .y=y_on_grid};
}

void tile_clicked(Map map, Vec2_Arr* targets)
{
  if(targets->size == 2) {
    //reset
    targets->size = 0;
  }

  Vector2 mouse_coord = GetMousePosition();
  if(!is_mouse_inside_grid(mouse_coord)){return;}
  Vec2 coord = get_tile_coord_under_pixel_coord(mouse_coord);
  targets->items[targets->size].x = coord.x;
  targets->items[targets->size].y = coord.y;
  targets->size++;
  printf("selecting: (%d, %d)\n", coord.x, coord.y);
}

int handle_input(int key, Map* map, Vec2* player_coord, Vec2_Arr* targets) 
{
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) tile_clicked(*map, targets);
    if (IsKeyPressed(KEY_UP)) move(map, player_coord, 0, -1);
    if (IsKeyPressed(KEY_DOWN)) move(map, player_coord, 0, 1);
    if (IsKeyPressed(KEY_LEFT)) move(map, player_coord, -1, 0);
    if (IsKeyPressed(KEY_RIGHT)) move(map, player_coord, 1, 0);
    if (IsKeyPressed(KEY_F) && targets->size == 2) move_terrain(map, targets->items[0], targets->items[1]);
    if (IsKeyPressed(KEY_R) && targets->size == 2) rotate_terrain(map, targets->items[0], targets->items[1]);
    if (IsKeyPressed(KEY_N))
    {
      init_map(map, map->width, map->height); 
      map_generator_random(map);
    }
  return 1;
}

bool is_mouse_inside_tile(Vector2 mouse_pos, Vec2 tile_coord, float cell_width, float cell_height) 
{
  return mouse_pos.x > tile_coord.x+ X_OFFSET && 
         mouse_pos.x < tile_coord.x + X_OFFSET+ cell_width &&
         mouse_pos.y > tile_coord.y+ Y_OFFSET && 
         mouse_pos.y < tile_coord.y + cell_height+ Y_OFFSET;
}

int main() 
{
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "dwarfy, the protector");
  // init game
  float cell_width = (float)GRID_WIDTH/(float)COLS;
  float cell_height = (float)GRID_HEIGHT/(float)ROWS;
  Map map;
  Entity_arr entities = {0};
  entities.cap = 3;
  entities.size = 3;
  entities.items = (Entity*)malloc(sizeof(Entity)*entities.cap);

  entities.items[1] = (Entity){.character = '@', .HP = 5};
  Vec2 player_coord = (Vec2){.x = 0, .y = 0};
  entities.items[2] = (Entity){.character = 'm', .HP = 5};

  init_map(&map, COLS, ROWS);
  map_generator_random(&map);
  // add "player"
  // map.cells[0] = (Tile){.entity_idx=1, .item_idx=0, .terrain_id=0};
  // add monster
  // map.cells[COLS + 1] = (Tile){.entity_idx=2, .item_idx=0, .terrain_id=0};

  float font_size = 60;
  Image floor_img = LoadImage("floor.png");
  ImageResizeNN(&floor_img, cell_width, cell_height);
  Texture2D floor_tex = LoadTextureFromImage(floor_img);

  Image pillar_img = LoadImage("pillar.png");
  ImageResizeNN(&pillar_img, cell_width, cell_height);
  Texture2D pillar_tex = LoadTextureFromImage(pillar_img);

  Image player_img = ImageCopy(floor_img);
  ImageDrawText(&player_img, "@", cell_width/2-font_size/3, cell_height/2-font_size/2, font_size, WHITE);
  Texture2D player_tex = LoadTextureFromImage(player_img);

  Image monster_img = ImageCopy(floor_img);
  ImageDrawText(&monster_img, "m", cell_width/2-font_size/4, cell_height/2-font_size/2, font_size, RED);
  Texture2D monster_tex = LoadTextureFromImage(monster_img);

  UnloadImage(floor_img);
  UnloadImage(pillar_img);
  UnloadImage(player_img);
  UnloadImage(monster_img);

  SetTargetFPS(30);
  Vec2_Arr targets = {0};
  targets.cap = 2;
  targets.items = (Vec2*)malloc(sizeof(Vec2)*2);
  while(!WindowShouldClose()){

    int key = GetKeyPressed();
    handle_input(key, &map, &player_coord, &targets);
    Vector2 mouse_pos = GetMousePosition();

    BeginDrawing();
      ClearBackground(BLACK);
      for(uint y = 0; y < ROWS; y++){
        for(uint x = 0; x < COLS; x++){
          Tile tile = map.cells[y*COLS + x];
          char c; 
          Color color = WHITE;
          Texture2D t;
          if(tile.entity_idx != 0 && tile.entity_idx < entities.size){
            Entity en = entities.items[tile.entity_idx];
            c = en.character;
            if(c == '@'){
              t = player_tex;
            } else if( c == 'm'){
              t = monster_tex;
            }
          } else {
            if(tile.terrain_id == 0){
              c = '.';
              t = floor_tex;
            } else if(tile.terrain_id == 1){
              c = '#';
              t = pillar_tex;
            }
          }
          Vec2 tile_coord = (Vec2){.x = cell_width*x, .y = cell_height*y};
          if(is_mouse_inside_tile(mouse_pos, tile_coord, cell_width, cell_height)) {
            color = YELLOW;
          }
          DrawTexture(t, tile_coord.x+X_OFFSET, tile_coord.y+Y_OFFSET, color);
        } 
      } 
      // DrawFPS(5, 5);
    EndDrawing();

  }
  UnloadTexture(floor_tex);
  UnloadTexture(player_tex);
  UnloadTexture(monster_tex);

  CloseWindow();                                     

}
