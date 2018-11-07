/* This file contain all creation and generation functions and structs of Obstacles and Powerups*/
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

/* Structs Definitions */

struct Obstacle {
	int id;
	bool on = false;
	double current_down_x;
	double current_down_y;
	int type;
	float hit;
	float width, height;
	double real_width, real_height;
}obstacles[16];

struct Powerup {
	int id;
	bool on = false;
	double current_down_x;
	double current_down_y;
	int type;
	float hit;
	float width, height;
	double real_width, real_height;
}powerups[8];

// Constants and Related Variables
int OBSTACLES_MAX_INDEX = 15;
int obstacles_counter = 0;
int POWERUPS_MAX_INDEX = 7;
int powerups_counter = 0;

// --- Object Creators & Generators
bool generateRandomObject(void);
void generateRandomObstacle(void);
void generateRandomPowerup(void);

Obstacle* makeObstacle(double x, double y, int t, float w, float h);
Obstacle* makeObstacle(double x, double y, int t);
Powerup* makePowerup(double x, double y, int t, float w, float h);
Powerup* makePowerup(double x, double y, int t);

// This is the end of the header guard
#endif