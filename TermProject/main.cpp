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
	float normal[3];
}FLTVECT;

typedef struct INT3VECT {
	int vertex[30];
	int count = 0;
	float normal[3];
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
void animationMenuFunc(int val);
void shiftColor();
void shiftLight();
void updateLightPos(GLfloat* currPos);

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
GLfloat lpos[] = { 0, 0, 0, 1 },
lpos2[] = { 0, 0, 0, 1 },
ldir[] = {0, 0, 0},
ldir2[] = {0, 0, 0},
lColorDiffuse[] = {1, 1, 1, 1};

//Lighting positions in polar
static GLfloat r = 3;//radius
GLfloat pi = 3.14159265359;//angle to z axis
GLfloat currPos[] = { pi, 0 };
GLfloat pos0[] = { pi, 0 };
GLfloat pos1[] = { pi/2, 0 };
GLfloat pos2[] = { pi/2, pi/2 };
GLfloat pos3[] = { 3 * pi / 2, -1*pi/4 };
GLfloat pos4[] = { 1.5 * pi / 2, 7 * pi / 8 };
GLfloat pos5[] = { 2.222 * pi / 2, 0 };
GLfloat pos6[] = { 2, 3 };
GLfloat pos7[] = { 1, 1 };
GLfloat* positions[] = { pos0, pos1, pos2, pos3, pos4, pos5, pos6, pos7 };
int nextPos = 0;
float posIncrement = 0.01;

//shading
int shadeMode = 0;
//0 = flat, 1 = Gouraud
float specular[4] = { 0.2, 0.2, 0.2, 1 };

//colors
static GLfloat black[] = { 0,0,0,0 };
static GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat red[] = { 1.0, 0.0, 0.0, 1.0 };
static GLfloat orange[] = { 1.0, 0.5, 0.0, 1.0 };
static GLfloat yellow[] = { 1.0, 1.0, 0.0, 1.0 };
static GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 };
static GLfloat blue[] = { 0.0, 0.0, 1.0, 1.0 };
static GLfloat cyan[] = { 0.0, 1.0, 1.0, 1.0 };
static GLfloat magenta[] = { 1.0, 0.0, 1.0, 1.0 };
static GLfloat* colors[] = { white, red, orange, yellow, green, blue, cyan, magenta };
int nextColor = 0;
//0-7 depending on next color
float colorIncrement = 0.01;
//amount color values change per step

//animation
int animationMode = 0;
//0 = none, 1 = light, 2 = color, 3 = light+color

//int for our menu
int mainMenu;
int modelMenu;
int animationMenu;
//int to keep track of polygon mode (fill by default)
int polygonMode = 2;

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
	}
	glColor3f(0, 0, 1);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);

	for (int f = 0; f < surfmesh.nf; ++f) {
		//grab number of vertexes defined by face
		glBegin(GL_POLYGON);

		//shading mode
		if (shadeMode == 0)
			glShadeModel(GL_FLAT);
		if(shadeMode == 1)
			glShadeModel(GL_SMOOTH);

		//flat shading
		if (shadeMode == 0)
			glNormal3fv(surfmesh.face[f].normal);

		for (int i = 0; i < surfmesh.face[f].count; ++i) {
			int a = surfmesh.face[f].vertex[i];
			
			//Gouraud shading
			if(shadeMode == 1)
				glNormal3fv(surfmesh.vertex[a].normal);

			glVertex3f(surfmesh.vertex[a].x, surfmesh.vertex[a].y, surfmesh.vertex[a].z);
		}
		glEnd();
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
	glEnable(GL_LIGHT1);

	updateLightPos(currPos);

	return true;
}

void display()
{
	glClearColor(0.1, 0.1, 0.1, 1.0);
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
	glLightfv(GL_LIGHT0, GL_SPECULAR, lColorDiffuse);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lColorDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);//remove ambient lighting for spotlight
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 60);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 50.0);

	glLightfv(GL_LIGHT1, GL_POSITION, lpos2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lColorDiffuse);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lColorDiffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, black);//remove ambient lighting for spotlight
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, ldir2);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 50.0);

	//light as a sphere
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, lColorDiffuse);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();
	glTranslatef(lpos[0], lpos[1], lpos[2]);
	glutSolidSphere(0.2, 10, 10);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(lpos2[0], lpos2[1], lpos2[2]);
	glutSolidSphere(0.2, 10, 10);
	glPopMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
	//end light
	
	//draw mesh (or OFF parse)
	glEnable(GL_NORMALIZE);
	drawMesh();

	glFlush();
	glutSwapBuffers();
	
	//handle animation
	switch (animationMode)
	{
	case 0:
		break;
	case 1:
		shiftLight();
		glutPostRedisplay();
		break;
	case 2:
		shiftColor();
		glutPostRedisplay();
		break;
	case 3:
		shiftLight();
		shiftColor();
		glutPostRedisplay();
		break;
	}
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);

	gluPerspective(40.0f, 1.0f * w / h, 1.0f, 10.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'l':
	case 'L':
		animationMode = 1;
		glutPostRedisplay();
		break;
	case 'c':
	case 'C':
		animationMode = 2;
		glutPostRedisplay();
		break;
	case 'b':
	case 'B':
		animationMode = 3;
		glutPostRedisplay();
		break;
	case 'n':
	case 'N':
		animationMode = 0;
		glutPostRedisplay();
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
	surfmesh = parseOFF("seashell.off");

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
	animationMenu = glutCreateMenu(animationMenuFunc);
	glutAddMenuEntry("None", 0);
	glutAddMenuEntry("Lights", 1);
	glutAddMenuEntry("Color", 2);
	glutAddMenuEntry("Both", 3);

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
	glutAddSubMenu("Animation", animationMenu);
	//glutAddMenuEntry("POINT", 0);
	//glutAddMenuEntry("LINE", 1);
	//glutAddMenuEntry("FILL", 2);
	glutAddMenuEntry("FLAT SHADING", 4);
	glutAddMenuEntry("SMOOTH SHADING", 5);
	glutAddMenuEntry("EXIT", 7);

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
	case 4:
		shadeMode = 0;
		break;
	case 5:
		shadeMode = 1;
		break;
	case 7:
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

void animationMenuFunc(int val) {
	switch (val)
	{
	case 0:
		animationMode = 0;
		glutPostRedisplay();
		break;
	case 1:
		animationMode = 1;
		glutPostRedisplay();
		break;
	case 2:
		animationMode = 2;
		glutPostRedisplay();
		break;
	case 3:
		animationMode = 3;
		glutPostRedisplay();
		break;
	default:
		break;
	}
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

void findNormals(SurFaceMesh *mesh) {
	//calculate normal at each face --FOR FLAT
	for (int face = 0; face < mesh->nf; ++face) {
		//find 3 vertex => 2 vectors => cross product => Face's Normal
		int vertNums[3] = { mesh->face[face].vertex[0],
							mesh->face[face].vertex[1],
							mesh->face[face].vertex[2] };
		float v1[3] = {	mesh->vertex[vertNums[1]].x - mesh->vertex[vertNums[0]].x,
						mesh->vertex[vertNums[1]].y - mesh->vertex[vertNums[0]].y,
						mesh->vertex[vertNums[1]].z - mesh->vertex[vertNums[0]].z };
		float v2[3] = { mesh->vertex[vertNums[2]].x - mesh->vertex[vertNums[0]].x,
						mesh->vertex[vertNums[2]].y - mesh->vertex[vertNums[0]].y,
						mesh->vertex[vertNums[2]].z - mesh->vertex[vertNums[0]].z };
		mesh->face[face].normal[0] = v1[1] * v2[2] - v2[1] * v1[2];
		mesh->face[face].normal[1] = -v1[0] * v2[2] + v2[0] * v1[2];
		mesh->face[face].normal[2] = v1[0] * v2[1] - v2[0] * v1[1];
	}
	
	//calculate normal at each vertex --FOR GAURAUDE
		//use face normals to find vertex normals
	int faces[200];
	int faceCount = 0;
	for (int vert = 0; vert < mesh->nv; ++vert) {//vert = current vert id
		//find all faces using vertex
			//weight by surface area
		faceCount = 0;
		for (int face = 0; face < mesh->nf; ++face) {//for faces
			for (int faceVert = 0; faceVert < mesh->face[face].count; ++faceVert) {//for verts in face
				if (mesh->face[face].vertex[faceVert] == vert) {
					faces[faceCount] = face;
					faceCount += 1;
				}
			}
		}
		//faces[] - list of faces using current vertex
		// Find face normals => average of normals => vertex normal
		float vertNormal[3] = {0,0,0};
		for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
			vertNormal[0] += mesh->face[faces[faceIndex]].normal[0];
			vertNormal[1] += mesh->face[faces[faceIndex]].normal[1];
			vertNormal[2] += mesh->face[faces[faceIndex]].normal[2];
		}
		vertNormal[0] = vertNormal[0] / faceCount;
		vertNormal[1] = vertNormal[1] / faceCount;
		vertNormal[2] = vertNormal[2] / faceCount;
		mesh->vertex[vert].normal[0] = vertNormal[0];
		mesh->vertex[vert].normal[1] = vertNormal[1];
		mesh->vertex[vert].normal[2] = vertNormal[2];
	}

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

	for (n = 0; n < surfmesh->nv; n++) {//scan vertex
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		surfmesh->vertex[n].x = x;
		surfmesh->vertex[n].y = y;
		surfmesh->vertex[n].z = z;
		surfmesh->vertexCount += 1;
	}

	for (n = 0; n < surfmesh->nf; n++) {//scan faces
		//scan only num of vertex
		//then for num of vertex, grab each vertex index individually
		//scan a new line last
		fscanf(fin, "%d", &a);
		if (a > 30) {
			printf("Error adding face(too many vertecies");
		}
		surfmesh->face[n].count = a;
		for (int vertNum = 0; vertNum < a; ++vertNum) {//each face
			fscanf(fin, "%d", &b);
			surfmesh->face[n].vertex[vertNum] = b;
		}
		fscanf(fin, "\n");
	}
	fclose(fin);
	centerModel(surfmesh);
	findNormals(surfmesh);
	return *surfmesh;
}

void shiftLight() {
	bool changePos[] = { false, false };
	for (int i = 0; i < 2; ++i) {
		if (currPos[i] < positions[nextPos][i]) {
			currPos[i] += posIncrement;
		}
		else {
			currPos[i] -= posIncrement;
		}
		if (abs(currPos[i] - positions[nextPos][i]) <= posIncrement * 1.5) {
			changePos[i] = true;
		}
	}
	if (changePos[0] && changePos[1]) {
		++nextPos;
		if (nextPos > 7)
			nextPos = 0;
	}
	updateLightPos(currPos);
}

//GLfloat currPos[] = { 0, 0 };//theta to y, phi to z;

//GLfloat lpos[] = {0, 2, 0, 1},
//ldir[] = { 0,-1,0 },
void updateLightPos(GLfloat* currPos) {
	//change theta, phi into x,y,z
	lpos[0] = r * cos(currPos[1]) * sin(currPos[0]);
	lpos[1] = r * sin(currPos[1]) * sin(currPos[0]);
	lpos[2] = r * cos(currPos[0]);
	lpos2[0] = -lpos[0];
	lpos2[1] = -lpos[1];
	lpos2[2] = -lpos[2];
	//update ldir to point at origin
	ldir[0] = -cos(currPos[1]) * sin(currPos[0]);
	ldir[1] = -sin(currPos[1]) * sin(currPos[0]);
	ldir[2] = -cos(currPos[0]);
	ldir2[0] = -ldir[0];
	ldir2[1] = -ldir[1];
	ldir2[2] = -ldir[2];
}

void shiftColor() {
	bool changeColor[] = { false, false, false };
	for (int color = 0; color < 3; ++color) {//each color
		if (lColorDiffuse[color] < colors[nextColor][color]) {//current color < wanted color
			lColorDiffuse[color] += colorIncrement;
		}
		else {
			lColorDiffuse[color] -= colorIncrement;
		}
		if (abs(lColorDiffuse[color] - colors[nextColor][color]) <= colorIncrement * 1.5) {
			changeColor[color] = true;
		}
	}
	if (changeColor[0] && changeColor[1] && changeColor[2]) {
		++nextColor;
		if (nextColor > 7)
			nextColor = 0;
	}
}

