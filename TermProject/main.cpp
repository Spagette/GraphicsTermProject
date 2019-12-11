#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h> // (or others, depending on the system in use)
#include <iostream>
#include <string.h>
#include <memory.h>
#include <string>

typedef struct FLTVECT {
	float x;
	float y;
	float z;
}FLTVECT;

typedef struct INT3VECT {
	int vertex[30];
	int count = 0;
}INT3VECT;
//todo: change struct from 3 diff ints to array of ints to allow num of vertex >3 per face

typedef struct SurFaceMesh {
	int nv;
	int nf;
	FLTVECT *vertex;
	INT3VECT *face;
}SurFaceMesh;

//additional functions
SurFaceMesh parseOFF(const char *model);
void createMenu();
void menuFunc(int val);
void modelMenuFunc(int val);

SurFaceMesh surfmesh;

bool fullscreen = false;
bool mouseDown = false;

//keep track of modifications
float xrot = 0.0f;
float yrot = 0.0f;

//decides whether we are rotating, scaling, translating ('r','s','t')
char transMode = 'r';

float xdiff = 0.0f;
float ydiff = 0.0f;

//int for our menu
int mainMenu;
int modelMenu;
//int to keep track of polygon mode (fill by default)
int polygonMode = 3;

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
	for (int f = 0; f < surfmesh.nf; ++f) {
		//grab three vertex indecies for each face
		//int a = surfmesh.face[f].vertex[0];
		//int b = surfmesh.face[f].vertex[1];
		//int c = surfmesh.face[f].vertex[2];
		//glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
		//glVertex3f(surfmesh.vertex[b].x, surfmesh.vertex[b].y, surfmesh.vertex[b].z);
		//glVertex3f(surfmesh.vertex[c].x, surfmesh.vertex[c].y, surfmesh.vertex[c].z);
		glBegin(GL_POLYGON);
		for (int i = 0; i < surfmesh.face[f].count; ++i) {
			int a = surfmesh.face[f].vertex[i];
			glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
		}
		glEnd();
	}

	//draw again if fill and line selected
	if (polygonMode == 3) {
		glColor3f(1, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		for (int f = 0; f < surfmesh.nf; ++f) {
			//grab three vertex indecies for each face
			//int a = surfmesh.face[f].vertex[0];
			//int b = surfmesh.face[f].vertex[1];
			//int c = surfmesh.face[f].vertex[2];
			//glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
			//glVertex3f(surfmesh.vertex[b].x, surfmesh.vertex[b].y, surfmesh.vertex[b].z);
			//glVertex3f(surfmesh.vertex[c].x, surfmesh.vertex[c].y, surfmesh.vertex[c].z);
			glBegin(GL_POLYGON);
			for (int i = 0; i < surfmesh.face[f].count; ++i) {
				int a = surfmesh.face[f].vertex[i];
				glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
			}
			glEnd();
		}
	}
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
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	//rotate
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
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

	gluPerspective(60.0f, 1.0f * w / h, 1.0f, 360.0f);

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
	surfmesh = parseOFF("space_shuttle.off");

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

	modelMenu = glutCreateMenu(modelMenuFunc);
	glutAddMenuEntry("Brain", 0);
	glutAddMenuEntry("Apple", 1);
	glutAddMenuEntry("asu", 2);
	glutAddMenuEntry("Dragon", 3);
	glutAddMenuEntry("Goblet", 4);
	glutAddMenuEntry("Head", 5);
	glutAddMenuEntry("Heart", 6);
	glutAddMenuEntry("Helm", 7);
	glutAddMenuEntry("House", 8);
	glutAddMenuEntry("King", 9);
	glutAddMenuEntry("Klingon", 10);
	glutAddMenuEntry("Mushroom", 11);
	glutAddMenuEntry("Pear", 12);
	glutAddMenuEntry("R2", 13);
	glutAddMenuEntry("Seashell", 14);
	glutAddMenuEntry("Space Shuttle", 15);
	glutAddMenuEntry("Space Station", 16);
	glutAddMenuEntry("Sword", 17);
	glutAddMenuEntry("Teapot", 18);
	glutAddMenuEntry("Volks", 19);
	glutAddMenuEntry("X29 Plane", 20);

	mainMenu = glutCreateMenu(menuFunc);
	glutAddSubMenu("Models", modelMenu);
	glutAddMenuEntry("POINT", 0);
	glutAddMenuEntry("LINE", 1);
	glutAddMenuEntry("FILL", 2);
	glutAddMenuEntry("BOTH", 3);
	glutAddMenuEntry("EXIT", 4);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//menu function
//polygonMode controls glPolygonMode
void menuFunc(int val) {
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

//model menu function
void modelMenuFunc(int val) {
	std::string filename;
	switch (val) {
	case 0:
		filename = "inputmesh.off";
		break;
	case 1:
		filename = "Apple.off";
		break;
	case 2:
		filename = "asu.off";
		break;
	case 3:
		filename = "dragon.off";
		break;
	case 4:
		filename = "goblet.off";
		break;
	case 5:
		filename = "head.off";
		break;
	case 6:
		filename = "heart.off";
		break;
	case 7:
		filename = "helm.off";
		break;
	case 8:
		filename = "house.off";
		break;
	case 9:
		filename = "king.off";
		break;
	case 10:
		filename = "klingon.off";
		break;
	case 11:
		filename = "mushroom.off";
		break;
	case 12:
		filename = "pear.off";
		break;
	case 13:
		filename = "r2.off";
		break;
	case 14:
		filename = "seashell.off";
		break;
	case 15:
		filename = "space_shuttle.off";
		break;
	case 16:
		filename = "space_station.off";
		break;
	case 17:
		filename = "Sword01.off";
		break;
	case 18:
		filename = "teapot.off";
		break;
	case 19:
		filename = "volks.off";
		break;
	case 20:
		filename = "x29_plane.off";
		break;
	}
	const char *filenamePointer = filename.c_str();
	surfmesh = parseOFF(filenamePointer);
	glutPostRedisplay();
}

SurFaceMesh parseOFF(const char* model) {
	int num, n, m;
	int a, b, c, d, e;
	float x, y, z;
	SurFaceMesh *surfmesh;
	char line[256];
	FILE *fin;

	if ((fin = fopen(model, "r")) == NULL) {
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
		//scan only num of vertex
		//then for num of vertex, grab each vertex index individually
		//scan a new line last

		fscanf(fin, "%d", &a);
		if (a > 30) {
			printf("Error adding face(too many vertecies");
		}
		surfmesh->face[n].count = a;
		for (int vertNum = 0; vertNum < a; ++vertNum) {
			fscanf(fin, "%d", &b);
			surfmesh->face[n].vertex[vertNum] = b;
		}
		fscanf(fin, "\n");

		/* old code
		fscanf(fin, "%d %d %d %d\n", &a, &b, &c, &d);
		surfmesh->face[n].vertex[0] = b;
		surfmesh->face[n].vertex[1] = c;
		surfmesh->face[n].vertex[2] = d;
		
		if (a != 3)
			printf("Errors: reading mesh .... \n");
			*/
	}
	fclose(fin);

	return *surfmesh;
}