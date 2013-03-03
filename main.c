//______________________________________________________________
//
//                  Program  : TicTacToe v 0.1     
//                  Author   : ZeldaPragma    
//                  Date     : 17-02-2013          
//                  Language : C                   
//______________________________________________________________
//
#include <keyboard_syscalls.h>   // Prizm specific headers
#include <keyboard.hpp> 
#include <display_syscalls.h>
#include <color.h>
#include <display.h>
#include <system.h>
#include <rtc.h>
#include <stdlib.h>              // C headers
#include <stdio.h>
#include <CONVERT_syscalls.h>
#include "data.h"

#define LCD_WIDTH_PX 384         // This is the size of the screen in pixels
#define LCD_HEIGHT_PX 216
#define MASK_PINK (color_t)0xF81F
#define false 0
#define true 1

int Rand(int lower, int upper);
int PRGM_GetKey(void);
void CopySpriteMasked(const char* data, int x, int y, int width, int height, int maskcolor);
void rectangle(int x, int y, int width, int height, unsigned short color);
void keymenu(void);
void keyupdate(void);
void full_screen_color(int COLOR);
int keydownlast(int basic_keycode);
int keydownhold(int basic_keycode);

_Bool Win(void);          // This are the game priorities 
_Bool AvoidLosing(void);
_Bool Fork(void);
_Bool AvoidFork(void);
_Bool CenterPlay(void);
_Bool OppositeCorner(void);
_Bool EmptyCorner(void);
_Bool EmptySide(void);

void Priority_Algorithm(void);  // This is the algorithm which orders the priorities,
void BackGround(void);          // choosing the best for each case
void Circles_or_Crosses(void);
void InicialData(void);
void InputWait(void);
void DrawSelect(void);
_Bool TiedGame(void);
_Bool GameEnd(void);

enum {play_screen, title_screen} program_mode = title_screen;
enum {player_vs_player, ai_vs_player, ai_vs_ai} mode = ai_vs_player;
enum {player, ai} first_turn = ai;

int turn, selected_x, selected_y, last_sel_x, last_sel_y, frame_count, turn_coord_y[12], turn_coord_x[12], Coord[5][5], x, y, a, b, win_look_for;
int AL_look_for, games_tied, circles_won, crosses_won, key;
_Bool circles_turn, F_continue, AF_continue, running, goto_jump;
char score[20];

static unsigned short holdkey[12];
static unsigned short lastkey[12];
static const unsigned short* keyboard_register = (unsigned short*)0xA44B0000;

int main() {
  Bdisp_AllClr_VRAM();     // We always have to do this to start off a program
  running = true;
  score[0] = ' ';
  frame_count = 1;  
  while(running) {         // Main game loop 
    keyupdate();
    switch (program_mode) {
    case title_screen:  // IN case we are in the main menu(title screen)
      BackGround();
      if (keydownlast(KEY_PRGM_F1)) {
        mode = player_vs_player;
        first_turn = ai;
        InicialData();
        program_mode = play_screen; }
      else if (keydownlast(KEY_PRGM_F2)) {
        mode = ai_vs_player; 
        first_turn = ai;
        InicialData();
        program_mode = play_screen; }
      else if (keydownlast(KEY_PRGM_F3)) {
        mode = ai_vs_ai;
        first_turn = ai;
        InicialData();
        program_mode = play_screen; }
      frame_count+=2;
      if (frame_count > 20) {
        frame_count = 1; }
      break;

    case play_screen:   // In case we are actually playing
      switch (mode) {
      case player_vs_player:
        switch(circles_turn) {
        case true:
          turn++;
          InputWait();
          Coord[selected_x][selected_y] = 1;
          CopySpriteMasked(Circle, ((68 * selected_x) - 68 ) + 93, (68 * selected_y) - 68, 64, 64, MASK_PINK);
          circles_turn = false;
          TiedGame();
          if (GameEnd() == true && TiedGame() == true) {
            games_tied++;
            InicialData(); } 
          break;
        case false:
          turn++;
          InputWait();
          Coord[selected_x][selected_y] = 2;
          CopySpriteMasked(Cross, ((68 * selected_x) - 68 ) + 93, (68 * selected_y) - 68, 64, 64, MASK_PINK);
          circles_turn = true;
          TiedGame();
          if (GameEnd() == true && TiedGame() == true) {
            games_tied++;
            InicialData(); }
          break; }
        break;
      case ai_vs_player:
        switch(circles_turn) {
        case true:
          win_look_for = 1;
          AL_look_for = 2;      // 2 means crosses
          turn++;
          Priority_Algorithm();
          if (goto_jump == true) {
            goto_jump = false; 
            goto jump; }
          Coord[a][b] = 1;
          CopySpriteMasked(Circle, ((68 * a) - 68 ) + 93, (68 * b) - 68, 64, 64, MASK_PINK);
          circles_turn = false;
          if (GameEnd() == true && TiedGame() == true) {  // If tied game is false, the codes goes to InicialData
            games_tied++;
            InicialData(); } 
          break;
        case false:
          turn++;
          InputWait();           // Wait for user input
          Coord[selected_x][selected_y] = 2;
          CopySpriteMasked(Cross, ((68 * selected_x) - 68 ) + 93, (68 * selected_y) - 68, 64, 64, MASK_PINK);
          circles_turn = true;
          TiedGame();
          if (GameEnd() == true && TiedGame() == true) { 
            games_tied++;
            InicialData(); } 
          break; }
        break;
      case ai_vs_ai:
        switch(circles_turn) {
        case true:
          win_look_for = 1;
          AL_look_for = 2;      // 2 means crosses
          turn++;
          Priority_Algorithm();
          Coord[a][b] = 1;
          CopySpriteMasked(Circle, ((68 * a) - 68 ) + 93, (68 * b) - 68, 64, 64, MASK_PINK); // Draw the cross
          circles_turn = false;
          if (turn == 10 && TiedGame() == true) {  // If tied game is false, the codes goes to InicialData
            games_tied++;
            InicialData(); } 
          break;
        case false:
          win_look_for = 2;
          AL_look_for = 1;     // 1 means circles
          turn++;
          Priority_Algorithm();
          Coord[a][b] = 2;
          CopySpriteMasked(Cross, ((68 * a) - 68 ) + 93, (68 * b) - 68, 64, 64, MASK_PINK); // Draw the cross
          circles_turn = true;
          if (turn == 10 && TiedGame() == true) {  // If tied game is false, the codes goes to InicialData
            games_tied++;
            InicialData(); }
          break; }  // switch circles or crosses
        break; }    // switch mode
      break; }      // switch program mode
  jump:
    if (keydownlast(KEY_PRGM_MENU)) {
      keymenu(); }
    if (keydownlast(KEY_PRGM_EXIT)) {
      games_tied = 0;
      circles_won = 0;
      crosses_won = 0;
      program_mode = title_screen; }
    Bdisp_PutDisp_DD();
  }
  return 0;
}

int Rand(int lower, int upper) {
  static int a = 123456789;
  static int b = 362436069;
  static int c = 521288629;
  static int d = 88675123;
  int t;
  t = a ^ (a << 11);
  a = b;
  b = c;
  c = d;
  return (d = d ^ (d >> 19) ^ (t ^ (t >> 8)))%upper + lower;
}

int PRGM_GetKey(void) {
  unsigned char buffer[12];
  PRGM_GetKey_OS( buffer );
  return ( buffer[1] & 0x0F ) * 10 + ( ( buffer[2] & 0xF0 ) >> 4 );
}

void CopySpriteMasked(const char* data, int x, int y, int width, int height, int maskcolor) {
  char* VRAM = (char*)0xA8000000;
  VRAM += 2*(LCD_WIDTH_PX*y + x);
  for(int j=y; j<y+height; j++) {
    for(int i=x; i<x+width;  i++) {
      if ((((((int)(*data))&0x000000FF)<<8) | ((((int)(*(data+1))))&0x000000FF)) != maskcolor) {
        *(VRAM++) = *(data++);
        *(VRAM++) = *(data++);
      } else { VRAM += 2; data += 2; }
    }
    VRAM += 2*(LCD_WIDTH_PX-width);
  }
}

void keymenu(void) {
  int key = KEY_PRGM_MENU;
  GetKey(&key);
  Bdisp_EnableColor(1);
  DrawFrame(COLOR_BLACK);
}

void rectangle(int x, int y, int width, int height, unsigned short color) {
  unsigned short*VRAM = (unsigned short*)0xA8000000;
  for(int j = y; j < y+height; j++) {
    for(int i = x; i < x+width; i++) {
      *(j*LCD_WIDTH_PX+i+VRAM) = color; }
  }
}

void keyupdate(void) {
  memcpy(holdkey, lastkey, sizeof(unsigned short)*8);
  memcpy(lastkey, keyboard_register, sizeof(unsigned short)*8);
}

int keydownlast(int basic_keycode) {
  int row, col, word, bit; 
  row = basic_keycode%10; 
  col = basic_keycode/10-1; 
  word = row>>1; 
  bit = col + 8*(row&1); 
  return (0 != (lastkey[word] & 1<<bit)); 
}

int keydownhold(int basic_keycode) {
  int row, col, word, bit; 
  row = basic_keycode%10; 
  col = basic_keycode/10-1; 
  word = row>>1; 
  bit = col + 8*(row&1); 
  return (0 != (holdkey[word] & 1<<bit)); 
}

void full_screen_color(int COLOR) {
  for(int i = 0; i < LCD_WIDTH_PX*LCD_HEIGHT_PX/2; i++) {  
    *((int*)0xA8000000+i) = (COLOR<<16)|COLOR; }
}

void BackGround () { // This function draws the inicial background
  full_screen_color(COLOR_WHITE);
  for(int i = 0; i <= 200; i+=20) {
    rectangle( 0, i, 384, 2, COLOR_LIGHTGRAY); }
  for(int i = 0; i < 19; i++) {
    rectangle((i * 20) + frame_count, 0, 2, 216, COLOR_LIGHTGRAY); }
  if (frame_count <= 4) {
    rectangle(380 + frame_count, 0, 2, 216, COLOR_LIGHTGRAY); }
  x = 90; y = 50;
  PrintMini(&x, &y, "[F1]", 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  x = 90; y = 100;
  PrintMini(&x, &y, "[F2]", 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  x = 90; y = 150;
  PrintMini(&x, &y, "[F3]", 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  x = 140; y = 50;
  PrintMini(&x, &y, "Player vs Player", 0x02, 0xffffffff, 0, 0, COLOR_RED, COLOR_WHITE, 1, 0);
  x = 140; y = 100;
  PrintMini(&x, &y, "Player vs AI", 0x02, 0xffffffff, 0, 0, COLOR_RED, COLOR_WHITE, 1, 0);
  x = 140; y = 150;
  PrintMini(&x, &y, "AI vs AI", 0x02, 0xffffffff, 0, 0, COLOR_RED, COLOR_WHITE, 1, 0);
  CopySpriteMasked(Logo, 92, 10, 196, 24, MASK_PINK); // Draw the Logo
}

void Circles_or_Crosses() {
  switch(mode) {
  case player_vs_player:
    if (circles_turn == true) {
      circles_won++; }
    else { crosses_won++; }
    break;
  case ai_vs_player:
    if (circles_turn == true) {
      circles_won++; }
    else { crosses_won++; }
    break;
  case ai_vs_ai:
    if (circles_turn == true) {
      circles_won++; }
    else  {crosses_won++; }
    break; }
  InicialData();
}

void InicialData() {
  Bdisp_AllClr_VRAM();     // We clear the VRAM
  x = 0; y = 0;
  PrintMini(&x, &y, "Tied:", 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  x = 0; y = 60;
  PrintMini(&x, &y, "Circles:", 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  x = 0; y = 120;
  PrintMini(&x, &y, "Crosses:", 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  itoa(games_tied,(char*)score+1);
  x = 0; y = 20;
  PrintMini(&x, &y, score, 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  itoa(circles_won,(char*)score+1);
  x = 0; y = 80;
  PrintMini(&x, &y, score, 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  itoa(crosses_won,(char*)score+1);
  x = 0; y = 140;
  PrintMini(&x, &y, score, 0x02, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
  rectangle(159, 0, 4, 200, COLOR_BLACK);  // Draw the grid
  rectangle(225, 0, 4, 200, COLOR_BLACK);
  rectangle(93, 66, 200, 4, COLOR_BLACK);
  rectangle(93, 132, 200, 4, COLOR_BLACK);
  selected_x = 1;
  selected_y = 1;
  for (a = 0; a < 5; a++) {
    for (b = 0; b < 5; b++) {
      Coord[a][b] = 0; }
  }
  goto_jump = false;
  turn = 0;
  F_continue = true;
  AF_continue = true;
  if (first_turn == ai) {        // This switches who goes first
    first_turn = player;
    circles_turn = false; }
  else if (first_turn == player) {
    first_turn = ai;
    circles_turn = true; }
}

void InputWait() {               // *We always have to keep track of the coordinates if each play for the player.
  _Bool valid;
  if (keydownlast(KEY_PRGM_SHIFT)) {
    valid = true;
    do {
      keyupdate();
      if (!keydownlast(KEY_PRGM_SHIFT) && keydownhold(KEY_PRGM_SHIFT)) {
        valid = false; }
      if (keydownlast(KEY_PRGM_MENU)) {
        keymenu(); }
    }  while (valid);
  }
  valid = true;
  do {                           // We have to get inside the loop at least once, so we use a do instead of a while loop.
    keyupdate();               
    if (keydownlast(KEY_PRGM_UP) && !keydownhold(KEY_PRGM_UP)) {
      last_sel_x = selected_x;
      last_sel_y = selected_y;
      if(selected_y == 1) {
        selected_y = 3; }
      else if (selected_y == 2) {
        selected_y = 1; }
      else { selected_y = 2; }
    }
    else if (keydownlast(KEY_PRGM_DOWN) && !keydownhold(KEY_PRGM_DOWN)) {
      last_sel_x = selected_x;
      last_sel_y = selected_y;
      if(selected_y == 1) {
        selected_y = 2; }
      else if (selected_y == 2) {
        selected_y = 3; }
      else { selected_y = 1; }
    }
    else if (keydownlast(KEY_PRGM_RIGHT) && !keydownhold(KEY_PRGM_RIGHT)) {
      last_sel_x = selected_x;
      last_sel_y = selected_y;
      if(selected_x == 1) {
        selected_x = 2; }
      else if (selected_x == 2) {
        selected_x = 3; }
      else { selected_x = 1; }
    }
    else if (keydownlast(KEY_PRGM_LEFT) && !keydownhold(KEY_PRGM_LEFT)) {
      last_sel_x = selected_x;
      last_sel_y = selected_y;
      if(selected_x == 1) {
        selected_x = 3; }
      else if (selected_x == 2) {
        selected_x = 1; }
      else { selected_x = 2; }
    }
    if(keydownlast(KEY_PRGM_SHIFT) && Coord[selected_x][selected_y] == 0) {
      valid = false; }
    if (keydownlast(KEY_PRGM_MENU)) {    // Case you press the menu button
      keymenu(); }
    if (keydownlast(KEY_PRGM_EXIT)) {
      games_tied = 0;
      circles_won = 0;
      crosses_won = 0;
      program_mode = title_screen;
      valid = false; }
    DrawSelected();                      // This draws the selection rectangle where it's needed
    Bdisp_PutDisp_DD();
  } while (valid);
  turn_coord_x[turn] = selected_x;  // *here
  turn_coord_y[turn] = selected_y;
}

void DrawSelected() {
  rectangle(((last_sel_x * 66) - 66) + 97, (last_sel_y * 66) - 62, 60, 2, COLOR_WHITE);      //  Delete last selected square
  rectangle(((last_sel_x * 66) - 66) + 97, (last_sel_y * 66) - 62, 2, 60, COLOR_WHITE);
  rectangle(((last_sel_x * 66) - 66) + 97 + 60, (last_sel_y * 66) - 62, 2, 60, COLOR_WHITE);
  rectangle(((last_sel_x * 66) - 66) + 97, (last_sel_y * 66) - 62 + 60, 62, 2, COLOR_WHITE);
  rectangle(((selected_x * 66) - 66) + 97, (selected_y * 66) - 62, 60, 2, COLOR_RED);        // Current selected square
  rectangle(((selected_x * 66) - 66) + 97, (selected_y * 66) - 62, 2, 60, COLOR_RED);
  rectangle(((selected_x * 66) - 66) + 97 + 60, (selected_y * 66) - 62, 2, 60, COLOR_RED);
  rectangle(((selected_x * 66) - 66) + 97, (selected_y * 66) - 62 + 60, 62, 2, COLOR_RED);
}

_Bool Win () {
  for (a = 1; a < 4; a++) {
    for(b = 1; b < 4; b++) {
      if (Coord[a][b] == win_look_for) {                                      // win_look_for can be 1 or 2, if win_look_for is 1, it will look for other 1's
        if (Coord[a+1][b] == win_look_for && a == 1 && Coord[a+2][b] == 0) {  // that the AI played, if it's 2, it will look for 2's(player input), in order to win.
          a = 3;
          return true; }
        else if (Coord[a-1][b] == win_look_for && a == 3 && Coord[a-2][b] == 0) {
          a = 1;
          return true; }
        else if (Coord[a][b+1] == win_look_for && b == 1 && Coord[a][b+2] == 0) {
          b = 3;
          return true; }
        else if (Coord[a][b-1] == win_look_for && b == 3 && Coord[a][b-2] == 0) {
          b = 1;
          return true; }
        else if (Coord[a+1][b-1] == win_look_for && a == 1 && b == 3 && Coord[3][1] == 0) { 
          a = 3; b = 1;
          return true; }
        else if (Coord[a-1][b+1] == win_look_for && a == 3 && b == 1 && Coord[1][3] == 0) {
          a = 1; b = 3;
          return true; }
        else if (Coord[a+1][b+1] == win_look_for && a == 1 && b == 1 && Coord[3][3] == 0) {
          a = 3; b = 3;
          return true; }
        else if (Coord[a-1][b-1] == win_look_for && a == 3 && b == 3 && Coord[1][1] == 0) {
          a = 1; b = 1;
          return true; }
        else if (((Coord[b][a] == win_look_for && a == 1 && b == 3) || Coord[a+2][b+2] == win_look_for) && Coord[2][2] == 0) { 
          a = 2;
          b = 2;
          return true; }
        else if ((Coord[a+2][b] == win_look_for && Coord[2][b] == 0) || (Coord[a][b+2] == win_look_for && Coord[a][2] == 0)) {
          if (Coord[a+2][b] == win_look_for) {
            a = 2; }
          else { b = 2; }
          return true; }
      }
    }
  }
  return false; 
}

_Bool AvoidLosing() {
  for (a = 1; a < 4; a++) {
    for(b = 1; b < 4; b++) {
      if (Coord[a][b] == AL_look_for) {  // if avoidlosing_look_for is 1, means AI 2 is playing, and he is trying to avoid losing, so the ai have to look_for 
        if (Coord[a+1][b] == AL_look_for && a == 1 && Coord[a+2][b] == 0) { // the plays of the adversary, in this case, the AI 1. (This only verifies for ai_vs_ai mode)
          a = 3;
          return true; }
        else if (Coord[a-1][b] == AL_look_for && a == 3 && Coord[a-2][b] == 0) {
          a = 1;
          return true; }
        else if (Coord[a][b+1] == AL_look_for && b == 1 && Coord[a][b+2] == 0) {
          b = 3;
          return true; }
        else if (Coord[a][b-1] == AL_look_for && b == 3 && Coord[a][b-2] == 0) {
          b = 1;
          return true; }
        else if (Coord[a+1][b-1] == AL_look_for && a == 1 && b == 3 && Coord[3][1] == 0) { 
          a = 3; b = 1;
          return true; }
        else if (Coord[a-1][b+1] == AL_look_for && a == 3 && b == 1 && Coord[1][3] == 0) {
          a = 1; b = 3;
          return true; }
        else if (Coord[a+1][b+1] == AL_look_for && a == 1 && b == 1 && Coord[3][3] == 0) {
          a = 3; b = 3;
          return true; }
        else if (Coord[a-1][b-1] == AL_look_for && a == 3 && b == 3 && Coord[1][1] == 0) {
          a = 1; b = 1;
          return true; }
        else if (((Coord[b][a] == AL_look_for && a == 1 && b == 3) || Coord[a+2][b+2] == AL_look_for) && Coord[2][2] == 0) {
          a = 2; b = 2;
          return true; }
        else if ((Coord[a][b+2] == AL_look_for && Coord[a][2] == 0 && b+2!=5) || (Coord[a+2][b] == AL_look_for && Coord[2][b] == 0 && a+2!=5)) {
          if (Coord[a+2][b] == AL_look_for && Coord[2][b] == 0) {
            a = 2; }
          else { b = 2; }
          return true; }
      }
    }
  }
  return false;  // The function will only return false at this point
}

_Bool Fork() { // Basicly this function will screw you up if you play two sides in a row
  if (first_turn == ai) {
    if (turn == 1) {
      CenterPlay();
      return true; }    
    else if (turn == 3) {
      if ((turn_coord_x[2] == 2 && turn_coord_y[2]!= 2) || (turn_coord_x[2] != 2 && turn_coord_y[2] == 2 )) {  // If a side was played
        if (turn_coord_x[2] == 2) {  
          for(b = 1; b < 4; b++) {
            if (Coord[2][b] == 0) {
              a = 2;
              turn_coord_x[3] = 2;
              turn_coord_y[3] = b;
              return true; }
          }
        }
        else {
          for(a = 1; a < 4; a++) {
            if (Coord[a][2] == 0) {
              b = 2;
              turn_coord_x[3] = a;
              turn_coord_y[3] = 2;
              return true; }
          }
        }
      }
      else { 
        F_continue = false;
        return false; }  // If the second play wasn't in a side
    }
    else if (turn == 5) {
      if ((turn_coord_x[4] == 2 && turn_coord_y[4] != 2) || (turn_coord_x[4] != 2 && turn_coord_y[4] == 2 )) {  // If play 4 was in a side
        if (turn_coord_x[3] == 2 && turn_coord_y[2] == 1) {
          for (a = 1; a < 4 ;a++) {    
            if (Coord[a][3] == 0) {
              b = 3;
              turn_coord_x[5] = a;
              turn_coord_y[5] = 3;
              return true; }
          }
        }
        else if (turn_coord_x[3] == 2 && turn_coord_y[2] == 3) { 
          for (a = 1; a < 4 ;a++) { 
            if (Coord[a][1] == 0) {
              b = 1;
              turn_coord_x[5] = a;
              turn_coord_y[5] = 1;
              return true; }
          }
        }
        else if (turn_coord_y[3] == 2 && turn_coord_x[2] == 1) {
          for (b = 1; b < 4 ;b++) {
            if (Coord[3][b] == 0) {
              a = 3;
              turn_coord_x[5] = 3;
              turn_coord_y[5] = b;
              return true; }
          }
        }
        else if (turn_coord_y[3] == 2 && turn_coord_x[2] == 3) {
          for (b = 1; b < 4 ;b++) { 
            if (Coord[1][b] == 0) {
              a = 1;
              turn_coord_x[5] = 1;
              turn_coord_y[5] = b;
              return true; }
          }
        }
      }  //*
      else {
        F_continue = false;
        return false; }   // If play 4 wasn't in a side
    }
    else {
      F_continue = false;
      return false; }     // If it's not turn 1, 3 or 5
  }
  else {
    F_continue = false; 
    return false; }       // If the first play wasn't made by the ai
}

_Bool AvoidFork() {  // In this function the AI will block a possible fork made by the opponent
  if (first_turn == player) {
    if (turn == 2) {
      if (turn_coord_x[1] != 2 && turn_coord_y[1] != 2) {
        CenterPlay();
        return true; }
      else {
        AF_continue = false;  // Never come back to this function
        return false; }       // If the fist play wasn't in a corner
    }
    else if (turn == 4) {  
      if (turn_coord_x[3] != 2 && turn_coord_y[3] != 2) {  // If a corner has been played
        EmptySide();
        return true; } 
      else {
        AF_continue = false;
        return false; }  // Ifthe third play wasn't in a corner
    }
    else {
      AF_continue = false;
      return false; }    // If the turn isn't 2 or 4 
  }
  else { 
    AF_continue = false; 
    return false; }      // If the first play wasn't made by the player
}

_Bool CenterPlay() {
  if (Coord[2][2] != 0) {
    return false; }
  else {
    a = 2;
    b = 2;
    return true; }
}

_Bool OppositeCorner() {
  if (Coord[2][1] == AL_look_for && Coord[3][2] == AL_look_for && Coord[3][1] == 0) {
    a = 3;
    b = 1;
    return true; }
  else if (Coord[2][1] == AL_look_for && Coord[1][2] == AL_look_for && Coord[1][1] == 0) {
    a = 1;
    b = 1;
    return true; }
  else if (Coord[1][2] == AL_look_for && Coord[2][3] == AL_look_for && Coord[1][3] == 0) {
    a = 1;
    b = 3;
    return true; }
  else if (Coord[2][3] == AL_look_for && Coord[3][2] == AL_look_for && Coord[3][3] == 0) {
    a = 3;
    b = 3;
    return true; }
  else if (Coord[1][1] == AL_look_for && Coord[3][3] == 0) {
    a = 3;
    b = 3;
    return true; }
  else if (Coord[3][1] == AL_look_for && Coord[1][3] == 0) {
    a = 1;
    b = 3;
    return true; }
  else if (Coord[1][3] == AL_look_for && Coord[3][1] == 0) {
    a = 3; 
    b = 1;
    return true; } 
  else if (Coord[3][3] == AL_look_for && Coord[1][1] == 0) {
    a = 1;
    b = 1;
    return true; }
  else { return false; }
}

_Bool EmptyCorner() {
  if (Coord[1][1] == 0 || Coord[1][3]== 0 || Coord[3][1] == 0 || Coord[3][3] == 0) {
    do {
      int corner = Rand(1, 4);
      if (corner == 1) {
        a = 1;
        b = 1; }
      else if (corner == 2) {
        a = 3;
        b = 1; }
      else if (corner == 3) {
        a = 1;
        b = 3; }
      else if (corner == 4) {
        a = 3;
        b = 3; }
    } while (Coord[a][b] != 0); 
    return true; }
    else { return false; }
}

_Bool EmptySide() {
  if (Coord[2][1] == 0 || Coord[1][2]== 0 || Coord[3][2] == 0 || Coord[2][3] == 0) {
   do {
      int side = Rand(1, 4);  // This way, the AI won't be playing always in the same place
      if (side == 1) {
        a = 2;
        b = 1; }
      else if (side == 2) {
        a = 1;
        b = 2; }
      else if (side == 3) {
        a = 3;
        b = 2; }
      else if (side == 4) {
        a = 2;
        b = 3; }
    } while (Coord[a][b] != 0); 
    return true; }
  else { return false; }
}

_Bool TiedGame() {
  for (a = 1; a < 4 ; a++) {
    if (Coord[a][1] == Coord[a][2] && Coord[a][2] == Coord[a][3] && Coord[a][1] != 0) {
      if (Coord[a][1] == 1) {
        circles_turn = true; }      // TiedGame means we are in turn 10,
      else if (Coord[a][1] == 2) {  // we can now change 'circles_turn',
        circles_turn = false; }     // this is useful to see who won (if anyone)
      Circles_or_Crosses();
      return false; }
  }
  for (b = 1; b < 4 ; b++) {
    if (Coord[1][b] == Coord[2][b] && Coord[2][b] == Coord[3][b] && Coord[1][b] != 0) {
      if (Coord[1][b] == 1) {
        circles_turn = true; }
      else if (Coord[1][b] == 2) {
        circles_turn = false; }
      Circles_or_Crosses();
      return false; }
  }
  if (((Coord[1][1] == Coord[2][2] && Coord[2][2] == Coord[3][3]) || (Coord[1][3] == Coord[2][2] && Coord[2][2] == Coord[3][1])) && Coord[2][2] != 0) {
    if (Coord[2][2] == 1) {
      circles_turn = true; }
    else if (Coord[2][2] == 2) {
      circles_turn = false; }
    Circles_or_Crosses();
    return false; 
  }
  return true;
} 

_Bool GameEnd () {
  for(int i = 1; i < 4; i++) {
    for(int j = 1; j < 4; j++) {
      if (Coord[i][j] == 0) {
        return false; }
    }
  }
  return true;
}

void Priority_Algorithm() {
  if (Win() == true) {
    Circles_or_Crosses();
    goto_jump = true;
    return; }
  else {
    if (AvoidLosing() == true) {
      return; }
    else {
      if (F_continue == true && mode == ai_vs_player && Fork() == true) {
        return; }
      else {
        if (AF_continue == true && mode == ai_vs_player && AvoidFork() == true) {
          return; }
        else {
          if (CenterPlay() == true) {
            return; }
          else {
            if (OppositeCorner() == true) {
              return; }
            else {
              if (EmptyCorner() == true) {
                return; }
              else { 
                EmptySide();
                return; 
              }
            }
          }
        }
      }
    }
  }
}
