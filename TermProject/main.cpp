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
	int vertexCount = 0;
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

//keep track of camera rotation
float xrot = 0.0f,
yrot = 0.0f,
xdiff = 0.0f,
ydiff = 0.0f,
xCamera=3, yCamera=0, zCamera=3;

//lighting
GLfloat lpos[] = {0, 2, 0, 1},
ldir[] = {0,-1,0};

//colors
static GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };
static GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat gray[] = { 0.5, 0.5, 0.5, 1.0 };
static GLfloat red[] = { 1.0, 0.0, 0.0, 1.0 };
static GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 };
static GLfloat blue[] = { 0.0, 0.0, 1.0, 1.0 };
static GLfloat yellow[] = { 1.0, 1.0, 0.0, 1.0 };
static GLfloat magenta[] = { 1.0, 0.0, 1.0, 1.0 };
static GLfloat cyan[] = { 0.0, 1.0, 1.0, 1.0 };
static GLfloat darkcyan[] = { 0.0, 0.4, 0.4, 1.0 };


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
	glShadeModel(GL_FLAT);
	glColor3f(0, 0, 1);
	for (int f = 0; f < surfmesh.nf; ++f) {
		//grab number of vertexes defined by face
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
			//grab number of vertexes defined by face
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
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHT0);

	return true;
}

void display()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//camera
	//calculate x,y,z for xrot yrot
	if (xrot > 3.1414 / 2.1 * 100)
		xrot = 3.1414 / 2.1 * 100;
	if (xrot < -3.1414 / 2.1 * 100)
		xrot = -3.1414 / 2.1 * 100;
	xCamera = 3 * sin(yrot / 100);
	yCamera = 3 * sin(xrot / 100);
	zCamera = 3 * (cos(yrot / 100) * -cos(xrot / 100));

	//eye-x,y,z;center-x,y,z;up-x,y,z;
	//xCamera, yCamera, zCamera,
	gluLookAt(
		xCamera, yCamera, zCamera,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	//end camera

	//light
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);

	//light as a sphere
	glMaterialfv(GL_FRONT, GL_EMISSION, white);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();
	glTranslatef(lpos[0], lpos[1], lpos[2]);
	glutSolidSphere(0.2, 10, 10);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_EMISSION, black);
	//end light
	
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

	gluPerspective(40.0f, 1.0f * w / h, 1.0f, 5.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
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
		xdiff = x - yrot;
		ydiff = -y + xrot;
		
	}
	else
		mouseDown = false;
}

void mouseMotion(int x, int y)
{
	if (mouseDown)
	{
		yrot = x - xdiff;
		xrot = y + ydiff;
		glutPostRedisplay();
	}
}

int main(int argc, char *argv[])
{
	printf("\n\
   Right click to access the menu\n\
	use mouse to move the camera\n\
	press 'ESC' to exit\n\
   \n");

	glutInit(&argc, argv);

	//grab surface mesh parse
	surfmesh = parseOFF("inputmesh.off");

	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow("Term Project");

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

void centerModel(SurFaceMesh *mesh) {
	//todo
	
	float max = -1;
	float ave[3] = {0, 0, 0};
	int vertexCount = mesh->vertexCount;
	//find absolute max of x,y,z
	for (int v = 0; v < vertexCount; ++v) {//vertexes
		float x = mesh->vertex[v].x,
			  y = mesh->vertex[v].y,
			  z = mesh->vertex[v].z;
		ave[0] += x;
		ave[1] += y;
		ave[2] += z;
		if (abs(x) > max)
			max = abs(x);
		if (abs(y) > max)
			max = abs(y);
		if (abs(z) > max)
			max = abs(z);
	}
	//find average position of points in x,y,z
	for (int d = 0; d < 3; ++d) {//directions
		ave[d] = ave[d] / float(vertexCount);
	}
	//shift all x,y,z so average is on origin
	//devide all x,y,z by max of respecting axis
	for (int vert = 0; vert < vertexCount; ++vert) {
		mesh->vertex[vert].x -= ave[0];
		mesh->vertex[vert].y -= ave[1];
		mesh->vertex[vert].z -= ave[2];
		
		mesh->vertex[vert].x = mesh->vertex[vert].x / max;
		mesh->vertex[vert].y = mesh->vertex[vert].y / max;
		mesh->vertex[vert].z = mesh->vertex[vert].z / max;
	}

	//object will be at most 1 big in each direction

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
	surfmesh->vertexCount = 0;
	for (n = 0; n < surfmesh->nv; n++) {
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		surfmesh->vertex[n].x = x;
		surfmesh->vertex[n].y = y;
		surfmesh->vertex[n].z = z;
		surfmesh->vertexCount += 1;
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
	centerModel(surfmesh);
	return *surfmesh;
}