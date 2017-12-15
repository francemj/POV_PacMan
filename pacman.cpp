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

#define numberOfGhosts 6

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
float x=7.5f;
float z=-0.5f;
float y=0.0f;

// key states
float dAngle=0.0f;
float dMoveFB = 0;
float dMoveLR = 0;
float dX=-1;

//collision safe point
int safeX;
int safeZ;

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

int mainWin, gameWin, topWin, sideWin, scoreWin;
int winBorder = 6;

//Map Generation
int width, height, maximum;
GLubyte* image;
const int first = 31;
const int second = 28;
float wallArray[first][second];

//Ghost Positions
float ghostPos [numberOfGhosts][2]= {{-14.5,12},{-14.5,-13},{13.5,-13},{13.5,12},{-2.5,0},{-2.5,-1}};
float ghost1step = 0;
float ghost2step = 0;
float ghost3step = 0;
float ghost4step = 0;
float ghost5step = 0;
float ghost6step = 0;

//Pac Dots and Power Ups
float pacDotsArray[first][second];

// Texture Data
GLubyte* floor_tex;
int widthTex, heightTex, maxTex;
GLuint textures[2];

//score
int score = 0;

// Power Up State
int poweredUp = 0;
int powerUpCounter = 0;

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
	
	return image;
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

	glMaterialfv(GL_FRONT, GL_DIFFUSE, glow_dif);
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_em);
}

void setPacDotsColour() {
	glow_em[0] = 1; 
	glow_em[1] = 1; 
	glow_em[2] = 0; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_em);
}

void setPacManColour() {
	glow_dif[0] = 1; 
	glow_dif[1] = 1; 
	glow_dif[2] = 0; 
	glow_dif[3] = 1;
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_dif);
}

void setPowerUpColour() {
	glow_em[0] = 1; 
	glow_em[1] = 0; 
	glow_em[2] = 1; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_em);
}

void setRedGlow() {
	glow_em[0] = 1; 
	glow_em[1] = 0; 
	glow_em[2] = 0; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_em);
}

void setGroundColour() {
	// Ambience
	glow_amb[0] = 0.5f;
	glow_amb[1] = 0.5f; 
	glow_amb[2] = 0.5f; 
	glow_amb[3] = 1;
	// Emission
	glow_em[0] = 0.5f;
	glow_em[1] = 0.5f; 
	glow_em[2] = 0.5f; 
	glow_em[3] = 1;
	glMaterialfv(GL_FRONT, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_em);
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

	glMaterialfv(GL_FRONT, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, glow_dif);
}

void setBlack(){
	for(int i = 0; i < 4; i++){
		glow_dif[i] = 0; 
	}
	glMaterialfv(GL_FRONT, GL_DIFFUSE, glow_dif);
}

void setWhite(){
	for(int i = 0; i < 4; i++){
		glow_dif[i] = 1; 
	}
	glMaterialfv(GL_FRONT, GL_DIFFUSE, glow_dif);
}

void setGhostColour() {
	glow_dif[0] = 1;
	glow_dif[1] = 0;
	glow_dif[2] = 0;
	glow_dif[3] = 1;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, glow_dif);
}

// 	***************************
// 			Dots Models
// 	***************************

void drawPacDots(int x, int z) {
	glPushMatrix();
		setPacDotsColour();
		glTranslatef(0, -0.25f + pacDotsArray[x][z], 0);
		glutSolidSphere(0.05,10,10);
		resetLightingProperties();
	glPopMatrix();
}

void drawPowerUps(int x, int z) {
	glPushMatrix();
		setPowerUpColour();
		glTranslatef(0, -0.25f + pacDotsArray[x][z], 0);
		glutSolidSphere(0.1,10,10);
		resetLightingProperties();
	glPopMatrix();
}

void drawPacMan() {
	setPacManColour();
	glutSolidSphere(0.2, 10, 10);
	resetLightingProperties();
}

void drawGhost(float x, float y, float z) {

	setGhostColour();

	// cylinder body using quadric object
	glTranslatef(x, y, z);
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

void drawFloor() {

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(15,-0.5, -14.5);

		glTexCoord2f(0, 10);
		glVertex3f(-16,-0.5, -14.5);

		glTexCoord2f(10, 10);
		glVertex3f(-16,-0.5, 13.5);

		glTexCoord2f(10, 0);
		glVertex3f(15,-0.5, 13.5);
	glEnd();
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

//Ghost Algorithms
void ghost1()
{
	if (ghost1step > -1 && ghost1step < 44)
	{
		ghostPos[0][1] -= 0.125;
	}
	else if (ghost1step >43 && ghost1step <60)
	{
		ghostPos[0][0] += 0.125;
	}
	else if (ghost1step >59 && ghost1step <84)
	{
		ghostPos[0][1] += 0.125;
	}
	else if (ghost1step >83 && ghost1step <96)
	{
		ghostPos[0][0] += 0.125;
	}
	else if (ghost1step >95 && ghost1step <116)
	{
		ghostPos[0][1] += 0.125;
	}
	else if (ghost1step >115 && ghost1step <145)
	{
		ghostPos[0][0] -= 0.125;
	}
}

void ghost2()
{
	if (ghost2step > -1 && ghost2step < 44)
	{
		ghostPos[1][1] += 0.125;
	}
	else if (ghost2step >43 && ghost2step <60)
	{
		ghostPos[1][0] += 0.125;
	}
	else if (ghost2step >59 && ghost2step <84)
	{
		ghostPos[1][1] -= 0.125;
	}
	else if (ghost2step >83 && ghost2step <96)
	{
		ghostPos[1][0] += 0.125;
	}
	else if (ghost2step >95 && ghost2step <116)
	{
		ghostPos[1][1] -= 0.125;
	}
	else if (ghost2step >115 && ghost2step <145)
	{
		ghostPos[1][0] -= 0.125;
	}
}

void ghost3()
{
	if (ghost3step > -1 && ghost3step < 44)
	{
		ghostPos[2][1] += 0.125;
	}
	else if (ghost3step >43 && ghost3step <56)
	{
		ghostPos[2][0] -= 0.125;
	}
	else if (ghost3step >55 && ghost3step <68)
	{
		ghostPos[2][1] -= 0.125;
	}
	else if (ghost3step >67 && ghost3step <80)
	{
		ghostPos[2][0] -= 0.125;
	}
	else if (ghost3step >79 && ghost3step <92)
	{
		ghostPos[2][1] -= 0.125;
	}
	else if (ghost3step >91 && ghost3step <104)
	{
		ghostPos[2][0] -= 0.125;
	}
	else if (ghost3step >103 && ghost3step <124)
	{
		ghostPos[2][1] -= 0.125;
	}
	else if (ghost3step >123 && ghost3step <136)
	{
		ghostPos[2][0] += 0.125;
	}
	else if (ghost3step >135 && ghost3step <144)
	{
		ghostPos[2][1] += 0.125;
	}
	else if (ghost3step >143 && ghost3step <156)
	{
		ghostPos[2][0] += 0.125;
	}
	else if (ghost3step >155 && ghost3step <164)
	{
		ghostPos[2][1] -= 0.125;
	}
	else if (ghost3step >163 && ghost3step <177)
	{
		ghostPos[2][0] += 0.125;
	}
}

void ghost4()
{
	if (ghost4step > -1 && ghost4step < 44)
	{
		ghostPos[3][1] -= 0.125;
	}
	else if (ghost4step >43 && ghost4step <56)
	{
		ghostPos[3][0] -= 0.125;
	}
	else if (ghost4step >55 && ghost4step <68)
	{
		ghostPos[3][1] += 0.125;
	}
	else if (ghost4step >67 && ghost4step <80)
	{
		ghostPos[3][0] -= 0.125;
	}
	else if (ghost4step >79 && ghost4step <92)
	{
		ghostPos[3][1] += 0.125;
	}
	else if (ghost4step >91 && ghost4step <104)
	{
		ghostPos[3][0] -= 0.125;
	}
	else if (ghost4step >103 && ghost4step <124)
	{
		ghostPos[3][1] += 0.125;
	}
	else if (ghost4step >123 && ghost4step <136)
	{
		ghostPos[3][0] += 0.125;
	}
	else if (ghost4step >135 && ghost4step <144)
	{
		ghostPos[3][1] -= 0.125;
	}
	else if (ghost4step >143 && ghost4step <156)
	{
		ghostPos[3][0] += 0.125;
	}
	else if (ghost4step >155 && ghost4step <164)
	{
		ghostPos[3][1] += 0.125;
	}
	else if (ghost4step >163 && ghost4step <176)
	{
		ghostPos[3][0] += 0.125;
	}
}

void ghost5()
{
	if (ghost5step > -1 && ghost5step < 8)
	{
		ghostPos[4][0] -= 0.125;
	}
	else if (ghost5step >7 && ghost5step <12)
	{
		ghostPos[4][1] += 0.125;
	}
	else if (ghost5step >11 && ghost5step <24)
	{
		ghostPos[4][0] -= 0.125;
	}
	else if (ghost5step >23 && ghost5step <36)
	{
		ghostPos[4][1] += 0.125;
	}
	else if (ghost5step >35 && ghost5step <48)
	{
		ghostPos[4][0] -= 0.125;
	}
	else if (ghost5step >47 && ghost5step <60)
	{
		ghostPos[4][1] += 0.125;
	}
	else if (ghost5step >59 && ghost5step <120)
	{
		ghostPos[4][0] += 0.125;
	}
	else if (ghost5step >119 && ghost5step <132)
	{
		ghostPos[4][1] -= 0.125;
	}
	else if (ghost5step >131 && ghost5step <168)
	{
		ghostPos[4][0] -= 0.125;
	}
	else if (ghost5step >167 && ghost5step <184)
	{
		ghostPos[4][1] -= 0.125;
	}
	else if (ghost5step >183 && ghost5step <200)
	{
		ghostPos[4][0] += 0.125;
	}
	else if (ghost5step >199 && ghost5step <208)
	{
		ghostPos[4][0] -= 0.125;
	}
}

void ghost6()
{
	if (ghost6step > -1 && ghost6step < 8)
	{
		ghostPos[5][0] -= 0.125;
	}
	else if (ghost6step >7 && ghost6step <12)
	{
		ghostPos[5][1] -= 0.125;
	}
	else if (ghost6step >11 && ghost6step <24)
	{
		ghostPos[5][0] -= 0.125;
	}
	else if (ghost6step >23 && ghost6step <36)
	{
		ghostPos[5][1] -= 0.125;
	}
	else if (ghost6step >35 && ghost6step <48)
	{
		ghostPos[5][0] -= 0.125;
	}
	else if (ghost6step >47 && ghost6step <60)
	{
		ghostPos[5][1] -= 0.125;
	}
	else if (ghost6step >59 && ghost6step <120)
	{
		ghostPos[5][0] += 0.125;
	}
	else if (ghost6step >119 && ghost6step <132)
	{
		ghostPos[5][1] += 0.125;
	}
	else if (ghost6step >131 && ghost6step <168)
	{
		ghostPos[5][0] -= 0.125;
	}
	else if (ghost6step >167 && ghost6step <184)
	{
		ghostPos[5][1] += 0.125;
	}
	else if (ghost6step >183 && ghost6step <200)
	{
		ghostPos[5][0] += 0.125;
	}
	else if (ghost6step >199 && ghost6step <208)
	{
		ghostPos[5][0] -= 0.125;
	}
}

void updateLightPosition(float x, float z){
	pos0[0] = x;
	pos0[0] = z;
	glLightfv(GL_LIGHT0, GL_POSITION, pos0);
}

void ghostHitDetection(){
	int ghostX;
	int ghostZ;
	for(int i = 0; i < numberOfGhosts; i++){
		ghostX = ghostPos[i][0];
		ghostZ = ghostPos[i][1];

		if(abs(x - ghostX) < 0.05 && abs(z - ghostZ) < 0.05){
			printf("ghost %i HIT\n", i);
			printf("xpos: %f, zpos: %f\n", x, z);
			printf("ghostx: %f, ghostz: %f\n", ghostX, ghostZ);
		}
	}
}

void activatePowerUp(){
	poweredUp = 1;
}

void deactivatePowerUp(){
	poweredUp = 0;
}

void pacDotsHitDetection(){
	int xVal = round(x + 15.5);
	int zVal = round(z + 14);
	if(pacDotsArray[xVal][zVal] == 0){
		if(wallArray[xVal][zVal] == 0){
			pacDotsArray[xVal][zVal] = -10.0f;
			score = score + 10;
		}
		else if(wallArray[xVal][zVal] == 68){
			pacDotsArray[xVal][zVal] = -10.0f;
			activatePowerUp();
		}
	}
}

bool checkPos(){
	int xVal = round(x + 15.5);
	int zVal = round(z + 14);
	if(wallArray[xVal][zVal] == 255){
		if(xVal != safeX){
			if(safeX < xVal){
				x = xVal - 16.05;
			}
			else{
				x = xVal - 14.95;
			}
		}
		else if(zVal != safeZ){
			if(safeZ < zVal){
				z = zVal - 14.55;
			}
			else{
				z = zVal - 13.45;
			}
		}
		
		return true;
	}

	else{
		safeX = xVal;
		safeZ = zVal;
	}

	return false;
}

bool collisionCheckX(){
	if(x <= -14.80){
		x = -14.75;
		return true;
	}
	else if(x >= 13.80){
		x = 13.75;
		return true;
	}

	return checkPos();

	return false;
}

bool collisionCheckZ(){
	if(z <= -13.30){
		z = -13.25;
		return true;
	}
	else if(z >= 12.30){
		z = 12.25;
		return true;
	}

	return checkPos();

	return false;
}

void updatePositionFB(float movement){
	if(collisionCheckX() == false){
		x += movement *0.1f*mX;
	}
	if(collisionCheckZ() == false){
		z += movement *0.1f*mZ;
	}
}

void updatePositionLR(float movement){
	if(collisionCheckX() == false){
		x += movement *0.1f*xL;
	}
	if(collisionCheckZ() == false){
		z += movement *0.1f*zL;
	}
	updateLightPosition(x, z);
}

void renderShapes() {
	//Ground Plane
	setGroundColour();
	drawFloor(); // Draw Floor
	resetLightingProperties();

	// create ghosts
	for(int i = 0; i < numberOfGhosts; i++) {
		glPushMatrix();
			drawGhost(ghostPos[i][0], -1.0f, ghostPos[i][1]);
		glPopMatrix();
	}
	
	//create map
	for (int i = 0; i < first; i++)
	{
		for (int j = 0; j < second; j++)
		{
			glPushMatrix();
			glTranslatef(i-15.5,0,j-14);
			if (wallArray[i][j] == 255)
			{
				setWallColour();

				if(poweredUp == 1)
				{
					glDisable(GL_LIGHTING);
					glColor4f(0, 0, 1, 0.5);
					glutSolidCube(1);
					glEnable(GL_LIGHTING);
				}
				else
				{
					glutSolidCube(1);
				}
			}
			else if (wallArray[i][j] == 68)
			{
				drawPowerUps(i, j);
			}
			else if (wallArray[i][j] == 0)
			{
				drawPacDots(i, j);
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

void renderGameWin() {
	glutSetWindow(gameWin);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	gluLookAt(x, y + 10, z, x,y - 1,z, 1, 0.0f, 0);

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
		glScalef(1.5,1.5,1.5);
		drawPacMan();
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
		glTranslatef(x,y,z);
		drawPacMan();
	glPopMatrix();

	renderShapes();
	glutSwapBuffers();
}

void setScene() {
	if (poweredUp == 1){
		if(powerUpCounter >= 500){
			deactivatePowerUp();
			powerUpCounter = 0;
		}
		powerUpCounter++;
	}

	if (dMoveFB || dMoveLR) {
		updatePositionFB(dMoveFB);
		updatePositionLR(dMoveLR);
	}

	//These "ghost" functions move the ghost in a particular pattern depending on starting positions
	ghost1();
	if (ghost1step == 143.5)
	{
		ghost1step = 0;
	}
	else
	{
		ghost1step += 0.5;
	}
	ghost2();
	if (ghost2step == 143.5)
	{
		ghost2step = 0;
	}
	else
	{
		ghost2step += 0.5;
	}
	ghost3();
	if (ghost3step == 175.5)
	{
		ghost3step = 0;
	}
	else
	{
		ghost3step += 0.5;
	}
	ghost4();
	if (ghost4step == 175.5)
	{
		ghost4step = 0;
	}
	else
	{
		ghost4step += 0.5;
	}

	ghost5();
	if (ghost5step == 207.5)
	{
		ghost5step = 0;
	}
	else
	{
		ghost5step += 0.5;
	}
	ghost6();
	if (ghost6step == 207.5)
	{
		ghost6step = 0;
	}
	else
	{
		ghost6step += 0.5;
	}


	ghostHitDetection();
	pacDotsHitDetection();
	renderGameWin();
	renderTopWin();
	renderSideWin();
	renderScoreWin();
	glutSetWindow(mainWin);
	glutPostRedisplay();
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
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT1, GL_POSITION, pos1);

	glMaterialfv(GL_FRONT, GL_AMBIENT, glow_amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, glow_dif);
	glMaterialfv(GL_FRONT, GL_SPECULAR, glow_spec);
	glMaterialfv(GL_FRONT, GL_EMISSION, glow_em);
	glMaterialf(GL_FRONT, GL_SHININESS, shiny);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	//	************************
	//			Textures
	//	************************
	//enable texturing
	glEnable(GL_TEXTURE_2D);
	//generate 2 texture IDs, store them in array "textures"
	glGenTextures(2, textures);
	//load the texture (snail)
	floor_tex = LoadPPM2("floor.ppm", &widthTex, &heightTex, &maxTex);
	//setup first texture (using snail image)
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	//set texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//create a texture using the "floor_tex" array data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthTex, heightTex, 0, GL_RGB, GL_UNSIGNED_BYTE, floor_tex);
	

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
	//map texture
	LoadPPM("map.ppm", &width, &height, &maximum);

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

