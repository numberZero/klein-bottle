#include <iostream>
#include <memory>
#include <GL/gl.h>
#include <SDL.h>
#include <Eigen/Core>

#ifdef NDEBUG
static long const grid_steps_u = 32;
static long const grid_steps_v = 24;
static long const sub_steps_u = 8;
static long const sub_steps_v = 8;
#else
static long const grid_steps_u = 24;
static long const grid_steps_v = 16;
static long const sub_steps_u = 3;
static long const sub_steps_v = 3;
#endif
static long const steps_u = grid_steps_u * sub_steps_u;
static long const steps_v = grid_steps_v * sub_steps_v;
static double const min_u = -M_PI;
static double const min_v = 0.0;
static double const range_u = 2*M_PI;
static double const range_v = M_PI;
static double const step_u = range_u / steps_u;
static double const step_v = range_v / steps_v;

static double const grid_offset = 0.005;
static double const surface_scale = 1 - grid_offset;
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
double speed[6] = { 1./13., 1./11., 1./7., 1./5., 1./3., 1./2. };
// double speed[6] = { 0.9, 0.7, 0.0, 0.5, 0.0, 0.0 };
// double speed[6] = { 0.9, 0.8, 0.7, 0.6, 0.5, 0.4 };
double angle[6] = { 0, 0, 0, 0, 0, 0 };
double dt;

bool animate = true;
bool show_grid = true;
bool show_surface = false;
bool color_2d = true;
bool color_3d = false;

bool events()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_KEYDOWN:
				switch(event.key.keysym.scancode)
				{
					case SDL_SCANCODE_A:
						animate = !animate;
						break;
					case SDL_SCANCODE_G:
						show_grid = !show_grid;
						break;
					case SDL_SCANCODE_S:
						show_surface = !show_surface;
						break;
					case SDL_SCANCODE_X:
						color_2d = !color_2d;
						break;
					case SDL_SCANCODE_Z:
						color_3d = !color_3d;
						break;
					case SDL_SCANCODE_ESCAPE:
						return false;
				}
				break;
			case SDL_QUIT:
				return false;
		}
	}
	return true;
}

Point4 makePoint(double u, double v)
{
	return {
		sin(u) * cos(v),
		sin(u) * sin(v),
		cos(u) + 0.33 * cos(2*v),
		0.33 * sin(2*v),
	};
}

void drawPoint(Point4 const &pt, bool colorize = true)
{
	if(colorize)
		glColor4f(1.0, pt[2], pt[3], 1.0);
	glVertex3f(pt[0], pt[1], pt[2]);
}

void colorize(long i, long j, bool colorize = false)
{
	if(!colorize)
		return;
	double u = step_u * i;
	double v = step_v * j;
	glColor4f(0.5 + 0.5 * cos(u), 0.5 * 0.5 + cos(2 * v), 0.0, 1.0);
}

void init()
{
	for(long j = 0; j != steps_v; ++j)
		for(long i = 0; i != steps_u; ++i)
			grid[j][i] = makePoint(min_u + step_u * i, min_v + step_v * j);
}

Matrix4 mat;

void calcRotationMatrix()
{
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
		if(animate)
			angle[k] += dt * speed[k];
	}
}

void drawGrid()
{
	glColor4f(0.0, 0.0, 0.8, 1.0);
	for(long i = 0; i != steps_u; i += sub_steps_u)
	{
		glBegin(GL_LINE_STRIP);
		for(long j = 0; j != steps_v; ++j)
		{
			colorize(i, j, color_2d);
			drawPoint(mat * grid[j][i], color_3d);
		}
		long i2 = (steps_u - i) % steps_u;
		colorize(i2, 0, color_2d);
		drawPoint(mat * grid[0][i2], color_3d);
		glEnd();
	}
	for(long j = 0; j != steps_v; j += sub_steps_v)
	{
		glBegin(GL_LINE_LOOP);
		for(long i = 0; i != steps_u; ++i)
		{
			colorize(i, j, color_2d);
			drawPoint(mat * grid[j][i], color_3d);
		}
		glEnd();
	}
}

void drawSurface()
{
	mat *= surface_scale;
	glColor4f(0.7, 0.7, 0.7, 0.5);
	glBegin(GL_QUADS);
	for(long j = 0; j != steps_v; ++j)
	{
		long j2 = (j + 1) % steps_v;
		for(long i = 0; i != steps_u; ++i)
		{
			long i1 = i;
			long i2 = (i + 1) % steps_u;
			drawPoint(mat * grid[j][i1], false);
			drawPoint(mat * grid[j][i2], false);
			if(!j2)
			{
				i1 = (steps_u - i1) % steps_u;
				i2 = (steps_u - i2) % steps_u;
			}
			drawPoint(mat * grid[j2][i2], false);
			drawPoint(mat * grid[j2][i1], false);
		}
	}
}

void quad(long i, double r, double g, double b)
{
	static double const quad_size = 16;
	glColor4f(r, g, b, 1.0);
	glBegin(GL_QUADS);
	glVertex2f(-400 + quad_size * 1.5*i, 300 - quad_size);
	glVertex2f(-400 + quad_size * (1.5*i+1), 300 - quad_size);
	glVertex2f(-400 + quad_size * (1.5*i+1), 300 - 2*quad_size);
	glVertex2f(-400 + quad_size * 1.5*i, 300 - 2*quad_size);
	glEnd();
}

void flag(long i, double r1, double g1, double b1, double r2, double g2, double b2, bool value)
{
	if(value)
		quad(i, r1, g1, b1);
	else
		quad(i, r2, g2, b2);
}

void step()
{
	long t_now = SDL_GetTicks();
	dt = (t_now - t_base) * 0.001;
	t_base = t_now;

	glClearColor(0.0, 0.0, 0.2, 1.0);
	glClearDepth(500.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	flag(1, 0.0, 1.0, 0.0, 0.8, 0.0, 0.0, animate);
	flag(2, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, show_grid);
	flag(3, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, show_surface);
	flag(4, 0.3, 0.8, 0.0, 0.6, 0.6, 0.6, color_2d);
	flag(5, 1.0, 0.3, 0.5, 0.6, 0.6, 0.6, color_3d);

	glScalef(150.0, 150.0, 150.0);

	calcRotationMatrix();

	if(show_grid)
		drawGrid();

	if(show_surface)
		drawSurface();

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
	window = SDL_CreateWindow("Klein bottle test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL);
	context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, context);
	t_base = SDL_GetTicks();
}

void initGL()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

// 	glEnable(GL_POLYGON_SMOOTH);
// 	glEnable(GL_LINE_SMOOTH);
	glLineWidth(0.7);

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
