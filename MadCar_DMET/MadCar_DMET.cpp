// MadCar_DMET.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "pch.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <iostream>
using namespace std;
#include <glut.h>

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

struct Car {
	double x_pos;
	double y_pos;
	double height;
	double width;
	double hp;
	bool alive;
	float acceleration;
	double registered_hp;
	double registered_acc;
} car;

/*	Methods Signatures */
void Display(void);
void Anim(void);
void animatePowerups(void);
void animateObstacles(void);
// --- Drawers
void drawTheCar(void);
void drawObstacles(void);
void drawWoodObstacle(Obstacle*);
void drawPowerups(void);
void drawPowerUpStrike(Powerup*);
void drawPowerUpBottle(Powerup*);
void drawHpBar(void);
void drawStatusPart(void);
void drawRoadLine(bool, double);
void drawRoadLines(void);
void drawRoadPlane(void);
void print(int, int, char *);
// --- Collision Detectors
bool carHit(Obstacle*);
bool carHit(Powerup*);
// --- Object Creators & Generators
bool generateRandomObject(void);
void generateRandomObstacle(void);
void generateRandomPowerup(void);
Obstacle* makeObstacle(double x, double y, int t, float w, float h);
Obstacle* makeObstacle(double x, double y, int t);
Powerup* makePowerup(double x, double y, int t, float w, float h);
Powerup* makePowerup(double x, double y, int t);
// --- Key Processors
void processSpecialKeys(int, int, int);
void processNormalKeys(unsigned char, int, int);
// --- Movers
void moveCar(int delta_x, int delta_y);
// --- Debugging Functions
void debugScreenLines(int x_splits, int y_splits);
// --- Other Helper Functions
double getLane(int);
void initConfig(void);
void initValues(void);
int getRandomValue(int u_v);
//-----------------

/*	Constants & Intial Values */
int WIN_W = 1080;
int WIN_H = 720;
float GLOBAL_SLOPE = -5;
double ROAD_START_Y_INIT = 20;
double SLIGHTLY_OFF_TOP_SCREEN = 4 * WIN_H / 3;
//const double SLIGHTLY_OFF_BOTTOM_SCREEN = - (WIN_H / 2);
int OBSTACLES_MAX_INDEX = 15;
int obstacles_counter = 0;
int POWERUPS_MAX_INDEX = 8;
int powerups_counter = 0;
bool DEBUGING_MODE = true;
bool VERBOSE = false;
// Speed Constraints
float DELTA_VELOCITY = 0.05;
float MAX_ACC = 1.0f;
float MIN_ACC = 0.5f;
float MAX_HP = 100;
// Global variables
double road_start_y;
int num_of_on_obstacles;
int num_of_on_powerups;
//-----------------

void Display(void)
{
	//glClearColor(0.1, 1, 1, 0.4f); // update the background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix(); // marking the initial state of the transformations matrix 

	/* Drawing the track */
	drawRoadPlane();
	drawRoadLines();

	/* Drawing Power-ups */
	drawPowerups();

	/* Drawing rock obstacles */
	drawObstacles();
	
	/* Drawing the CAR OUTLINE*/
	drawTheCar();
	
	/* Draw GAME-STATUS PRINTS*/
	drawStatusPart();
	drawHpBar();

	if(DEBUGING_MODE)
		debugScreenLines(22, 22);

	glPopMatrix(); //	returning to the initial state of the transformations matrix 
	//glutSwapBuffers();

	glFlush();
}

void debugObstaclesOutOfRange() {
	Powerup* pu;
	for (int pu_num = POWERUPS_MAX_INDEX; pu_num >= 0; pu_num--) {
		pu = &powerups[pu_num];
		if (pu->on)
		{
			double x = pu->current_down_x;
			double y = pu->current_down_y;
			float w = pu->width;
			float h = pu->height;
	
			if (x < -2) {
				printf("\n @debugger: Powerup bottle::\n id= %d,\nx= %f, y= %f,\nw= %f, h= %f\n",
					pu->id, x, y, w, h);
				if (x < -5)
					glutIdleFunc(NULL);
			}
		}
	}
}

void Anim()
{
	if (!car.alive) {
		glutIdleFunc(NULL);
		return;
	}
	if (road_start_y > - 9*WIN_H/16 )
	{
		road_start_y-= car.acceleration;
	}
	else
	{
		road_start_y = ROAD_START_Y_INIT;
	}

	//debugObstaclesOutOfRange();


	animatePowerups();
	animateObstacles();

	Powerup pu = powerups[powerups_counter];
	Obstacle ob = obstacles[obstacles_counter];
	if (pu.current_down_y < WIN_H && ob.current_down_y < WIN_H)
	{
		if (2 * num_of_on_powerups >= num_of_on_obstacles)
		{
			generateRandomObstacle();
		}
		else 
		{
			generateRandomPowerup();
			//generateRandomObject();
		}
	}

	glutPostRedisplay();
}

void animatePowerups() {
	double SLIGHTLY_OFF_BOTTOM_SCREEN = -(WIN_H / 2);
	//printf("\n\nSLIGHTLY_OFF_BOTTOM_SCREEN = %f", SLIGHTLY_OFF_BOTTOM_SCREEN);
	//printf("\n\nSLIGHTLY_OFF_TOP_SCREEN = %f\n", SLIGHTLY_OFF_TOP_SCREEN);

	Powerup* pu;
	for (int pu_num = POWERUPS_MAX_INDEX; pu_num >= 0; pu_num--) {
		pu = &powerups[pu_num];

		if (pu->on && ((pu->current_down_y) > SLIGHTLY_OFF_BOTTOM_SCREEN))
		{
			pu->current_down_y -= car.acceleration;
			if (carHit(pu)) {
				pu->on = false;
				if (pu->type == 1) {
					if (car.acceleration < MAX_ACC) {
						car.acceleration += (pu->hit)*MAX_ACC;
					}
				}
				else
				{
					if (car.hp + pu->hit <= MAX_HP) {
						car.hp += (pu->hit)*MAX_HP;
					}
				}
			}
		}
		else
		{
			//destroyPowerup(pu);
			pu->on = false;
			pu->id = -1;
			num_of_on_powerups--;
		}
	}
}

void animateObstacles() {
	double SLIGHTLY_OFF_BOTTOM_SCREEN = -(WIN_H / 2);
	//printf("\n\nSLIGHTLY_OFF_BOTTOM_SCREEN = %f", SLIGHTLY_OFF_BOTTOM_SCREEN);
	//printf("\n\nSLIGHTLY_OFF_TOP_SCREEN = %f\n", SLIGHTLY_OFF_TOP_SCREEN);

	Obstacle* ob;
	for (int ob_num = OBSTACLES_MAX_INDEX; ob_num >= 0; ob_num--) {
		ob = &obstacles[ob_num];

		if (ob->on && ((ob->current_down_y) > SLIGHTLY_OFF_BOTTOM_SCREEN))
		{
			ob->current_down_y -= car.acceleration;
			if (carHit(ob)) {
				ob->on = false;
				car.hp -= (ob->hit)*MAX_HP;

				if (car.acceleration >= MIN_ACC) {
					car.acceleration -= (ob->hit)*MAX_ACC;
				}

				if (car.hp <= 0) {
					car.alive = false;
				}
			}
		}
		else
		{
			ob->on = false;
			ob->id = -1;
			num_of_on_obstacles--;
		}
	}

}

/* Collision Detectors*/
bool carHit(Powerup *ob)
{
	double x = ob->current_down_x;
	double y = ob->current_down_y;
	double h = ob->height;
	double w = ob->width;
	double r_w = ob->real_width;

	float margin = h / 5;
	double ob_down_y = ob->current_down_y + margin;
	double ob_h = h;
	double ob_x_left = x - r_w / 2;
	double ob_x_right = x + r_w / 2;


	//For Debugging
	if (DEBUGING_MODE && VERBOSE) {
		char ob_id = ob->id;
		printf("\n\nPowerup ::\nx_id = %d\n", ob_id);
		printf("y axis ==== ob_down_y = %f, car.y_pos + car.height = %f \n", ob_down_y, car.y_pos + car.height);
		printf("y axis ==== ob_down_y + ob_h = %f, car_y_pos = %f \n", ob_down_y + ob_h, car.y_pos);
		printf("x axis ==== ob_x_left = %f, car.x_pos - car.width / 2 = %f \n", ob_x_left, car.x_pos - car.width / 2);
		printf("x axis ==== ob_x_right = %f, car.x_pos + car.width / 2 = %f \n", ob_x_right, car.x_pos + car.width / 2);
		printf("some vals ==== ob_x_left = %f, ob_x_right = %f, ob_current_width = %f\n",
			ob_x_left, ob_x_right, ob->real_width);
		printf("some vals ==== car.x_pos = %f, car.width = %f\n\n", car.x_pos, car.width);
	}


	if (ob_down_y <= car.y_pos + car.height &&
		ob_h + ob_down_y >= car.y_pos)
	{
		double car_x_left = car.x_pos - car.width / 2;
		double car_x_right = car.x_pos + car.width / 2;

		if ((car_x_left <= ob_x_left && car_x_right >= ob_x_left) ||
			(car_x_left <= ob_x_right && car_x_right >= ob_x_left)) {

			return true;
		}
	}

	return false;
}

bool carHit(Obstacle *ob)
{
	float margin = ob->height / 5;
	double ob_down_y = ob->current_down_y + margin;
	double ob_h = ob->height;
	double ob_x_left = ob->current_down_x - ob->real_width / 2;
	double ob_x_right = ob->current_down_x + ob->real_width / 2;

	//For Debugging
	/*
	if (DEBUGING_MODE && VERBOSE) {
		char ob_id = ob->id;
		printf("\n\nObstacle ::\nx_id = %d\n", ob_id);
		printf("y axis ==== ob_down_y = %f, car.y_pos + car.height = %f \n", ob_down_y, car.y_pos + car.height);
		printf("y axis ==== ob_down_y + ob_h = %f, car_y_pos = %f \n", ob_down_y + ob_h, car.y_pos);
		printf("x axis ==== ob_x_left = %f, car.x_pos - car.width / 2 = %f \n", ob_x_left, car.x_pos - car.width / 2);
		printf("x axis ==== ob_x_right = %f, car.x_pos + car.width / 2 = %f \n", ob_x_right, car.x_pos + car.width / 2);
		printf("some vals ==== ob_x_left = %f, ob_x_right = %f, ob_current_width = %f\n",
			ob_x_left, ob_x_right, ob->real_width);
		printf("some vals ==== car.x_pos = %f, car.width = %f\n\n", car.x_pos, car.width);
	}
	*/

	if (ob_down_y <= car.y_pos + car.height &&
		ob_h + ob_down_y >= car.y_pos)
	{
		double car_x_left = car.x_pos - car.width / 2;
		double car_x_right = car.x_pos + car.width / 2;

		if ((car_x_left <= ob_x_left && car_x_right >= ob_x_left) ||
			(car_x_left <= ob_x_right && car_x_right >= ob_x_left)) {

			return true;
		}
	}

	return false;
}

/* Game Drawers */
void drawTheCar()
{
	glPushMatrix();
	glTranslated(car.x_pos, car.y_pos, 0);
	float x_scale = 6, y_scale = 6;
	glScaled(x_scale, y_scale, 1);
	car.height = 20 * y_scale;
	car.width = 16 * x_scale;

	glBegin(GL_QUAD_STRIP);
	float x, y;
	float r = 1.0f, g = 0.0f, b = 0.5f;
	glColor3f(r, g, b);
	x = 7.0f;
	for (y = 20; y >= 0; y -= 1) {
		if (int(y) % 5 == 0) {
			x = (x == 7.0f || x == 8.0f) ? 5.0f : 8.0f;
		}
		r -= 0.03;
		glColor3f(r, g, b);
		glVertex3f(x, y, 0.0f);
		glVertex3f(-x, y, 0.0f);
	}
	glEnd();
	glPopMatrix();
}

// Obstacles Drawers
void drawObstacles() {
	double x_pos, y_pos;
	Obstacle *ob;
	for (int ob_num = OBSTACLES_MAX_INDEX; ob_num >= 0; ob_num--) {
		ob = &obstacles[ob_num];
		if (ob->on)
		{
			x_pos = ob->current_down_x;
			y_pos = ob->current_down_y;
			if (ob->type == 0) {
				drawWoodObstacle(ob);
			}
			else
			{
				// TODO
			}
		}
	}
}

void drawWoodObstacle(Obstacle *ob)
{
	double x = ob->current_down_x;
	double y = ob->current_down_y;
	float w = ob->width;
	float h = ob->height;

	glPushMatrix();

	float depth_ratio = 5 * (1 - (y / WIN_H));
	float base_width = w;
	double depth_change = base_width / depth_ratio;
	float slope = GLOBAL_SLOPE - (depth_change / abs(GLOBAL_SLOPE));
	
	double symbolic_height = (WIN_H / depth_ratio);

	float x_scale = 6, y_scale = 6;

	//base_width = base_width * ((WIN_H - y) / WIN_H);

	if (x <= getLane(1) || x >= getLane(3))
	{
		slope = (x >= getLane(3)) ? slope : -slope;
		x = (symbolic_height + (slope*x)) / slope;
		//up_right_x = ((up_y - down_y) + (slope*down_right_x)) / slope;

	}

	glTranslated(x, y, 0);
	glScaled(x_scale, y_scale, 1);

	float width = base_width;
	float minor_width = width/3;
	float height = h;

	float delta_height;
	float up_y;

	//double down_right_x, up_right_x, down_left_x, up_left_x;

	for (float i = -width/2; i < width/2; i+=minor_width) {

		delta_height = height/5;
		up_y = (height - delta_height);

		glBegin(GL_POLYGON);
		glColor3f(0.8, 0.6, 0.2);

		glVertex2d(i, up_y);
		glVertex2d(i, 0);
		glVertex2d(i + minor_width, 0);
		glVertex2d(i + minor_width, up_y);
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex2d(i, up_y);
		glVertex2d(i + (minor_width / 2), height);
		glVertex2d(i + minor_width, up_y);
		glEnd();
		
		glPushMatrix();
		glPointSize(100);
		glBegin(GL_LINE_STRIP);
		glColor3f(0.8, 0.2, 0.2);
		glVertex2d(i, up_y);
		glVertex2d(i, 0);
		glVertex2d(i + minor_width, 0);
		glVertex2d(i + minor_width, up_y);
		glEnd();
		glPopMatrix();
	}

	ob->real_width = base_width*x_scale;
	ob->real_height = height*y_scale;

	glPopMatrix();

	// For Debugging
	if (DEBUGING_MODE) {
		char* id[20];
		sprintf_s((char *)id, 20, "%d", ob->id);
		print(x - width / 2, y, (char*)id);
	}
}

// Powerups Drawers
void drawPowerups() {
	double x_pos, y_pos;

	Powerup *pu;
	for (int pu_num = POWERUPS_MAX_INDEX; pu_num >= 0; pu_num--) {
		pu = &powerups[pu_num];
		if (pu->on)
		{
			x_pos = pu->current_down_x;
			y_pos = pu->current_down_y;
			

			if (pu->type == 0) {
				
				drawPowerUpBottle(pu);	
			}
			else
			{
				drawPowerUpStrike(pu);
			}
		}
	}
}

void drawPowerUpStrike(Powerup *pu)
{
	double x = pu->current_down_x;
	double y = pu->current_down_y;
	float w = pu->width;
	float h = pu->height;

	if (x < -2) {
		printf("\n @drawer_start: Powerup Strike::\n id= %d,\nx= %f, y= %f,\nw= %f, h= %f\n",
			pu->id, x, y, w, h);
		if (x < -5)
			glutIdleFunc(NULL);
	}

	glPushMatrix();

	float depth_ratio = 5 * (1 - (y / WIN_H));
	float base_width = w;
	double depth_change = base_width / depth_ratio;
	float slope = GLOBAL_SLOPE - (depth_change / abs(GLOBAL_SLOPE));

	double symbolic_height = (WIN_H / depth_ratio);

	float x_scale = 6, y_scale = 6;

	if (x <= getLane(1) || x >= getLane(3))
	{
		slope = (x >= getLane(3)) ? slope : -slope;
		x = (symbolic_height + (slope*x)) / slope;
	}
	
	glTranslated(x, y, 0);
	glScaled(x_scale, y_scale, 1);
	glRotated(-3, 0, 0, 1);

	float width = base_width;
	float minor_width = width / 3;
	float height = h;


	glBegin(GL_TRIANGLES);
	glColor3f(1, 1, 0.1);
	glVertex2f(2.8, 9);
	glVertex2f(-2.5, 5);
	glVertex2f(1, 3);
	glVertex2f(0, 5);
	glVertex2f(2.8, 3);
	glVertex2f(-1, 0);
	glEnd();
	glPopMatrix();

	pu->real_width = 5 * x_scale;
	pu->real_height = 10 * y_scale;

	glPopMatrix();

	// For Debugging
	if (DEBUGING_MODE) {
		glColor3f(0, 0, 0);
		char* id[20];
		sprintf_s((char *)id, 20, "%d", pu->id);
		print(x - pu->real_width / 2, y, (char*)id);
	}
}

void drawPowerUpBottle(Powerup *pu)
{
	double x = pu->current_down_x;
	double y = pu->current_down_y;
	float w = pu->width;
	float h = pu->height;

	if (x < -2) {
		printf("\n @drawer_start: Powerup bottle::\n id= %d,\nx= %f, y= %f,\nw= %f, h= %f\n",
			pu->id, x, y, w, h);
		if(x<-5)
			glutIdleFunc(NULL);
	}

	glPushMatrix();

	float depth_ratio = 5 * (1 - (y / WIN_H));
	float base_width = w;
	double depth_change = base_width / depth_ratio;
	float slope = GLOBAL_SLOPE - (depth_change / abs(GLOBAL_SLOPE));

	double symbolic_height = (WIN_H / depth_ratio);

	float x_scale = 6, y_scale = 6;

	if (x <= getLane(1) || x >= getLane(3))
	{
		slope = (x >= getLane(3)) ? slope : -slope;
		x = (symbolic_height + (slope*x)) / slope;
	}
	
		

	glTranslated(x, y, 0);
	glScaled(x_scale, y_scale, 1);
	glRotated(30, 0, 0, 1);

	float width = base_width;
	float minor_width = width / 3;
	float height = h;

	//float delta_height;
	//float up_y;

	glBegin(GL_POLYGON);
	glColor3f(1, 0.5, 0.1);
	glColor3f(0, 0, 0);
	glVertex2f(-1, 6);
	glVertex2f(-1.5, 6);
	glVertex2f(-1.5, 6.5);
	glVertex2f(1.5, 6.5);
	glVertex2f(1.5, 6);
	glVertex2f(1, 6);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1, 0.5, 0.1);
	glVertex2f(1, 6);
	glVertex2f(1, 5);
	glVertex2f(-1, 5);
	glVertex2f(-1, 6);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1, 0.1, 0.1);
	glVertex2f(1, 5);
	glVertex2f(2, 5);
	glVertex2f(2, 0);
	glVertex2f(-2, 0);
	glVertex2f(-2, 5);
	glVertex2f(-1, 5);
	glEnd();

	pu->real_width = 4 * x_scale;
	pu->real_height = 13 * y_scale;
	glPopMatrix();

	// For Debugging
	if (DEBUGING_MODE) {
		glColor3f(0, 0, 0);
		char* id[20];
		sprintf_s((char *)id, 20, "%d", pu->id);
		print(x - pu->real_width / 2, y, (char*)id);
	}
}

// Status Drawers
void drawStatusPart() {
	glPushMatrix();

	float r = 0.3, g = 0.1, b = 0.3;
	glTranslated(WIN_W / 2, 0, 0);

	double width = WIN_W;
	double original_width = width;
	double height = WIN_H / 8;
	double down_y = (WIN_H - WIN_H / 8);
	double y_point_of_change = height / 3 + down_y;

	glBegin(GL_QUAD_STRIP);
	int y;
	for (y = WIN_H; y >= down_y; y -= height / 12) {
		glColor3f(r, g, b);
		if (y < y_point_of_change) {
			width += 5 * GLOBAL_SLOPE;

			r += 0.1;
		}
		else
		{
			r -= 0.005;
			b += 0.005;
		}

		glVertex2f(-width / 2, y);
		glVertex2f(width / 2, y);
	}
	glEnd();

	// return y to previous value to map the outline
	y += height / 12;

	glBegin(GL_LINE_LOOP);
	glPointSize(50);
	glColor3f(1, 1, 1); // WHITE
	glVertex2f(-width / 2, y);
	glVertex2f(width / 2, y);
	width = original_width;
	glVertex2f(width / 2, y_point_of_change);
	glVertex2f(width / 2, WIN_H);
	glVertex2f(-width / 2, WIN_H);
	glVertex2f(-width / 2, y_point_of_change);
	glEnd();

	glPopMatrix();

}

void drawHpBar() {

	glPushMatrix();

	double x_pos, y_pos;
	double width = 0.3f* WIN_W;
	double height = 0.03f * WIN_H;

	x_pos = 0.08 * WIN_W;
	y_pos = 0.92f * WIN_H;
	glTranslated(x_pos, y_pos, 0);
	double margin = 50;


	double INIT_CAR_HP = 100;
	double hp_width_fill = width * (car.hp / INIT_CAR_HP);

	glBegin(GL_POLYGON);
	glColor3f(0.9, 0.2, 0.3); // RED
	glVertex2f(0, height);
	glVertex2f(0, 0);
	glVertex2f(hp_width_fill, 0);
	glVertex2f(hp_width_fill, height);
	glEnd();

	glPushMatrix();

	glBegin(GL_LINE_LOOP);
	glPointSize(50);
	glScaled(1.5, 1.5, 1);
	glColor3f(1, 1, 1); // WHITE
	glVertex2f(0, height);
	glVertex2f(0, 0);
	glVertex2f(width, 0);
	glVertex2f(width, height);
	glEnd();

	glColor3f(1, 1, 1);
	char* id[20];
	sprintf_s((char *)id, 20, "HP:");
	print(-margin, margin / 4, (char*)id);

	glPopMatrix();

	glPopMatrix();
}

// Road Drawers
void drawRoadLine(bool left, double down_y) 
{
	
	float r = 0.2, g = 0.3, b = 0.7;
	r = 0.9, g = 0.8, b = 1;
	glColor3f(r, g, b);
	
	float depth_ratio = 5 * (1 + (down_y / WIN_H));
	double height = (WIN_H/depth_ratio);
	float base_width = (WIN_W / 18);
	double depth_change = base_width / depth_ratio;
	float slope = GLOBAL_SLOPE - (depth_change / abs(GLOBAL_SLOPE));
	double up_y = height + down_y;
	double down_right_x, up_right_x, down_left_x, up_left_x;

	//height = height * (1 - down_y / up_y);
	base_width = base_width * (((WIN_H - down_y) /WIN_H));
	

	if (left)
	{
		slope = -slope;
		down_right_x = (3 * (WIN_W / 8));
		up_right_x = down_right_x;//((up_y - down_y) + (slope * down_right_x)) / slope - depth_change;
		down_left_x = down_right_x - base_width;
		up_left_x = ((up_y - down_y) + (slope*down_left_x)) / slope;
		
	}
	else
	{
		down_left_x = (5 * (WIN_W / 8));
		up_left_x = down_left_x;//((up_y - down_y) + (slope * down_left_x)) / slope + depth_change;
		down_right_x = down_left_x + base_width;
		up_right_x = ((up_y - down_y) + (slope*down_right_x)) / slope;
		
	}


	glBegin(GL_POLYGON);
	glVertex2f(down_left_x, down_y);
	glVertex2f(down_right_x, down_y);
	glVertex2f(up_right_x, up_y);
	glVertex2f(up_left_x, up_y);
	glEnd();
}

void drawRoadLines(void)
{
	for (int y = road_start_y; y < (WIN_H - WIN_H/10); y += 3*WIN_H / 10) {
		drawRoadLine(false, y);
		drawRoadLine(true, y);
	}
}

void drawRoadPlane(void)
{
	double x_pos, y_pos;
	glPushMatrix();

	float r = 0.7f, g = 0.66f, b = 1.0f;
	glColor3f(r, g, b);

	x_pos = (WIN_W / 2);
	y_pos = 0;
	glTranslated(x_pos, y_pos, 0);

	float slope = GLOBAL_SLOPE;
	double down_right_x = (3 * (WIN_W / 4)) / 2;
	double up_right_x = (WIN_H + (slope*down_right_x)) / slope;

	glBegin(GL_POLYGON);
	glVertex2f(down_right_x, 0);
	glVertex2f(up_right_x, WIN_H);
	glVertex2f(-up_right_x, WIN_H);
	glVertex2f(-down_right_x, 0);
	glEnd();
	glPopMatrix();
}

// Text Drawers
void print(int x, int y, char *string)
{
	int len, i;

	//set the position of the text in the window using the x and y coordinates
	glRasterPos2f(x, y);
	//printf("String to print = '%s'", string);
	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

/* Movers */
void moveCar(int delta_x, int delta_y) {

	double car_road_margin = WIN_W / 32;
	double car_x_pos_tmp = car.x_pos;
	car.x_pos += delta_x;

	if (car.x_pos < (getLane(1) - car_road_margin) ||
		car.x_pos > (getLane(3) + car_road_margin)) {
		car.x_pos = car_x_pos_tmp;
	}
	//printf("\n x_pos = %f, getLane(1) - car_road_margin = %f", car_x_pos, (getLane(3) + car_road_margin));

}

/*Prepherals Processors*/
void processSpecialKeys(int k, int x, int y)
{
	if (k == GLUT_KEY_RIGHT)
		moveCar(15, 0);
	if (k == GLUT_KEY_LEFT)
		moveCar(-15, 0);
	if (k == GLUT_KEY_DOWN) {
		initValues(); // Reset game values
		glutIdleFunc(Anim);
	}

	glutPostRedisplay();
}

void processNormalKeys(unsigned char k, int x, int y)
{
	if (k == 'd')
		moveCar(15, 0);
	if (k == 'a')
		moveCar(-15, 0);
	if (k == 'w')//if the letter w is pressed, then the object will be translated in the y axis by 10. (moving upwords)
		moveCar(0, 10);
	if (k == 's')//if the letter w is pressed, then the object will be translated in the y axis by -10. (moving downwords)
		moveCar(0, -10);

	glutPostRedisplay();//redisplay to update the screen with the changes
}

void passiveMotion(int x, int y)
{
	double car_road_margin = WIN_W / 32;
	double car_x_pos_tmp = car.x_pos;

	car.x_pos = x;

	if (car.x_pos < (getLane(1) - car_road_margin) ||
		car.x_pos >(getLane(3) + car_road_margin)) {
		car.x_pos = car_x_pos_tmp;
	}

	glutPostRedisplay();
}

/* Objects Generators */
bool generateRandomObject() {

	int obstacle_powerup = getRandomValue(2);
	switch (obstacle_powerup) {
		case(1): 
			if (obstacles[obstacles_counter + 1].on)
				return false;
			generateRandomObstacle(); 
		case(2):
			if (powerups[powerups_counter + 1].on)
				return false; 
			generateRandomPowerup();
	}
	return true;
}

void generateRandomObstacle() {
	int lane_num = getRandomValue(3);
	if(DEBUGING_MODE)
		printf("\nGenerating Obstacle:: lane_num = %d\n", lane_num);
	int obstacle_type = (getRandomValue(10) >= 5) ? 0 : 1;
	// TODO, add obstacle_type to the called method
	makeObstacle(getLane(lane_num), SLIGHTLY_OFF_TOP_SCREEN, 0);
}

void generateRandomPowerup() {
	int lane_num = getRandomValue(3);
	if(DEBUGING_MODE)
		printf("\nGenerating Powerup:: lane_num = %d\n", lane_num);
	int powerup_type = (getRandomValue(10) >= 5 )? 0: 1;
	makePowerup(getLane(lane_num), SLIGHTLY_OFF_TOP_SCREEN, powerup_type);
}

/* Structs Creators */
Powerup* makePowerup(double x, double y, int t, float w, float h) {

	if (++powerups_counter > POWERUPS_MAX_INDEX) {
		powerups_counter = 0;
	}

	Powerup* pu = &powerups[powerups_counter];
	pu->on = true;
	pu->current_down_y = y;
	pu->current_down_x = x;
	pu->type = (t <= 0) ? 0 : 1;
	pu->hit = 0.2;
	pu->width = (w > 0) ? w : pu->width;
	pu->height = (h > 0) ? w : pu->height;
	pu->real_width = pu->width;
	pu->real_height = pu->height;
	pu->id = powerups_counter;

	x = pu->current_down_x;
	y = pu->current_down_y;
	w = pu->width;
	h = pu->height;
	if (x < 0) {
		printf("\n @makePowerup: Powerup bottle::\n id= %d,\nx= %f, y= %f,\nw= %f, h= %f\n",
			pu->id, x, y, w, h);
		glutIdleFunc(NULL);
	}

	num_of_on_powerups++;
	return pu;
}

Powerup* makePowerup(double x, double y, int t)
{
	return makePowerup(x, y, t, -1, -1);
}

Obstacle* makeObstacle(double x, double y, int t, float w, float h) {

	if (++obstacles_counter > OBSTACLES_MAX_INDEX) {
		obstacles_counter = 0;
	}

	Obstacle* ob = &obstacles[obstacles_counter];
	ob->on = true;
	ob->current_down_y = y;
	ob->current_down_x = x;
	ob->type = (t <= 0) ? 0 : 1;
	ob->hit = (t <= 0) ? 0.2 : 0.4;
	ob->width = (w > 0) ? w : ob->width;
	ob->height = (h > 0) ? w : ob->height;
	ob->real_width = ob->width;
	ob->real_height = ob->height;
	ob->id = obstacles_counter;

	num_of_on_obstacles++;
	return ob;
}

Obstacle* makeObstacle(double x, double y, int t)
{
	return makeObstacle(x, y, t, -1, -1);
}

/* Debugging Functions */
void debugScreenLines(int width_minor, int height_minor) {
	glColor3f(1, 0, 0);
	for (int i = 0; i < WIN_W; i += WIN_W / width_minor) {
		glBegin(GL_LINES);
		glVertex2f(i, 0);
		glVertex2f(i, WIN_H);
		glEnd();
	}
	for (int j = 0; j < WIN_H; j += WIN_H / height_minor) {
		glBegin(GL_LINES);
		glVertex2f(0, j);
		glVertex2f(WIN_W, j);
		glEnd();
	}
}

/* Other Helpfull Functions */
double getLane(int lane_num)
{
	if (lane_num > 0 && lane_num <= 3) {
		//	return (lane_num * 2) *(WIN_W / 8);
	}

	switch (lane_num) {
	case(1): return 5 * WIN_W / 22;
	case(2): return WIN_W / 2;
	case(3): return 17 * WIN_W / 22;
	}

	return 0;
}

int getRandomValue(int maximum) {
	/* initialize random seed: */
	srand(time(NULL));
	/* generate secret number between 1 and maximum: */
	return (rand() % maximum + 1);
}

void initConfig() {

	glutInitWindowSize(WIN_W, WIN_H);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("MadCar - DMET");
	glutDisplayFunc(Display);
	glutIdleFunc(Anim);
	glClearColor(0.1f, 0.1f, 0.2f, 0.0f);

	//glutFullScreen();           // making the window full screen

	glutPassiveMotionFunc(passiveMotion);
	//glutMouseFunc(actM);							
	glutKeyboardFunc(processNormalKeys);
	//glutKeyboardUpFunc(keyUp);					
	glutSpecialFunc(processSpecialKeys);
	//glutSpecialUpFunc(speUp);						
	//glutTimerFunc(0, time, 0);					//call the timer function
	//glPointSize(25);								//change the point size to be 25
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);

	//glDisable(GL_LIGHTING);

	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	WIN_W = m_viewport[2];
	WIN_H = m_viewport[3];
	//printf("%d, %d, %d, %d", WIN_W, WIN_H);

}

void initValues() {

	//Intialize Car values
	car.x_pos = getLane(2);
	car.y_pos = WIN_H / 8;
	car.alive = true;
	car.hp = MAX_HP;
	car.acceleration = MAX_ACC;

	//Intialize Obstacles values
	for (int i = 0; i <= OBSTACLES_MAX_INDEX; i++) {
		Obstacle *ob = &obstacles[i];
		ob->on = false;
		ob->height = 8;
		ob->width = 16;
		ob->id = -1;
		ob->type = 0; // TODO just for now until I make the other type
	}

	//Intialize Powerups values
	for (int i = 0; i <= POWERUPS_MAX_INDEX; i++) {
		Powerup *pu = &powerups[i];
		pu->on = false;
		pu->id = -1;
		pu->type = -1; // TODO just for now until I make the other type
	}

	generateRandomObject();

	road_start_y = 20;
	num_of_on_obstacles = 0;
	num_of_on_powerups = 0;
}

int main(int argc, char** argr)
{
	glutInit(&argc, argr);
	// Init OpenGL Configs
	initConfig();
	
	// Intializations of variables
	initValues();
	
	glutMainLoop();
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu