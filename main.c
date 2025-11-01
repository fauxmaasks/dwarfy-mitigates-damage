#include "raylib.h"
#include <malloc.h>
// #include <wchar.h>
#define ROGUE_H_IMPLEMENTATION
#include "rogue.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
// slightly smaller than 1920 x 1080 to fit the screen better
#define SCREEN_WIDTH 1900
#define SCREEN_HEIGHT 1000
#define GRID_WIDTH 1200
#define GRID_HEIGHT 900
#define X_OFFSET 100
#define Y_OFFSET 20
#define COLS 20
#define ROWS 10
#define MAX_PILLAR_NUM 20

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

#define PLAYER_TEAM 1
#define NEUTRAL_TEAM 2
#define ENEMY_TEAM 3

#define PLAYER_TYPE 1
#define NEUTRAL_TYPE 2
#define BOMBY_ENEMY_TYPE 3
#define CYCLOPS_ENEMY_TYPE 4
#define DRAGON_ENEMY_TYPE 5

// #define EMPTY_ENTITY_IDX RAND_MAX
#define EMPTY_ENTITY_IDX 100

// order of precendence
// 1: ++ -- 	Prefix increment and decrement[note 1] 	Right-to-left
// + - 	Unary plus and minus
// ! ~ 	Logical NOT and bitwise NOT
// (type) 	Cast
// * 	Indirection (dereference)
// & 	Address-of
// sizeof 	Size-of[note 2]
// _Alignof 	Alignment requirement(C11) ++ --
// 2: ++ -- 	Prefix increment and decrement[note 1] 	Right-to-left
// + - 	Unary plus and minus
// ! ~ 	Logical NOT and bitwise NOT
// (type) 	Cast
// * 	Indirection (dereference)
// & 	Address-of
// sizeof 	Size-of[note 2]
// _Alignof 	Alignment requirement(C11)
// 3: * / % 	Multiplication, division, and remainder
// 4: + - 	Addition and subtraction
// 5: << >> 	Bitwise left shift and right shift
// 6:  	< <= 	For relational operators < and ≤ respectively
//      > >= 	For relational operators > and ≥ respectively
// 7: == != 	For relational = and ≠ respectively

unsigned int t;
typedef unsigned int uint;

typedef enum Directions{
  North,
  East,
  South,
  West,
}Directions;

const Vec2 DIRECTIONS[4] = {
  {0, -1},
  {1, 0},
  {0, 1},
  {-1, 0},
};

typedef struct Tile {
  // 0 idx means it has nothing
  uint entity_idx;
  uint item_idx;
  uint terrain_id;
  bool checked;
} Tile;

typedef struct Map {
  uint width;
  uint height;
  Tile *cells;
} Map;

typedef struct Entity {
  char character;
  int HP;
  int teamID;
  int type;
  Directions dir;
} Entity;

typedef struct Entity_arr {
  uint cap;
  uint size;
  Entity *items;
} Entity_arr;

uint mode = NORMAL_MODE;

uint coord_to_idx(Vec2 c) { return c.y * COLS + c.x; }

double distf(Vec2 c1, Vec2 c2) {
  return sqrt(pow(c2.x - c1.x, 2) + pow(c2.y - c1.y, 2));
}

bool is_inside_rec(Rec r, Vec2 new_coord) {
  return new_coord.x >= r.coord.x && new_coord.y >= r.coord.y &&
         new_coord.x < r.coord.x + r.width &&
         new_coord.y < r.coord.y + r.height;
}

bool is_inside_map(Map map, Vec2 new_coord) {
  return new_coord.x >= 0 && new_coord.y >= 0 && new_coord.x < map.width &&
         new_coord.y < map.height;
}

bool is_valid_to_move(Map map, Vec2 new_coord) {
  return map.cells[coord_to_idx(new_coord)].entity_idx == RAND_MAX &&
         map.cells[coord_to_idx(new_coord)].terrain_id == FLOOR_ID;
}

bool are_coords_equal(Vec2 c1, Vec2 c2) {
  return c1.x == c2.x && c1.y == c2.y;
}

void get_atk_coords_from_entity(Map map, Entity en, int x, int y, Vec2_Arr* result){
  switch(en.type) {
    case CYCLOPS_ENEMY_TYPE: {
      // get a line on entities dir
      int total_tiles;
      if (en.dir == North) {
        total_tiles = y; 
      } else if (en.dir == South) {
        total_tiles = ROWS - y; 
      } else if (en.dir == West) {
        total_tiles = x; 
      } else if (en.dir == East) {
        total_tiles = COLS - x; 
      }
      result->cap = total_tiles;
      result->items = (Vec2*)malloc(sizeof(Vec2)*total_tiles);
      result->size = total_tiles;
      Vec2 curr_coord = {x, y};
      Vec2 dir_coord = DIRECTIONS[en.dir];
      for(int i = 0; i < total_tiles; i++){
        curr_coord.x += dir_coord.x; 
        curr_coord.y += dir_coord.y; 
        result->items[i].x = curr_coord.x;
        result->items[i].y = curr_coord.y;
      }
      break;
    }
    case BOMBY_ENEMY_TYPE: {
      // get a 9 tiles around bomby + self tile 
      result->cap = 9;
      result->items = (Vec2*)malloc(sizeof(Vec2)*9);
      result->size = 9;
      int count = 0;
      for(int j = -1; j < 2; j++){
        for(int i = -1; i < 2; i++){
          result->items[count] = (Vec2){.x = x+i, .y = y+j};
          count++;
          result->size=count;
        }
      }
      break;
    }
    case DRAGON_ENEMY_TYPE: {
      // cone - for now very simplified version, rectangle!
      int height, width, center_x, center_y;
      Vec2 en_dir = DIRECTIONS[en.dir];
      if(en.dir == North || en.dir == South){
        height = 5;
        width = 3;
      } else {
        height = 3;
        width = 5;
      }
      center_x = x + en_dir.x*3;
      center_y = y + en_dir.y*3;
      result->cap = width*height;
      result->items = (Vec2*)malloc(sizeof(Vec2)*width*height);
      result->size = width*height;

      int count = 0;
      for (int y = -height / 2; y <= height / 2; y++) {
        for (int x = -width / 2; x <= width / 2; x++) {
          Vec2 c = {center_x + x, center_y + y};
          result->items[count].x = c.x;
          result->items[count].y = c.y;
          count++;
        }
      }
      break;
    }
  }
}

Vec2 get_new_direction_not_going_back_nor_going_to_bound(Rec bounds,
                                                         Vec2 moving_coord,
                                                         int speed, Vec2 dir) {
  Vec2 new_dir;
  // try going to perpendicularly first
  new_dir.x = rand() % 2 == 0 ? dir.y : -dir.y;
  new_dir.y = rand() % 2 == 0 ? dir.x : -dir.x;
  Vec2 next_coord_2 = {moving_coord.x + new_dir.x * speed * 2,
                       moving_coord.y + new_dir.y * speed * 2};
  if (!is_inside_rec(bounds, next_coord_2)) {
    new_dir.x *= -1;
    new_dir.y *= -1;
    next_coord_2 = (Vec2){moving_coord.x + new_dir.x * speed * 2,
                          moving_coord.y + new_dir.y * speed * 2};
    if (!is_inside_rec(bounds, next_coord_2)) {
      // if both don't work, go backwards
      new_dir.x = -dir.x;
      new_dir.y = -dir.y;
    }
  }

  return new_dir;
}

void add_walls_to_borders(Map *map, int origin_x, int origin_y) {
  // "flood fills" walls around
  Vec2 tiles[COLS * ROWS];
  tiles[0].x = origin_x;
  tiles[0].y = origin_y;
  int size = 1;
  int iter = 0;
  do {
    size--;
    Vec2 curr_coord;
    curr_coord.x = tiles[size].x;
    curr_coord.y = tiles[size].y;
    for (int y = -1; y <= 1; y++) {
      for (int x = -1; x <= 1; x++) {
        int sum = rg_abs(x) + rg_abs(y);
        if (y == 0 && x == 0) {
          continue;
        }
        Vec2 new_coord = {curr_coord.x + x, curr_coord.y + y};
        if (is_inside_map(*map, new_coord)) {
          if (map->cells[coord_to_idx(new_coord)].checked) {
            continue;
          } else if (map->cells[coord_to_idx(new_coord)].terrain_id ==
                     FLOOR_ID) {
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
  } while (size > 0);
}

void make_quadrants(Vec2 **qs, int *size, Map map, int x_splits, int y_splits) {
  *size = x_splits * y_splits;
  Vec2 *new_q = (Vec2 *)malloc(sizeof(Vec2) * (*size));
  int i = 0;
  for (int x = 0; x < x_splits; x++) {
    for (int y = 0; y < y_splits; y++) {
      new_q[i] =
          (Vec2){.x = x * map.width / x_splits, .y = y * map.height / y_splits};
      i++;
    }
  }
  *qs = new_q;
}

void map_generator_random(Map *map) {
  int pillar_min_size = 1;
  int pillar_max_size = 2;

  Vec2 *qs;
  int size;
  int x_split = 4;
  int y_split = 2;
  int q_width = map->width / x_split;
  int q_height = map->height / y_split;
  make_quadrants(&qs, &size, *map, x_split, y_split);
  int count = 0;
  do {
    int pillar_size = (rand() % (pillar_max_size - pillar_min_size + 1)) + pillar_min_size;

    Vec2 q = qs[rand() % size];
    int pillar_x = (rand() % (q_width - pillar_size - 1)) + q.x + 1;
    int pillar_y = (rand() % (q_height - pillar_size - 1)) + q.y + 1;

    for (int y = pillar_y; y < pillar_y + pillar_size; y++) {
      for (int x = pillar_x; x < pillar_x + pillar_size; x++) {
        Vec2 c = (Vec2){.x = x, .y = y};
        if (is_inside_map(*map, c)) {
          map->cells[coord_to_idx(c)].terrain_id = WALL_ID;
        }
      }
    }
    count++;

  } while (count < MAX_PILLAR_NUM);
  free(qs);
}

bool is_rectangle_available_to_dig(Map *map, int origin_x, int origin_y,
                                   int width, int height) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Vec2 c = {origin_x + x, origin_y + y};
      if (is_inside_map(*map, c) &&
          map->cells[coord_to_idx(c)].terrain_id == FLOOR_ID) {
        return false;
      }
    }
  }
  return true;
}


void dig_rectangle_from_center(Map *map, int center_x, int center_y, int width, int height) {
  for (int y = -height / 2; y <= height / 2; y++) {
    for (int x = -width / 2; x <= width / 2; x++) {
      Vec2 c = {center_x + x, center_y + y};
      if (is_inside_map(*map, c)) {
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      }
    }
  }
}

void dig_rectangle(Map *map, int origin_x, int origin_y, int width, int height) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Vec2 c = {origin_x + x, origin_y + y};
      if (is_inside_map(*map, c)) {
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      }
    }
  }
}

bool is_circle_available_to_dig(Map *map, int origin_x, int origin_y, int radius) {
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      Vec2 c = {origin_x + x, origin_y + y};
      if (is_inside_map(*map, c) &&
          distf((Vec2){origin_x, origin_y}, c) < radius + 0.5 &&
          map->cells[coord_to_idx(c)].terrain_id == FLOOR_ID) {
        return false;
      }
    }
  }
  return true;
}

void dig_circle(Map *map, int origin_x, int origin_y, int radius) {
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      Vec2 c = {origin_x + x, origin_y + y};
      if (is_inside_map(*map, c) &&
          distf((Vec2){origin_x, origin_y}, c) < radius + 0.5) {
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      }
    }
  }
}

void dig_elipsis(Map *map, int origin_x, int origin_y, int radius_x, int radius_y) {
  // elipsis formula = x^2/a^2 + y^2/b^2 = 1, a and b are the horizontal and
  // vertical extremities sort by left top point to right bottom actually it's
  // the left most, if sorting by both x and y mistakes occur
  for (int y = origin_y - radius_y - 1; y <= origin_y + radius_y + 1; y++) {
    for (int x = origin_x - radius_x - 1; x <= origin_x + radius_x + 1; x++) {
      Vec2 c = {x, y};
      float dist = ((float)pow(x - origin_x, 2) / (float)pow(radius_x, 2) +
                    (float)pow(y - origin_y, 2) / (float)pow(radius_y, 2));
      if (is_inside_map(*map, c) && dist <= 1.0f) {
        map->cells[coord_to_idx(c)].terrain_id = FLOOR_ID;
      }
    }
  }
}

void dig_room_with_door(Map *map, int size, Vec2 c, Vec2 dir, int shape_id) {
  Vec2 door_coord = (Vec2){c.x + dir.x, c.y + dir.y};
  // move in direction until out of bounds or find a non floor
  while (is_inside_map(*map, door_coord) &&
         map->cells[coord_to_idx(door_coord)].terrain_id == FLOOR_ID) {
    door_coord.x += dir.x;
    door_coord.y += dir.y;
  };

  // make sure the coord is not out of bounds and add the door
  if (is_inside_map(*map, door_coord)) {
    map->cells[coord_to_idx(door_coord)].terrain_id = DOOR_ID;
  } else {
    return;
  }

  // calculate where the room top left coord will be
  Vec2 origin = {0};
  origin.x = door_coord.x;
  origin.y = door_coord.y;
  if (shape_id == RECT_ID) {
    if (dir.x < 0) {
      // if dir left, on x -1*size, on y -1 (size/2 round down) (from door)
      origin.x -= size;
      origin.y -= (int)(size / 2);
    } else if (dir.x > 0) {
      // if dir right, on x +1, on y -1 (size/2 round down)
      origin.x += 1;
      origin.y -= (int)(size / 2);
    } else if (dir.y < 0) {
      // if dir up, on x -1*size/2, on y -1
      origin.x -= (int)(size / 2);
      origin.y -= size;
    } else if (dir.y > 0) {
      // if dir down, on x -1*size, on y +1
      origin.x -= (int)(size / 2);
      origin.y += 1;
    }
  }
  if (shape_id == CIRCLE_ID) {
    if (dir.x < 0) {
      // if dir left, on x -1*size, on y -1 (size/2 round down) (from door)
      origin.x -= size - 1;
    } else if (dir.x > 0) {
      // if dir right, on x +1, on y -1 (size/2 round down)
      origin.x += size + 1;
    } else if (dir.y < 0) {
      // if dir up, on x -1*size/2, on y -1
      origin.y -= size - 1;
    } else if (dir.y > 0) {
      // if dir down, on x -1*size, on y +1
      origin.y += size + 1;
    }
  }

  if (shape_id == RECT_ID && is_rectangle_available_to_dig(
                                 map, origin.x, origin.y, size + 1, size + 1)) {
    dig_rectangle(map, origin.x, origin.y, size, size);
  } else {
    printf("not able to dig rect at given coord and size\n");
  }
  if (shape_id == CIRCLE_ID &&
      is_circle_available_to_dig(map, origin.x, origin.y, size + 1)) {
    dig_circle(map, origin.x, origin.y, size);
  } else {
    printf("not able to dig rect at given coord and size\n");
  }
}

void dig_tunnel_thickness(Map *map, int tunnel_size, Vec2 curr_direction,
                          Vec2 moving_coord) {
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
      if (size_decrementor % 2 == 1) {
        if (curr_direction.x != 0) {
          increment.y = (int)(size_decrementor / 2);
        } else if (curr_direction.y != 0) {
          increment.x = (int)(size_decrementor / 2);
        }
      } else {
        if (curr_direction.x != 0) {
          increment.y = -size_decrementor / 2;
        } else if (curr_direction.y != 0) {
          increment.x = -size_decrementor / 2;
        }
      }
      Vec2 new_tunnel = {moving_coord.x + increment.x,
                         moving_coord.y + increment.y};
      if (is_inside_map(*map, new_tunnel)) {
        map->cells[coord_to_idx(new_tunnel)].terrain_id = FLOOR_ID;
      }
      size_decrementor--;
    } while (size_decrementor > 1);

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

    // } while(is_inside_map(*map, moving_coord));
  } while (iter < 1000);
}

Vec2 dig_tunnel_tile_step_v2(Map *map, Rec bounds, int tunnel_size,
                             Vec2 direction, Vec2 moving_coord, int steps) {
  if (tunnel_size == 0)
    return moving_coord;
  // if no tunnel size or tunne is out of bounds, don't dig
  for (int i = 0; i < steps; i++) {

    if (!is_inside_rec(bounds, (Vec2){moving_coord.x, moving_coord.y}))
      return (Vec2){moving_coord.x - direction.x, moving_coord.y - direction.y};

    map->cells[coord_to_idx(moving_coord)].terrain_id = FLOOR_ID;

    int size_decrementor = tunnel_size;
    // dig tunnel sides
    do {
      Vec2 increment = {0};
      if (size_decrementor % 2 == 1) {
        if (direction.x != 0) {
          increment.y = (int)(size_decrementor / 2);
        }
        if (direction.y != 0) {
          increment.x = (int)(size_decrementor / 2);
        }
      } else {
        if (direction.x != 0) {
          increment.y = -size_decrementor / 2;
        }
        if (direction.y != 0) {
          increment.x = -size_decrementor / 2;
        }
      }
      Vec2 new_tunnel = {moving_coord.x + increment.x,
                         moving_coord.y + increment.y};
      if (is_inside_rec(bounds, new_tunnel)) {
        map->cells[coord_to_idx(new_tunnel)].terrain_id = FLOOR_ID;
      }
      size_decrementor--;
    } while (size_decrementor > 1);

    // take a step in the direction
    moving_coord.x += direction.x;
    moving_coord.y += direction.y;
  }
  return moving_coord;
}

Vec2 dig_tunnel_tile_step(Map *map, int tunnel_size, Vec2 direction,
                          Vec2 moving_coord) {
  if (tunnel_size == 0)
    return moving_coord;
  // digging the "head" tunnel tile
  if (!is_inside_map(*map, (Vec2){moving_coord.x, moving_coord.y}))
    return moving_coord;
  map->cells[coord_to_idx(moving_coord)].terrain_id = FLOOR_ID;

  Vec2 changing_coord = {moving_coord.x, moving_coord.y};
  changing_coord.x += direction.x;
  changing_coord.y += direction.y;

  int size_decrementor = tunnel_size;
  // dig tunnel sides
  do {
    Vec2 increment = {0};
    if (size_decrementor % 2 == 1) {
      if (direction.x != 0) {
        increment.y = (int)(size_decrementor / 2);
      }
      if (direction.y != 0) {
        increment.x = (int)(size_decrementor / 2);
      }
    } else {
      if (direction.x != 0) {
        increment.y = -size_decrementor / 2;
      }
      if (direction.y != 0) {
        increment.x = -size_decrementor / 2;
      }
    }
    size_decrementor--;
  } while (size_decrementor > 1);

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

  // } while(is_inside_map(*map, moving_coord));
  return moving_coord;
}

void tunneler(Map *map) 
{
  // simple tunneler that starts in the middle and digs a cross section
  // improvement thoughts: make each direction more windy (snakey), make them connect on the edges
  int x = map->width / 2;
  int y = map->height / 2;
  int tunnel_min_size = 3;
  int tunnel_max_size = 6;

  // have a chance to spawn more tunnelers either at the mid of the path

  // going in a cross direction from the center
  Vec2 directions[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  for (int i = 0; i < 4; i++) {
    Vec2 starting_coord = {x, y};
    Vec2 curr_direction = directions[i];
    int tunnel_size = rand() % (tunnel_max_size) + tunnel_min_size;
    dig_tunnel_thickness(map, tunnel_size, curr_direction, starting_coord);
  }
}

int choose_random_shape() 
{
  const int shape_chance = rand() % 1000;
  if (shape_chance <= 500) {
    return RECT_ID;
  } else if (shape_chance > 500) {
    return CIRCLE_ID;
  } else {
    return RECT_ID;
  }
  return RECT_ID;
}

void tunneler_v3(Map *map, Rec bounds, Vec2 dir, Vec2 origin_coord,
                 int tunnel_max_thick, int max_walk, int speed, int turn_chance,
                 int room_spawn_chance) {
  // NOTES: Kyz the Wiz: parameters:
  // Width, direction, speed, chance to turn, chance to create rooms,
  // size and shape of rooms to create,
  // how much space to leave between self and other dungeon objects,
  // when to quit
  int x, y;
  if (origin_coord.x == -100 || origin_coord.y == -100) {
    x = map->width / 2;
    y = map->height * 19 / 20;
  } else {
    x = origin_coord.x;
    y = origin_coord.y;
  }

  if (speed == -100) {
    speed = 5;
  } else if (speed <= 0) {
    return;
  }
  if (turn_chance == -100) {
    turn_chance = 300;
  } else if (turn_chance <= 0) {
    return;
  }
  if (room_spawn_chance == -100) {
    room_spawn_chance = 100; // 10% (100 out of 1000)
  }
  int tunnel_min_size = 5;
  int tunnel_max_size = 5;
  int tunnel_size;

  // deliberately set to -1 means randomize
  if (tunnel_max_thick == -100) {
    tunnel_size =
        (rand() % (tunnel_max_size - tunnel_min_size + 1)) + tunnel_min_size;
  } else if (tunnel_max_thick <= 0) {
    // setting as 0 (should) means stop, branching path that got too thin
    return;
  } else {
    if (tunnel_max_thick == 1) {
      tunnel_size = 1;
    } else {
      tunnel_size = rand() % (tunnel_max_thick - 1) + 1; // from 1 to max thick
    }
  }

  if (bounds.width <= 0 || bounds.height <= 0) {
    // if unset, default to the whole map as the bounds
    bounds.coord.x = 10;
    bounds.coord.y = 10;
    bounds.width = map->width - 10;
    bounds.height = map->height - 10;
  }
  Vec2 down = {0, 1};
  Vec2 up = {0, -1};
  Vec2 right = {1, 0};
  Vec2 left = {-1, 0};
  Vec2 directions[4] = {up, down, left, right};
  if (dir.x == -100 || dir.y == -100) {
    dir = up;
  }

  if (max_walk == -100) {
    max_walk = rand() % (30) + 5; // 5 to 35 steps
  } else if (max_walk <= 0) {
    return;
  }
  Vec2 moving_coord = {x, y};
  int tiles_walked = 0;
  do {

    if (max_walk == 0)
      return;
    // dig current location
    // prob move speed steps
    moving_coord = dig_tunnel_tile_step_v2(map, bounds, tunnel_size, dir,
                                           moving_coord, speed);

    // chance to turn
    if (rand() % 1000 <= turn_chance) {
      // lower the chance to turn? maybe not, only if turning twice in close
      // proximity? dig a junction
      dig_rectangle_from_center(map, moving_coord.x, moving_coord.y,
                                tunnel_size + 2, tunnel_size + 2);
      if (rand() % 4 <= 2) {
        // chance to switch the moving coord for interesting paths
        //  should be picking somewhere inside the junction rect
        moving_coord.x += rand() % (tunnel_size + 2) - tunnel_size / 2;
        moving_coord.y += rand() % (tunnel_size + 2) - tunnel_size / 2;
      }
      dir = get_new_direction_not_going_back_nor_going_to_bound(
          bounds, moving_coord, speed, dir);
      // Vec2 new_moving_coord = (Vec2){moving_coord.x + dir.x*speed,
      // moving_coord.y + dir.y*speed}; if(!is_inside_rec(bounds,
      // moving_coord)){
      //   dir = get_new_direction_not_going_back_nor_going_to_bound(bounds,
      //   moving_coord, speed, dir);
      // }

      // if(!is_inside_rec(bounds, new_moving_coord)){
      // }
      ;
      // Vec2 next_2_coord = (Vec2){moving_coord.x + 3*dir.x, moving_coord.y +
      // 3*dir.y}; if(is_inside_map(*map, next_coord) && is_inside_map(*map,
      // next_2_coord) && map->cells[coord_to_idx(next_2_coord)].terrain_id !=
      // FLOOR_ID){ instead of stoping try changing the direction dir = ;
      // next_coord = (Vec2){moving_coord.x + dir.x*speed, moving_coord.y +
      // dir.y*speed};
    }

    // change direction to not be the same
    //  if x is not zero, then y won't be zero, and vice versa
    //  50/50 to turn
    //  if(rand()%2 == 0){
    //    new_dir.x *= -1;
    //    new_dir.y *= -1;
    //  }
    //
    //
    //  int new_turn_chance = turn_chance;
    //  if(rand()%2 == 0){
    //    Vec2 new_dir;
    //    new_dir.x = dir.y;
    //    new_dir.y = dir.x;
    //  tunneler_v3(map, bounds, new_dir, moving_coord, tunnel_size-1,
    //  max_walk/2, speed, new_turn_chance, 0); } else {
    //    Vec2 new_dir_2;
    //    new_dir_2.x = dir.y*-1;
    //    new_dir_2.y = dir.x*-1;
    //    tunneler_v3(map, bounds, new_dir_2, moving_coord, tunnel_size-1,
    //    max_walk/2, speed, new_turn_chance, 0);
    //  }
    //  turn_chance -= 100;
    //
    //
    // } else {
    //   // turn_chance += 50;
    // }

    // check next steps
    Vec2 next_coord =
        (Vec2){moving_coord.x + dir.x * speed, moving_coord.y + dir.y * speed};
    // Vec2 next_2_coord = (Vec2){moving_coord.x + 3*dir.x, moving_coord.y +
    // 3*dir.y}; if(is_inside_map(*map, next_coord) && is_inside_map(*map,
    // next_2_coord) && map->cells[coord_to_idx(next_2_coord)].terrain_id !=
    // FLOOR_ID){
    if (!is_inside_rec(bounds, next_coord)) {
      // instead of stoping try changing the direction
      dir = get_new_direction_not_going_back_nor_going_to_bound(
          bounds, moving_coord, speed, dir);
      next_coord = (Vec2){moving_coord.x + dir.x * speed,
                          moving_coord.y + dir.y * speed};
      if (!is_inside_rec(bounds, next_coord)) {
        dir = get_new_direction_not_going_back_nor_going_to_bound(
            bounds, moving_coord, speed, dir);
      }
    }

    // if(!is_rectangle_available_to_dig(map, next_coord.x, next_coord.y,
    // tunnel_size/2, tunnel_size/2)){
    //   break;
    // }
    // if(!is_rectangle_available_to_dig(map, next_coord.x, next_coord.y, 2,
    // 2)){
    //   break;
    // }
    tiles_walked++;
  } while (is_inside_rec(bounds, moving_coord) && tiles_walked < 1000);
}

void tunneler_v2(Map *map, Vec2 start_dir, Vec2 origin_coord,
                 int tunnel_max_thick, int horizontal_set_walk,
                 int vertical_set_walk, int max_iter) {
  // NOTES:
  // Kyz wiz: parameters:
  // Width, direction, speed, chance to turn, chance to create rooms,
  // size and shape of rooms to create,
  // how much space to leave between self and other dungeon objects,
  // when to quit
  int x, y;
  if (origin_coord.x <= -1) {
    x = map->width / 2;
    y = map->height * 19 / 20;
  } else {
    x = origin_coord.x;
    y = origin_coord.y;
  }

  Vec2 dir;
  Vec2 down = {0, 1};
  Vec2 up = {0, -1};
  Vec2 right = {1, 0};
  Vec2 left = {-1, 0};
  if (start_dir.x <= -1) {
    dir = up;
  } else {
    dir = start_dir;
  }

  int tunnel_min_size = 1;
  int tunnel_max_size = 3;
  int tunnel_size;

  // deliberately set to -1 means randomize
  if (tunnel_max_thick <= -1) {
    tunnel_size = (rand() % tunnel_max_size) + tunnel_min_size;
  } else if (tunnel_max_thick == 0) {
    // setting as 0 (should) means stop, branching path that got too thin
    return;
  } else {
    tunnel_size = rand() % (tunnel_max_thick);
  }

  int max_walk_horizontal;
  if (horizontal_set_walk <= -1) {
    max_walk_horizontal = rand() % (COLS / 20) + 10;
  } else {
    max_walk_horizontal = horizontal_set_walk;
  }

  int max_walk_vertical;
  if (vertical_set_walk <= -1) {
    max_walk_vertical = (rand() % (ROWS / 20) + 10);
  } else {
    max_walk_vertical = vertical_set_walk;
  }

  // have a chance to spawn more tunnelers either at the mid of the path
  Vec2 moving_coord = {x, y};
  int tiles_walked = 0;
  int iter = 0;
  do {

    if (max_walk_vertical == 0 || max_walk_horizontal == 0)
      return;
    // walking vertically
    if (dir.y != 0 && tiles_walked > max_walk_vertical) {
      // make the chance to turn right or left dependant on the current coord
      // relative to the map?
      if (moving_coord.x > COLS / 4) {
        dir = rand() % 2 == 0 ? left : right;
      } else {
        dir = right;
      }
      tiles_walked = 0;
      // chance to spawn another tunneler
      if (moving_coord.x > COLS / 4 && rand() % 2 == 0) {
        tunneler_v2(map, left, moving_coord, tunnel_size - 1,
                    max_walk_horizontal / 3, max_walk_vertical / 3,
                    max_iter / 2);
      } else {
        tunneler_v2(map, dir, moving_coord, tunnel_size - 1,
                    max_walk_horizontal / 2, max_walk_vertical / 2,
                    max_iter / 2);
      }
      tunnel_size = rand() % (tunnel_max_size) + tunnel_min_size;
      max_walk_horizontal = (rand() % (COLS * 3 / 8) + (COLS / 8));
      // if walking horizontally and y is below 1/2 of the map
    } else if (dir.x != 0 && tiles_walked > max_walk_horizontal &&
               moving_coord.y >= ROWS / 2) {
      if (moving_coord.y <= ROWS / 2) {
        dir = down;
      } else {
        dir = up;
      }
      tiles_walked = 0;
      // spawn on opposite direction (split)
      if (moving_coord.y > ROWS / 2 && rand() % 2 == 0) {
        tunneler_v2(map, dir, moving_coord, tunnel_size - 1,
                    max_walk_horizontal / 3, max_walk_vertical / 3,
                    max_iter / 2);
      }
      tunnel_size = rand() % (tunnel_max_size) + tunnel_min_size;
      max_walk_vertical = (rand() % (ROWS / 2) + (ROWS / 3));
      // if walking horizontally and y is below 1/2 of the map
    }
    moving_coord = dig_tunnel_tile_step(map, tunnel_size, dir, moving_coord);

    // 5% chance to spawn a room (every tile moved)
    if (rand() % 1000 < -1) {
      // if(rand()%1000 < 50){
      int room_chance = rand() % 100;
      Vec2 room_dir = dir.x == 0 ? (Vec2){1, 0} : (Vec2){0, 1};
      int room_size = (rand() % 6) + 3;
      // randomly choose up/down/left/right
      if (room_chance % 2 == 0) {
        room_dir.x *= -1;
        room_dir.y *= -1;
      }
      int shape = choose_random_shape();
      // check_if_room_fits(*map, room_size, moving_coord, room_dir, shape);
      // dig_room_v2(map, room_size, moving_coord, room_dir, shape);

      int shape_chance = rand() % 100;
      int shape_id = RECT_ID;
      if (shape_chance <= 90) {
        // } else if(shape_chance > 60){
      } else {
        shape_id = CIRCLE_ID;
      }
      dig_room_with_door(map, room_size, moving_coord, room_dir, shape_id);
    }
    // take a step
    Vec2 next_coord = (Vec2){moving_coord.x + dir.x, moving_coord.y + dir.y};
    Vec2 next_2_coord =
        (Vec2){moving_coord.x + 3 * dir.x, moving_coord.y + 3 * dir.y};
    // if(is_inside_map(*map, next_coord) && is_inside_map(*map, next_2_coord)
    // && map->cells[coord_to_idx(next_2_coord)].terrain_id != FLOOR_ID){
    if (is_inside_map(*map, next_coord)) {
      moving_coord.x += dir.x;
      moving_coord.y += dir.y;
    } else {
      break;
    }
    tiles_walked++;
    iter++;
  } while (is_inside_map(*map, moving_coord) && iter < max_iter);
}

void init_completely_walled_map(Map *map, uint width, uint height) {
  map->width = width;
  map->height = height;
  Tile *cells = (Tile *)malloc(sizeof(Tile) * width * height);
  for (uint j = 0; j < height; j++) {
    for (uint i = 0; i < width; i++) {
      cells[j * width + i] = (Tile){0};
      cells[j * width + i].terrain_id = STONE_ID;
      cells[j * width + i].checked = false;
    }
  }
  map->cells = cells;
  return;
}

void init_map(Map *map, uint width, uint height) {
  map->width = width;
  map->height = height;
  Tile *cells = (Tile *)malloc(sizeof(Tile) * width * height);
  for (uint j = 0; j < height; j++) {
    for (uint i = 0; i < width; i++) {
      cells[j * width + i] = (Tile){0};
      cells[j * width + i].terrain_id = FLOOR_ID;
      cells[j * width + i].entity_idx = EMPTY_ENTITY_IDX;
    }
  }
  map->cells = cells;
  return;
}

bool move(Map *map, Vec2 *entity_coord, int dir_x, int dir_y) {
  Vec2 new_coord =
      (Vec2){.x = entity_coord->x + dir_x, .y = entity_coord->y + dir_y};
  if (is_inside_map(*map, new_coord) && is_valid_to_move(*map, new_coord)) {
    map->cells[coord_to_idx(new_coord)].entity_idx =
        map->cells[coord_to_idx(*entity_coord)].entity_idx;
    map->cells[coord_to_idx(*entity_coord)].entity_idx = 0;
    entity_coord->x = new_coord.x;
    entity_coord->y = new_coord.y;
    return true;
  }
  return false;
}

void swap_in_sequence(Map *map, Vec2 origin_coord, Vec2 *sequence, int size) {
  Tile t1 = {0};
  Tile t2 = {0};
  bool is_first_pass = true;
  for (int i = 0; i <= size; i++) {
    Vec2 current_coord = {origin_coord.x + sequence[i % size].x,
                          origin_coord.y + sequence[i % size].y};
    if (is_first_pass) {
      // first tile, just save it
      t1.entity_idx = map->cells[coord_to_idx(current_coord)].entity_idx;
      t1.terrain_id = map->cells[coord_to_idx(current_coord)].terrain_id;
      is_first_pass = false;
    } else {
      // save the current tile into t2
      t2.entity_idx = map->cells[coord_to_idx(current_coord)].entity_idx;
      t2.terrain_id = map->cells[coord_to_idx(current_coord)].terrain_id;
      // replace currect tile to the previous one
      map->cells[coord_to_idx(current_coord)].entity_idx = t1.entity_idx;
      map->cells[coord_to_idx(current_coord)].terrain_id = t1.terrain_id;
      // save the "old" current tile  into t1
      t1.entity_idx = t2.entity_idx;
      t1.terrain_id = t2.terrain_id;
    }
  }
}
bool rotate_terrain(Map *map, Vec2 origin_coord, Vec2 rotation_dir, Entity_arr* entities) {
  // for now size is fixed 2x2, 3x3 is interesting too
  int size = 3;
  int x_dir = origin_coord.x < rotation_dir.x ? 1 : -1;
  // int y_dir = origin_coord.y < rotation_dir.y ? -1 : 1;

  bool is_clockwise = x_dir == 1;

  if (origin_coord.x > (COLS - size) || origin_coord.y > (ROWS - size)) {
    return false;
  }
  if (size == 2) {
    static const int seq_size = 4;
    if (is_clockwise) {
      Vec2 sequence[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
      swap_in_sequence(map, origin_coord, sequence, seq_size);
    } else {
      Vec2 sequence[4] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
      swap_in_sequence(map, origin_coord, sequence, seq_size);
    }
  } else if (size == 3) {
    const int seq_size = 8;
    if (is_clockwise) {
      Vec2 sequence[8] = {{0, 0}, {1, 0}, {2, 0}, {2, 1},
                          {2, 2}, {1, 2}, {0, 2}, {0, 1}};
      swap_in_sequence(map, origin_coord, sequence, seq_size);
      Vec2 middle_coord = (Vec2){.x = origin_coord.x + 1, .y = origin_coord.y + 1}; 
      if(is_inside_map(*map, middle_coord)){
        int en_idx = map->cells[coord_to_idx(middle_coord)].entity_idx;
        entities->items[en_idx].dir = (entities->items[en_idx].dir + 1)%4;
      }
    } else {
      Vec2 sequence[8] = {{0, 0}, {0, 1}, {0, 2}, {1, 2},
                          {2, 2}, {2, 1}, {2, 0}, {1, 0}};
      swap_in_sequence(map, origin_coord, sequence, seq_size);
      Vec2 middle_coord = (Vec2){.x = origin_coord.x + 1, .y = origin_coord.y + 1}; 
      if(is_inside_map(*map, middle_coord)){
        int en_idx = map->cells[coord_to_idx(origin_coord)].entity_idx;
        if (en_idx != EMPTY_ENTITY_IDX){
          entities->items[en_idx].dir = (entities->items[en_idx].dir - 1)%4;
        }
      }
    }
  }
  return true;
}

bool move_terrain(Map *map, Vec2 origin_coord, Vec2 target_coord) {
  printf("moving terrain: from(%d,%d) to(%d,%d)\n", origin_coord.x,
         origin_coord.y, target_coord.x, target_coord.y);
  // does it make sense to move an empty terrain? what would it do?

  int t_id = map->cells[coord_to_idx(origin_coord)].terrain_id;
  int en_id = map->cells[coord_to_idx(origin_coord)].entity_idx;
  if(t_id == FLOOR_ID){
    printf("no behavior for moving an empty floor at (%d, %d)", origin_coord.x, origin_coord.y);
    return false;
  }
  Vec2_Arr tiles = {0};
  tiles.cap = 50;
  tiles.items = (Vec2*)malloc(sizeof(Vec2)*tiles.cap);
  rg_bresenham_line(origin_coord, target_coord, &tiles);
  // start with i=1 to skip first tile
  if(tiles.size == 1){
    printf("unable to move terrain to same tile");
    return false;
  }
  for(int i = 1; i < tiles.size; i++){
      Vec2 curr_coord = tiles.items[i];
      int curr_t_id = map->cells[coord_to_idx(curr_coord)].terrain_id;
      int curr_en_id = map->cells[coord_to_idx(curr_coord)].entity_idx;
      if(curr_t_id == FLOOR_ID && curr_en_id == EMPTY_ENTITY_IDX && i != tiles.size - 1){
        // if the terrain is clear of other terrain or entities, and we are not at the end of the path
        continue;
      } 
      Vec2 prev_coord = tiles.items[i-1]; 
      map->cells[coord_to_idx(prev_coord)].terrain_id = t_id;
      map->cells[coord_to_idx(prev_coord)].entity_idx = en_id;
      if(are_coords_equal(prev_coord, origin_coord)){
        break; 
      }
      map->cells[coord_to_idx(origin_coord)].terrain_id = FLOOR_ID;
      map->cells[coord_to_idx(origin_coord)].entity_idx = EMPTY_ENTITY_IDX;
      return true;
  }
  return false;
  // if (is_inside_map(*map, target_coord) &&
  //   is_valid_to_move(*map, target_coord)) {
  //   // what about entities? should they be there?
  //
  //   map->cells[coord_to_idx(target_coord)].terrain_id = map->cells[coord_to_idx(origin_coord)].terrain_id;
  //   // it's not swaping, it's removing what was previously there
  //   map->cells[coord_to_idx(origin_coord)].terrain_id = 0;
  //   origin_coord.x = target_coord.x;
  //   origin_coord.y = target_coord.y;
  //   return true;
  // }
  // return false;
}

bool is_mouse_inside_grid(Vector2 mouse_coord) {
  return mouse_coord.x > X_OFFSET && mouse_coord.x < GRID_WIDTH + X_OFFSET &&
         mouse_coord.y > Y_OFFSET && mouse_coord.y < GRID_HEIGHT + Y_OFFSET;
}

Vec2 get_tile_coord_under_pixel_coord(Vector2 pixel_coord) {
  float cell_width = (float)GRID_WIDTH / (float)COLS;
  float cell_height = (float)GRID_HEIGHT / (float)ROWS;
  float x_on_grid = (int)((pixel_coord.x - X_OFFSET) / cell_width);
  float y_on_grid = (int)((pixel_coord.y - Y_OFFSET) / cell_height);
  return (Vec2){.x = x_on_grid, .y = y_on_grid};
}

void tile_clicked(Map map, Vec2_Arr *targets) {
  if (targets->size == 2) {
    // reset
    targets->size = 0;
  }

  Vector2 mouse_coord = GetMousePosition();
  if (!is_mouse_inside_grid(mouse_coord)) {
    return;
  }
  Vec2 coord = get_tile_coord_under_pixel_coord(mouse_coord);
  targets->items[targets->size].x = coord.x;
  targets->items[targets->size].y = coord.y;
  targets->size++;
  printf("selecting: (%d, %d)\n", coord.x, coord.y);
}

int handle_input(int key, Map *map, Vec2 *player_coord, Vec2_Arr *targets, Entity_arr* entities) {
  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    tile_clicked(*map, targets);
  if (IsKeyPressed(KEY_UP))
    move(map, player_coord, 0, -1);
  if (IsKeyPressed(KEY_DOWN))
    move(map, player_coord, 0, 1);
  if (IsKeyPressed(KEY_LEFT))
    move(map, player_coord, -1, 0);
  if (IsKeyPressed(KEY_RIGHT))
    move(map, player_coord, 1, 0);
  if (IsKeyPressed(KEY_ONE) && targets->size == 2)
    move_terrain(map, targets->items[0], targets->items[1]);
  if (IsKeyPressed(KEY_TWO) && targets->size == 2)
    rotate_terrain(map, targets->items[0], targets->items[1], entities);
  if (IsKeyPressed(KEY_N)) {
    init_map(map, map->width, map->height);
    map_generator_random(map);
    // t = time(NULL);
    // srand(t);
    // init_completely_walled_map(map, COLS, ROWS);
    // tunneler_v3(map, (Rec){-100, -100, -100, -100}, (Vec2){-100, -100},
    //             (Vec2){-100, -100}, -100, -100, -100, -100, -100);
  }
  return 1;
}

bool is_mouse_inside_tile(Vector2 mouse_pos, Vec2 tile_coord, float cell_width,
                          float cell_height) {
  return mouse_pos.x > tile_coord.x + X_OFFSET &&
         mouse_pos.x < tile_coord.x + X_OFFSET + cell_width &&
         mouse_pos.y > tile_coord.y + Y_OFFSET &&
         mouse_pos.y < tile_coord.y + cell_height + Y_OFFSET;
}

Texture2D img_file_to_texture(char* filename, float cell_width, float cell_height) {
  Image img = LoadImage(filename);
  ImageResizeNN(&img, cell_width, cell_height);
  Texture2D txt = LoadTextureFromImage(img);
  UnloadImage(img);
  return txt;
}

int main() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "dwarfy, the protector");
  // init game
  float cell_width = (float)GRID_WIDTH / (float)COLS;
  float cell_height = (float)GRID_HEIGHT / (float)ROWS;
  Map map;
  Entity_arr entities = {0};
  int num_of_monsters = 4;
  int num_of_neutrals = 6;
  // first place should be empty?
  // very weird thing, prob switch it -> make a constant that symbolizes the "undefined" id
  entities.cap = 2+num_of_monsters+num_of_neutrals;
  entities.items = (Entity *)malloc(sizeof(Entity) * entities.cap);

  entities.items[0] = (Entity){.character = '@', .HP = 5, .teamID = PLAYER_TEAM, .type = PLAYER_TYPE };
  Vec2 player_coord = (Vec2){.x = 0, .y = 0};

  entities.items[1] = (Entity){.character = 'c', .HP = 5, .teamID = ENEMY_TEAM, .type = CYCLOPS_ENEMY_TYPE, .dir = North};
  entities.items[2] = (Entity){.character = 'b', .HP = 3, .teamID = ENEMY_TEAM, .type = BOMBY_ENEMY_TYPE, .dir = North};
  entities.items[3] = (Entity){.character = 'd', .HP = 10, .teamID = ENEMY_TEAM, .type = DRAGON_ENEMY_TYPE, .dir = East };
  entities.items[4] = (Entity){.character = 'c', .HP = 5, .teamID = ENEMY_TEAM, .type = CYCLOPS_ENEMY_TYPE, .dir = South };
  entities.size = 5;

  t = time(NULL);
  srand(t);
  init_map(&map, COLS, ROWS);
  map_generator_random(&map);
  // add "player"
  map.cells[0] = (Tile){.entity_idx=0, .item_idx=0, .terrain_id=0};
  // add monster
  // (10, 8), (10, 3), (3, 5), (17, 5)
  map.cells[8*COLS + 10].entity_idx = 1;
  map.cells[3*COLS + 10].entity_idx = 2;
  map.cells[5*COLS + 3].entity_idx = 3;
  map.cells[5*COLS + 17].entity_idx = 4;

  float font_size = 40;
  Texture2D floor_tex = img_file_to_texture("floor.png", cell_width, cell_height);
  Texture2D pillar_tex = img_file_to_texture("pillar.png", cell_width, cell_height);
  Texture2D stone_tex = img_file_to_texture("stone.png", cell_width, cell_height);
  Texture2D door_tex = img_file_to_texture("door.png", cell_width, cell_height);
  Texture2D player_tex = img_file_to_texture("baldy-dwarf.png", cell_width, cell_height);
  Texture2D cyclop_tex = img_file_to_texture("eye.png", cell_width, cell_height);
  Texture2D bomb_tex = img_file_to_texture("bomb.png", cell_width, cell_height);
  Texture2D dragon_tex = img_file_to_texture("dragon.png", cell_width, cell_height);


  SetTargetFPS(30);
  Vec2_Arr targets = {0};
  targets.cap = 2;
  targets.items = (Vec2 *)malloc(sizeof(Vec2) * 2);
  while (!WindowShouldClose()) {
    int key = GetKeyPressed();
    handle_input(key, &map, &player_coord, &targets, &entities);
    Vector2 mouse_pos = GetMousePosition();

    BeginDrawing();
    ClearBackground(BLACK);
    int text_x = X_OFFSET + COLS * cell_width + 30;
    int text_y = Y_OFFSET;
    Vec2_Arr enemy_coords = {0};
    enemy_coords.items = (Vec2*)malloc(sizeof(Vec2)*num_of_monsters);
    enemy_coords.cap = num_of_monsters;
    for (uint y = 0; y < ROWS; y++) {
      for (uint x = 0; x < COLS; x++) {
        Tile tile = map.cells[y * COLS + x];
        char c;
        Color color = WHITE;
        Texture2D t_entity;
        Texture2D t_terrain;
        bool has_entity = false;
        if (tile.entity_idx != EMPTY_ENTITY_IDX && tile.entity_idx < entities.size) {
          has_entity = true;
          Entity en = entities.items[tile.entity_idx];
          c = en.character;

          if (c == '@') {
            t_entity = player_tex;
          } else {
            enemy_coords.items[enemy_coords.size] = (Vec2){x, y};
            enemy_coords.size++;
            if (c == 'c') {
              t_entity = cyclop_tex;
            } else if(  c == 'b' ) {
              t_entity = bomb_tex;
            } else if(c == 'd'){
              t_entity = dragon_tex;
            }
          }
        } 
        if (tile.terrain_id == FLOOR_ID) {
          c = '.';
          t_terrain = floor_tex;
        } else if (tile.terrain_id == WALL_ID) {
          c = '#';
          t_terrain = pillar_tex;
        } else if (tile.terrain_id == STONE_ID) {
          c = '0';
          t_terrain = stone_tex;
        } else if (tile.terrain_id == DOOR_ID) {
          c = '+';
          t_terrain = door_tex;
        }
        Vec2 tile_coord = (Vec2){.x = cell_width * x, .y = cell_height * y};
        if (is_mouse_inside_tile(mouse_pos, tile_coord, cell_width,
                                 cell_height)) {
          color = YELLOW;
          DrawText(TextFormat("Mouse xy: %d, %d", x, y), text_x, SCREEN_HEIGHT - 200, font_size, RAYWHITE);
        }
        DrawTexture(t_terrain, tile_coord.x + X_OFFSET, tile_coord.y + Y_OFFSET, color);
        if (has_entity) 
          DrawTexture(t_entity, tile_coord.x + X_OFFSET, tile_coord.y + Y_OFFSET, color);
      }
    }
    for(int i = 0; i < enemy_coords.size; i++){
      int x = enemy_coords.items[i].x;
      int y = enemy_coords.items[i].y;
      int entity_idx = map.cells[y*COLS + x].entity_idx;
      Entity entity = entities.items[entity_idx];
      Vec2_Arr affected_tiles = {0};
      get_atk_coords_from_entity(map, entity, x, y, &affected_tiles);
      for(int j = 0; j < affected_tiles.size; j++){
        Vec2 coord = affected_tiles.items[j];
        if(!is_inside_map(map, coord)){
          continue;
        }
        Tile tile = map.cells[coord.y * COLS + coord.x];
        Vec2 tile_coord = (Vec2){.x = cell_width * coord.x, .y = cell_height * coord.y};
        Texture2D t_terrain;
        Texture2D t_entity;
        bool has_entity = false;

        if (tile.entity_idx != EMPTY_ENTITY_IDX && tile.entity_idx < entities.size) {
          has_entity = true;
          Entity en = entities.items[tile.entity_idx];
          char c = en.character;

          if (c == '@') {
            t_entity = player_tex;
          } else {
            if (c == 'c') {
              t_entity = cyclop_tex;
            } else if(  c == 'b' ) {
              t_entity = bomb_tex;
            } else if(c == 'd'){
              t_entity = dragon_tex;
            }
          }
        } 
        if (tile.terrain_id == FLOOR_ID) {
          t_terrain = floor_tex;
        } else if (tile.terrain_id == WALL_ID) {
          t_terrain = pillar_tex;
        } else if (tile.terrain_id == STONE_ID) {
          t_terrain = stone_tex;
        } else if (tile.terrain_id == DOOR_ID) {
          t_terrain = door_tex;
        }
        DrawTexture(t_terrain, tile_coord.x + X_OFFSET, tile_coord.y + Y_OFFSET, RED);
        if(has_entity){
          DrawTexture(t_entity, tile_coord.x + X_OFFSET, tile_coord.y + Y_OFFSET, RED);
        }
      }
      free(affected_tiles.items);
    }
    // spells
    DrawText(TextFormat("Spells Available", t), text_x, text_y, font_size, WHITE);
    text_y += font_size + 2;
    DrawText(TextFormat("1 - Move Earth", t), text_x, text_y , font_size, RAYWHITE);
    text_y += font_size + 2;
    DrawText(TextFormat("2 - Rotate Earth", t), text_x, text_y, font_size, RAYWHITE);
    text_y += font_size + 2;
    free(enemy_coords.items);

    // DrawFPS(5, 5);
    EndDrawing();
  }
  UnloadTexture(floor_tex);
  UnloadTexture(player_tex);
  UnloadTexture(player_tex);
  UnloadTexture(cyclop_tex);
  UnloadTexture(bomb_tex);
  UnloadTexture(dragon_tex);
  UnloadTexture(pillar_tex);
  UnloadTexture(stone_tex);
  UnloadTexture(door_tex);
}
