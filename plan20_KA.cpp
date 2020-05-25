// plan.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h"
#include<iostream>
#include <cmath>
//#include <time.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glut.h>
// #include <GL/glxint.h>
//#include <GL/glx.h>
#include <vector>
//#include "freeglut_std.h"s
//#include "freeglut_ext.h"

//#include "freeglut_std.h"s
//#include "freeglut_ext.h"
using namespace std;
double DT = 1e-5;
double scena[6] = {-500., 500., 500., -500., -500., 500. };
double rad[5] = { 130., 6., 12.6, 12., 5 }; //солнце, марс, земля, венера
//double rast[4] = { 0., rad[0] + 220., rad[0] + 140., rad[0] + 100. };
//double x[4] = { 0, 0, 0, 0 };
//double y[4] = { 0};
double rbg[15] = { 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 100, 0, 0, 255, 0 };//ж/к/с/з




void init()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glShadeModel(GL_FLAT);
	glOrtho(scena[0], scena[1], scena[2], scena[3], scena[4], scena[5]);
}

class Vector3d {
public:
	float x;
	float y;
	float z;

	float r() const {
		return sqrt(x*x + y*y + z*z);
	}

	Vector3d() : x(0), y(0), z(0) {}
	Vector3d(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	Vector3d(const Vector3d &f) : x(f.x), y(f.y), z(f.z) {}
	/*const struct Vector3d& operator = (const struct Vector3d& m)
	{
		this->x = m.x;
		this->y = m.y;
		this->y = m.z;

		return *this;
	}*/


	struct Vector3d operator * (double t) const
	{
		return Vector3d(x * t, y * t, z*t);
	}

	struct Vector3d operator+ (const struct Vector3d m) const
	{
		return Vector3d(x + m.x, y + m.y, z + m.z);
	}
	struct Vector3d operator - (const struct Vector3d m) const
	{
		return Vector3d(x - m.x, y - m.y, z - m.z);
	}
};

class  Body {
private:
	double dp; //u * dm —- испульс топлив
	double dm;
public:
	struct Vector3d pos, vel;
	double mass;
	double own_mass;

	Body() = default;

	~Body() {}

	Body(struct Vector3d pos_, struct Vector3d vel_, double mass_) :
		mass(mass_), pos(pos_), vel(vel_)
	{}

	Body(struct Vector3d pos_, struct Vector3d vel_, double own_mass_, double fuel_mass_, double fuel_output_,double fuel_vel_) {
		mass = own_mass_ + fuel_mass_;
		own_mass = own_mass_;
		dm = fuel_output_ * DT;
		dp = fuel_vel_ * dm;
		pos = pos_;
		vel = vel_;
	}

	void update_state(const struct Vector3d &acceleration) {
		vel = vel + acceleration * DT;
		pos = pos + vel * DT;
	}

	void update_state(const struct Vector3d &acceleration_external, const struct Vector3d &direction_thrust) {
		vel = vel + direction_thrust * (dp / mass) + acceleration_external * DT;
		pos = pos + vel * DT;
		mass -= dm;
	}

	struct Vector3d get_acceleration_without_G(const struct Vector3d &pos_) {
		const double r_abs = abs((pos - pos_).r());
		if (r_abs < 1e-6) {
			return Vector3d(0, 0, 0);
		}
		std::cout << "pos=" << pos.x << "," << pos.y << ", pos_=" << pos_.x << "," << pos_.y << std::endl;
		std::cout << "dpos=" << (pos - pos_).x << "," << (pos - pos_).y << std::endl;
		struct Vector3d a = (pos - pos_) * (mass / (r_abs * r_abs * r_abs));
		return a;
	}
};

class Storage {
public:
	std::vector<Body> body_list;
};

Storage *gStorage = 0;

const int n = 5;

void display()
{
	std::vector<Body> &body_list = gStorage->body_list;
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	for (int i = 0; i < 5; i++)
	{
//		cout << "x[" << i << "]=" << x[i] << '\n';
//		cout << "y[" << i << "]=" << y[i] << '\n';
		glTranslatef(gStorage->body_list[i].pos.x, gStorage->body_list[i].pos.y, 0.);
		glColor3f(rbg[i * 3], rbg[i * 3 + 1], rbg[i * 3 + 2]);
		glutWireSphere(rad[i], 90, 90);
		glTranslatef(-gStorage->body_list[i].pos.x, -gStorage->body_list[i].pos.y, 0);
	}

	glLineWidth(1);       // ширину линии устанавливаем 1
	glBegin(GL_LINES);
	glColor3d(1, 0, 0);     // красный цвет
	glVertex3d(scena[0], 0, 0); // первая линия
	glVertex3d(scena[1], 0, 0);
	glVertex3d(0, scena[2], 0); // вторая линия
	glVertex3d(0, scena[3], 0);
	glEnd();
	glutSwapBuffers();
}


void timer(int value)
{
	std::vector<Body> &body_list = gStorage->body_list;

	for (int i = 0; i < gStorage->body_list.size(); i++) {
		std::cout << "body[" << i << "]={" <<
			gStorage->body_list[i].pos.x << ", " << gStorage->body_list[i].pos.y << "}" << std::endl;
	}
	//body_list[0].update_state(body_list[0].get_acceleration_without_G({100, 200}));228
	std::vector<Body> old_body_list = body_list;
	for (int i = 0; i < body_list.size(); ++i) {
		struct Vector3d acc { 0, 0, 0 };
		for (int j = 0; j < body_list.size(); ++j) {
			if (i == j) {
				continue;
			}
			acc = acc + old_body_list[j].get_acceleration_without_G(old_body_list[i].pos);
		}
		body_list[i].update_state(acc);
	}

//	for (int i = 0; i < body_list.size(); i++)
//	{
//		x[i] = gStorage->body_list[i].pos.x;
//		y[i] = gStorage->body_list[i].pos.y;
//	}
	glutPostRedisplay();
	glutTimerFunc(1, timer, 1000);
}

int main(int argc, char * argv[])
{


	gStorage = new Storage();
	std::vector<Body> &body_list = gStorage->body_list;
	body_list.emplace_back(Vector3d(1, 6, 0), Vector3d(0, 0, 0), 2e10);

	body_list.emplace_back(Vector3d(1, 150.6, 0), Vector3d(10000, 0, 0), 60e-1);
	body_list.emplace_back(Vector3d(1, 200.6, 0), Vector3d(10000, 0, 0), 60.5e-1);
	body_list.emplace_back(Vector3d(1, 300.6, 0), Vector3d(10000, 0, 0), 2000e-1);
	 body_list.emplace_back(Vector3d(1, 200.6, 0), Vector3d(1000, 0, 0), 2e-1, 1e-1, 2.5, 6);

	//	 body_list.emplace_back(Vector3d{555, 400}, Vector3d{0,0}, );
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);//GLUT_DOUBLE
	glutInitWindowSize(scena[1], scena[2]);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Gravitation");
	init();
	glutDisplayFunc(display);
	glutTimerFunc(1000, timer, 1000);
	//glutReshapeFunc(reshape);
	glutMainLoop();
	// glutTimerFunc(1, timer, 0);

	delete gStorage;
	return 0;
}
