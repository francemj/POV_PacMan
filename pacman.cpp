#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

float angle = 0.0f;

// hold main camera
float mX=0.0f;
float mZ=-1.0f;
float mY=0.0f;

// hold xz camera
float x=0.0f;
float z=5.0f;
float y=1.75f;

// key states
float dAngle=0.0f;
float dMove = 0;
float dX=-1;

// width and height of window
int winHeight;
int winWidth;

// computer framerate
int frame;
long time, timebase;
char s[50];

int mainWin, gameWin, topWin, sideWin, scoreWin;
int winBorder = 6;

// Light Position
float pos[4] = {x,0,z,0};

float glow_amb[] = {0.1f, 0, 0.1f, 1.0};
float glow_dif[] = {1, 0, 0, 1.0};
float glow_spec[] = {0.1f, 0.1f, 0.1f, 1.0};
float glow_em[] = {0,0,0,0.5f};
float shiny = 5; //10, 100

void setProjection(int width, int height) {
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, width, height);
	gluPerspective(45, (width*1.0)/height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void resize(int width, int height) {
	if(height == 0)
		height = 1;

	winWidth = width;
	winHeight = height;

	glutSetWindow(gameWin);
	glutPositionWindow(winBorder,winBorder);
	glutReshapeWindow(2*(winWidth/3.0) - 2*winBorder,  winHeight - winBorder*3/2);
	setProjection(2*(winWidth/3.0) - 2*winBorder, winHeight - winBorder*3/2);

	glutSetWindow(topWin);
	// resize and reposition the sub window
	glutPositionWindow(2*(winWidth/3.0) + winBorder/2, winBorder);
	glutReshapeWindow(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	setProjection(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);

	// set subwindow 3 as the active window
	glutSetWindow(sideWin);
	// resize and reposition the sub window
	glutPositionWindow(2*(winWidth/3.0) + winBorder/2, (winHeight/3.0) + winBorder/2);
	glutReshapeWindow(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	setProjection(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);

	glutSetWindow(scoreWin);
	glutPositionWindow(2*(winWidth/3.0) + winBorder/2, 2*(winHeight/3.0) + winBorder/2);
	glutReshapeWindow(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	setProjection(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);

}

// 	**********************************
// 			Lighting Functions
// 	**********************************

void resetLightingProperties() {

	glow_dif[0] = 0;
	glow_dif[1] = 0;
	glow_dif[2] = 1;
	glow_dif[3] = 1;

	for(int i = 0; i < 4; i++){
		glow_em[i] = 0; 
	}

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
}

void setPacDotsColour() {
	glow_em[0] = 1; 
	glow_em[1] = 1; 
	glow_em[2] = 0; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
}

void setPacManColour() {
	glow_dif[0] = 1; 
	glow_dif[1] = 1; 
	glow_dif[2] = 0; 
	glow_dif[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_dif);
}

void setPowerUpColour() {
	glow_em[0] = 1; 
	glow_em[1] = 0; 
	glow_em[2] = 1; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
}

void setRedGlow() {
	glow_em[0] = 1; 
	glow_em[1] = 0; 
	glow_em[2] = 0; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
}

void setGroundColour() {
	glow_em[0] = 0; 
	glow_em[1] = 0; 
	glow_em[2] = 1; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
}

void setBlack(){
	for(int i = 0; i < 4; i++){
		glow_dif[i] = 0; 
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
}

void setWhite(){
	for(int i = 0; i < 4; i++){
		glow_dif[i] = 1; 
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
}

void setGhostColour() {
	glow_dif[0] = 0.25f; 
	glow_dif[1] = 0.25f; 
	glow_dif[2] = 0.25f; 
	glow_dif[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
}

// 	***************************
// 			Dots Models
// 	***************************

void drawPacDots() {
	glPushMatrix();
		setPacDotsColour();
		glTranslatef(0,1.5f,0);
		glutSolidSphere(0.15,50,50);
		resetLightingProperties();
	glPopMatrix();
}

void drawPowerUps() {
	glPushMatrix();
		setPowerUpColour();
		glTranslatef(0,1.5f,0);
		glutSolidSphere(0.3,50,50);
		resetLightingProperties();
	glPopMatrix();
}

void drawPacMan() {
	glutSolidSphere(0.5, 10, 10);
}

void drawGhost() {

	setGhostColour();

	// cylinder body using quadric object
	glTranslatef(0.0f, 0.75f, 0.0f);
	glRotatef(-90, 1.0, 0.0, 0.0);
	GLUquadricObj *quadObj;
	quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	gluCylinder(quadObj, 0.6, 0.6, 1.2, 40, 6);
	glRotatef(90, 1.0, 0.0, 0.0);

	// head
	glTranslatef(0.0f, 1.2f, 0.0f);
	glutSolidSphere(0.6f,20,20);

	// eyes
	glPushMatrix();
	setWhite();
	glTranslatef(0.1f, 0.10f, 0.6f);
	glutSolidSphere(0.05f,10,10);
	glTranslatef(-0.2f, 0.0f, 0.0f);
	glutSolidSphere(0.05f,10,10);
	glPopMatrix();
	resetLightingProperties();
	
}

// create a string using glut
void renderBitmapString(
	float x,
	float y,
	float z,
	void *font,
	char *string
	) {
	char *c;
	glRasterPos3f(x,y,z);
	for(c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

void switchPerspectiveProj() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void switchOrthographicProj() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, winWidth, winHeight, 0);
	glMatrixMode(GL_MODELVIEW);
}

void updatePosition(float movement){
	x += movement *0.1f*mX;
	z += movement *0.1f*mZ;
}

void renderShapes() {
	// create 100x100 grey background for ground
	setGroundColour();
		glBegin(GL_QUADS);
			glVertex3f(-100.0f, 0.0f, -100.0f);
			glVertex3f(-100.0f, 0.0f,  100.0f);
			glVertex3f( 100.0f, 0.0f,  100.0f);
			glVertex3f( 100.0f, 0.0f, -100.0f);
		glEnd();
	resetLightingProperties();

	// create ghosts
	for(int i=-3; i < 3; i++) {
		glPushMatrix();
		glTranslatef(i*10.0f, 0.0f, 10.0f);
		drawGhost();
		glPopMatrix();
	}

	// create Pac Dots
	for(int i = -3; i < 3; i++)
		for(int j=-3; j < 3; j++)
		{
			glPushMatrix();
			glTranslatef(i*10.0f+5, 0.0f, j * 10.0f+5);
			drawPacDots();
			glPopMatrix();
		}

	// create Power Ups
	for(int i = -3; i < 3; i++)
		for(int j=-3; j < 3; j++)
		{
			glPushMatrix();
			glTranslatef(i*10.0f, 0.0f, j*10.0f+5);
			drawPowerUps();
			glPopMatrix();
		}
}

// render mainWin, gameWin, topWin, sideWin, scoreWin
void renderMainWin() {
	glutSetWindow(mainWin);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}

void renderGameWin() {
	glutSetWindow(gameWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(x, y, z, x + mX,y + mY,z + mZ, 0.0f,1.0f,0.0f);

	// create yellow circle
	glPushMatrix();
		setPacManColour();
		glTranslatef(x,y,z);
		drawPacMan();
		resetLightingProperties();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void renderTopWin() {
	glutSetWindow(topWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(x, y + 15, z, x,y - 1,z, mX, 0.0f, mZ);

	// create yellow circle
	glPushMatrix();
		setPacManColour();
		glTranslatef(x,y,z);
		drawPacMan();
		resetLightingProperties();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void renderSideWin() {
	glutSetWindow(sideWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(x-mZ*10, y, z+mX*10, x, y, z, 0.0f, 1.0f, 0.0f);

	// create yellow circle
	glPushMatrix();
		setPacManColour();
		glTranslatef(x,y,z);
		drawPacMan();
		resetLightingProperties();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void renderScoreWin() {
	glutSetWindow(scoreWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	frame++;

	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf(s,"PacMan FPS:%4.2f",
			frame*1000.0/(time-timebase));
		timebase = time;
		frame = 0;
	}

	switchOrthographicProj();

	glPushMatrix();
	glLoadIdentity();
	renderBitmapString(5,30,0,GLUT_BITMAP_HELVETICA_12,s);
	glPopMatrix();

	switchPerspectiveProj();

	// create yellow circle
	glPushMatrix();
		setPacManColour();
		glTranslatef(x,y,z);
		drawPacMan();
		resetLightingProperties();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void setScene() {
	if (dMove) {
		updatePosition(dMove);
		glutSetWindow(mainWin);
		glutPostRedisplay();
	}

	renderGameWin();
	renderTopWin();
	renderSideWin();
	renderScoreWin();
}

void keyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {

		case 27: {
			glutDestroyWindow(mainWin);
			exit(0);
			break;
		}
	}
	glutPostRedisplay();
}

void dSpecialKeyboard(int key, int xIn, int yIn) {
	switch (key) {
		case GLUT_KEY_UP : dMove = 0.5f; break;
		case GLUT_KEY_DOWN : dMove = -0.5f; break;
	}
	glutSetWindow(mainWin);
	glutPostRedisplay();
}


void uSpecialKeyboard(int key, int xIn, int yIn) {
	switch (key) {
		case GLUT_KEY_UP :
		case GLUT_KEY_DOWN : dMove = 0; break;
	}
}

void mouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {

		// onClick UP
		if (state == GLUT_UP) {
			
		}
		else  { // onClick DOWN

		}
	}
}

void mouseMotion(int x, int y){
}

void mouseMove(int x, int y) {
	dAngle = (x - dX) * 0.010f;

	mX = sin(angle + dAngle);
	mZ = -cos(angle + dAngle);

	glutSetWindow(mainWin);
	glutPostRedisplay();
}


void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glow_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

	// register callbacks
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(dSpecialKeyboard);
	glutSpecialUpFunc(uSpecialKeyboard);
	glutMouseFunc(mouseButton);
	glutPassiveMotionFunc(mouseMove);
}

int main(int argc, char **argv) {

	// init GLUT and create main window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	mainWin = glutCreateWindow("PacMan 3D");

	// callbacks for main window
	glutDisplayFunc(setScene);
	glutReshapeFunc(resize);
	init();

	// sub windows
	gameWin = glutCreateSubWindow(mainWin, winBorder,winBorder,2*(winWidth/3.0) - 2*winBorder, winHeight - winBorder*3/2);
	glutDisplayFunc(renderGameWin);
	init();

	topWin = glutCreateSubWindow(mainWin, 2*(winWidth/3.0) + winBorder/2, winBorder, winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	glutDisplayFunc(renderTopWin);
	init();

	sideWin = glutCreateSubWindow(mainWin, 2*(winWidth/3.0) + winBorder/2, (winHeight/3.0) + winBorder/2, winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	glutDisplayFunc(renderSideWin);
	init();

	scoreWin = glutCreateSubWindow(mainWin, 2*(winWidth/3.0) + winBorder/2, 2*(winHeight/3.0) + winBorder/2, winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	glutDisplayFunc(renderScoreWin);
	init();

	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}

