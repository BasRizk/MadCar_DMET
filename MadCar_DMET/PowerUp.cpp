#include <iostream>
using namespace std;


class Obstacle {
public:
	double current_down_y;
	double current_down_x;
	int type;
	int hit;
	float width = 6, height = 12;
	//Obstacle(double, double, int, float, float);
	//Obstacle(double, double, int);
	void set_values(double, double, int);
	int rectangularArea() { return (width*height); }
};

/*
Obstacle::Obstacle(double x, double y, int t, float w, float h)
{
	current_down_y = y;
	current_down_left_x = x;
	type = (t <= 0)? 0: 1;
	hit = (type <= 0) ? 0.2: 0.5;
	width = w;
	height = h;
}
*/

void Obstacle::set_values(double x, double y, int t) {
	current_down_y = y;
	current_down_x = x;
	type = (t <= 0) ? 0 : 1;
	hit = (type <= 0) ? 0.2 : 0.5;
}
/*
Obstacle::Obstacle(double x, double y, int t)
{
	current_down_y = y;
	current_down_left_x = x;
	type = (t <= 0) ? 0 : 1;
	hit = (type <= 0) ? 0.2 : 0.5;
	width = 12;
	height = 6;
}
*/