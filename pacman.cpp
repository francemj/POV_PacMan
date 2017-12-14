#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

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
float z=0.0f;
float y=0.0f;

// key states
float dAngle=0.0f;
float dMove = 0;
float dX=-1;

// width and height of window
int winHeight;
int winWidth;

// computer framerate
int frame;
long currtime, timebase;
char s[50];

int mainWin, gameWin, topWin, sideWin, scoreWin;
int winBorder = 6;

//Map Generation
int width, height, maximum;
GLubyte* image;
//std::vector<std::vector<float>> wallMap;
const int first = 28;
const int second = 31;
float wallArray[first][second];

//this material is not needed, testing purposes only
float amb_floor[4] = {0.2,0.2,0.2,1};
float diff_floor[4] = {0.8,0.8,0.8,1};
float spec_floor[4] = {0.0,0.0,0.0,0.1};
float shine_floor = 0;
float amb_turq[4] ={ 0.1f, 0.18725f, 0.1745f, 0.8f };
float diff_turq[4] ={0.396f, 0.74151f, 0.69102f, 0.8f };
float spec_turq[4] ={0.297254f, 0.30829f, 0.306678f, 0.8f };
float shine_turq = 12.8f;
float amb_gold[4] ={ 0.24725f, 0.1995f, 0.0745f, 1.0f };
float diff_gold[4] ={0.75164f, 0.60648f, 0.22648f, 1.0f };
float spec_gold[4] ={0.628281f, 0.555802f, 0.366065f, 1.0f };
float shine_gold =51.2f ;
float amb_white[4] ={ 0.05f,0.05f,0.05f,1.0f };
float diff_white[4] ={ 0.5f,0.5f,0.5f,1.0f};
float spec_white[4] ={ 0.7f,0.7f,0.7f,1.0f};
float shine_white = 10.0f;
float amb_red[4] ={ 0.05f,0.0f,0.0f,1.0f };
float diff_red[4] ={ 0.5f,0.4f,0.4f,1.0f};
float spec_red[4] ={ 0.7f,0.04f,0.04f,1.0f};
float shine_red = 10.0f;



//testing purposes only
/* LIGHTING */
float light0_pos[] = {15.0, 15.0, -10.0, 1.0};
float amb0[4] = {1, 1, 1, 1};
float diff0[4] = {1, 1, 1, 1};
float spec0[4] = {1, 1, 1, 1};
float light1_pos[] = {10.0, 15.0, -10.0, 1.0};
float amb1[4] = {1, 1, 1, 1};
float diff1[4] = {1, 1, 1, 1};
float spec1[4] = {1, 1, 1, 1};

GLubyte* LoadPPM(char* file, int* width, int* height, int* maximum)
{
    GLubyte* img;
    FILE *fd;
    int n, m;
    int  k, nm;
    char c;
    int i;
    char b[100];
    float s;
    int red, green, blue;
    
    /* first open file and check if it's an ASCII PPM (indicated by P3 at the start) */
    fd = fopen(file, "r");
    fscanf(fd,"%[^\n] ",b);
    if(b[0]!='P'|| b[1] != '3')
    {
        printf("%s is not a PPM file!\n",file);
        exit(0);
    }
    printf("%s is a PPM file\n", file);
    fscanf(fd, "%c",&c);
    
    /* next, skip past the comments - any line starting with #*/
    while(c == '#')
    {
        fscanf(fd, "%[^\n] ", b);
        printf("%s\n",b);
        fscanf(fd, "%c",&c);
    }
    ungetc(c,fd);
    
    /* now get the dimensions and maximum colour value from the image */
    fscanf(fd, "%d %d %d", &n, &m, &k);
    
    /* calculate number of pixels and allocate storage for this */
    nm = n*m;
    img = (GLubyte*)malloc(3*sizeof(GLuint)*nm);
    s=255.0/k;
	
	//std::vector<float> tempVec;
    /* for every pixel, grab the read green and blue values, storing them in the image data array */
    for(i=0;i<nm;i++)
    {
//		tempVec.clear();
        fscanf(fd,"%d %d %d",&red, &green, &blue );
        img[3*nm-3*i-3]=red*s;
//		if (i%n == 0)
//		{
//			wallMap.push_back(tempVec);	
//		}
//		wallMap.at(floor(i/n)).push_back(red);
		wallArray[(int) floor(i/n)][i%n] = img[3*nm-3*i-3];
//		printf("%d, %d, %d\n", (int) floor(i/n), i%n, wallMap.at(floor(i/n)).at(i%n));
    }
    
    /* finally, set the "return parameters" (width, height, maximum) and return the image array */
    *width = n;
    *height = m;
    *maximum = k;
}

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

	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_red);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_red);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_red);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_red);

	// cylinder body using quadric object
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(-90, 1.0, 0.0, 0.0);
	GLUquadricObj *quadObj;
	quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	gluCylinder(quadObj, 0.25, 0.25, 1.2, 40, 6);
	glRotatef(90, 1.0, 0.0, 0.0);

	// head
	glTranslatef(0.0f, 1.2f, 0.0f);
	glutSolidSphere(0.25f,20,20);

	// eyes
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_white);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_white);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_white);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_white);
	glTranslatef(-0.2f, 0.10f, 0.1f);
	glutSolidSphere(0.03f,10,10);
	glTranslatef(0.0f, 0.0f, -0.2f);
	glutSolidSphere(0.03f,10,10);
	glPopMatrix();
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
	//Ground Plane
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_floor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_floor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_floor);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_floor);
	glPushMatrix();
	glTranslatef(0,-1,0);
	glColor3f(0,0,0); //color of floor
	glScalef(31,1,28); //size of floor
	glutSolidCube(1);
	glPopMatrix();

	// create ghosts
	for(int i=-3; i < 3; i++) {
		glPushMatrix();
		glTranslatef(-1.0f, -1.0f, i);
		drawGhost();
		glPopMatrix();
	}

	//testing purposes only

	/* LIGHTING */
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);
	

	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_turq);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_turq);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_turq);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_turq);
	
	//create map
	for (int i = 0; i < first; i++)
	{
		for (int j = 0; j < second; j++)
		{
			glPushMatrix();
			glTranslatef(i-15.5,0,j-14);
			if (wallArray[i][j] == 255)
			{
				glutSolidCube(1);
			}
			glPopMatrix();	
		}
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
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_gold);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_gold);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_gold);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_gold);
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
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_gold);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_gold);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_gold);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_gold);
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
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_gold);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_gold);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_gold);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_gold);
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

	currtime=glutGet(GLUT_ELAPSED_TIME);
	if (currtime - timebase > 1000) {
		sprintf(s,"PacMan FPS:%4.2f",
			frame*1000.0/(currtime-timebase));
		timebase = currtime;
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
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb_gold);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff_gold);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_gold);
	glMaterialf(GL_FRONT, GL_SHININESS, shine_gold);
	glTranslatef(x,y,z);
	glutSolidSphere(0.2, 4, 4);
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
    glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	
	//quick lighting, testing purposes only
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// register callbacks
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(dSpecialKeyboard);
	glutSpecialUpFunc(uSpecialKeyboard);
	glutMouseFunc(mouseButton);
	glutPassiveMotionFunc(mouseMove);
}

int main(int argc, char **argv) {
	
	LoadPPM("map.ppm", &width, &height, &maximum);
	printf("%d\n", sizeof(wallArray)/sizeof(wallArray[0]));
	printf("%d\n", sizeof(wallArray[0]));
	// init GLUT and create main window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0,0);
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

