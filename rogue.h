#ifndef ROGUE_H_
#define ROGUE_H_

typedef struct Vec2 
{
  int x;
  int y; 
} Vec2;

typedef struct Rec
{
  Vec2 coord;
  int width;
  int height;
}Rec;

typedef struct Vec2_Rec 
{
  unsigned int cap;
  unsigned int size;
  Rec* items;
} Vec2_Rec;

typedef struct Vec2_Arr 
{
  unsigned int cap;
  unsigned int size;
  Vec2* items;
} Vec2_Arr;

extern int rg_abs(int);
extern int rg_is_Vec2_equal(Vec2 v0, Vec2 v1);
extern int rg_bresenham_line(Vec2 pFrom, Vec2 pTo, Vec2_Arr* array);
extern void rg_reverse_arr(Vec2_Arr* arr);

// #ifdef ROGUE_H_IMPLEMENTATION

int rg_abs(int a) 
{
  if (a < 0) {
    return -1*a;
  }
  return a;
}

int rg_is_Vec2_equal(Vec2 v0, Vec2 v1) 
{
  if(v0.x == v1.x && v0.y == v1.y)
    return 1;
  return 0;
}

void rg_reverse_arr(Vec2_Arr* arr)
{
  for(int i = 0, j = arr->size - 1; i < j; i++, j--){
    Vec2 temp;
    temp.x = arr->items[j].x;
    temp.y = arr->items[j].y;
    arr->items[j].x = arr->items[i].x;
    arr->items[j].y = arr->items[i].y;
    arr->items[i].x = temp.x;
    arr->items[i].y = temp.y;
  }
}

int rg_bresenham_line(Vec2 pFrom, Vec2 pTo, Vec2_Arr* array)
{
  // the return shows if it succeded or not
  // resource that helped most of the code here https://www.youtube.com/watch?v=CceepU1vIKo

  array->size = 0;
  int is_reversed = 0;
  if(rg_is_Vec2_equal(pFrom, pTo)){
    return 0;
  }
  if(rg_abs(pTo.y - pFrom.y) < rg_abs(pTo.x - pFrom.x)){
    // x moves more than y, draw horizontal
    if (pFrom.x > pTo.x ){
      // if going left (negative), mirror
      int temp_x = pFrom.x; int temp_y = pFrom.y;
      pFrom.x = pTo.x; pFrom.y = pTo.y;
      pTo.x = temp_x; pTo.y = temp_y;
      is_reversed = 1;
    }
    int dx = pTo.x - pFrom.x;
    int dy = pTo.y - pFrom.y;
    int dir = 1;
    if (dy < 0) 
      // if going down, also invert
      dir = -1;
    dy *= dir;
    if (dx == 0){
      // cannot proceed cause pFrom and pTo is on the same tile
      return 0;
    }
    int y = pFrom.y;
    int e = 2*dy - dx;
    for(int x = pFrom.x; x <= pTo.x; x++){
      if(array->size >= array->cap){
        // will overflow the array
        return 0;
      }
      array->items[array->size].x = x;
      array->items[array->size].y = y;
      array->size += 1;
      if (e >= 0){
        y += dir;
        e -= 2*dx;
      }
      e += 2*dy;
    }
  } else {
    // y moves more than x, draw vertical
    if (pFrom.y > pTo.y){
      int temp_x = pFrom.x;
      int temp_y = pFrom.y;
      pFrom.x = pTo.x; pFrom.y = pTo.y;
      pTo.x = temp_x; pTo.y = temp_y;
      is_reversed = 1;
    }
    int dx = pTo.x - pFrom.x;
    int dy = pTo.y - pFrom.y;
    int dir = 1;
    if (dx < 0) 
      dir = -1;
    dx *= dir;
    if (dy == 0){
      // cannot proceed cause pFrom and pTo is on the same tile
      return 0;
    }
    int x = pFrom.x;
    int e = 2*dx - dy;
    for(int y = pFrom.y; y <= pTo.y; y++){
      if(array->size >= array->cap){
        // will overflow the array
        return 0;
      }
      array->items[array->size].x = x;
      array->items[array->size].y = y;
      array->size += 1;
      if (e >= 0){
        x += dir;
        e -= 2*dy;
      }
      e += 2*dx;
    }
  }
  if(is_reversed){
    rg_reverse_arr(array);
  }
  // success
  return 1;
}


// #endif //ROGUE_H_IMPLEMENTATION
#endif // ROGUE_H_
