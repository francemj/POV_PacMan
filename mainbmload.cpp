#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#define X 0
#define Y 1
#define Z 2
#define W 3

//Globals
//when adjusting size, array bounds must also be adjusted as we can not use size as the bound as it is not an integer constant
int size = 65;
float heightMap [65][65]; //the heightmap 2D array
float normalMap [65][65][3]; //3D array to store x, y, and z values of normal vectors for each x,z point
float camPos[] = {(float) +size/2+10, 60.0f, (float) size/2+10};	//where the camera is
float camTarget[] = {0, 0, 0};// the coordinates of where the camera is pointed
float light0Pos[] = {(float) size/2, 0, (float) size/2,1.0};
float light1Pos[] = {0, 0, 0, 1.0};
float light1diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
float light1specular[] = { 1.0, 1.0, 1.0, 1.0 };
float xAngle = 0.0;
float yAngle = 0.0;
bool lighting; //toggles lighting
bool shading; //toggles shading
int disp; //max change in height
int terrainCircleSize = 10; //circle size
float max;
float min;
bool power2; //true if the size gives an integer value for the equation 2^n + 1 = size, false otherwise
int wireframe = 0; //0 means solid polygons, 1 means wireframe, 2 means both
int genMode = 0; //0 means circle algorithm, 1 means fault algorithm, 2 means midpoint algorithm
int meshShape = 0; //0 means triangles, 1 means quads


void computeNormals()
{
	//This function estimates the normal vector at each point using the cross product of two of the edge vectors
	for (int l = 0; l < size; l++)
	{
		for (int m = 0; m < size; m++)
		{
			float a [3], b [3], c [3], length;
			if (l == size-1)
			{
				if (m == size-1)
				{
					c[X] = 0.0f; c[Y] = heightMap[l][m-1]-heightMap[l][m]; c[Z] = -1.0f; //P2 = (x,z-1) because there's (x,z+1) DNE
					b[X] = -1.0f; c[Y] = heightMap[l-1][m]-heightMap[l][m]; c[Z] = 0.0f; //P2 = (x-1,z) because there's (x+1,z) DNE
				} 
				else 
				{
					c[X] = 0.0f; c[Y] = heightMap[l][m+1]-heightMap[l][m]; c[Z] = 1.0f; 
					b[X] = -1.0f; c[Y] = heightMap[l-1][m]-heightMap[l][m]; c[Z] = 0.0f; //P2 = (x-1,z) because there's (x+1,z) DNE
				}
			} 
			else
			{
				if (m == size-1)
				{
					c[X] = 0.0f; c[Y] = heightMap[l][m-1]-heightMap[l][m]; c[Z] = -1.0f; //P2 = (x,z-1) because there's (x,z+1) DNE
					b[X] = 1.0f; c[Y] = heightMap[l+1][m]-heightMap[l][m]; c[Z] = 0.0f;
				}
				else
				{
					c[X] = 0.0f; c[Y] = heightMap[l][m+1]-heightMap[l][m]; c[Z] = 1.0f; 
					b[X] = 1.0f; c[Y] = heightMap[l+1][m]-heightMap[l][m]; c[Z] = 0.0f;
				}
			}
			a[X] = b[Y]*c[Z] - b[Z]*c[Y]; //calculate the x value of the normal vector
			a[Y] = b[Z]*c[X] - b[X]*c[Z]; //calculate the y value of the normal vector
			a[Z] = b[X]*c[Y] - b[Y]*c[X]; //calculate the z value of the normal vector
			length = sqrt(pow(a[X],2)+pow(a[Y],2)+pow(a[Z],2)); //calculate the magnitude of the normal vector
			normalMap[l][m][X] = a[X]/length; //normalized x magnitude of normal vector
			normalMap[l][m][Y] = a[Y]/length; //normalized x magnitude of normal vector
			normalMap[l][m][Z] = a[Z]/length; //normalized x magnitude of normal vector
		}
	}
}

void generateMap()
{
	//This for loop is needed because when we reset the height map by pressing 'r', we have to actually clear the height map
	for (int j = 0; j < size; j++)
	{
		for (int k = 0; k < size; k++)
		{
			heightMap[j][k] = 0; //fills the heightmap with zero to begin with
		}
	}
	//This section is the circle algorithm which creates a random heightmap which we can then use to create our terrain
	max = 0;
	disp = 2;
	for (int i = 0; i < 500; i++)
	{
		int circleX = rand() % size; //random X value
		int circleZ = rand() % size; //random Z value (Y is the height)
		for (int j = 0; j < size; j++)
		{
			for (int k = 0; k < size; k++)
			{
				float dist = sqrt(pow(circleX - j,2)+pow(circleZ - k,2)); //current point's distance from the center of the circle
				float pd = dist*2/terrainCircleSize;
				if (fabs(pd) <= 1.0)
				{
					heightMap[j][k] += disp/2 + cos(pd*3.14)*disp/2; //increases height based on the max height difference and distance from center
					if (heightMap[j][k] > max)
					{
						max = heightMap[j][k]; //this max is used to calculate the shade used when rendering
					}
				}
			}
		}
	}
	computeNormals();
}

void drawQuadMesh()
{
	/* The process for drawing the mesh:
	1. Calculate the colour of the vertex
	2. Retrieve normal vector of the vertex
	3. Set the colour of the vertex
	4. Create vertex */
	if (wireframe == 0){
		for(int i = 0; i < size; i++){
			glBegin(GL_QUAD_STRIP);
			for(int j = 0; j < size; j++){
				float shade1 = heightMap[i][j]*0.7/max+0.3;
				glNormal3f(normalMap[i][j][X],normalMap[i][j][Y],normalMap[i][j][Z]);
				glColor3f(shade1, shade1, shade1);
				glVertex3f(-(size/2)+i,heightMap[i][j],j-(size/2));
				float shade2 = heightMap[i+1][j]*0.7/max+0.3;
				glNormal3f(normalMap[i+1][j][X],normalMap[i+1][j][Y],normalMap[i+1][j][Z]);
				glColor3f(shade2, shade2, shade2);
				glVertex3f(-(size/2)+i+1,heightMap[i+1][j],j-(size/2));
			}
			glEnd();
		}
	} else if (wireframe == 1){
		for(int i = 0; i < size; i++){			
			glBegin(GL_LINE_LOOP);
			for(int j = 0; j < size-1; j++){
				float shade1 = heightMap[i][j]*0.7/max+0.3;
				glNormal3f(normalMap[i][j][X],normalMap[i][j][Y],normalMap[i][j][Z]);
				glColor3f(shade1, shade1, shade1);
				glVertex3f(-(size/2)+i,heightMap[i][j],j-(size/2));
				float shade2 = heightMap[i+1][j]*0.7/max+0.3;
				glNormal3f(normalMap[i+1][j][X],normalMap[i+1][j][Y],normalMap[i][j+1][Z]);
				glColor3f(shade2, shade2, shade2);
				glVertex3f(-(size/2)+i+1,heightMap[i+1][j],j-(size/2));
				float shade3 = heightMap[i+1][j+1]*0.7/max+0.3;
				glNormal3f(normalMap[i+1][j+1][X],normalMap[i+1][j+1][Y],normalMap[i+1][j+1][Z]);
				glColor3f(shade3, shade3, shade3);
				glVertex3f(-(size/2)+i+1,heightMap[i+1][j+1],j+1-(size/2));
				float shade4 = heightMap[i][j+1]*0.7/max+0.3;
				glNormal3f(normalMap[i][j+1][X],normalMap[i][j+1][Y],normalMap[i][j+1][Z]);
				glColor3f(shade4, shade4, shade4);
				glVertex3f(-(size/2)+i,heightMap[i][j+1],j+1-(size/2));

			}
			glEnd();
		}
	} else {
		for(int i = 0; i < size; i++){
			glBegin(GL_QUAD_STRIP);
			for(int j = 0; j < size; j++){
				float shade1 = heightMap[i][j]*0.7/max+0.3;
				glNormal3f(normalMap[i][j][X],normalMap[i][j][Y],normalMap[i][j][Z]);
				glColor3f(shade1, shade1, shade1);
				glVertex3f(-(size/2)+i,heightMap[i][j],j-(size/2));
				float shade2 = heightMap[i+1][j]*0.7/max+0.3;
				glNormal3f(normalMap[i+1][j][X],normalMap[i+1][j][Y],normalMap[i+1][j][Z]);
				glColor3f(shade2, shade2, shade2);
				glVertex3f(-(size/2)+i+1,heightMap[i+1][j],j-(size/2));
			}
			glEnd();
			glBegin(GL_LINE_LOOP);
			glColor3f(0, 0, 0);
			for(int j = 0; j < size-1; j++){
				glNormal3f(normalMap[i][j][X],normalMap[i][j][Y],normalMap[i][j][Z]);
				glVertex3f(-(size/2)+i,heightMap[i][j],j-(size/2));
				glNormal3f(normalMap[i+1][j][X],normalMap[i+1][j][Y],normalMap[i+1][j][Z]);
				glVertex3f(-(size/2)+i+1,heightMap[i+1][j],j-(size/2));
				glNormal3f(normalMap[i][j+1][X],normalMap[i][j+1][Y],normalMap[i][j+1][Z]);
				glVertex3f(-(size/2)+i+1,heightMap[i+1][j+1],j+1-(size/2));
				glNormal3f(normalMap[i+1][j+1][X],normalMap[i+1][j+1][Y],normalMap[i+1][j+1][Z]);
				glVertex3f(-(size/2)+i,heightMap[i][j+1],j+1-(size/2));
			}
			glEnd();
		}
	}  
}

//OpenGL functions
//keyboard stuff
void keyboard(unsigned char key, int xIn, int yIn)
{
	int mod = glutGetModifiers();
	switch (key)
	{
		case 'q':
		case 27:	//27 is the esc key
			exit(0);
			break;

		case 'l':
			if (lighting == 0){
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				glEnable(GL_LIGHT1);
				lighting = 1;
			} else {
				glDisable(GL_LIGHTING);
				glDisable(GL_LIGHT0);
				glDisable(GL_LIGHT1);
				lighting = 0;
			}
			break;

		case 'H':
			light1Pos[Y]+=0.5;
			break;

		case 'N':
			light1Pos[Y]-=0.5;
			break;
		
		case 'B':
			light1Pos[X]+=0.5;
			break;
		
		case 'M':
			light1Pos[X]-=0.5;
			break;
		
		case 'G':
			light1Pos[Z]+=0.5;
			break;
		
		case 'J':
			light1Pos[Z]-=0.5;
			break;

		case 'h':
			light0Pos[Y]+=0.5;
			break;

		case 'n':
			light0Pos[Y]-=0.5;
			break;
		
		case 'b':
			light0Pos[X]+=0.5;
			break;
		
		case 'm':
			light0Pos[X]-=0.5;
			break;
		
		case 'g':
			light0Pos[Z]+=0.5;
			break;
		
		case 'j':
			light0Pos[Z]-=0.5;
			break;
	}
	glutPostRedisplay();
}

void special(int key, int xIn, int yIn)
{
	switch (key){
		case GLUT_KEY_DOWN:
			xAngle++;
			break;
		case GLUT_KEY_UP:
			xAngle--;
			break;
		case GLUT_KEY_LEFT:
			yAngle++;
			break;
		case GLUT_KEY_RIGHT:
			yAngle--;
			break;
	}
}

//initialization
void init(void)
{
	glClearColor(0, 0, 0, 0);
	glColor3f(1, 1, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 100);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

/* display function - GLUT display callback function
 *		clears the screen, sets the camera position, draws the ground plane and movable box
 */
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camPos[0], camPos[1], camPos[2], camTarget[0], camTarget[1], camTarget[2], 0,1,0);

	if (lighting == 1){
		glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light1diffuse);
		glLightfv(GL_LIGHT1, GL_SPECULAR, light1specular);
		glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
	} 

	glPushMatrix();
		glRotatef(yAngle,0,size/2,0);
		glRotatef(xAngle,1,0,0);
		drawQuadMesh();
	glPopMatrix();

	//flush out to single buffer
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(0, w, 0, h);
	gluPerspective(45, (float)((w+0.0f)/h), 1, 100);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void FPSTimer(int value){ //60fps
	glutTimerFunc(17, FPSTimer, 0);
	glutPostRedisplay();
}

/* main function - program entry point */
int main(int argc, char** argv)
{
	generateMap();
	lighting = 0;
	shading = 0;
	int currSize = size-1;
	/*the following while loop determines whether the terrains dimensions have an integer value of n,
	which fulfills the following expression 2^n+1 = size*/
	while (currSize > 2)
	{
		currSize /= 2;
		if (currSize == 2){
			power2 = true;
		}
	}
	printf("Welcome to Matt Franceschini's SFWRENG 3GC3 Assignment 2: Terrain.\nThe following is a list of keyboard shortcuts that you can use to control the different aspects of the program.\nw = Change wireframe mode. (Default is solid polygon, other options are wireframe, and both wireframe and solid polygons)\nr = Reset. (This gives a random new terrain using whichever terrain algorithm is currently selected)\nt = Triangle Mesh Mode (This is the default)\ny = Quads Mesh Mode\ns = Shading Mode (Default is flat shading, other option is Gouraud shading)\nl = Lighting toggle (Default is off, other option is on)\nd = Terrain Generation Mode (Default is circle method, other options are fault algorithm and midpoint algorithm)\nb,m = Light 1 X movement\nh,n = Light 1 Y movement\ng,j = Light 1 Z movement\nThe previous listed keys (g,h,j,b,n,m) move Light 2 when capitized.\n");
	//glut Initialization function calls
	glutInit(&argc, argv);		//starts up GLUT
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(400, 400);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("3GC3 Assignment 2");	//creates the window

	//display callback
	glutDisplayFunc(display);

	//keyboard callback
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);

	//resize callback
	glutReshapeFunc(reshape);

	//fps timer callback
	glutTimerFunc(17, FPSTimer, 0);

	init();

	glutMainLoop();	
	return(0);
}