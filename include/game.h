#ifndef GAME_H
#define GAME_H

#include "texture.h"
#include "controller.h"

struct Position
{
 int x,y;
};

void Load_effects();
void Load_levels_paths();

const int NUMBER_OF_PLAYERS=2;
const int NUMBER_OF_LINES=10,NUMBER_OF_COLUMNS=10;

class Game
{
 private:
 int level=0;
 Position players_positions[NUMBER_OF_PLAYERS];
 Position starting_position,winning_position;
 int number_of_lines,number_of_columns;
 char map[NUMBER_OF_LINES][NUMBER_OF_COLUMNS];
 Timer players_timers[NUMBER_OF_PLAYERS];
 int players_cooldowns[NUMBER_OF_PLAYERS];
 Timer winning_effect_timer[NUMBER_OF_PLAYERS];

 public:
 void Load();
 void Clear();
 void Handle_Event(int _player);
 void Handle_Events();
 void Start(Texture *_screen);
 bool Check_winners();
 bool Check_win(int _player);
 void Check_level_ending(Texture *_screen);
 void Play_Ending(Texture *_screen);
};

#endif // GAME_H
