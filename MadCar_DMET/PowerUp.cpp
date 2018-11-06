class PowerUp {
	double current_down_left_x;
	double current_down_y;
	int type;
	float width, height;

public:
	PowerUp(double, double, int, float, float);
	int rectangularArea() { return (width*height); }
};

PowerUp::PowerUp(double x, double y, int t, float w, float h) {
	current_down_left_x = x;
	current_down_y = y;
	type = t;
	width = w;
	height = h;
}