// ====================================================================
/* Assignment 2: Moving Cameras and Humanoid Robots
   Ricky Atkinson
   Computer Graphics, Fall 2025
   Kent State University                                             */
// ====================================================================
/* CONTROLS:
     ‘w’: Display the wireframe model (edge lines) of the entire scene and objects.
     ‘s’ : Display the solid model.
     ‘c’ : Toggle screen clearing on/off ( ‘on’ shows only a black background)
     ‘a’: Toggle axis display on/off at the origin of the world coordinate.
     ‘d’: Toggle the dancing animation on/off
     F1: Toggle the rear camera view (Camera 2) on/off
     F2: Toggle the entire scene view (Camera 3) on/off
     F3: Switch between Camera 1 (First Person View (FPV)) and Camera 3 (Entire Scene View(ESV))
     Up: Move the camera forward
     Down: Move the camera backward
     Left: Rotate the camera to the left
     Right: Rotate the camera to the right
*/
// ====================================================================
#include <GL/glut.h>
#include <iostream>

using namespace std;

// Window dimensions
int winW = 1000, winH = 800;


//======================================================================
// Forward function declarations
static void PrintInstructions();
static void KeyboardInput(unsigned char key, int x, int y);
static void SpecialInput(int key, int x, int y);

//======================================================================

static void PrintInstructions() {
	std::cout << "Instructions:\n";
	std::cout << "  'w': Display the wireframe model (edge lines) of the entire scene and objects.\n";
	std::cout << "  's': Display the solid model.\n";
	std::cout << "  'c': Toggle screen clearing on/off ('on' shows only a black background).\n";
	std::cout << "  'a': Toggle axis display on/off at the origin of the world coordinate.\n";
	std::cout << "  'd': Toggle the dancing animation on/off.\n";
	std::cout << "  F1: Toggle the rear camera view (Camera 2) on/off.\n";
	std::cout << "  F2: Toggle the entire scene view (Camera 3) on/off.\n";
	std::cout << "  F3: Switch between Camera 1 (First Person View (FPV)) and Camera 3 (Entire Scene View(ESV)).\n";
	std::cout << "  Up: Move the camera forward.\n";
	std::cout << "  Down: Move the camera backward.\n";
	std::cout << "  Left: Rotate the camera to the left.\n";
	std::cout << "  Right: Rotate the camera to the right.\n";
	std::cout << "  'q' or ESC: Quit the application.\n";
}

static void KeyboardInput(unsigned char key, int x, int y) {
	std::cout << "[Debug][Keyboard] Key pressed: " << key << std::endl;
	switch (key) {
	case 'w':
		break;
	case 's':
		break;
	case 'c':
		break;
	case 'a':
		break;
	case 'd':
		break;
	case 'q': // Quit with either 'q' or ESC
	case 27:  // ESCAPE key
		exit(0);
		break;

	}
}

static void SpecialInput(int key, int x, int y) {
	std::cout << "[Debug][Special] Key pressed: ";
	switch (key) {
	case GLUT_KEY_F1:
		std::cout << "F1" << std::endl;
		break;
	case GLUT_KEY_F2:
		std::cout << "F2" << std::endl;
		break;
	case GLUT_KEY_F3:
		std::cout << "F3" << std::endl;
		break;
	case GLUT_KEY_UP:
		std::cout << "ArrowUp" << std::endl;
		break;
	case GLUT_KEY_DOWN:
		std::cout << "ArrowDown" << std::endl;
		break;
	case GLUT_KEY_LEFT:
		std::cout << "ArrowLeft" << std::endl;
		break;
	case GLUT_KEY_RIGHT:
		std::cout << "ArrowRight" << std::endl;
		break;
	}
}

// Display Function
void MyDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, 300, 300);
	glColor3f(1.0, 0.0, 0.0); // R=1,G=0,B=0 -> red color
	// drawing a square
	glBegin(GL_POLYGON);
	glVertex3f(-0.5, -0.5, 0.0);
	glVertex3f(0.5, -0.5, 0.0);
	glVertex3f(0.5, 0.5, 0.0);
	glVertex3f(-0.5, 0.5, 0.0);
	glEnd();
	// end drawing
	glFlush();
}

// MAIN
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB); // RGB mode
	glutInitWindowSize(winW, winH); // window size
	glutInitWindowPosition(0, 0);
	glutCreateWindow("OpenGL Sample Drawing");
	glClearColor(0.0, 0.0, 0.0, 1.0); // clear the window screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	PrintInstructions();

	glutDisplayFunc(MyDisplay); // call the drawing function
	glutKeyboardFunc(KeyboardInput);
	glutSpecialFunc(SpecialInput);

	glutMainLoop();
	return 0;
}
