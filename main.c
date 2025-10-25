#include "raylib.h"
#include <malloc.h>
// #include <wchar.h>
#define ROGUE_H_IMPLEMENTATION
#include "rogue.h"
#include <time.h>
#include <stdlib.h>
#include <math.h>
// 1920 x 1080
#define SCREEN_WIDTH 1900
#define SCREEN_HEIGHT 1000
#define GRID_WIDTH 1200
#define GRID_HEIGHT 900
#define X_OFFSET 100
#define Y_OFFSET 20
#define COLS 120
#define ROWS 120

#define NORMAL_MODE 0
#define TARGETING_MODE 1


#define FLOOR_ID 0
#define WALL_ID 1
#define STONE_ID 2
#define DOOR_ID 3

#define RECT_ID 100
#define CIRCLE_ID 101
#define ELIPSIS_ID 102
#define TRIANGLE_ID 103

// order of precendence
//1 -
// ++ -- 	Prefix increment and decrement[note 1] 	Right-to-left
// + - 	Unary plus and minus
// ! ~ 	Logical NOT and bitwise NOT
// (type) 	Cast
// * 	Indirection (dereference)
// & 	Address-of
// sizeof 	Size-of[note 2]
// _Alignof 	Alignment requirement(C11) ++ --
//2 -	++ -- 	Prefix increment and decrement[note 1] 	Right-to-left
// + - 	Unary plus and minus
// ! ~ 	Logical NOT and bitwise NOT
// (type) 	Cast
// * 	Indirection (dereference)
// & 	Address-of
// sizeof 	Size-of[note 2]
// _Alignof 	Alignment requirement(C11)

typedef unsigned int uint;

typedef struct Tile {
  // 0 idx means it has nothing
  uint entity_idx;
  uint item_idx;
  uint terrain_id;
  bool checked;
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

double distf(Vec2 c1, Vec2 c2) 
{
  return sqrt(pow(c2.x-c1.x, 2) + pow(c2.y-c1.y, 2));
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
         map.cells[coord_to_idx(new_coord)].terrain_id == FLOOR_ID;
}

void add_walls_to_borders(Map* map, int origin_x, int origin_y)
{
  Vec2 tiles[COLS*ROWS];
  tiles[0].x = origin_x;
  tiles[0].y = origin_y;
  int size = 1;
  int iter = 0;
  do{
    size--;
    Vec2 curr_coord;
    curr_coord.x = tiles[size].x;
    curr_coord.y = tiles[size].y;
    for(int y = -1; y <= 1; y++){
      for(int x = -1; x <= 1; x++){
        int sum = rg_abs(x) + rg_abs(y);
        if(y == 0 && x == 0) {continue;}
        Vec2 new_coord = {curr_coord.x + x, curr_coord.y + y};
        if(is_inside_bounds(*map, new_coord)){
          if(map->cells[coord_to_idx(new_coord)].checked ){
            continue;
          } else if (map->cells[coord_to_idx(new_coord)].terrain_id == FLOOR_ID) {
            tiles[size].x = new_coord.x;
            tiles[size].y = new_coord.y;
            size++;
            map->cells[coord_to_idx(new_coord)].checked = true; 
          } else {
            map->cells[coord_to_idx(new_coord)].terrain_id = WALL_ID; 
            map->cells[coord_to_idx(new_coord)].checked = true; 
          }
        }
      }
    }
    // if (iter > 100) break;
  // } while(size > 0 || range < 5);
  } while(size > 0);
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

// void (Map* map, int room_size) {
//
// }
bool is_rectangle_empty(Map* map, int  origin_x, int origin_y, int width, int height) 
{
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      Vec2 c = {origin_x + x, origin_y + y};
      if(!is_inside_bounds(*map, c) || (is_inside_bounds(*map, c) && map->cells[coord_to_idx(c)].terrain_id == FLOOR_ID)){
        return false;
      }  
    }
  }
  return true;
}
void dig_rectangle(Map* map, int  origin_x, int origin_y, int width, int height)
{
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      Vec2 c = {origin_x + x, origin_y + y};
      if(is_inside_bounds(*map, c)){
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      }  
    }
  }
}
bool is_circle_empty(Map* map, int  origin_x, int origin_y, int radius)
{
  for(int y = -radius; y <= radius ; y++){
    for(int x = -radius; x <= radius; x++){
      Vec2 c = {origin_x + x, origin_y + y};
      if(!is_inside_bounds(*map, c) || (is_inside_bounds(*map, c) && distf((Vec2){origin_x, origin_y}, c) < radius + 0.5 && map->cells[coord_to_idx(c)].terrain_id == FLOOR_ID)){
        return false;
      }  
    }
  }
  return true;
}

void dig_circle(Map* map, int  origin_x, int origin_y, int radius)
{
  for(int y = -radius; y <= radius ; y++){
    for(int x = -radius; x <= radius; x++){
      Vec2 c = {origin_x + x, origin_y + y};
      if(is_inside_bounds(*map, c) && distf((Vec2){origin_x, origin_y}, c) < radius + 0.5){
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      }  
    }
  }
}

void dig_elipsis(Map* map, int origin_x, int origin_y, int radius_x, int radius_y)
{
  // elipsis formula = x^2/a^2 + y^2/b^2 = 1, a and b are the horizontal and vertical extremities
  // sort by left top point to right bottom
  // actually it's the left most, if sorting by both x and y mistakes occur
  for(int y = origin_y-radius_y-1; y <= origin_y+radius_y+1; y++){
    for(int x = origin_x-radius_x-1; x <= origin_x+radius_x+1; x++){
      Vec2 c = {x, y};
      float dist = ((float)pow(x-origin_x, 2)/(float)pow(radius_x, 2) + (float)pow(y-origin_y, 2)/(float)pow(radius_y, 2));
      if(is_inside_bounds(*map, c) && dist <= 1.0f)
      {
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      } 
    }
  }
}

void dig_room_with_door(Map* map, int size, Vec2 c, Vec2 dir, int shape_id)
{
    Vec2 door_coord = (Vec2){c.x + dir.x, c.y + dir.y};
    // move in direction until out of bounds or find a non floor
    while(is_inside_bounds(*map,  door_coord) && map->cells[coord_to_idx(door_coord)].terrain_id == FLOOR_ID){
      door_coord.x += dir.x; 
      door_coord.y += dir.y; 
    };

    // make sure the coord is not out of bounds and add the door
    if(is_inside_bounds(*map, door_coord)){
      map->cells[coord_to_idx(door_coord)].terrain_id = DOOR_ID;
    } else {
      return;
    }

    // calculate where the room top left coord will be
    Vec2 origin = {0};
    origin.x = door_coord.x;
    origin.y = door_coord.y;
    if(shape_id == RECT_ID){
      if(dir.x < 0){
        // if dir left, on x -1*size, on y -1 (size/2 round down) (from door) 
        origin.x -= size;
        origin.y -= (int)(size/2);
      } else if (dir.x > 0){
        // if dir right, on x +1, on y -1 (size/2 round down)
        origin.x += 1;
        origin.y -= (int)(size/2);
      } else if (dir.y < 0){
        // if dir up, on x -1*size/2, on y -1
        origin.x -= (int)(size/2);
        origin.y -= size;
      } else if (dir.y > 0){
        // if dir down, on x -1*size, on y +1
        origin.x -= (int)(size/2);
        origin.y += 1;
      }
    }
    if(shape_id == CIRCLE_ID) {
      if(dir.x < 0){
        // if dir left, on x -1*size, on y -1 (size/2 round down) (from door) 
        origin.x -= size-1;
      } else if (dir.x > 0){
        // if dir right, on x +1, on y -1 (size/2 round down)
        origin.x += size+1;
      } else if (dir.y < 0){
        // if dir up, on x -1*size/2, on y -1
        origin.y -= size-1;
      } else if (dir.y > 0){
        // if dir down, on x -1*size, on y +1
        origin.y += size+1;
      }
    }

    if(is_rect){
      dig_rectangle(map, origin.x, origin.y, size, size);
    }
    if(is_circle){
      dig_circle(map, origin.x, origin.y, size);
    }

    // using the top left coord (origin) and the width, height, dig out the room
}

void dig_tunnel_thickness(Map* map, int tunnel_size, Vec2 curr_direction, Vec2 moving_coord)
{
  int iter = 0;
  do {
    iter++;
    // if size == 1
    map->cells[coord_to_idx(moving_coord)].terrain_id = FLOOR_ID;
    moving_coord.x += curr_direction.x;
    moving_coord.y += curr_direction.y;
    
    int size_decrementor = tunnel_size;
    // dig tunnel sides
    do {
      Vec2 increment = {0};
      if (size_decrementor % 2 == 1){
        if(curr_direction.x != 0) {
          increment.y = (int)(size_decrementor/2); 
        } else if(curr_direction.y != 0) {
          increment.x = (int)(size_decrementor/2); 
        }
      } else {
        if(curr_direction.x != 0) {
          increment.y = -size_decrementor/2; 
        } else if(curr_direction.y != 0) {
          increment.x = -size_decrementor/2; 
        }
      }
      Vec2 new_tunnel = {moving_coord.x + increment.x, moving_coord.y + increment.y};
      if(is_inside_bounds(*map, new_tunnel)){
        map->cells[coord_to_idx(new_tunnel)].terrain_id = FLOOR_ID;
      }
      size_decrementor--;
    } while(size_decrementor > 1);
    
    // after digging the tunnel part
    // have a chance to spawn a room on a perpendicular direction to the tunnel
    // int room_chance = rand() % 100;
    // if(room_chance <= 20){
    //   Vec2 room_dir = curr_direction.x == 0 ? (Vec2){1, 0} : (Vec2){0, 1};
    //   int room_size = (rand()%4) + 2;
    //   // randomly choose up/down/left/right
    //   if(room_chance % 2 == 0){
    //     room_dir.x *= -1;
    //     room_dir.y *= -1;
    //   }
    //   dig_room(map, room_size, moving_coord, room_dir);
    // }

  // } while(is_inside_bounds(*map, moving_coord));
  } while(iter < 1000);
}

Vec2 dig_tunnel_tile_step(Map* map, int tunnel_size, Vec2 curr_direction, Vec2 moving_coord)
{
  if(tunnel_size == 0) return moving_coord;
  // digging the "head" tunnel tile
  if(!is_inside_bounds(*map, (Vec2){moving_coord.x, moving_coord.y})) return moving_coord;
  map->cells[coord_to_idx(moving_coord)].terrain_id = FLOOR_ID;

  Vec2 changing_coord = {moving_coord.x, moving_coord.y};
  changing_coord.x += curr_direction.x;
  changing_coord.y += curr_direction.y;
  
  int size_decrementor = tunnel_size;
  // dig tunnel sides
  do {
    Vec2 increment = {0};
    if (size_decrementor % 2 == 1){
      if(curr_direction.x != 0) {
        increment.y = (int)(size_decrementor/2); 
      } 
      if(curr_direction.y != 0) {
        increment.x = (int)(size_decrementor/2); 
      }
    } else {
      if(curr_direction.x != 0) {
        increment.y = -size_decrementor/2; 
      } 
      if(curr_direction.y != 0) {
        increment.x = -size_decrementor/2; 
      }
    }
    Vec2 new_tunnel = {changing_coord.x + increment.x, changing_coord.y + increment.y};
    if(is_inside_bounds(*map, new_tunnel)){
      map->cells[coord_to_idx(new_tunnel)].terrain_id = FLOOR_ID;
    }
    size_decrementor--;
  } while(size_decrementor > 1);
  
  // after digging the tunnel part
  // have a chance to spawn a room on a perpendicular direction to the tunnel
  // int room_chance = rand() % 100;
  // if(room_chance <= 20){
  //   Vec2 room_dir = curr_direction.x == 0 ? (Vec2){1, 0} : (Vec2){0, 1};
  //   int room_size = (rand()%4) + 2;
  //   // randomly choose up/down/left/right
  //   if(room_chance % 2 == 0){
  //     room_dir.x *= -1;
  //     room_dir.y *= -1;
  //   }
  //   dig_room(map, room_size, moving_coord, room_dir);
  // }

  // } while(is_inside_bounds(*map, moving_coord));
  return moving_coord;
}

void tunneler(Map* map)
{
  int x = map->width/2;
  int y = map->height/2;
  int tunnel_min_size = 3;
  int tunnel_max_size = 6;

  // have a chance to spawn more tunnelers either at the mid of the path

  // going in a cross direction from the center
  Vec2 directions[4] = {{0, 1}, {0, -1}, {1,0},{-1,0}};
  for(int i = 0; i < 4; i++) {
    Vec2 starting_coord = {x, y}; 
    Vec2 curr_direction = directions[i];
    int tunnel_size = rand()%(tunnel_max_size) + tunnel_min_size;
    dig_tunnel_thickness(map, tunnel_size, curr_direction, starting_coord);
  }
}

int choose_random_shape(){
  const int shape_chance = rand()%1000;
    if (shape_chance <= 500){
      return RECT_ID;
    } else if ( shape_chance  > 500){
      return CIRCLE_ID;
    } else {
      return RECT_ID;
    }
  return RECT_ID;
}
// void dig_room_v2(map, room_size, moving_coord, room_dir, shape)
// {
// }
bool check_if_room_fits(Map map, int room_size, Vec2 moving_coord, Vec2 room_dir, int shape_id)
{
  
}
void tunneler_v2(Map* map, Vec2 start_dir, Vec2 origin_coord, int tunnel_max_thick, int horizontal_set_walk, int vertical_set_walk, int max_iter)
{
  // NOTES:
  // Kyz wiz: parameters:
  // Width, direction, speed, chance to turn, chance to create rooms,
  // size and shape of rooms to create,
  // how much space to leave between self and other dungeon objects,
  // when to quit
  int x, y;
  if(origin_coord.x <= -1){
    x = map->width/20;
    y = map->height/20;
  } else {
    x = origin_coord.x;
    y = origin_coord.y;
  }

  Vec2 dir;
  Vec2 down = {0, 1};
  Vec2 up = {0, -1};
  Vec2 right = {1, 0};
  Vec2 left = {-1, 0};
  if(start_dir.x <= -1){
    dir = down;
  } else {
    dir = start_dir;
  }

  int tunnel_min_size = 1;
  int tunnel_max_size = 3;
  int tunnel_size;

  // deliberately set to -1 means randomize
  if(tunnel_max_thick <= -1){
    tunnel_size = (rand()%tunnel_max_size) + tunnel_min_size;
  } else if (tunnel_max_thick == 0){
    // setting as 0 (should) means stop (branching path that got too thin
    return; 
  } else {
    tunnel_size = rand()%(tunnel_max_thick);
  }

  int max_walk_horizontal;
  if(horizontal_set_walk <= -1){
    max_walk_horizontal = rand()%(COLS/20)+10;
  } else {
    max_walk_horizontal = horizontal_set_walk;
  }

  int max_walk_vertical;
  if(vertical_set_walk <= -1){
    max_walk_vertical  = (rand()%(ROWS/20)+10);
  } else {
    max_walk_vertical = vertical_set_walk;
  }

  // have a chance to spawn more tunnelers either at the mid of the path

  // going in a cross direction from the center
  // start top left and move towards bottom right kinda
  Vec2 moving_coord = {x, y}; 
  int tiles_walked = 0;
  int iter = 0;
  do{
    if(max_walk_vertical == 0 || max_walk_horizontal == 0) return;
    // walking vertically
    if(dir.y != 0 && tiles_walked > max_walk_vertical){// ROWS*8/10){
      if(moving_coord.x > COLS/4){
        dir = rand()%2 == 0 ? left : right;
      } else {
        dir = right;
      }
      tiles_walked = 0;
      if(moving_coord.x > COLS/4 && rand()%2 == 0) {
       tunneler_v2(map, left, moving_coord, tunnel_size - 1, max_walk_horizontal/3, max_walk_vertical/3, max_iter/2);
      } else {
        tunneler_v2(map, dir, moving_coord, tunnel_size - 1, max_walk_horizontal/2, max_walk_vertical/2, max_iter/2);
      }
      tunnel_size = rand()%(tunnel_max_size) + tunnel_min_size;
      max_walk_horizontal = (rand()%(COLS*3/8)+(COLS/8));
      // chance to spawn a new branching path
    // } else if(dir.x == up.x && dir.y == up.y && tiles_walked > max_walk_vertical){// ROWS*8/10){
    //   dir = right;
    //   tiles_walked = 0;
    //   if(moving_coord.x > COLS/4 && rand()%2 == 0) {
    //    tunneler_v2(map, left, moving_coord, tunnel_size - 1, max_walk_horizontal/2, max_walk_vertical/2);
    //   }
    //   tunnel_size = rand()%(tunnel_max_size) + tunnel_min_size;
    //   max_walk_horizontal = (rand()%(COLS*3/8)+(COLS/8));
    } else if(dir.x != 0 && tiles_walked > max_walk_horizontal && moving_coord.y >= ROWS/2){// ROWS*8/10){
      dir = up;
      tiles_walked = 0;
      if(moving_coord.y > ROWS/2 && rand()%2 == 0) {
       tunneler_v2(map, down, moving_coord, tunnel_size - 1, max_walk_horizontal/3, max_walk_vertical/3, max_iter/2);
      }
      tunnel_size = rand()%(tunnel_max_size) + tunnel_min_size;
      max_walk_vertical = (rand()%(ROWS/2)+(ROWS/3));
    } else if(dir.x != 0 && tiles_walked > max_walk_horizontal && moving_coord.y <= ROWS/2){// ROWS*8/10){
      dir = down;
      tiles_walked = 0;
      if(moving_coord.y < ROWS/2 && rand()%2 == 0) {
       tunneler_v2(map, up, moving_coord, tunnel_size - 1, max_walk_horizontal/3, max_walk_vertical/3, max_iter/2);
      }
      tunnel_size = rand()%(tunnel_max_size) + tunnel_min_size;
      max_walk_vertical = (rand()%(ROWS/2)+(ROWS/3));
    }
    moving_coord = dig_tunnel_tile_step(map, tunnel_size, dir, moving_coord);

    // 5% chance to spawn a room (every tile moved)
    if(rand()%1000 < 50){
      int room_chance = rand() % 100;
      Vec2 room_dir = dir.x == 0 ? (Vec2){1, 0} : (Vec2){0, 1};
      int room_size = (rand()%6) + 3;
      // randomly choose up/down/left/right
      if(room_chance % 2 == 0){
        room_dir.x *= -1;
        room_dir.y *= -1;
      }
      int shape = choose_random_shape();
      check_if_room_fits(*map, room_size, moving_coord, room_dir, shape);
      // dig_room_v2(map, room_size, moving_coord, room_dir, shape);

      int shape_chance = rand()%100;
      int shape_id = RECT_ID;
      if(shape_chance <= 90) {
      // } else if(shape_chance > 60){
      } else {
       shape_id = CIRCLE_ID;
      }
      dig_room_with_door(map, room_size, moving_coord, room_dir, shape_id);
    }
    // take a step
    if(is_inside_bounds(*map, (Vec2){moving_coord.x + dir.x, moving_coord.y + dir.y})){
      moving_coord.x += dir.x;
      moving_coord.y += dir.y;

    } else {
      break;
    }
    tiles_walked++;
    iter++;
  }while(is_inside_bounds(*map, moving_coord) && iter < max_iter);

}

void init_completely_walled_map(Map* map, uint width, uint height)
{
  map->width = width;
  map->height = height;
  Tile* cells = (Tile*)malloc(sizeof(Tile)*width*height);
  for(uint j = 0; j < height; j++){
    for(uint i = 0; i < width; i++){
        cells[j*width + i] = (Tile){0};
        cells[j*width + i].terrain_id = STONE_ID;
        cells[j*width + i].checked = false;
    } 
  } 
  map->cells = cells;
  return;
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

  srand(time(NULL));
  // init_map(&map, COLS, ROWS);
  init_completely_walled_map(&map, COLS, ROWS);
  // dig_elipsis(&map, COLS/2, ROWS/2, 15, 8);
  // dig_rectangle(&map, COLS/2, ROWS/2, 4, 6);
  // add_walls_to_borders(&map, (COLS+1)/2, (ROWS+1)/2);
  // dig_circle(&map, COLS/2, ROWS/2, 14);
  tunneler_v2(&map, (Vec2){-1, -1}, (Vec2){-1, -1}, -1, -1, -1, 800);
  // map_generator_random(&map);
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

  Image stone_img = LoadImage("stone.png");
  ImageResizeNN(&stone_img, cell_width, cell_height);
  Texture2D stone_tex = LoadTextureFromImage(stone_img);

  Image door_img = LoadImage("door.png");
  ImageResizeNN(&door_img, cell_width, cell_height);
  Texture2D door_tex = LoadTextureFromImage(door_img);

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
            if(tile.terrain_id == FLOOR_ID){
              c = '.';
              t = floor_tex;
            } else if(tile.terrain_id == WALL_ID){
              c = '#';
              t = pillar_tex;
            } else if(tile.terrain_id == STONE_ID){
              c = '0';
              t = stone_tex;
            } else if(tile.terrain_id == DOOR_ID){
              c = '+';
              t = door_tex;
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
