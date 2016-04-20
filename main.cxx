#include <iostream>
#include <memory>
#include <GL/gl.h>
#include <SDL.h>
#include <Eigen/Core>

static long const grid_steps_u = 10;
static long const grid_steps_v = 10;
static long const sub_steps_u = 10;
static long const sub_steps_v = 10;
static long const steps_u = grid_steps_u * sub_steps_u;
static long const steps_v = grid_steps_v * sub_steps_v;
static double const min_u = -0.5 * M_PI;
static double const min_v = 0.0;
static double const range_u = M_PI;
static double const range_v = M_PI;
static double const step_u = range_u / steps_u;
static double const step_v = range_v / steps_v;

// struct Point4
// {
// 	double x, y, z, w;
// };

typedef Eigen::Vector4d Point4;
typedef Eigen::Matrix4d Matrix4;

SDL_Window *window;
static SDL_GLContext context;
static long t_base;
Point4 grid[steps_v][steps_u];

long pairs[6][2] = {
	{ 0, 1 },
	{ 0, 2 },
	{ 0, 3 },
	{ 1, 2 },
	{ 1, 3 },
	{ 2, 3 },
};
double speed[6] = { 0.9, 0.7, 0.0, 0.5, 0.0, 0.0 };
// double speed[6] = { 0.9, 0.8, 0.7, 0.6, 0.5, 0.4 };
double angle[6] = { 0, 0, 0, 0, 0, 0 };

bool events()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				return false;
		}
	}
	return true;
}

Point4 makePoint(double u, double v)
{
	return {
		sin(v) * cos(2*u),
		sin(v) * sin(2*u),
		cos(v),
		0,
	};
}

void drawPoint(Point4 const &pt, bool colorize = true)
{
	if(colorize)
		glColor4f(1.0, pt[2], pt[3], 1.0);
	glVertex3f(pt[0], pt[1], pt[2]);
}

void init()
{
	for(long i = 0; i != steps_u; ++i)
		for(long j = 0; j != steps_v; ++j)
			grid[i][j] = makePoint(step_u * i, step_v * j);
}

void step()
{
	long t_now = SDL_GetTicks();
	double dt = (t_now - t_base) * 0.001;
	t_base = t_now;

	glClearColor(0.0, 0.0, 0.2, 1.0);
	glClearDepth(500.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glScalef(150.0, 150.0, 150.0);

	Matrix4 mat;
	mat.setIdentity();
	for(long k = 0; k != 6; ++k)
	{
		double c = cos(angle[k]);
		double s = sin(angle[k]);
		Matrix4 rot;
		rot.setIdentity();
		rot.setIdentity();
		rot(pairs[k][0], pairs[k][0]) = c;
		rot(pairs[k][1], pairs[k][1]) = c;
		rot(pairs[k][0], pairs[k][1]) = s;
		rot(pairs[k][1], pairs[k][0]) = -s;
		mat *= rot;
		angle[k] += dt * speed[k];
	}

	for(long i = 0; i != steps_u; i += sub_steps_u)
	{
		glBegin(GL_LINE_LOOP);
		for(long j = 0; j != steps_v; ++j)
			drawPoint(mat * grid[j][i]);
		glEnd();
	}
	for(long j = 0; j != steps_v; j += sub_steps_v)
	{
		glBegin(GL_LINE_LOOP);
		for(long i = 0; i != steps_u; ++i)
			drawPoint(mat * grid[j][i]);
		glEnd();
	}
/*
	glColor4f(0.0, 1.0, 0.0, 0.3);
	glBegin(GL_QUADS);
	for(long j = 0; j != steps_v; ++j)
	{
		long j2 = (j + 1) % steps_v;
		for(long i = 0; i != steps_u; ++i)
		{
			long i2 = (i + 1) % steps_u;
			drawPoint(0.99 * mat * grid[j][i], false);
			drawPoint(0.99 * mat * grid[j2][i], false);
			drawPoint(0.99 * mat * grid[j2][i2], false);
			drawPoint(0.99 * mat * grid[j][i2], false);
		}
	}
*/
	glEnd();
	glFlush();
	glFinish();
	SDL_GL_SwapWindow(window);
}

void run()
{
	while(events())
		step();
}

void initSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Klein bottle test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
	context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, context);
	t_base = SDL_GetTicks();
}

void initGL()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.5);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, -200, 200);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
	initSDL();
	initGL();
	init();
	run();
	SDL_Quit();
	return 0;
}
