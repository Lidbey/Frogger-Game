/*	
	Name: Frogger Game
	Author: Wojciech Wicki
	Version 0.1
*/


#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>


extern "C" {
#include"../Frogger/SDL2-2.0.10/include/SDL.h"
#include"../Frogger/SDL2-2.0.10/include/SDL_main.h"
}


//DEFINES
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define UP 1
#define DOWN 2
#define RIGHT 3
#define LEFT 4
#define STOP 5


//STRUCTS


//position
typedef struct {
	int x;
	int y;
}pos;

//objects parameters
typedef struct {
	SDL_Surface* image;
	short int action;
	pos pos;
	int width;
} object;


//score for each thing
typedef struct {
	short int pos;
	short int end;
	short int frog;
	short int bee;
	int total;
	short int flag;
} score;


//bee (bonuses)
typedef struct {
	SDL_Surface* image;
	short int seed;
} bee;

//information of game state(to save/load game)
typedef struct {
	bool frogend[5];
	score score;
	pos frog;
	struct {
		pos pos;
		int rand;
		int action;
	}lostfrog;
	struct {
		pos pos;
		int rand;
	}obstacles[5];
	struct {
		pos pos;
		int rand;
	}cars[5];
	short int beeseed;
	double worldTime;
	int health;
	int etap;
}saved;

//number of objects in each row
typedef struct {
	int cars[5];
	int woods[5];
}obstacle_number;

//information about loading game from file
typedef struct {
	struct {
		pos pos;
		int rand;
	}cars[5];
	struct {
		pos pos;
		int rand;
	}obst[5];
	pos frog;
	int health;
	int maxTime;
	obstacle_number nums;
}gamefile;

//filelines for highscores
typedef struct {
	char name[64];
	int score;
}fileline;

//information about window, renderer etc
typedef struct {
	SDL_Surface* screen; 
	SDL_Surface* charset; 
	SDL_Window* window; 
	SDL_Renderer* renderer; 
	SDL_Texture* scrtex;
}scr;

typedef struct {
	struct {
		int pos;
		short int values[400];
	}before;
	struct {
		int pos;
		short int values[400];
	}after;
	bool flag;
}keys;


//FUNCTIONS

//turtle animations
void turtles(object* turtle, bool if90, SDL_Surface* pattern[])
{
	static int time;
	static int time2;
	if (if90 == 1)
	{
		time++;
		if (time > 3000)
		{
			turtle->image = pattern[5];
			turtle->width = 90;
			time = 0;
		}
		else if (time > 2500)
		{
			turtle->image = pattern[9];
			turtle->width = 0;
		}
		else if (time > 2200)
			turtle->image = pattern[8];
		else if (time > 1900)
			turtle->image = pattern[7];
		else if (time > 1600)
			turtle->image = pattern[6];
	}
	else if (if90 == 0)
	{
		time2++;
		if (time2 > 2500)
		{
			turtle->image = pattern[0];
			turtle->width = 60;
			time2 = 0;
		}
		else if (time2 > 2000)
		{
			turtle->image = pattern[4];
			turtle->width = 0;
		}
		else if (time2 > 1700)
			turtle->image = pattern[3];
		else if (time2 > 1400)
			turtle->image = pattern[2];
		else if (time2 > 1100)
			turtle->image = pattern[1];
	}
}

//more complex modulo for screen objects
int mod(int number)
{
	while (number < -90)
		number += 1280;
	while (number > 1190)
		number -= 1280;
	return number;
}

//checks if any car hit our frog
bool checkcars(object frog, object* car[][5], obstacle_number nums)
{
	for (int i = 0; i < 5 ; i++)
		for (int j = 0; j < nums.cars[i]; j++)
			if ((abs(car[0][i][j].pos.x - frog.pos.x) < car[0][i][j].width/2+frog.width/2) && (abs(car[0][i][j].pos.y - frog.pos.y) < 35))
				return 1;
	
	return 0;
}

//checks if frog is on any obstacle <- lose condition
bool checkobstacle(object frog, object* obstacle[][5], obstacle_number nums)
{
	for(int i = 0; i < 5; i++)
		for (int j = 0; j < nums.woods[i]; j++)
			if (((abs(obstacle[1][i][j].pos.x - frog.pos.x )) < obstacle[1][i][j].width/2) && (abs(obstacle[1][i][j].pos.y - frog.pos.y)) < 35)
				return 1;
	
	return 0;
}

//checks if frog is on obstacle <- moving condition
bool frogonobstacle(object frog, object obstacle)
{
		if (((abs(obstacle.pos.x - frog.pos.x)) < obstacle.width/2) && (abs(obstacle.pos.y - frog.pos.y)) < 35)
			return 1;

	return 0;
}

//moving an object
void moveobs(object* object, bool side)
{
	if (side == 0)
	{
		object->pos.x++;
		if (object->pos.x > 1190)
			object->pos.x = -90;
	}
	else if (side == 1)
	{
		object->pos.x--;
		if (object->pos.x < -90)
			object->pos.x = 1190;
	}
}

//moving of a lost frog(score)
void movelostfrog(object* object, short int side)
{
	static int pos;
	if (side == 0)
		pos = 0;
	else if (side == RIGHT)
	{
		object->pos.x++;
		pos++;
		if (pos % 46 == 0)
		{
			pos = 0;
			object->action = 0;
			object->image = SDL_LoadBMP("../Frogger/photos/eti2.bmp");
		}
	}
	else if (side == LEFT)
	{
		object->pos.x--;
		pos++;
		if (pos % 46 == 0)
		{
			pos = 0;
			object->action = 0;
			object->image = SDL_LoadBMP("../Frogger/photos/eti4.bmp");
		}
	}
}

//moving of frog
void movefrog(object* object, short int side)
{
	static int pos;
	if (side == 0)
		pos = 0;
	else if (side == UP)
	{
		object->pos.y--;
		pos++;
		if (pos % 35 == 0)
		{
			pos = 0;
			object->action = 0;
			object->image = SDL_LoadBMP("../Frogger/photos/eti.bmp");
		}
	}
	else if (side == DOWN)
	{
		object->pos.y++;
		pos++;
		if (pos % 35 == 0)
		{
			pos = 0;
			object->action = 0;
			object->image = SDL_LoadBMP("../Frogger/photos/eti3.bmp");
		}
	}
	else if (side == RIGHT)
	{
		object->pos.x++;
		pos++;
		if (pos % 46 == 0)
		{
			pos = 0;
			object->action = 0;
			object->image = SDL_LoadBMP("../Frogger/photos/eti2.bmp");
		}
	}
	else if (side == LEFT)
	{
		object->pos.x--;
		pos++;
		if (pos % 46 == 0)
		{
			pos=0;
			object->action = 0;
			object->image = SDL_LoadBMP("../Frogger/photos/eti4.bmp");
		}
	}
	else if (side == STOP)
	{
		pos++;
		if (pos % 200 == 0)
		{
			pos = 0;
			object->action = 0;
		}
	}
}

void movefrogbot(object* object, short int side)
{
	static int pos;
	if (side == 0)
		pos = 0;
	else if (side == UP)
	{
		object->pos.y--;
		pos++;
		if (pos % 35 == 0)
		{
			pos = 0;
			object->action = 0;
		}
	}
	else if (side == STOP)
	{
		pos++;
		if (pos % 200 == 0)
		{
			pos = 0;
			object->action = 0;
		}
	}
	else if (side == RIGHT)
	{
		object->pos.x++;
		pos++;
		if (pos % 46 == 0)
		{
			pos = 0;
			object->action = 0;
		}
	}
	else if (side == LEFT)
	{
		object->pos.x--;
		pos++;
		if (pos % 46 == 0)
		{
			pos = 0;
			object->action = 0;
		}
	}
}

//saving game to file
bool savegame(saved save) {
	FILE* fptr;
	fptr = fopen("savegame.txt", "w+");
	if (fptr == NULL)
		return 0;
	fprintf(fptr, "beeseed= %d\n", save.beeseed);
	for (int i = 0; i < 5; i++)
	{
		fprintf(fptr, "car %d pos x= %d y=%d\n", i, save.cars[i].pos.x, save.cars[i].pos.y);
		fprintf(fptr, "car %d speed= %d\n", i, save.cars[i].rand);
		fprintf(fptr, "obstacle %d pos x= %d y=%d\n", i, save.obstacles[i].pos.x, save.obstacles[i].pos.y);
		fprintf(fptr, "obstacle %d speed= %d\n", i, save.obstacles[i].rand);
		fprintf(fptr, "frogend %d= %d\n", i, save.frogend[i]);
	}
	fprintf(fptr, "health= %d\n", save.health);
	fprintf(fptr, "worldTime= %f\n", save.worldTime);
	fprintf(fptr, "frog pos x= %d y=%d\n", save.frog.x, save.frog.y);
	fprintf(fptr, "lost frog pos x= %d y= %d rand= %d action= %d\n", save.lostfrog.pos.x, save.lostfrog.pos.y, save.lostfrog.rand, save.lostfrog.action);
	fprintf(fptr, "score bee= %d score end= %d\n", save.score.bee, save.score.end);
	fprintf(fptr, "score flag= %d score frog= %d\n", save.score.flag, save.score.frog);
	fprintf(fptr, "score pos= %d score total= %d\n", save.score.pos, save.score.total);
	fprintf(fptr, "etap= %d", save.etap);
	fclose(fptr);
	return 1;
}

//loading game from file
bool loadgame(saved* save) {
	FILE* fptr;
	fptr = fopen("savegame.txt", "r");
	int results = 0;
	if (fptr == NULL) return 0;
	results = fscanf(fptr, "beeseed= %hd", &save->beeseed);
	if (results != 1) return 0;
	for (int i = 0; i < 5; i++)
	{
		results = fscanf(fptr, " car %*d pos x= %d y=%d", &save->cars[i].pos.x, &save->cars[i].pos.y);
		if (results != 2) return 0;
		results = fscanf(fptr, " car %*d speed= %d", &save->cars[i].rand);
		if (results != 1) return 0;
		results = fscanf(fptr, " obstacle %*d pos x= %d y=%d", &save->obstacles[i].pos.x, &save->obstacles[i].pos.y);
		if (results != 2) return 0;
		results = fscanf(fptr, " obstacle %*d speed= %d", &save->obstacles[i].rand);
		if (results != 1) return 0;
		int temp = 0;
		results = fscanf(fptr, " frogend %*d= %d", &temp);
		if (results != 1) return 0;
		if (temp != 0 && temp != 1) return 0;
		save->frogend[i] = temp;
	}
	results=fscanf(fptr, " health= %d", &save->health);
	if (results != 1) return 0;
	results=fscanf(fptr, " worldTime= %lf", &save->worldTime);
	if (results != 1) return 0;
	results=fscanf(fptr, " frog pos x= %d y=%d", &save->frog.x, &save->frog.y);
	if (results != 2) return 0;
	results=fscanf(fptr, " lost frog pos x= %d y= %d rand= %d action= %d", &save->lostfrog.pos.x, &save->lostfrog.pos.y, &save->lostfrog.rand, &save->lostfrog.action);
	if (results != 4) return 0;
	results=fscanf(fptr, " score bee= %hd score end= %hd", &save->score.bee, &save->score.end);
	if (results != 2) return 0;
	results=fscanf(fptr, " score flag= %hd score frog= %hd", &save->score.flag, &save->score.frog);
	if (results != 2) return 0;
	results=fscanf(fptr, " score pos= %hd score total= %d", &save->score.pos, &save->score.total);
	if (results != 2) return 0;
	results = fscanf(fptr, " etap= %d", &save->etap);
	if (results != 1) return 0;
	fclose(fptr);
	return 1;
}

//loading particular level of a game
bool loadetap(gamefile* gamefile, int etap)
{
	char name[32] = "./etapy/etapx.txt";
	name[12] = '0' + etap;
	FILE* fptr;
	fptr = fopen(name, "r");
	int results = 0;

	if (fptr == NULL) return 0;
	for (int i = 0; i < 5; i++)
	{
		results = fscanf(fptr, "car %*d pos x= %d y=%d\n", &gamefile->cars[i].pos.x, &gamefile->cars[i].pos.y);
		if (results != 2) return 0;
		results = fscanf(fptr, "car %*d speed= %d\n", &gamefile->cars[i].rand);
		if (results != 1) return 0;
		results = fscanf(fptr, "obstacle %*d pos x= %d y=%d\n", &gamefile->obst[i].pos.x, &gamefile->obst[i].pos.y);
		if (results != 2) return 0;
		results = fscanf(fptr, "obstacle %*d speed= %d\n", &gamefile->obst[i].rand);
		if (results != 1) return 0;
	}
	results = fscanf(fptr, "health= %d\n", &gamefile->health);
	if (results != 1) return 0;
	results = fscanf(fptr, "frog pos x= %d y=%d\n", &gamefile->frog.x, &gamefile->frog.y);
	if (results != 2) return 0;
	results = fscanf(fptr, "maxTime= %d\n", &gamefile->maxTime);
	if (results != 1) return 0;
	results = fscanf(fptr, "number of cars 1= %d 2= %d 3= %d 4= %d 5=%d\n", &gamefile->nums.cars[0], &gamefile->nums.cars[1], &gamefile->nums.cars[2], &gamefile->nums.cars[3], &gamefile->nums.cars[4]);
	if (results != 5) return 0;
	results = fscanf(fptr, "number of obstacles 1= %d 2= %d 3= %d 4= %d 5=%d\n", &gamefile->nums.woods[0], &gamefile->nums.woods[1], &gamefile->nums.woods[2], &gamefile->nums.woods[3], &gamefile->nums.woods[4]);
	if (results != 5) return 0;
	fclose(fptr);
	return 1;
}

//loading lines of a file and returns them
fileline* highscoreplayers()
{
	bool ok;
	FILE* fptr;
	fptr = fopen("highscore.txt", "r+");
	if (fptr == NULL) return NULL;
	fileline* line=(fileline*)malloc(10*sizeof(fileline));
	if (line == NULL) return NULL;
	for (int i = 0; i < 10; i++)
	{
		ok = fscanf(fptr, "%*d %s %d\n", &line[i].name, &line[i].score);
		if (!ok) return NULL;
	}
	return line;
}

//saving player to highscore list
bool highscore(int score, char name[64])
{
	FILE* fptr;
	fptr = fopen("highscore.txt", "r+");
	if (fptr == NULL) return 0;
	fileline* line = highscoreplayers();
	if (line == NULL) return 0;
	int pos = 10;
	for(int i=0; i<10; i++)
		if (line[i].score < score)
		{
			pos = i;
			break;
		}
	if (pos != 10)
	{
		for (int i = 9; i >pos; i--)
			line[i] = line[i - 1];
		line[pos].name[0] = name[0];
		for(int i=1; i<64 && line[pos].name[i-1]!='\0'; i++)
			line[pos].name[i] = name[i];
		line[pos].score = score;
	}
	fseek(fptr, 0, SEEK_SET);
	for (int i = 0; i < 10; i++)
		fprintf(fptr, "%d %s %d\n", i + 1, line[i].name, line[i].score);
	fclose(fptr);
	return 1;
}

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostokπta o d≥ugoúci bokÛw l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

//initialize objects in each row based on first
void init(object* obstacle[2][5], obstacle_number nums)
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 1; j < nums.cars[i]; j++)
		{
			obstacle[0][i][j] = obstacle[0][i][j - 1];
			if (i == 0)
				obstacle[0][i][j].pos.x = mod(obstacle[0][i][j - 1].pos.x + 80);
			else if (i == 1)
				obstacle[0][i][j].pos.x = mod(obstacle[0][i][j - 1].pos.x + 90);
			else if (i == 2)
				obstacle[0][i][j].pos.x = mod(obstacle[0][i][j - 1].pos.x + 104);
			else if (i == 3)
				obstacle[0][i][j].pos.x = mod(obstacle[0][i][j - 1].pos.x + 125);
			else if (i == 4)
				obstacle[0][i][j].pos.x = mod(obstacle[0][i][j - 1].pos.x + 160);
		}
		for (int j = 1; j < nums.woods[i]; j++)
		{
			obstacle[1][i][j] = obstacle[1][i][j - 1];
			obstacle[1][i][j].pos.x = mod(obstacle[1][i][j - 1].pos.x + 1280/nums.woods[i]);
		}
	}
}

//transmits score points from one game to another if player passes an level
int transport(int score, bool option)
{
	static int total;
	if (option)
		total = score;
	return total;
}

//returns level from saved file
int savedetap()
{
	saved save;
	loadgame(&save);
	return save.etap;
}

//bot - checks if frog can safely jump to cars row
bool distance(int frog, object* obst, int number)
{
	int flag = 1;
	for (int i = 0; i < number; i++)
		if (abs(frog - obst[i].pos.x) < 90)
			flag = 0;
	return flag;
}

//bot - checks if frogr is able to jump on wood/frog
bool distancewood(int frog, object* obst, int number)
{
	int flag = 0;
	for (int i = 0; i < number; i++)
		if (abs(obst[i].pos.x - frog) < 25)
			flag = 1;
	return flag;
}

//returns number of saved files with particular levels
int countetaps() {
	int i = 0;
	char name[32] = "./etapy/etapx.txt";
	FILE* fptr;
	for (int i = 1; i < 10; i++)
	{
		name[12] = '0' + i;
		fptr = fopen(name, "r");
		if (fptr == NULL) return i-1;
		fclose(fptr);
	}
	return 0;
}

int numofgames(bool option)
{
	static int x = 0;
	x++;
	if (option)
		x = 0;
	return x;
}


keys simgame(saved save)
{
	//load-etap save - stan gry
	keys key = { 0 };
	//initializing variables
	if (loadgame(&save) == 0)
		return key;
	SDL_Surface* noll = NULL;
	int load = save.etap;
	int t1, quit, frames, t2=0;
	double delta, worldTime, fpsTimer, fps;
	bool frogend[5] = { 0,0,0,0,0 };
	bool pause = 0, exit = 0, gameover = 0, flag = 1, bot = 0;
	int health = 5, maxTime = 50, temp = rand() % 300;
	bee bee = { noll, rand() % 5 };
	score score = { 0,0,0,0,0,0 };
	score.total = 0;
	object frog = { noll, 0, SCREEN_WIDTH / 2, 462, 30 };
	obstacle_number nums = { 7, 6, 5, 4, 3, 7, 7, 5, 9, 6 };
	object* obstacle[2][5];
	key.before.pos = 0;
	key.after.pos = 0;
	
	int gotohalf=0;

	//loading number of obstacles in each row
	if (load > 0)
	{
		gamefile file;
		if (!loadetap(&file, load)) return key;
		nums = file.nums;
	}

	//allocating memory for obstacles in each row
	for (int i = 0; i < 5; i++)
	{
		obstacle[0][i] = (object*)malloc(nums.cars[i] * sizeof(object));
		if (obstacle[0][i] == NULL)
			return key;
		obstacle[1][i] = (object*)malloc(nums.woods[i] * sizeof(object));
		if (obstacle[1][i] == NULL)
			return key;
	}

	//initializing first obstacle of each row(as every other is different)
	obstacle[0][0][0] = { noll , (rand() % 6) + 5, rand() % 1280 - 90, 427, 30 };
	obstacle[0][1][0] = { noll, (rand() % 6) + 4, rand() % 1280 - 90, 392, 30 };
	obstacle[0][2][0] = { noll, (rand() % 6) + 3, rand() % 1280 - 90, 357, 30 };
	obstacle[0][3][0] = { noll, (rand() % 6) + 2, rand() % 1280 - 90, 322, 30 };
	obstacle[0][4][0] = { noll, (rand() % 6) + 1, rand() % 1280 - 90, 287, 30 };
	obstacle[1][0][0] = { noll, (rand() % 6) + 5, rand() % 1280 - 90, 217, 90 };
	obstacle[1][1][0] = { noll , (rand() % 6) + 5, rand() % 1280 - 90, 182 , 90 };
	obstacle[1][2][0] = { noll, (rand() % 6) + 5, rand() % 1280 - 90, 147 , 180 };
	obstacle[1][3][0] = { noll, (rand() % 6) + 5, rand() % 1280 - 90, 112 , 60 };
	obstacle[1][4][0] = { noll , (rand() % 6) + 5, rand() % 1280 - 90, 77, 135 };

	//initialize game level
	if (load > 0)
	{
		gamefile file;
		if (!loadetap(&file, load)) return key;
		nums = file.nums;
		health = file.health;
		frog.pos = file.frog;
		maxTime = file.maxTime;
		for (int i = 0; i < 5; i++)
		{
			obstacle[0][i][0].pos = file.cars[i].pos;
			obstacle[1][i][0].pos = file.obst[i].pos;
			obstacle[0][i][0].action = file.cars[i].rand;
			obstacle[1][i][0].action = file.obst[i].rand;
		}
	}

	init(obstacle, nums);

	object lostfrog = { frog.image, 0, obstacle[1][1][2].pos.x - 30, obstacle[1][1][2].pos.y, 30 };
	t1 = 0;

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;

	//load game from file(if game is saved)
	worldTime = save.worldTime;
	health = save.health;
	bee.seed = save.beeseed;
	for (int i = 0; i < 5; i++)
	{
		frogend[i] = save.frogend[i];
		obstacle[1][i][0].pos = save.obstacles[i].pos;
		obstacle[0][i][0].pos = save.cars[i].pos;
		obstacle[0][i][0].action = save.cars[i].rand;
		obstacle[1][i][0].action = save.obstacles[i].rand;
	}
	score = save.score;
	lostfrog.pos = save.lostfrog.pos;
	lostfrog.action = save.lostfrog.action;
	temp = save.lostfrog.rand;
	frog.pos = save.frog;
	init(obstacle, nums);

	//game loop
	while (!exit) {
			t2++;
			delta = ((double)t2 - (double)t1) * 0.001;
			t1 = t2;

			worldTime += delta;

			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < nums.woods[i]; j++)
				{
					if (t2 % obstacle[1][i][j].action == 0)
					{
						moveobs(&obstacle[1][i][j], obstacle[1][i][j].pos.y % 2);
						if (frogonobstacle(frog, obstacle[1][i][j]))
							moveobs(&frog, obstacle[1][i][j].pos.y % 2);
					}
				}
			}

			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < nums.cars[i]; j++)
				{
					if (t2 % obstacle[0][i][j].action == 0)
						moveobs(&obstacle[0][i][j], obstacle[0][i][j].pos.y % 2);
				}
			}

			movefrogbot(&frog, frog.action);

			//end conditions, calculating scores etc
			if (frog.pos.y < 252 && frog.pos.y >42 && !checkobstacle(frog, obstacle, nums))
			{
				health--;
			}
			else if (checkcars(frog, obstacle, nums) || frog.pos.x < -10 || frog.pos.x > 650)
			{
				health--;
			}
			else if (worldTime > maxTime)
			{
				health--;
			}
			else if (frog.pos.y == 42)
			{

				if (frog.pos.x < 340 && frog.pos.x > 300 && frogend[2] != 1)
				{
					frogend[2] = 1;
				}
				else if (frog.pos.x < 202 && frog.pos.x > 162 && frogend[1] != 1)
				{
					frogend[1] = 1;
				}
				else if (frog.pos.x < 64 && frog.pos.x > 24 && frogend[0] != 1)
				{
					frogend[0] = 1;
				}
				else if (frog.pos.x < 478 && frog.pos.x > 438 && frogend[3] != 1)
				{
					frogend[3] = 1;
				}
				else if (frog.pos.x < 616 && frog.pos.x > 576 && frogend[4] != 1)
				{
					frogend[4] = 1;
				}
				else
					health--;
			}

			//bot
			if (!frog.action)
			{
				if (health != save.health || key.before.pos==400 || key.after.pos==400)
				{
					worldTime = save.worldTime;
					health = save.health;
					for (int i = 0; i < 5; i++)
					{
						frogend[i] = save.frogend[i];
						obstacle[1][i][0].pos = save.obstacles[i].pos;
						obstacle[0][i][0].pos = save.cars[i].pos;
						obstacle[0][i][0].action = save.cars[i].rand;
						obstacle[1][i][0].action = save.obstacles[i].rand;
					}
					frog.pos = save.frog;
					frog.action = 0;
					init(obstacle, nums);
					if (key.before.pos == 400 || key.flag!=1)
						key.before.pos = 0;
					key.after.pos = 0;
				}
				for (int i = 0; i < 5; i++)
					if (frogend[i] != save.frogend[i])
					{
						for (int i = 0; i < 2; i++)
							for (int j = 0; j < 5; j++)
								free(obstacle[i][j]);
						return key;
					}
				if (((frog.pos.y - 77) % 35 == 0) && (frog.action == 0)) {
					frog.action = rand() % 4 + 1;
					if (frog.action == 2)
						frog.action = STOP;
					if (frog.pos.y <= 252)
					{
						key.flag = 1;
						key.after.values[key.after.pos] = frog.action;
						key.after.pos++;
					}
					else if (key.flag != 1)
					{
						key.before.values[key.before.pos] = frog.action;
						key.before.pos++;
					}
					else
					{
						frog.action = key.before.values[gotohalf++];
					}
					if (gotohalf == key.before.pos)
					{
						gotohalf = 0;
					}
					if (numofgames(0) == 1000000)
					{
						numofgames(1);
						key.after.pos = 0;
						return key;
					}
				}
			}
			frames++;
		}
		return key;
	}

//game, returns win/lose
//gets window parameters, level(if should be loaded), starting points and checks if the game should be loaded from file
bool game(scr d, int load, int startpts, bool savedgame){

	//initializing variables
	saved save = { 0 };
	if (savedgame)
	{
		if (loadgame(&save) == 0)
			return 0;
		load = save.etap;
	}
	int positionbot = 0;
	int t1, t2=0, quit, frames;
	double delta, worldTime, fpsTimer, fps;
	bool frogend[5] = { 0,0,0,0,0 };
	SDL_Surface* frogimg = SDL_LoadBMP("../Frogger/photos/frogend.bmp");
	SDL_Surface* timeimg[2] = { SDL_LoadBMP("../Frogger/photos/time.bmp"),
		SDL_LoadBMP("../Frogger/photos/time2.bmp") };
	SDL_Surface* caranim[2] = { SDL_LoadBMP("../Frogger/photos/Autoanim.bmp"),
		SDL_LoadBMP("../Frogger/photos/Autoanim2.bmp") };
	bool pause = 0, exit = 0, gameover = 0, flag = 1, bot = 0;
	int health = 5, maxTime = 50, temp = rand() % 300;
	bee bee = { SDL_LoadBMP("../Frogger/photos/bee.bmp"), rand() % 5 };
	score score = { 0,0,0,0,0,0 };
	score.total = startpts;
	SDL_Event event;
	SDL_Surface* plansza;
	object frog = { SDL_LoadBMP("../Frogger/photos/eti.bmp"), 0, SCREEN_WIDTH / 2, 462, 30 };
	obstacle_number nums = { 7, 6, 5, 4, 3, 7, 7, 5, 9, 6 };
	object* obstacle[2][5];
	keys key;
	
	//loading number of obstacles in each row
	if (load > 0)
	{
		gamefile file;
		if (!loadetap(&file, load)) return 0;
		nums = file.nums;
	}

	//allocating memory for obstacles in each row
	for (int i = 0; i < 5; i++)
	{
		obstacle[0][i] = (object*)malloc(nums.cars[i] * sizeof(object));
		if (obstacle[0][i] == NULL)
			return 0;
		obstacle[1][i] = (object*)malloc(nums.woods[i] * sizeof(object));
		if (obstacle[1][i] == NULL)
			return 0;
	}

	//initializing first obstacle of each row(as every other is different)
		obstacle[0][0][0] = { SDL_LoadBMP("../Frogger/photos/Auto.bmp"), (rand() % 6) + 5, rand() % 1280 - 90, 427, 30 };
		obstacle[0][1][0] = { SDL_LoadBMP("../Frogger/photos/Auto2.bmp"), (rand() % 6) + 4, rand() % 1280 - 90, 392, 30 };
		obstacle[0][2][0] = { SDL_LoadBMP("../Frogger/photos/Auto3.bmp"), (rand() % 6) + 3, rand() % 1280 - 90, 357, 30 };
		obstacle[0][3][0] = { SDL_LoadBMP("../Frogger/photos/Auto4.bmp"), (rand() % 6) + 2, rand() % 1280 - 90, 322, 30 };
		obstacle[0][4][0] = { SDL_LoadBMP("../Frogger/photos/Auto5.bmp"), (rand() % 6) + 1, rand() % 1280 - 90, 287, 30 };
		obstacle[1][0][0] = { SDL_LoadBMP("../Frogger/photos/turtle90.bmp"), (rand() % 6) + 5, rand() % 1280 - 90, 217, 90 };
		obstacle[1][1][0] = { SDL_LoadBMP("../Frogger/photos/obstacle90.bmp"), (rand() % 6) + 5, rand() % 1280 - 90, 182 , 90 };
		obstacle[1][2][0] = { SDL_LoadBMP("../Frogger/photos/obstacle180.bmp"), (rand() % 6) + 5, rand() % 1280 - 90, 147 , 180 };
		obstacle[1][3][0] = { SDL_LoadBMP("../Frogger/photos/turtle60.bmp"), (rand() % 6) + 5, rand() % 1280 - 90, 112 , 60 };
		obstacle[1][4][0] = { SDL_LoadBMP("../Frogger/photos/obstacle135.bmp"), (rand() % 6) + 5, rand() % 1280 - 90, 77, 135 };

		//initialize game level
		if (load > 0)
		{
			gamefile file;
			if (!loadetap(&file, load)) return 0;
			nums = file.nums;
			health = file.health;
			frog.pos = file.frog;
			maxTime = file.maxTime;
			for (int i = 0; i < 5; i++)
			{
				obstacle[0][i][0].pos = file.cars[i].pos;
				obstacle[1][i][0].pos = file.obst[i].pos;
				obstacle[0][i][0].action = file.cars[i].rand;
				obstacle[1][i][0].action = file.obst[i].rand;
			}
		}

	init(obstacle, nums);

	//pattern for animations
	SDL_Surface* pattern[10] = {
	SDL_LoadBMP("../Frogger/photos/turtle60.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle601.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle602.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle603.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle604.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle90.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle901.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle902.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle903.bmp"),
	SDL_LoadBMP("../Frogger/photos/turtle904.bmp")
	};

	SDL_Surface* hp = SDL_LoadBMP("../Frogger/photos/hp.bmp");
	SDL_Surface* menu[4] = {
		SDL_LoadBMP("../Frogger/photos/Pause.bmp"),
		SDL_LoadBMP("../Frogger/photos/Quit.bmp"),
		SDL_LoadBMP("../Frogger/photos/GameOver.bmp"),
		SDL_LoadBMP("../Frogger/photos/Win.bmp")
	};

	object lostfrog = { frog.image, 0, obstacle[1][1][2].pos.x - 30, obstacle[1][1][2].pos.y, 30 };

	//load game map
	plansza = SDL_LoadBMP("../Frogger/photos/plansza.bmp");
	if (plansza == NULL) {
		printf("SDL_LoadBMP(plansza.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(d.charset);
		SDL_FreeSurface(d.screen);
		SDL_DestroyTexture(d.scrtex);
		SDL_DestroyWindow(d.window);
		SDL_DestroyRenderer(d.renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];


	//colors
	int bialy = SDL_MapRGB(d.screen->format, 0xFF, 0xFF, 0xFF);
	int czarny = SDL_MapRGB(d.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(d.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(d.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(d.screen->format, 0x11, 0x11, 0xCC);

	//set background to be invisible
	SDL_SetColorKey(menu[0], SDL_TRUE, bialy);
	SDL_SetColorKey(menu[1], SDL_TRUE, bialy);
	SDL_SetColorKey(menu[2], SDL_TRUE, bialy);
	SDL_SetColorKey(menu[3], SDL_TRUE, bialy);
	SDL_SetColorKey(hp, SDL_TRUE, bialy);
	SDL_SetColorKey(frogimg, SDL_TRUE, bialy);
	for (int i = 0; i < 5; i++)
		SDL_SetColorKey(obstacle[0][i][0].image, SDL_TRUE, bialy);
	SDL_SetColorKey(bee.image, SDL_TRUE, bialy);
	SDL_SetColorKey(caranim[0], SDL_TRUE, bialy);
	SDL_SetColorKey(caranim[1], SDL_TRUE, bialy);


	//information
	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;

	//load game from file(if game is saved)
	if (savedgame)
	{
		worldTime = save.worldTime;
		health = save.health;
		bee.seed = save.beeseed;
		for (int i = 0; i < 5; i++)
		{
			frogend[i] = save.frogend[i];
			obstacle[1][i][0].pos=save.obstacles[i].pos;
			obstacle[0][i][0].pos = save.cars[i].pos;
			obstacle[0][i][0].action = save.cars[i].rand;
			obstacle[1][i][0].action = save.obstacles[i].rand;
		}
		score = save.score;
		lostfrog.pos=save.lostfrog.pos;
		lostfrog.action = save.lostfrog.action;
		temp=save.lostfrog.rand;
		frog.pos = save.frog;
		init(obstacle, nums);
	}

	//game loop
	while (!exit) {

		if (!pause && flag==1)
		{
			t2++;
			delta = ((double)t2 - (double)t1) * 0.001;
			t1 = t2;

			worldTime += delta;

			SDL_FillRect(d.screen, NULL, niebieski);
			DrawSurface(d.screen, plansza, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			
			//move and print woods/frogs and frog if on any of it
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < nums.woods[i]; j++)
				{
					DrawSurface(d.screen, obstacle[1][i][j].image, obstacle[1][i][j].pos.x, obstacle[1][i][j].pos.y);
					if (t2 % obstacle[1][i][j].action == 0)
					{
						moveobs(&obstacle[1][i][j], obstacle[1][i][j].pos.y % 2);
						if (frogonobstacle(frog, obstacle[1][i][j]))
							moveobs(&frog, obstacle[1][i][j].pos.y % 2);
					}
				}
			}

			//move and print cars
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < nums.cars[i]; j++)
				{
					DrawSurface(d.screen, obstacle[0][i][j].image, obstacle[0][i][j].pos.x, obstacle[0][i][j].pos.y);
					
					if ((t2 / 600) % 2)
						DrawSurface(d.screen, caranim[0], obstacle[0][i][j].pos.x, obstacle[0][i][j].pos.y);
					else if ((t2 / 300) % 2)
						DrawSurface(d.screen, caranim[1], obstacle[0][i][j].pos.x, obstacle[0][i][j].pos.y);

					if (t2 % obstacle[0][i][j].action == 0)
						moveobs(&obstacle[0][i][j], obstacle[0][i][j].pos.y % 2);
				}
			}

			//some turtles animation
			turtles(&obstacle[1][3][0], 0, pattern);
			turtles(&obstacle[1][0][0], 1, pattern);

			//time bar
			DrawSurface(d.screen, (int)worldTime < (maxTime-10) ? timeimg[0] : timeimg[1], 320 + (int)(worldTime * (640/maxTime)), SCREEN_HEIGHT - 5);

			//frog move
			DrawSurface(d.screen, frog.image, frog.pos.x, frog.pos.y);
			movefrog(&frog, frog.action);

			//draw bee(score)
			if (!score.bee)
				DrawSurface(d.screen, bee.image, 44 + 138 * bee.seed, 42);

			//draw and move lost frog
			if (lostfrog.action != 8)
			{
				DrawSurface(d.screen, lostfrog.image, lostfrog.pos.x, lostfrog.pos.y);
				if (t2 % obstacle[1][1][2].action == 0)
					lostfrog.pos.x++;
				if (obstacle[1][1][2].pos.x == 0)
					lostfrog.pos.x = -30;
				if (obstacle[1][1][2].pos.x == temp)
				{
					lostfrog.action = RIGHT;
					lostfrog.image = SDL_LoadBMP("../Frogger/photos/Prawo.bmp");
				}
				if (obstacle[1][1][2].pos.x == 300 + temp && lostfrog.pos.x > obstacle[1][1][2].pos.x)
				{
					lostfrog.action = LEFT;
					lostfrog.image = SDL_LoadBMP("../Frogger/photos/Lewo.bmp");
				}
				movelostfrog(&lostfrog, lostfrog.action);
			}

			//check if lost frog is taken by frog
			if (frogonobstacle(frog,lostfrog))
			{
				lostfrog.action = 8;
				lostfrog.pos = frog.pos;
				score.frog = 200;
			}

			//printing end points and win condition
			flag = 0;
			for (int i = 0; i < 5; i++)
				if (frogend[i] == 1)
					DrawSurface(d.screen, frogimg, 44 + 138 * i, 42);
				else
					flag = 1;

			//print frog health
			for (int i = 0, y = 120; i < health; i++, y += 30)
			{
				DrawSurface(d.screen, hp, 15, y);
			}

			//position score
			if ((abs(frog.pos.y - 462)) / 35 * 10 > score.pos)
				score.pos = (abs(frog.pos.y - 462)) / 35 * 10;

			//print bonus on frog if got to end
			if (score.flag && worldTime < 4)
			{
				sprintf(text, "%d", 200 * score.flag);
				DrawString(d.screen, frog.pos.x - (int)strlen(text) * 8 / 2, frog.pos.y - 20, text, d.charset);
			}
			else
				score.flag = 0;

			//end conditions, calculating scores etc
			if (frog.pos.y < 252 && frog.pos.y >42 && !checkobstacle(frog, obstacle, nums))
			{
				health--,score.frog = 0;
				frog = { SDL_LoadBMP("../Frogger/photos/eti.bmp"), 0, SCREEN_WIDTH / 2, 462 };
				worldTime = 0;
			}
			else if (checkcars(frog, obstacle, nums) || frog.pos.x < -10 || frog.pos.x > 650)
			{
				frog = { SDL_LoadBMP("../Frogger/photos/eti.bmp"), 0, SCREEN_WIDTH / 2, 462 };
				health--, score.frog = 0;
				worldTime = 0;
			}
			else if (worldTime > maxTime)
			{
				frog = { SDL_LoadBMP("../Frogger/photos/eti.bmp"), 0, SCREEN_WIDTH / 2, 462 };
				health--, score.frog = 0;
				worldTime = 0;
			}
			else if (frog.pos.y == 42)
			{

				if (frog.pos.x < 340 && frog.pos.x > 300 && frogend[2] != 1)
				{
					frogend[2] = 1;
					score.end += (maxTime - (int)worldTime) * 10 + 50;
					score.total += score.pos + score.frog;
					score.pos = 0;
				}
				else if (frog.pos.x < 202 && frog.pos.x > 162 && frogend[1] != 1)
				{
					frogend[1] = 1;
					score.end += (maxTime - (int)worldTime) * 10 + 50;
					score.total += score.pos + score.frog;
					score.pos = 0;
				}
				else if (frog.pos.x < 64 && frog.pos.x > 24 && frogend[0] != 1)
				{
					frogend[0] = 1;
					score.end += (maxTime - (int)worldTime) * 10 + 50;
					score.total += score.pos + score.frog;
					score.pos = 0;
				}
				else if (frog.pos.x < 478 && frog.pos.x > 438 && frogend[3] != 1)
				{
					frogend[3] = 1;
					score.end += (maxTime - (int)worldTime) * 10 + 50;
					score.total += score.pos + score.frog;
					score.pos = 0;
				}
				else if (frog.pos.x < 616 && frog.pos.x > 576 && frogend[4] != 1)
				{
					frogend[4] = 1;
					score.end += (maxTime - (int)worldTime) * 10 + 50;
					score.total += score.pos + score.frog;
					score.pos = 0;
				}
				else
					health--, score.frog = 0;
				frog = { SDL_LoadBMP("../Frogger/photos/eti.bmp"), 0, SCREEN_WIDTH / 2, 462 };

				if (frogend[bee.seed] == 1)
				{
					score.flag = 1;
					score.bee = 200;
					bee.seed = 8;
				}

				if (score.frog)
					score.flag++;
				score.frog = 0;
				worldTime = 0;
			}

			if (health == 0)
			{
				quit = 1;
				pause = 1;
				gameover = 1;
				health = 5;
				for (int i = 0; i < 5; i++) frogend[i] = 0;
				score.total += score.end + score.pos + score.bee;
				score.end = 0;
				score.pos = 0;
				score.bee = 0;
			}

			//fps(information)
			fpsTimer += delta;
			if (fpsTimer > 0.5) {
				fps = (double)frames * 2;
				frames = 0;
				fpsTimer -= 0.5;
			};

			//information box
			sprintf(text, "Esc - wyjscie, czas trwania = %.1lf s  %.0lf klatek / s, ilosc pkt: %d", worldTime, fps, score.pos + score.total + score.end + score.bee);
			DrawString(d.screen, d.screen->w / 2 - (int)strlen(text) * 8 / 2, 10, text, d.charset);
		}

		//end of game
		if (flag == 0)
		{
			char name[64]=" ";
			score.total += score.end + score.bee + score.pos;
			score.end = 0;
			score.bee = 0;
			score.pos = 0;
			transport(score.total, 1);
			if (load == countetaps())
			{
				int i = 0;
				SDL_StartTextInput();
				while (event.key.keysym.sym != SDLK_RETURN)
				{
					while (SDL_PollEvent(&event))
					{
						if (event.type == SDL_TEXTINPUT && i != 63)
						{
							if (*event.text.text != ' ')
							{
								name[i] = *event.text.text;
								i++;
								name[i] = '\0';
							}
						}
					}
					sprintf(text, "Score: %d, Input name: %s", score.total, name);
					DrawString(d.screen, 220, SCREEN_HEIGHT / 2, text, d.charset);
					SDL_UpdateTexture(d.scrtex, NULL, d.screen->pixels, d.screen->pitch);
					SDL_RenderCopy(d.renderer, d.scrtex, NULL, NULL);
					SDL_RenderPresent(d.renderer);
					DrawSurface(d.screen, menu[3], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3);
				}
				SDL_StopTextInput();
				if (i)
					highscore(score.total, name);
			}
		}
		else if (gameover)
		{
			char name[64]=" ";
			int i = 0;
			SDL_StartTextInput();
			while (event.key.keysym.sym != SDLK_RETURN)
			{
				while (SDL_PollEvent(&event))
				{
					if (event.type == SDL_TEXTINPUT && i != 63)
					{
						if (*event.text.text != ' ')
						{
							name[i] = *event.text.text;
							i++;
							name[i] = '\0';
						}
					}
				}
					sprintf(text, "Score: %d, Input name: %s", score.total, name);
					DrawString(d.screen, 220, SCREEN_HEIGHT / 2, text, d.charset);
					SDL_UpdateTexture(d.scrtex, NULL, d.screen->pixels, d.screen->pitch);
					SDL_RenderCopy(d.renderer, d.scrtex, NULL, NULL);
					SDL_RenderPresent(d.renderer);
					DrawSurface(d.screen, menu[2], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3);
			}
			SDL_StopTextInput();
			if (i)
				highscore(score.total, name);
		}
		else if (quit)
			DrawSurface(d.screen, menu[1], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), t1 = SDL_GetTicks();
		else if (pause)
			DrawSurface(d.screen, menu[0], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), t1 = SDL_GetTicks();

		//print screen
			SDL_UpdateTexture(d.scrtex, NULL, d.screen->pixels, d.screen->pitch);
			SDL_RenderCopy(d.renderer, d.scrtex, NULL, NULL);
			SDL_RenderPresent(d.renderer);
			SDL_SetColorKey(frog.image, SDL_TRUE, bialy);
			SDL_SetColorKey(lostfrog.image, SDL_TRUE, bialy);
			

			//bot
			/*if (bot)
			{
				if (!frog.action)
				{
					
					if (frog.pos.y == 462) {
						if (distance(frog.pos.x, obstacle[0][0], nums.cars[0]) == 1)
							frog.action = UP;
					}
					else if (frog.pos.y == 427) {
						if (distance(frog.pos.x, obstacle[0][1], nums.cars[1]) == 1)
							frog.action = UP;
						else if (obstacle[0][0][0].pos.x - 70 == frog.pos.x)
						{
							if (frog.pos.x != 44)
								frog.action = LEFT;
							else
								frog.action = DOWN;
						}
					}
					else if (frog.pos.y == 392) {
						if (distance(frog.pos.x, obstacle[0][2], nums.cars[2]) == 1)
							frog.action = UP;
						else if (obstacle[0][1][nums.cars[1] - 1].pos.x + 70 == frog.pos.x)
						{
							if (frog.pos.x != 596)
								frog.action = RIGHT;
							else
								frog.action = DOWN;
						}
					}
					else if (frog.pos.y == 357) {
						if (distance(frog.pos.x, obstacle[0][3], nums.cars[3]) == 1)
							frog.action = UP;
						else if (obstacle[0][2][0].pos.x - 70 == frog.pos.x)
						{
							if (frog.pos.x != 44)
								frog.action = LEFT;
							else
								frog.action = DOWN;
						}
					}
					else if (frog.pos.y == 322) {
						if (distance(frog.pos.x, obstacle[0][4], nums.cars[4]) == 1)
							frog.action = UP;
						else if (obstacle[0][3][nums.cars[3] - 1].pos.x + 70 == frog.pos.x)
						{
							if (frog.pos.x != 596)
								frog.action = RIGHT;
							else
								frog.action = DOWN;
						}
					}
					else if (frog.pos.y == 287)
						frog.action = UP;
					else if (frog.pos.y == 252) {
						if (frog.pos.x < 320)
							frog.action = RIGHT;
						else if (distancewood(frog.pos.x, obstacle[1][0], nums.woods[0]) && abs(frog.pos.x - obstacle[1][0][0].pos.x) > 40)
							frog.action = UP;
					}
					else if (frog.pos.y == 217) {
						if (distancewood(frog.pos.x, obstacle[1][1], nums.woods[1]) && frog.pos.x < 320)
							frog.action = UP;
					}
					else if (frog.pos.y == 182) {
						if (distancewood(frog.pos.x, obstacle[1][2], nums.woods[2]) && frog.pos.x > 320)
							frog.action = UP;
					}
					else if (frog.pos.y == 147) {
						if (distancewood(frog.pos.x, obstacle[1][3], nums.woods[3]) && frog.pos.x < 320 && abs(frog.pos.x - obstacle[1][3][0].pos.x)>40)
							frog.action = UP;
					}
					else if (frog.pos.y == 112) {
						if ((distancewood(frog.pos.x, obstacle[1][4], nums.woods[4]) && !frogend[4])||frog.pos.x>630)
						{
							if (frog.pos.x > 610)
								frog.action = UP;
						}
						else if (distancewood(frog.pos.x, obstacle[1][4], nums.woods[4]) && !frogend[3])
						{
							if (frog.pos.x > 482)
								frog.action = UP;
						}
						else if (distancewood(frog.pos.x, obstacle[1][4], nums.woods[4]) && !frogend[2])
						{
							if (frog.pos.x > 344)
								frog.action = UP;
						}
						else if (distancewood(frog.pos.x, obstacle[1][4], nums.woods[4]) && !frogend[1])
						{
							if (frog.pos.x > 206)
								frog.action = UP;
						}
						else if (distancewood(frog.pos.x, obstacle[1][4], nums.woods[4]) && !frogend[0])
							if (frog.pos.x > 68)
								frog.action = UP;
					}
					else if (frog.pos.y == 77) {
						if (596 - frog.pos.x<20 && 596 - frog.pos.x>-20 && !frogend[4])
							frog.action = UP;
						else if (458 - frog.pos.x<10 && 458 - frog.pos.x>-20 && !frogend[3])
							frog.action = UP;
						else if (320 - frog.pos.x<10 && 320 - frog.pos.x>-20 && !frogend[2])
							frog.action = UP;
						else if (182 - frog.pos.x<10 && 182 - frog.pos.x>-20 && !frogend[1])
							frog.action = UP;
						else if (44 - frog.pos.x<10 && 44 - frog.pos.x>-20 && !frogend[0])
							frog.action = UP;
					}
				}
			}*/




			// handling of events (if there were any)
			while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_KEYDOWN:
						if (flag && event.key.keysym.sym == SDLK_ESCAPE) quit = 1, pause = 1;
						else if (event.key.keysym.sym == SDLK_p && quit != 1) pause = !pause;
						else if (quit == 1 && event.key.keysym.sym == SDLK_y) exit = 1;
						else if (quit == 1 && event.key.keysym.sym == SDLK_n) (gameover == 1 ? score.total = 0 : NULL), quit = 0, pause = 0, gameover = 0;
						else if ((!flag || gameover) && event.key.keysym.sym == SDLK_RETURN) exit = 1;
						else if (event.key.keysym.sym == SDLK_1) frogend[0] = 1; //hack
						else if (event.key.keysym.sym == SDLK_2) frogend[1] = 1; //hack
						else if (event.key.keysym.sym == SDLK_3) frogend[2] = 1; //hack
						else if (event.key.keysym.sym == SDLK_4) frogend[3] = 1; //hack
						else if (event.key.keysym.sym == SDLK_5) frogend[4] = 1; //hack
						else if (event.key.keysym.sym == SDLK_x) {
							bot = !bot; //bot
							if (bot) {
								positionbot = 0;
								save.beeseed = bee.seed;
								for (int i = 0; i < 5; i++)
								{
									save.cars[i].pos = obstacle[0][i][0].pos;
									save.cars[i].rand = obstacle[0][i][0].action;
								}
								for (int i = 0; i < 5; i++)
								{
									save.obstacles[i].pos = obstacle[1][i][0].pos;
									save.obstacles[i].rand = obstacle[1][i][0].action;
								}
								save.frog = frog.pos;
								save.score = score;
								for (int i = 0; i < 5; i++)
									save.frogend[i] = frogend[i];
								save.health = health;
								save.lostfrog.pos = lostfrog.pos;
								save.lostfrog.rand = temp;
								save.lostfrog.action = lostfrog.action;
								save.worldTime = worldTime;
								save.etap = load;
								savegame(save);
								numofgames(1);
								key = simgame(save);
								t2 = 1;
							}
						}
						else if (event.key.keysym.sym == SDLK_z) //save
						{
							save.beeseed = bee.seed;
							for (int i = 0; i < 5; i++)
							{
								save.cars[i].pos = obstacle[0][i][0].pos;
								save.cars[i].rand = obstacle[0][i][0].action;
							}
							for (int i = 0; i < 5; i++)
							{
								save.obstacles[i].pos = obstacle[1][i][0].pos;
								save.obstacles[i].rand = obstacle[1][i][0].action;
							}
							save.frog = frog.pos;
							save.score = score;
							for (int i = 0; i < 5; i++)
								save.frogend[i] = frogend[i];
							save.health = health;
							save.lostfrog.pos = lostfrog.pos;
							save.lostfrog.rand = temp;
							save.lostfrog.action = lostfrog.action;
							save.worldTime = worldTime;
							save.etap = load;
							savegame(save);
						}
						else if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) && frog.action == 0 && frog.pos.y != 42)
						{
							frog.action = UP;
							frog.image = SDL_LoadBMP("../Frogger/photos/Gora.bmp");
							SDL_SetColorKey(frog.image, SDL_TRUE, bialy);
						}
						else if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) && frog.action == 0 && frog.pos.y != 462)
						{
							frog.action = DOWN;
							frog.image = SDL_LoadBMP("../Frogger/photos/Dol.bmp");
							SDL_SetColorKey(frog.image, SDL_TRUE, bialy);
						}
						else if ((event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) && frog.action == 0 && frog.pos.x != 596)
						{
							frog.action = RIGHT;
							frog.image = SDL_LoadBMP("../Frogger/photos/Prawo.bmp");
							SDL_SetColorKey(frog.image, SDL_TRUE, bialy);
						}
						else if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) && frog.action == 0 && frog.pos.x != 44)
						{
							frog.action = LEFT;
							frog.image = SDL_LoadBMP("../Frogger/photos/Lewo.bmp");
							SDL_SetColorKey(frog.image, SDL_TRUE, bialy);
						}
						break;
					case SDL_QUIT:
						exit = 1;
						break;
					};
			}
			if (bot)
			{
				if (((frog.pos.y - 77) % 35 == 0) && (frog.action == 0)) {
					if (positionbot < key.before.pos)
					{
						frog.action = key.before.values[positionbot];
						positionbot++;
					}
					else if (positionbot >= key.before.pos && positionbot-key.before.pos < key.after.pos)
					{
						frog.action = key.after.values[positionbot-key.before.pos];
						positionbot++;
					}
				}
			}
			frames++;
			}
			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 5; j++)
					free(obstacle[i][j]);
			return !flag;
}



#ifdef __cplusplus
extern "C"
#endif


int main(int argc, char** argv) {

	//init of window and menu
	int rc;
	scr d;
	SDL_Event event;
	SDL_Surface* menu = SDL_LoadBMP("../Frogger/photos/menu/menu.bmp");
	bool exit = 0;
	short int pos = 0;
	int etapy = countetaps();
	SDL_Surface* menupos[5] = {
		SDL_LoadBMP("../Frogger/photos/menu/PlayGame.bmp"),
		SDL_LoadBMP("../Frogger/photos/menu/LoadGame.bmp"),
		SDL_LoadBMP("../Frogger/photos/menu/HighScores.bmp"),
		SDL_LoadBMP("../Frogger/photos/menu/QuitGame.bmp"),
		SDL_LoadBMP("../Frogger/photos/menu/HighScore.bmp") };

	srand((unsigned int)time(NULL));

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	//fullscreen mode
	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	//                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&d.window, &d.renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(d.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(d.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(d.window, "Wojciech Wicki Frogger");


	d.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	d.scrtex = SDL_CreateTexture(d.renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	//make cursor invisible
	SDL_ShowCursor(SDL_DISABLE);

	//load charset
	d.charset = SDL_LoadBMP("../Frogger/photos/cs8x8.bmp");
	if (d.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(d.screen);
		SDL_DestroyTexture(d.scrtex);
		SDL_DestroyWindow(d.window);
		SDL_DestroyRenderer(d.renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(d.charset, true, 0x000000);

	//menu loop
	while (!exit)
	{
		DrawSurface(d.screen, menu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		for (int i = 0; i < 4; i++)
			DrawSurface(d.screen, menupos[i], SCREEN_WIDTH / 2, 180 + i * 70), SDL_SetColorKey(menupos[i], false, 0x22B14C);
		SDL_SetColorKey(menupos[pos], true, 0x22B14C);
		SDL_UpdateTexture(d.scrtex, NULL, d.screen->pixels, d.screen->pitch);
		//		SDL_RenderClear(renderer);
		SDL_RenderCopy(d.renderer, d.scrtex, NULL, NULL);
		SDL_RenderPresent(d.renderer);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
				{
					if (pos != 0)
						pos--;
				}
				else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
				{
					if (pos != 3)
						pos++;
				}
				else if (event.key.keysym.sym == SDLK_RETURN)
				{
					//start game
					if (pos == 0)
					{
						//game(d, 0, 0); <- random game
						for (int i = 1; i <= etapy; i++)
						{
							if (!game(d, i, transport(0, 0), 0))
								break;
						}
						transport(0, 1);
					}
					//load game
					else if (pos == 1)
					{
						bool flag = 1;
						for (int i = savedetap(); i <= etapy; i++)
						{
							if (!game(d, i, transport(0, 0), flag))
								break;
							flag = 0;
						}
						transport(0, 1);

					}
					//highscore
					else if (pos == 2)
					{
						char text[128];
						fileline* players = highscoreplayers();
						DrawSurface(d.screen, menupos[4], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
						for (int i = 0; i < 10; i++)
						{
							sprintf(text, "%s", players[i].name);
							DrawString(d.screen, 160, 117+i*35, text, d.charset);
							sprintf(text, "%d", players[i].score);
							DrawString(d.screen, 480, 117+i*35, text, d.charset);
						}
						SDL_UpdateTexture(d.scrtex, NULL, d.screen->pixels, d.screen->pitch);
						SDL_RenderCopy(d.renderer, d.scrtex, NULL, NULL);
						SDL_RenderPresent(d.renderer);		
						bool done = 0;
						while (!done)
							while (SDL_PollEvent(&event))
								if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN))
									done = 1;
					}
					//exit
					else if (pos == 3)
						exit = 1;
				}
				break;
			case SDL_QUIT:
				exit = 1;
				break;
			};
		}
	}
	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(d.charset);
	SDL_FreeSurface(d.screen);
	SDL_DestroyTexture(d.scrtex);
	SDL_DestroyRenderer(d.renderer);
	SDL_DestroyWindow(d.window);

	SDL_Quit();
	return 0;
	};