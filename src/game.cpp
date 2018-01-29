#include "game.h"

const char PATH_TO_EFFECTS[]={"res/effects.cfg"};
const int EFFECTS=1;
Mix_Chunk *effects[EFFECTS];

void Load_effects()
{
 FILE *in=fopen(PATH_TO_EFFECTS,"r");
 if(in==NULL)
    return;
 int sqq=0;
 while(!feof(in))
       {
        char aux[TEXT_LENGTH_MAX];
        fgets(aux,sizeof aux,in);
        int sq=strlen(aux);
        if(aux[sq]=='\n')
           aux[sq]=NULL;
        effects[sqq]=Mix_LoadWAV(aux);
       }
 fclose(in);
}

const char PATH_TO_LEVELS[]={"res/levels.cfg"};

const int LEVELS=10;
int number_of_levels;
char levels_paths[LEVELS][TEXT_LENGTH_MAX];

void Load_levels_paths()
{
 FILE *in=fopen(PATH_TO_LEVELS,"r");
 int number_of_levels=0;
 while(!feof(in))
       {
        fgets(levels_paths[number_of_levels],sizeof levels_paths[number_of_levels],in);
        int sq=strlen(levels_paths[number_of_levels])-1;
        if(levels_paths[number_of_levels][sq]=='\n')
           levels_paths[number_of_levels][sq]=NULL;
        number_of_levels++;
       }
 fclose(in);
}

void Game::Load()
{
 FILE *in=fopen(levels_paths[level],"r");
 fscanf(in,"%d %d ",&number_of_lines,&number_of_columns);
 for(int i=0;i<number_of_lines;i++)
     for(int j=0;j<number_of_columns;j++)
         {
          int aux;
          fscanf(in,"%d ",&aux);
          map[i][j]=(aux==1);
         }
 fscanf(in,"%d %d ",&starting_position.x,&starting_position.y);
 fscanf(in,"%d %d ",&winning_position.x,&winning_position.y);
 for(int i=0;i<NUMBER_OF_PLAYERS;i++)
     {
      players_positions[i]=starting_position;
      players_timers[i].start();
      winning_effect_timer[i].start();
      players_cooldowns[i]=0;
     }
 fclose(in);
}

void Game::Clear()
{
 for(int i=0;i<NUMBER_OF_PLAYERS;i++)
     players_positions[i]={0,0};
 for(int i=0;i<number_of_lines;i++)
     for(int j=0;j<number_of_columns;j++)
         map[i][j]=false;
 number_of_lines=0;number_of_columns=0;
}

void Game::Handle_Events()
{
 for(int i=0;i<NUMBER_OF_PLAYERS;i++)
     Handle_Event(i);
}

const int EVENT_DELAY=200,HIT_DELAY=1000;
const int AXIS_DEAD_ZONE=16000;
const double HIT_VIBRATION_STRENGTH=1,MOVEMENT_VIBRATION_STRENGTH=0.4,WIN_VIBRATION_STRENGTH=0.5;
const int HIT_VIBRATION_DURATION=1000,MOVEMENT_VIBRATION_DURATION=200,WIN_VIBRATION_DURATION=200;

void Game::Handle_Event(int _player)
{
 if(players_timers[_player].get_ticks()>players_cooldowns[_player])
    {
     bool pressed=false;
     bool hit_obstacle=false;
     if(controller[_player].Get_StickY()<-AXIS_DEAD_ZONE)
        {
         pressed=true;
         int newx=std::max(0,players_positions[_player].x-1);
         if(map[newx][players_positions[_player].y]==0)
            players_positions[_player].x=newx;
         else
            hit_obstacle=true;
        }
     if(controller[_player].Get_StickY()>AXIS_DEAD_ZONE)
        {
         pressed=true;
         int newx=std::min(number_of_lines-1,players_positions[_player].x+1);
         if(map[newx][players_positions[_player].y]==0)
            players_positions[_player].x=newx;
         else
            hit_obstacle=true;
        }
     if(controller[_player].Get_StickX()<-AXIS_DEAD_ZONE)
        {
         pressed=true;
         int newy=std::max(0,players_positions[_player].y-1);
         if(map[players_positions[_player].x][newy]==0)
            players_positions[_player].y=newy;
         else
            hit_obstacle=true;
        }
     if(controller[_player].Get_StickX()>AXIS_DEAD_ZONE)
        {
         pressed=true;
         int newy=std::min(number_of_columns-1,players_positions[_player].y+1);
         if(map[players_positions[_player].x][newy]==0)
            players_positions[_player].y=newy;
         else
            hit_obstacle=true;
        }
     if(pressed)
        players_timers[_player].start();
     else
        return;
     if(hit_obstacle)
        {
         controller[_player].Stop_Vibration();
         players_cooldowns[_player]=HIT_DELAY;
         controller[_player].Vibrate(HIT_VIBRATION_STRENGTH,HIT_VIBRATION_DURATION);
        }
     else
        {
         controller[_player].Stop_Vibration();
         players_cooldowns[_player]=EVENT_DELAY;
         controller[_player].Vibrate(MOVEMENT_VIBRATION_STRENGTH,MOVEMENT_VIBRATION_DURATION);
        }
    }
}

bool Game::Check_winners()
{
 bool ret=true;
 for(int i=0;i<NUMBER_OF_PLAYERS;i++)
     ret=(ret && Check_win(i));
 return ret;
}

bool Game::Check_win(int _player)
{
 if(players_positions[_player].x==winning_position.x && players_positions[_player].y==winning_position.y)
    {
     if(winning_effect_timer[_player].get_ticks()>WIN_VIBRATION_DURATION)
        {
         controller[_player].Stop_Vibration();
         winning_effect_timer[_player].start();
         controller[_player].Vibrate(WIN_VIBRATION_STRENGTH,WIN_VIBRATION_DURATION);
        }
     return true;
    }
 return false;
}

const int FRAMES_PER_SECOND=30;

void Game::Start(Texture *_screen)
{
 bool quit=false;
 Timer fps_timer;
 while(!quit)
       {
        fps_timer.start();
        SDL_PumpEvents();
        Update_Controllers_Events();
        Check_level_ending(_screen);
        Handle_Events();
        quit=(keystates[SDL_QUIT] || keystates[SDL_SCANCODE_ESCAPE] || controller[0].Pressed_Back_button() || controller[1].Pressed_Back_button());
        if(fps_timer.get_ticks()<1000/FRAMES_PER_SECOND)
           SDL_Delay((1000/FRAMES_PER_SECOND)-fps_timer.get_ticks());
       }
}

void Game::Check_level_ending(Texture *_screen)
{
 if(Check_winners())
    {
     if(level==number_of_levels)
        Play_Ending(_screen);
     else
        level++;
    }
}

char LOADING_IMAGE_PATH[]={"res/images/ending_image.png"};

void Game::Play_Ending(Texture *_screen)
{
 Texture *ending_image=Load_Transparent_Texture(LOADING_IMAGE_PATH);
 Apply_Texture(0,0,ending_image,_screen);
 Flip_Buffers(_screen);
 SDL_Delay(4000);
}
