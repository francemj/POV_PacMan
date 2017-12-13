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

//values for left and right
float xL;
float xR;
float zL;
float zR;

// hold xz camera
float x=0.0f;
float z=5.0f;
float y=1.75f;

// key states
float dAngle=0.0f;
float dMoveFB = 0;
float dMoveLR = 0;
float dX=-1;

// width and height of window
int winHeight;
int winWidth;

// computer framerate
int frame;
long time, timebase;
char s[50];

//keyboard buttons array
bool keys[4];

int mainWin, gameWin, topWin, sideWin, scoreWin;
int winBorder = 6;

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

void drawGhost() {

	glColor3f(1.0f, 0.0f, 0.0f);

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
	glColor3f(1.0f,1.0f,1.0f);
	glTranslatef(0.1f, 0.10f, 0.6f);
	glutSolidSphere(0.05f,10,10);
	glTranslatef(-0.2f, 0.0f, 0.0f);
	glutSolidSphere(0.05f,10,10);
	glPopMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
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

void updatePositionFB(float movement){
	x += movement *0.1f*mX;
	z += movement *0.1f*mZ;
	//printf("x: %f, z: %f, mX: %f, mY: %f\n", x, z, mX, mZ);
}

void updatePositionLR(float movement){
	x += movement *0.1f*xL;
	z += movement *0.1f*zL;
	//printf("x: %f, z: %f, mX: %f, mY: %f\n", x, z, mX, mZ);
}

void renderShapes() {
	// create 100x100 grey background for ground
	glColor3f(0.9f, 0.9f, 0.9f);
		glBegin(GL_QUADS);
			glVertex3f(-100.0f, 0.0f, -100.0f);
			glVertex3f(-100.0f, 0.0f,  100.0f);
			glVertex3f( 100.0f, 0.0f,  100.0f);
			glVertex3f( 100.0f, 0.0f, -100.0f);
		glEnd();

	// create ghosts
	for(int i=-3; i < 3; i++) {
		glPushMatrix();
		glTranslatef(i*10.0f, 0.0f, 10.0f);
		drawGhost();
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
	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(x,y,z);
	glutSolidSphere(0.2, 4, 4);
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
	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(x,y,z);
	glutSolidSphere(0.2, 4, 4);
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
	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(x,y,z);
	glutSolidSphere(0.2, 4, 4);
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
	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(x,y,z);
	glutSolidSphere(0.2, 4, 4);
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void setScene() {
	if (dMoveFB || dMoveLR) {
		updatePositionFB(dMoveFB);
		updatePositionLR(dMoveLR);
		glutSetWindow(mainWin);
		glutPostRedisplay();
	}

	renderGameWin();
	renderTopWin();
	renderSideWin();
	renderScoreWin();
}

void keyboardActions(){
	if(keys[0] == true){//move forward
		if(keys[1] == true){
			dMoveFB = 0;
		}
		else{
			dMoveFB = 0.5f;

			if(keys[2] == true){
				dMoveLR = -0.5f;
			}
			else if(keys[3] == true){
				dMoveLR = 0.5f;
			}
		}
	}

	else if(keys [1] == true){//move backwards
		dMoveFB = -0.5f;

		if(keys[2] == true){
			dMoveLR = -0.5f;
		}
		else if(keys[3] == true){
			dMoveLR = 0.5f;
		}
	}

	else if(keys [2] == true){//move left
		if(keys[3] == true){
			dMoveLR = 0;
		}
		else{
			dMoveLR = -0.5f;
		}
	}

	else if(keys [3] == true){//move right
		dMoveLR = 0.5f;
	}

	else{
		dMoveFB = 0;
		dMoveLR = 0;
	}
}

void keyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {

		case 27: {
			glutDestroyWindow(mainWin);
			exit(0);
			break;
		}

		case 'w': keys[0] = true; break;
		case 's': keys[1] = true; break;
		case 'a': keys[2] = true; break;
		case 'd': keys[3] = true; break;
	}
	glutSetWindow(mainWin);
	keyboardActions();
	glutPostRedisplay();
}

void uKeyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {
		case 'w': keys[0] = false; break;
		case 's': keys[1] = false; break;
		case 'a': keys[2] = false; break;
		case 'd': keys[3] = false; break;
	}
	keyboardActions();
}

void dSpecialKeyboard(int key, int xIn, int yIn) {
	switch (key) {
		case GLUT_KEY_UP : dMoveFB = 0.5f; break;
		case GLUT_KEY_DOWN : dMoveFB = -0.5f; break;
	}
	glutSetWindow(mainWin);
	glutPostRedisplay();
}


void uSpecialKeyboard(int key, int xIn, int yIn) {
	switch (key) {
		case GLUT_KEY_UP :
		case GLUT_KEY_DOWN : dMoveFB = 0; break;
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

	xL = sin(1.62 + dAngle);
	zL = -cos(1.62 + dAngle);

	glutSetWindow(mainWin);
	glutPostRedisplay();
}


void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//hides cursor for game
	glutSetCursor(GLUT_CURSOR_NONE);

	// register callbacks
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(uKeyboard);
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
	glutInitWindowSize(1300,800);
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

