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

double* m_start = new double[3];
double* m_end = new double[3];

// textures

GLuint textures[2];

// Textures
//an array for iamge data
GLubyte* ghosts_tex;
GLubyte* team_tex;

// hold main camera
float mX=0.0f;
float mZ=-1.0f;
float mY=0.0f;

// hold mouse position
float mouseX, mouseY;

//values for left and right
float xL;
float xR;
float zL;
float zR;

//values for buttons
float xB1;
float yB1;
float zB1;
float scaleB1;

float xB2;
float yB2;
float zB2;
float scaleB2;

// hold xz camera
float x=7.5f;
float z=-0.5f;
float y=0.0f;
float textX=0.0f;
float textY=0.0f;
float textZ=0.0f;

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

// Light Position
float pos0[4] = {x,0,z,0};
float pos1[4] = {x,5,z,0};

float glow_amb[] = {0.1f, 0, 0.1f, 1.0};
float glow_dif[] = {1, 0, 0, 1.0};
float glow_spec[] = {0.1f, 0.1f, 0.1f, 1.0};
float glow_em[] = {0,0,0,0.5f};
float shiny = 5; //10, 100

long currtime, timebase;
char s[50];

//keyboard buttons array
bool keys[4];

int mainWin, introWin, gameWin, topWin, sideWin, scoreWin;
int winBorder = 6;
bool isStarted = false;

//Map Generation
int width, height, maximum;
GLubyte* image;
const int first = 31;
const int second = 28;
float wallArray[first][second];

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
	
    /* for every pixel, grab the read green and blue values, storing them in the image data array */
    for(i=0;i<nm;i++)
    {
        fscanf(fd,"%d %d %d",&red, &green, &blue );
        img[3*nm-3*i-3]=red*s;
		wallArray[(int) floor(i/n)][i%n] = img[3*nm-3*i-3];
    }
    
    /* finally, set the "return parameters" (width, height, maximum) and return the image array */
    *width = n;
    *height = m;
    *maximum = k;
}

GLubyte* LoadPPM2(char* file, int* width, int* height, int* maximum)
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
	
	fd = fopen(file, "r");
	fscanf(fd,"%[^\n] ",b);
	if(b[0]!='P'|| b[1] != '3'){
		printf("%s is not a PPM file!\n",file); 
		exit(0);
	}
	fscanf(fd, "%c",&c);
	while(c == '#') 
	{
		fscanf(fd, "%[^\n] ", b);
		printf("%s\n",b);
		fscanf(fd, "%c",&c);
	}
	ungetc(c,fd); 
	fscanf(fd, "%d %d %d", &n, &m, &k);
	nm = n*m;
	img = (GLubyte*)malloc(3*sizeof(GLuint)*nm);


	s=255.0/k;

	for(i=0;i<nm;i++) {
		fscanf(fd,"%d %d %d",&red, &green, &blue );
		img[3*nm-3*i-3]=red*s;
		img[3*nm-3*i-2]=green*s;
		img[3*nm-3*i-1]=blue*s;
	}

	*width = n;
	*height = m;
	*maximum = k;

	return img;
}

void setProjection(int width, int height) {
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, width, height);
	gluPerspective(45, (width*1.0)/height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void reset() {
	x=7.5f;
	z=-0.5f;
	y=0.0f;
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
	glutPositionWindow(2*(winWidth/3.0) + winBorder/2, winBorder);
	glutReshapeWindow(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	setProjection(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);

	glutSetWindow(sideWin);
	glutPositionWindow(2*(winWidth/3.0) + winBorder/2, (winHeight/3.0) + winBorder/2);
	glutReshapeWindow(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	setProjection(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);

	glutSetWindow(scoreWin);
	glutPositionWindow(2*(winWidth/3.0) + winBorder/2, 2*(winHeight/3.0) + winBorder/2);
	glutReshapeWindow(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);
	setProjection(winWidth/3-winBorder*3/2, winHeight/3 - winBorder*3/2);

	glutSetWindow(introWin);
	glutPositionWindow(winBorder, winBorder);
	glutReshapeWindow((winWidth-winBorder), (winHeight-winBorder));
	setProjection((winWidth-winBorder), (winHeight-winBorder));


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
	// Ambience
	glow_amb[0] = 0.25f;
	glow_amb[1] = 0.25f; 
	glow_amb[2] = 0.25f; 
	glow_amb[3] = 1;
	// Emission
	glow_em[0] = 0.25f;
	glow_em[1] = 0.25f; 
	glow_em[2] = 0.25f; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
}

void setWallColour() {
	// Ambience
	glow_amb[0] = 0;
	glow_amb[1] = 0; 
	glow_amb[2] = 1; 
	glow_amb[3] = 1;
	
	// Diffuse
	glow_dif[0] = 0; 
	glow_dif[1] = 0; 
	glow_dif[2] = 1; 
	glow_dif[3] = 1;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
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
	glow_dif[0] = 1;
	glow_dif[1] = 0;
	glow_dif[2] = 0;
	glow_dif[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
}

// 	***************************
// 			Dots Models
// 	***************************

void drawPacDots() {
	glPushMatrix();
		setPacDotsColour();
		glTranslatef(0,-0.25f,0);
		glutSolidSphere(0.05,10,10);
		resetLightingProperties();
	glPopMatrix();
}

void drawPowerUps() {
	glPushMatrix();
		setPowerUpColour();
		glTranslatef(0,-0.25f,0);
		glutSolidSphere(0.1,50,50);
		resetLightingProperties();
	glPopMatrix();
}

void drawPacMan() {
	setPacManColour();
	glutSolidSphere(0.2, 10, 10);
	resetLightingProperties();
}

void drawGhost() {

	setGhostColour();

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
		setWhite();
		glTranslatef(-0.2f, 0.10f, 0.1f);
		glutSolidSphere(0.03f,10,10);
		glTranslatef(0.0f, 0.0f, -0.2f);
		glutSolidSphere(0.03f,10,10);
	glPopMatrix();
	resetLightingProperties();
}

// create a string using glut
void renderStrokeString(
	void *font,
	char *string
	) {
	char *c;
	for (c=string; *c != '\0'; c++) {
		glutStrokeCharacter(font, *c);
	}
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
	//Ground Plane
	setGroundColour();
	glPushMatrix();
		glTranslatef(-0.5,-1,-0.5);
		glColor3f(0,0,0); //color of floor
		glScalef(31,1,28); //size of floor
		glutSolidCube(1);
	glPopMatrix();
	resetLightingProperties();

	// create ghosts
	for(int i=-3; i < 3; i++) {
		glPushMatrix();
			glTranslatef(-1.0f, -1.0f, i);
			drawGhost();
		glPopMatrix();
	}
	
	//create map
	setWallColour();
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
			else if (wallArray[i][j] == 68)
			{
				drawPowerUps();
			}
			else if (wallArray[i][j] == 0)
			{
				drawPacDots();
			}
			glPopMatrix();	
		}
	}
	resetLightingProperties();
}

// render mainWin, gameWin, topWin, sideWin, scoreWin
void renderMainWin() {
	glutSetWindow(mainWin);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
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
	glOrtho(0.0, winWidth, 1.0, winHeight, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
}

void renderIntroWin() {
	glutSetWindow(introWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(textX + mouseX/50.0, textY + 90, textZ + mouseY/50.0, textX, textY, textZ, 1,0.0f,0);

	switchOrthographicProj();
	char main[10] = {'G','a','m','e',' ','I','d','l','e'};
	char pacman3d[10] = {'P','a','c','M','a','n','3','D',};
	char reset[13] = {'P','l','a','y', '/','r','e','s','e','t'};
	char resume[10] = {'r','e','s','u','m','e'};

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(2*winWidth/5,1*winHeight/8,0);
	glScalef(0.4,0.4,0.4);
	setWallColour();
	renderStrokeString(GLUT_STROKE_ROMAN,main);
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(2*winWidth/5,5*winHeight/6,0);
	glScalef(0.4,0.4,0.4);
	setWallColour();
	renderStrokeString(GLUT_STROKE_ROMAN,pacman3d);
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(5*winWidth/11,4*winHeight/6,0);
	glScalef(0.2,0.2,0.2);
	setRedGlow();
	renderStrokeString(GLUT_STROKE_ROMAN,reset);
	glPopMatrix();


	glPushMatrix();
	glLoadIdentity();
	glTranslatef(6*winWidth/13,2*winHeight/7,0);
	glScalef(0.2,0.2,0.2);
	setRedGlow();
	renderStrokeString(GLUT_STROKE_ROMAN,resume);
	glPopMatrix();



	switchPerspectiveProj();
	
	// create yellow cube
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	setPacManColour();
	glTranslatef(1.5,0,0);
	glScalef(8, 8, 10);
	glTranslatef(textX-1,textY,textZ);
	// glColor3f(1,0,0);
	glutSolidCube(1);
	scaleB1 = 2;
	scaleB2 = 2;
	xB1 = -5;
	yB1 = 0;
	zB1 = 0;
	xB2 = 10;
	yB2 = 0;
	zB2 = 0;
	glTranslatef(1.5, 0, 0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glutSwapBuffers();
}

void renderGameWin() {
	glutSetWindow(gameWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pos0[0] = x;
	pos0[2] = z;
	glLightfv(GL_LIGHT0, GL_POSITION, pos0);

	pos1[0] = x;
	pos1[2] = z;
	glLightfv(GL_LIGHT0, GL_POSITION, pos1);

	glLoadIdentity();
	gluLookAt(x, y, z, x + mX,y + mY,z + mZ, 0.0f,1.0f,0.0f);

	// create PacMan
	glPushMatrix();
		glTranslatef(x,y,z);
		drawPacMan();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void renderTopWin() {
	glutSetWindow(topWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	// change y + 90 to whatever value for height change
	gluLookAt(x, y + 20, z, x,y - 1,z, 1, 0.0f, 0);


	// create PacMan
	glPushMatrix();
		glTranslatef(x,y,z);
		drawPacMan();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void renderSideWin() {
	glutSetWindow(sideWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(x-mZ*1.1, y, z+mX*1.1, x, y, z, 0.0f, 1.0f, 0.0f);

	// create PacMan
	glPushMatrix();
		glTranslatef(x,y,z);
		drawPacMan();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}



void renderScoreWin() {
	glutSetWindow(scoreWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
		
	gluLookAt(x, y + 15, z, x,y ,z, mX, 0.0f, mZ);
	// create yellow circle
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0);
	
	glTranslatef(x,y,z);
	glScalef(2,2,2);
	glRotatef(130,1,1,1);
	glutSolidCube(1);

	frame++;

	currtime=glutGet(GLUT_ELAPSED_TIME);
	if (currtime - timebase > 1000) {
		sprintf(s,"PacMan FPS:%4.2f",
			frame*1000.0/(currtime-timebase));
		timebase = currtime;
		frame = 0;
	}

	char score[10] = {'S','c','o','r','e',':',' ','x',};

	switchOrthographicProj();

	glPushMatrix();
	glLoadIdentity();
	glColor3f(0.0,0,0.0);
	glTranslatef(10,90,0);
	glScalef(0.4,0.4,0.4);
	setRedGlow();
	renderStrokeString(GLUT_STROKE_ROMAN,score);
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glColor3f(0.0,0,0.0);
	glTranslatef(10,10,0);
	glScalef(0.4,0.4,0.4);
	setPacDotsColour();
	renderStrokeString(GLUT_STROKE_ROMAN,s);
	glPopMatrix();

	switchPerspectiveProj();

	// create yellow circle
	glPushMatrix();
		glTranslatef(x,y,z);
		drawPacMan();
	glPopMatrix();


	// glPopMatrix();
	// // glPushMatrix();
	// // glLoadIdentity();
	// glTranslatef(100.0, 50.0, 1.0);
	// glRotatef(30, 1.0, 0, 0);
	// glColor3f(0,1.0,0);
	// glutSolidCube(50);
	// glPopMatrix();

	glutSwapBuffers();
}

void setScene() {
	if ((dMoveFB || dMoveLR)) {
		if (isStarted) {
			updatePositionFB(dMoveFB);
			updatePositionLR(dMoveLR);
			glutSetWindow(mainWin);
			glutPostRedisplay();
		}
	}
	if (isStarted) {
		renderGameWin();
		renderTopWin();
		renderSideWin();
		renderScoreWin();
		glutPostRedisplay();
	} else {
		renderIntroWin();
		glutPostRedisplay();
	}
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
			else{
				dMoveLR = 0.0f;
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
		else{
			dMoveLR = 0.0f;
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
		case 'q': {
			exit(0); break;
		}
		case 'p': {
			if (!isStarted) {
				glutSetWindow(introWin);
				glutHideWindow();
				isStarted = true;
				printf("Game Paused\n");
			} else {
				glutSetWindow(introWin);
				glutShowWindow();
				printf("Game Unpaused\n");
				isStarted = false;
			}
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
	glutPostRedisplay();
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
		// onClick DOWN
			if(state == GLUT_DOWN){

			if(!isStarted) {

				// printf("clicking\n");
				// printf("(%f,%f,%f)----(%f,%f,%f)\n", m_start[0], m_start[1], m_start[2], m_end[0], m_end[1], m_end[2]);

				double matModelView[16], matProjection[16];
				int viewport[4];

				glGetDoublev(GL_MODELVIEW_MATRIX, matModelView);
				glGetDoublev(GL_PROJECTION_MATRIX, matProjection);
				glGetIntegerv(GL_VIEWPORT, viewport);

				double winX = (double)x;
				double winY = viewport[3] - (double)y;

				gluUnProject(winX, winY, 0.0, matModelView, matProjection, viewport, &m_start[0], &m_start[1],  &m_start[2]);
				gluUnProject(winX, winY, 160.0, matModelView, matProjection, viewport, &m_end[0], &m_end[1],  &m_end[2]);
				
				// printf("(%f,%f,%f)----(%f,%f,%f)\n\n", m_start[0], m_start[1], m_start[2], m_end[0], m_end[1], m_end[2]);

				//------------------------------------------

				double* R0 = new double[3];
				double* Rd = new double[3];

				double xDiff = m_end[0] - m_start[0];
				double yDiff = m_end[1] - m_start[1];
				double zDiff = m_end[2] - m_start[2];

				double mag = sqrt(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
				R0[0] = m_start[0];
				R0[1] = m_start[1];
				R0[2] = m_start[2];

				Rd[0] = xDiff / mag;
				Rd[1] = yDiff / mag;
				Rd[2] = zDiff / mag;

				double A = Rd[0] * Rd[0] + Rd[1] * Rd[1] + Rd[2] * Rd[2];

				double* R0Pc = new double[3];

				// button 1
				R0Pc[0] = R0[0] - xB1;
				R0Pc[1] = R0[1] - yB1;
				R0Pc[2] = R0[2] - zB1;

					scaleB1 = 8;
					scaleB2 = 8;
					xB1 = 0.5;
					yB1 = 0;
					zB1 = 0;
					xB2 = 7;
					yB2 = 0;
					zB2 = 0;

				double B = 2 * ( R0Pc[0] * Rd[0] + R0Pc[1] * Rd[1] + R0Pc[2] * Rd[2]);
				double C = (R0Pc[0]*R0Pc[0] + R0Pc[1] * R0Pc[1] + R0Pc[2] * R0Pc[2]) - (scaleB1*scaleB1);

				double discriminent = B*B - 4* A *C;

				double t1, t2;
				if(discriminent < 0){
					// printf("no intersection!\n");
				}
				else{
					printf("click button 1\n");
					t1 = (-B + sqrt(discriminent)) / (2*A);
					t2 = (-B - sqrt(discriminent)) / (2*A);
					isStarted = true;
					// printf("Intersection with object %i at t= %f, %f\n", i, t1, t2);
					glutSetWindow(introWin);
					glutHideWindow();
					glutSetWindow(mainWin);
					glutPostRedisplay();

				}
				if (!isStarted) {
					// button 2
					R0Pc[0] = R0[0] - xB2;
					R0Pc[1] = R0[1] - yB2;
					R0Pc[2] = R0[2] - zB2;

					B = 2 * ( R0Pc[0] * Rd[0] + R0Pc[1] * Rd[1] + R0Pc[2] * Rd[2]);
					C = (R0Pc[0]*R0Pc[0] + R0Pc[1] * R0Pc[1] + R0Pc[2] * R0Pc[2]) - (scaleB2*scaleB2);

					discriminent = B*B - 4* A *C;

					if(discriminent < 0) {
						// printf("no intersection!\n");
					}
					else {
						printf("click button 2\n");
						t1 = (-B + sqrt(discriminent)) / (2*A);
						t2 = (-B - sqrt(discriminent)) / (2*A);
						reset();
						isStarted = true;
						glutSetWindow(introWin);
						glutHideWindow();
						glutSetWindow(mainWin);
						glutPostRedisplay();
					}
				}

			}
			else  { // onClick DOWN
				// printf("x,y,z: %f,%f,%f", mX,y,mZ);
			}
		}
	}
}

void mouseMotion(int x, int y){
}

void mouseMove(int x, int y) {
	if (isStarted) {
		dAngle = (x - dX) * 0.010f;
	
		mX = sin(angle + dAngle);
		mZ = -cos(angle + dAngle);

		xL = sin(1.62 + dAngle);
		zL = -cos(1.62 + dAngle);

		glutSetWindow(mainWin);
		glutPostRedisplay();
	} else {
		mouseX = x;
		mouseY = y;
	}
}


void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT1, GL_POSITION, pos1);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glow_dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glow_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow_em);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	// Check for Removal
	// glEnable(GL_COLOR_MATERIAL);
	// glColorMaterial(GL_FRONT, GL_DIFFUSE);

	//hides cursor for game
	// glutSetCursor(GLUT_CURSOR_NONE);

	// register callbacks
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(uKeyboard);
	glutSpecialFunc(dSpecialKeyboard);
	glutSpecialUpFunc(uSpecialKeyboard);
	glutMouseFunc(mouseButton);
	glutPassiveMotionFunc(mouseMove);

	// texture stuff

	glGenTextures(2, textures);

	//load the texture
	ghosts_tex = LoadPPM2("ghosts.ppm", &width, &height, &maximum);

	//setup first texture
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	//set texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	//create a texture using array data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ghosts_tex);

	team_tex = LoadPPM2("team.ppm", &width, &height, &maximum);

	//setup first texture
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	//set texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	//create a texture using array data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, team_tex);
}

int main(int argc, char **argv) {
	//map texture
	LoadPPM("map.ppm", &width, &height, &maximum);
	printf("%d\n", sizeof(wallArray)/sizeof(wallArray[0]));
	printf("%d\n", sizeof(wallArray[0]));

	// init GLUT and create main window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(1300,800);
	mainWin = glutCreateWindow("PacMan 3D");

	// callbacks for main window
	glutDisplayFunc(setScene);
	glutReshapeFunc(resize);
	glutIdleFunc(setScene);
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

	introWin = glutCreateSubWindow(mainWin, winBorder,winBorder,(winWidth-winBorder), (winHeight-winBorder));
	glutDisplayFunc(renderIntroWin);
	init();


	// enter GLUT event processing cycle
	glutMainLoop();
	
	return 1;
}

