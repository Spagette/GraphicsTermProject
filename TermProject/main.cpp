#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h> // (or others, depending on the system in use)
#include <iostream>
#include <string.h>
#include <memory.h>

typedef struct {
	float x;
	float y;
	float z;
}FLTVECT;

typedef struct {
	int a;
	int b;
	int c;
}INT3VECT;

typedef struct {
	int nv;
	int nf;
	FLTVECT *vertex;
	INT3VECT *face;
}SurFaceMesh;

//additional functions
SurFaceMesh parseOFF();
void createMenu();
void menu(int val);

SurFaceMesh surfmesh;

bool fullscreen = false;
bool mouseDown = false;

//keep track of modifications
float xrot = 0.0f;
float yrot = 0.0f;
float xtran = 0.0f;
float ytran = 0.0f;
float zoom = 1.0f;

//decides whether we are rotating, scaling, translating ('r','s','t')
char transMode = 'r';

float xdiff = 0.0f;
float ydiff = 0.0f;
float xdiff2 = 0.0f;
float ydiff2 = 0.0f;
float xdiff3 = 0.0f;

//int for our menu
int mainMenu;
//int to keep track of polygon mode (fill by default)
int polygonMode = 1;

void drawMesh() {
	switch (polygonMode) {
	case 0:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case 1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 3:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	}

	glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	for (int f = 0; f < surfmesh.nf; ++f) {
		//grab three vertex indecies for each face
		int a = surfmesh.face[f].a;
		int b = surfmesh.face[f].b;
		int c = surfmesh.face[f].c;
		glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
		glVertex3f(surfmesh.vertex[b].x, surfmesh.vertex[b].y, surfmesh.vertex[b].z);
		glVertex3f(surfmesh.vertex[c].x, surfmesh.vertex[c].y, surfmesh.vertex[c].z);
	}
	glEnd();

	//draw again if fill and line selected
	if (polygonMode == 3) {
		glColor3f(1, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_TRIANGLES);
		for (int f = 0; f < surfmesh.nf; ++f) {
			//grab three vertex indecies for each face
			int a = surfmesh.face[f].a;
			int b = surfmesh.face[f].b;
			int c = surfmesh.face[f].c;
			glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
			glVertex3f(surfmesh.vertex[b].x, surfmesh.vertex[b].y, surfmesh.vertex[b].z);
			glVertex3f(surfmesh.vertex[c].x, surfmesh.vertex[c].y, surfmesh.vertex[c].z);
		}
	}

	glEnd();
}

bool init()
{
	glClearColor(0.93f, 0.93f, 0.93f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);

	return true;
}

void display()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(
		30.0f, 90.0f, 30.0f,
		0.0f, 20.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	//rotate
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
	//translate
	glTranslatef(xtran, 0.0f, ytran);
	//scale
	glScalef(zoom, zoom, zoom);
	//draw mesh (or OFF parse)
	drawMesh();

	glFlush();
	glutSwapBuffers();
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);

	gluPerspective(60.0f, 1.0f * w / h, 1.0f, 120.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle()
{
	if (!mouseDown)
	{
		xrot += 0.3f;
		yrot += 0.4f;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't':
		transMode = 't';
		break;
	case 'r':
		transMode = 'r';
		break;
	case 's':
		transMode = 's';
		break;
	case 27:
		exit(1); break;
	default:
		break;
	}
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;

		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(500, 500);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = true;
		switch (transMode) {
		case 'r':
			xdiff = x - yrot;
			ydiff = -y + xrot;
			break;
		case 't':
			xdiff2 = x - ytran;
			ydiff2 = -y + xtran;
		case 's':
			xdiff3 = x;
		}
	}
	else
		mouseDown = false;
}

void mouseMotion(int x, int y)
{
	if (mouseDown)
	{
		switch (transMode) {
		case 'r':
			yrot = x - xdiff;
			xrot = y + ydiff;
			break;
		case 't':
			xtran = x - xdiff2;
			ytran = y + ydiff2;
			break;
		case 's':
			zoom = ((float)x / (float)xdiff3);
			if (zoom < 0.5f)
				zoom = 0.5f;
			if (zoom > 2.0f)
				zoom = 2.0f;
			break;
		}
		glutPostRedisplay();
	}
}

int main(int argc, char *argv[])
{
	printf("\n\
   Right click to access the menu\n\
	use mouse to modify the object \n\
	press 't', 'r', or 's' to switch to translation, rotation, or scaling\n\
	press 'ESC' to exit\n\
   \n");

	glutInit(&argc, argv);

	//grab surface mesh parse
	surfmesh = parseOFF();

	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow("13 - Solid Shapes");

	//create menu
	createMenu();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	//glutIdleFunc(idle);

	if (!init())
		return 1;

	glutMainLoop();

	return 0;
}

//create menu
void createMenu() {

	mainMenu = glutCreateMenu(menu);
	glutAddMenuEntry("POINT", 0);
	glutAddMenuEntry("LINE", 1);
	glutAddMenuEntry("FILL", 2);
	glutAddMenuEntry("BOTH", 3);
	glutAddMenuEntry("EXIT", 4);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//menu function
//polygonMode controls glPolygonMode
void menu(int val) {
	switch (val) {
	case 0:
		polygonMode = 0;
		break;
	case 1:
		polygonMode = 1;
		break;
	case 2:
		polygonMode = 2;
		break;
	case 3:
		polygonMode = 3;
		break;
	case 4:
		exit(0);
		break;
	}
	glutPostRedisplay();
}

SurFaceMesh parseOFF() {
	int num, n, m;
	int a, b, c, d, e;
	float x, y, z;
	SurFaceMesh *surfmesh;
	char line[256];
	FILE *fin;


	if ((fin = fopen("inputmesh.off", "r")) == NULL) {
		printf("read error...\n");
		exit(0);
	};

	/* OFF format */
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'O' && line[1] == 'F' && line[2] == 'F')
			break;
	}
	fscanf(fin, "%d %d %d\n", &m, &n, &num);

	surfmesh = (SurFaceMesh*)malloc(sizeof(SurFaceMesh));
	surfmesh->nv = m;
	surfmesh->nf = n;
	surfmesh->vertex = (FLTVECT *)malloc(sizeof(FLTVECT)*surfmesh->nv);
	surfmesh->face = (INT3VECT *)malloc(sizeof(INT3VECT)*surfmesh->nf);

	for (n = 0; n < surfmesh->nv; n++) {
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		surfmesh->vertex[n].x = x;
		surfmesh->vertex[n].y = y;
		surfmesh->vertex[n].z = z;
	}

	for (n = 0; n < surfmesh->nf; n++) {
		fscanf(fin, "%d %d %d %d\n", &a, &b, &c, &d);
		surfmesh->face[n].a = b;
		surfmesh->face[n].b = c;
		surfmesh->face[n].c = d;
		if (a != 3)
			printf("Errors: reading mesh .... \n");
	}
	fclose(fin);

	return *surfmesh;
}