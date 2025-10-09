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
// ====================================================================
#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <vector>

using namespace std;

// Pi
constexpr float PI = 3.14159265f;

// Window dimensions
int winW = 1000, winH = 800;

// Global toggles/flags
bool clearScreen = false; // Toggle for clearing the screen
bool displayAxis = true;  // Toggle for displaying the axis

// Dancing, music, and animation parameters
bool dancing = false;       // Toggle for dancing animation
bool musicToggle = false;   // Toggle for music playback (tied to dancing i.e. music plays when dancing is enabled)
static float tDance = 0.0f; // Dance time parameter
static int prevMs = 0;		// Previous time in milliseconds

// Dance modes
//   * INDIVIDUAL: Each robot dances independently with its own style and parameters
//   * GROUP: All robots dance in unison with the same style and parameters
// Default value is GROUP
enum DanceType {INDIVIDUAL, GROUP};
DanceType currentDanceType = GROUP;

// Dance styles
//   * SWAY: Gentle side-to-side swaying motion
//   * SPIN: Spinning around in place
//   * BOUNCE: Up-and-down bouncing motion (more intense SWAY basically)
// Randomized per robot in INDIVIDUAL mode, user selectable in GROUP mode
enum DanceStyle {SWAY, SPIN, BOUNCE};
static DanceStyle gStyle = SWAY;         // Chosen group style

// Group dance parameters (used in GROUP mode)
static bool  gLockStyleAndAmps = false;  // true => identical style & amplitudes
static float gSpeed = 1.0f;            // master group frequency (cycles/sec)
static float gPhase = 0.0f;            // master group phase offset

// Default dance amplitudes (used in GROUP mode)
static float gArmDeg = 22.0f;
static float gLegDeg = 28.0f;
static float gHeadDeg = 8.0f;
static float gTwistDeg = 6.0f;
static float gBobAmp = 0.10f;

// BONUS: Disco mode (lighting and colors)
// Togglable with 'L'
bool discoMode = false;
float lightPhase = 0.0f;

// Render modes
enum RenderMode { WIREFRAME, SOLID };
RenderMode currentRenderMode = SOLID;

// Camera views
enum ViewType { MAIN_FPV, MAIN_BIRD, MINI_REAR, MINI_BIRD };


//======================================================================
// Struct: Vec3 (and derivative ColorRGB)
// Three component vector structure to keep things more organized
// Vector operations include addition, subtraction, and scalar multiplication
struct Vec3 {
	float x, y, z;
	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
	Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
};

struct ColorRGB : public Vec3 {
	float& r; float& g; float& b;

	ColorRGB() : Vec3(0, 0, 0), r(x), g(y), b(z) {}
	ColorRGB(float r, float g, float b) : Vec3(r, g, b), r(x), g(y), b(z) {}

	void ApplyGL() const { glColor3f(r, g, b); }
};

// Colors for robot parts
static ColorRGB colTorso(0.20f, 0.60f, 1.00f);
static ColorRGB colHead(1.00f, 0.80f, 0.30f);
static ColorRGB colArms(0.90f, 0.15f, 0.15f);
static ColorRGB colLegs(0.20f, 0.85f, 0.35f);

// Struct: Robot
// Represents a humanoid robot with position, dance parameters, and style
struct Robot { 
	float x = 0, z = 0, phase = 0; 
	float speed = 1.0f;
	
	float armDeg = 25.0f;  // Arm swing amplitude in degrees
	float legDeg = 30.0f;  // Leg swing amplitude in degrees
	float headDeg = 10.0f; // Head nod amplitude in degrees
	float twistDeg = 8.0f; // Body twist amplitude in degrees
	float bobAmp = 0.10f;  // Body bob amplitude in world units
	DanceStyle style = (DanceStyle)(rand() % 3); // Random dance style assigned here

};

// Create a collection of robots with random positions and parameters
std::vector<Robot> robots;
const int ROBOT_COUNT = 20; // Count of Robots

//======================================================================
// Class: Camera
// Camera class to manage position, orientation, and perspective
class Camera {
public:
	Vec3 position;		// Camera eye position
	float yaw = 0.0f;	// Rotation about the Y axis (in degrees); 0 looks towards -Z
	float pitch = 0.0f; // Rotation about the X axis (in degrees)
	float fovY = 60.0f; // Field of view in Y direction (in degrees)
	float zNear = 0.1f; // Near and far clipping planes
	float zFar = 500.0f;

	// Movement along the camera's forward vector and rotation methods (XZ-plane)
	void MoveForward(float distance) {
		position.x += distance * sinf(yaw * PI / 180.0f);
		position.z -= distance * cosf(yaw * PI / 180.0f);
	}

	// Yaw/Pitch rotation methods with clamping
	void RotateYaw(float angle) {
		yaw += angle;
		if (yaw >= 360.0f) yaw -= 360.0f;
		if (yaw < 0.0f) yaw += 360.0f;
	}
	void RotatePitch(float angle) {
		pitch += angle;
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;
	}

	// Apply the camera transformation ((inverse of the camera's position and orientation)
	void ApplyView() {
		glRotatef(-pitch, 1, 0, 0);
		glRotatef(+yaw, 0, 1, 0);
		glTranslatef(-position.x, -position.y, -position.z);
	}
	// Set the perspective projection matrix based on the aspect ratio
	void SetPerspective(float aspect) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovY, aspect, zNear, zFar);
		glMatrixMode(GL_MODELVIEW);
	}
	// Debugging: Print the camera's current position and orientation
	void PrintStatus() {
		std::cout << "[Camera] Position: (" << position.x << ", " << position.y << ", " << position.z << "), Yaw: " << yaw << ", Pitch: " << pitch << std::endl;
	}

	// Draw a simple marker to represent the camera's position and orientation
	void DrawMarker(float size = 0.8f) {
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		
		glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glRotatef(-yaw, 0, 1, 0);

		// Body
		glPushMatrix();
		glScalef(size, size, size * 1.4f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glutSolidCube(1.0f);
		glPopMatrix();

		// Forward cone
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -size * 1.05f);
		glRotatef(180.0f, 1, 0, 0);
		glColor3f(1.0f, 1.0f, 0.0f);
		glutSolidCone(size * 0.3f, size * 0.6f, 10, 1);
		glPopMatrix();
		
		glPopMatrix();
		glPopAttrib();


	}
};

// Camera instance
Camera cam;
bool showRear = true;
bool showBird = true;
bool mainIsBird = false;


//======================================================================
// Forward function declarations
static void PrintInstructions();
static void DrawAxes();
static void DrawGround();
static void DrawRobot(const Robot& r);
static void DrawCube(float size);
static void DrawSphere(float radius, int slices = 20, int stacks = 20);
static void UpdateAnim(int);
static void SetupDiscoLighting();
static void RenderView(ViewType type);
static void RenderWorld();
static void InitScene();
static void KeyboardInput(unsigned char key, int x, int y);
static void SpecialInput(int key, int x, int y);
static void MyDisplay();
static void Reshape(int w, int h);

//======================================================================
// Prints instructions to the console
static void PrintInstructions() {
	std::cout << "Instructions:\n";
	std::cout << "  'w': Display the wireframe model (edge lines) of the entire scene and objects.\n";
	std::cout << "  's': Display the solid model.\n";
	std::cout << "  'c': Toggle screen clearing on/off ('on' shows only a black background).\n";
	std::cout << "  'a': Toggle axis display on/off at the origin of the world coordinate.\n";
	std::cout << "  'd': Toggle the dancing animation and music on/off.\n";
	std::cout << "  'm': Toggle the dancing mode (group/individual).\n";
	std::cout << "  'L': Toggle disco lighting mode on/off.\n";
	std::cout << "  '1': Change the group dancing mode to SWAY\n";
	std::cout << "  '2': Change the group dancing mode to SPIN\n";
	std::cout << "  '3': Change the group dancing mode to BOUNCE\n";
	std::cout << "  F1: Toggle the rear camera view (Camera 2) on/off.\n";
	std::cout << "  F2: Toggle the entire scene view (Camera 3) on/off.\n";
	std::cout << "  F3: Switch between Camera 1 (First Person View (FPV)) and Camera 3 (Entire Scene View(ESV)).\n";
	std::cout << "  Up: Move the camera forward.\n";
	std::cout << "  Down: Move the camera backward.\n";
	std::cout << "  Left: Rotate the camera to the left.\n";
	std::cout << "  Right: Rotate the camera to the right.\n";
	std::cout << "  'q' or ESC: Quit the application.\n";
	std::cout << "Press 'i' at any time to bring up this information again." << std::endl;
}

// Draws set of axes at the origin in world space
static void DrawAxes() {
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	const float L = 100.0f; // Length of the axes

	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0, 0, 0); glVertex3f(L, 0, 0); // X axis in red
	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0, 0, 0); glVertex3f(0, L, 0); // Y axis in green
	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0, 0, 0); glVertex3f(0, 0, L); // Z axis in blue

	glEnd();
	glPopAttrib();
}

// Draws a simple ground plane for the robots to inhabit instead of the endless void
static void DrawGround() {
	glPushMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_QUADS);
		glVertex3f(-100.0f, 0.0f, -100.0f);
		glVertex3f(100.0f, 0.0f, -100.0f);
		glVertex3f(100.0f, 0.0f, 100.0f);
		glVertex3f(-100.0f, 0.0f, 100.0f);
	glEnd();
	glPopMatrix();
}

// Draws the robot at the origin with animation based on its parameters
static void DrawRobot(const Robot &r) {
	// Robot dimensions
	const float torsoW = 1.5f, torsoH = 2.5f, torsoD = 1.0f;
	const float headR = 0.75f;
	const float armW = 0.5f, armH = 2.0f, armD = 0.5f;
	const float legW = 0.5f, legH = 2.5f, legD = 0.5f;

	// Pivots
	const float shoulderY = legH + torsoH * 0.85f;
	const float hipY = legH;
	const float torsoCenY = legH + torsoH * 0.5f;

	// Phase generation
	// The dance argument (arg) is a phase that advances with tDance
	// It is calculated differently based on the dance type (GROUP or INDIVIDUAL)
	float useFreq = (currentDanceType == GROUP) ? gSpeed : r.speed;
	float usePhase = (currentDanceType == GROUP) ? gPhase : r.phase;
	float arg = 2.0f * PI * useFreq * tDance + usePhase;

	// Style/Amplitude
	DanceStyle useStyle = (currentDanceType == GROUP) ? gStyle: r.style;

	float armAmp = (currentDanceType == GROUP && gLockStyleAndAmps) ? gArmDeg : r.armDeg;
	float legAmp = (currentDanceType == GROUP && gLockStyleAndAmps) ? gLegDeg : r.legDeg;
	float headAmp = (currentDanceType == GROUP && gLockStyleAndAmps) ? gHeadDeg : r.headDeg;
	float bobAmp = (currentDanceType == GROUP && gLockStyleAndAmps) ? gBobAmp : r.bobAmp;
	float twistAmp = (currentDanceType == GROUP && gLockStyleAndAmps) ? gTwistDeg : r.twistDeg;

	float baseSpin = 0.0f;
	
	// Modify amplitudes based on style
	switch (useStyle) {
	case SWAY:
		twistAmp *= 0.5f;
		break;
	case SPIN:
		twistAmp *= 2.2f;
		armAmp *= 0.5f;
		legAmp *= 0.5f;
		bobAmp *= 0.8f;
		baseSpin = fmodf(360.0f * (arg / (2.0f * PI)), 360.0f);
		break;
	case BOUNCE:
		bobAmp *= 2.0f;
		headAmp *= 1.5f;
		armAmp *= 0.6f;
		legAmp *= 0.6f;
		break;
	}

	// Calculate animation offsets
	const float armSwingL = dancing ? armAmp * sinf(2.0f * arg) : 0.0f;
	const float armSwingR = dancing ? -armAmp * sinf(2.0f * arg) : 0.0f;
	const float legSwingL = dancing ? legAmp * sinf(2.0f * arg) : 0.0f;
	const float legSwingR = dancing ? -legAmp * sinf(2.0f * arg) : 0.0f;
	const float bodyBob = dancing ? bobAmp * sinf(4.0f * arg) : 0.0f;
	const float bodyYaw = dancing ? twistAmp * sinf(1.0f * arg) : 0.0f;
	const float headNod = dancing ? headAmp * sinf(2.5f * arg) : 0.0f;

	// Body
	glPushMatrix();
	glTranslatef(0.0f, bodyBob, 0.0f);
	glTranslatef(0.0f, torsoCenY, 0.0f);
	glRotatef(bodyYaw, 0, 1, 0);
	glRotatef(baseSpin, 0, 1, 0);
	glTranslatef(0.0f, -torsoCenY, 0.0f);

	// Torso
	glPushMatrix();
	glTranslatef(0.0f, legH + torsoH * 0.5f, 0.0f);
	glScalef(torsoW, torsoH, torsoD);
	colTorso.ApplyGL();
	DrawCube(1.0f);
	glPopMatrix();

	// Head
	glPushMatrix();
	glTranslatef(0.0f, legH + torsoH, 0.0f);
	glRotatef(headNod, 1, 0, 0);
	glTranslatef(0.0f, headR, 0.0f);
	colHead.ApplyGL();
	DrawSphere(headR);
	glPopMatrix();

	// Left Arm
	glPushMatrix();
	float shoulderXL = -(torsoW * 0.5f + armW * 0.5f);
	glTranslatef(shoulderXL, shoulderY, 0.0f);
	glRotatef(armSwingL, 0, 0, 1);
	glTranslatef(0.0f, -armH * 0.5f, 0.0f);
	glScalef(armW, armH, armD);
	colArms.ApplyGL();
	DrawCube(1.0f);
	glPopMatrix();

	// Right Arm
	glPushMatrix();
	float shoulderXR = (torsoW * 0.5f + armW * 0.5f);
	glTranslatef(shoulderXR, shoulderY, 0.0f);
	glRotatef(armSwingR, 0, 0, 1);
	glTranslatef(0.0f, -armH * 0.5f, 0.0f);
	glScalef(armW, armH, armD);
	colArms.ApplyGL();
	DrawCube(1.0f);
	glPopMatrix();

	// Left Leg
	glPushMatrix();
	glTranslatef(-0.40f, hipY, 0.0f);
	glRotatef(legSwingL, 1, 0, 0);
	glTranslatef(0.0f, -legH * 0.5f, 0.0f);
	glScalef(legW, legH, legD);
	colLegs.ApplyGL();
	DrawCube(1.0f);
	glPopMatrix();

	// Right Leg
	glPushMatrix();
	glTranslatef(0.40f, hipY, 0.0f);
	glRotatef(legSwingR, 1, 0, 0);
	glTranslatef(0.0f, -legH * 0.5f, 0.0f);
	glScalef(legW, legH, legD);
	colLegs.ApplyGL();
	DrawCube(1.0f);
	glPopMatrix();

	glPopMatrix(); // body
}

// Draws a cube of given size
static void DrawCube(float size) {
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT);

	switch (currentRenderMode) {
	case WIREFRAME:
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glDisable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f); // Wireframe color
		glLineWidth(1.0f);
		glutWireCube(size);
		break;
	case SOLID:
		if (discoMode) glEnable(GL_LIGHTING);
		glutSolidCube(size);
		break;
	}

	glPopAttrib();
}

// Draws a sphere of given radius and detail
static void DrawSphere(float radius, int slices, int stacks) {
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT);

	switch (currentRenderMode) {
	case WIREFRAME:
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Set globally in RenderWorld()
		//glDisable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f); // Wireframe color
		glLineWidth(1.0f);
		glutWireSphere(radius, slices, stacks);
		break;
	case SOLID:
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (discoMode) glEnable(GL_LIGHTING);
		glutSolidSphere(radius, slices, stacks);

	}

	glPopAttrib();
}

// Animation update function called by GLUT timer
// Keeps animation time continuous instead of per-frame to prevent unexpected speed changes
static void UpdateAnim(int value) {
	int now = glutGet(GLUT_ELAPSED_TIME);
	if (prevMs == 0) prevMs = now;
	float dt = (now - prevMs) * 0.001f;
	prevMs = now;

	if (dancing) {

		tDance += dt;
		if (tDance > 10000.0f) {
			tDance = fmodf(tDance, 6.28318f);
		}

		
	}
	glutPostRedisplay();
	glutTimerFunc(16, UpdateAnim, 0); // Approx ~60 FPS
}

// BONUS: Sets up disco lighting if discoMode is enabled
// Enables a single, colorful moving light that orbits the scene
static void SetupDiscoLighting() {
	if (!discoMode) return;
	glEnable(GL_LIGHT0);
	lightPhase += 0.03f;

	float lx = 8.0f * cosf(lightPhase);
	float lz = 8.0f * sinf(lightPhase);
	float ly = 10.0f + 2.0f * sinf(lightPhase * 2.0f);
	GLfloat lightPos[] = { lx, ly, lz, 1.0f };
	GLfloat color[] = { 0.5f + 0.5f * sinf(lightPhase),
						0.5f + 0.5f * sinf(lightPhase + 2.0f),
						0.5f + 0.5f * sinf(lightPhase + 4.0f), 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);

}

// Renders a specific view type (main FPV, main bird, mini rear, mini bird)
static void RenderView(ViewType type) {
	// Miniviews use scissor + viewport so we can clear only that section of the screen
	auto applyMini = [&](int x, int y, int w, int h, Vec3 color) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(x, y, w, h);
		glViewport(x, y, w, h);
		glClearColor(color.x, color.y, color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		};
	auto finishMini = [&]() {
		glDisable(GL_SCISSOR_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		};

	switch (type) {
	case MAIN_FPV:
		glViewport(0, 0, winW, winH);
		cam.SetPerspective((float)winW / (float)winH);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		cam.ApplyView();
		RenderWorld();
		break;
	case MAIN_BIRD:
		glViewport(0, 0, winW, winH);
		cam.SetPerspective((float)winW / (float)winH);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 25.0f, 25.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f);
		RenderWorld();
		cam.DrawMarker();
		break;
	case MINI_REAR: {
		int w = winW / 4, h = winH / 4, x = 0, y = winH - h;
		applyMini(x, y, w, h, Vec3(0.05f, 0.05f, 0.05f));
		cam.SetPerspective((float)w / (float)h);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		Camera rear = cam; rear.RotateYaw(180.0f);
		rear.ApplyView();
		RenderWorld();
		finishMini();
		}

		break;
	case MINI_BIRD: {
		int w = winW / 4, h = winH / 4, x = winW - w, y = winH - h;
		applyMini(x, y, w, h, Vec3(0.05f, 0.05f, 0.05f));
		cam.SetPerspective((float)w / (float)h);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 25.0f, 25.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f);
		RenderWorld();
		cam.DrawMarker(0.25f);
		finishMini();
		}
		
		break;
	}
}

// Initializes the scene by creating robots with random positions and parameters
// Each robot gets its unique animation parameters that are used in INDIVIDUAL dance mode
// In GROUP mode, these parameters are overridden by the group parameters
static void InitScene() {
	srand(time(nullptr));
	robots.clear();

	// Checks if two robots are too close
	auto tooClose = [](float x1, float z1, float x2, float z2, float minDist) {
		float dx = x1 - x2, dz = z1 - z2; return dx * dx + dz * dz < minDist * minDist;
		};

	const float MIN_DIST = 2.5f; // Minimum distance between robots
	for (int i = 0; i < ROBOT_COUNT; ++i) {
		Robot r;
		int tries = 0; // Checks to find a non-colliding position for the robots
		do {
			float R = 8.0f + (rand() % 400) / 100.0f * 8.0f;
			float A = (rand() % 1000) / 1000.0f * 2.0f * PI;
			r.x = R * cosf(A); r.z = R * sinf(A);
		} while (++tries < 200 && any_of(robots.begin(), robots.end(),
			[&](const Robot& o) {return tooClose(r.x, r.z, o.z, o.z, MIN_DIST); }));
		
		// Randomize robot individual dance parameters
		r.phase = (rand() % 1000) / 1000.0f * 2.0f * PI;
		r.speed = 0.9f + (rand() % 100) / 1000.0f * 0.2f;
		r.armDeg = 15.0f + (rand() % 1700) / 100.0f;
		r.legDeg = 22.0f + (rand() % 1700) / 100.0f;
		r.headDeg = 5.0f + (rand() % 1500) / 100.0f;
		r.twistDeg = 5.0f + (rand() % 1500) / 100.0f;
		r.bobAmp = 0.05f + (rand() % 1500) / 10000.0f;
		robots.push_back(r);
	}
}

// Renders the entire world scene including ground, axes, and all robots
static void RenderWorld() {
	glPolygonMode(GL_FRONT_AND_BACK, currentRenderMode == WIREFRAME ? GL_LINE : GL_FILL);
	DrawGround();
	if (displayAxis) {
		DrawAxes();
	}

	for (const auto& rb : robots) {
		glPushMatrix();
		glTranslatef(rb.x, 0.0f, rb.z);
		DrawRobot(rb);
		glPopMatrix();
	}
	SetupDiscoLighting();

}

// Keyboard input handling function
static void KeyboardInput(unsigned char key, int x, int y) {
	std::cout << "[Debug][Keyboard] Key pressed: " << key << std::endl;
	switch (key) {
	case 'w':
		currentRenderMode = WIREFRAME;
		glutPostRedisplay();
		break;
	case 's':
		currentRenderMode = SOLID;
		glutPostRedisplay();
		break;
	case 'c':
		clearScreen = !clearScreen;
		std::cout << "[Flag] clearScreen: " << (clearScreen ? "ON" : "OFF") << std::endl;
		glutPostRedisplay();
		break;
	case 'a':
		displayAxis = !displayAxis;
		std::cout << "[Flag] displayAxis: " << (displayAxis ? "ON" : "OFF") << std::endl;
		glutPostRedisplay();
		break;
	case 'd':
		dancing = !dancing;
		musicToggle = dancing; // Sync music with dancing
		std::cout << "[Flag] dancing: " << (dancing ? "ON" : "OFF") << std::endl;
		if (musicToggle) {
			PlaySound(TEXT("dance1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		}
		else {
			PlaySound(NULL, 0, 0);
		}
		glutPostRedisplay();
		break;
	case 'm':
		currentDanceType = (currentDanceType == INDIVIDUAL) ? GROUP : INDIVIDUAL;
		std::cout << "[Flag] Dance Type: " << (currentDanceType == INDIVIDUAL ? "INDIVIDUAL" : "GROUP") << std::endl;
		if (currentDanceType == GROUP) {
			gLockStyleAndAmps = true;
			gStyle = SWAY;
			gSpeed = 1.0f;
			gPhase = 0.0f;
		}
		else {
			gLockStyleAndAmps = false;
		}
		glutPostRedisplay();
		break;
	case 'l':
		discoMode = !discoMode;
		if (discoMode) glEnable(GL_LIGHTING);
		else glDisable(GL_LIGHTING);
		std::cout << "[Flag] Disco Mode: " << (discoMode ? "ON" : "OFF") << std::endl;
		break;
	case 'i':
		PrintInstructions();
		break;
	case '1':
		gStyle = SWAY;
		std::cout << "[Flag] Dance Style: SWAY" << std::endl;
		glutPostRedisplay();
		break;
	case '2':
		gStyle = SPIN;
		std::cout << "[Flag] Dance Style: SPIN" << std::endl;
		glutPostRedisplay();
		break;
	case '3':
		gStyle = BOUNCE;
		std::cout << "[Flag] Dance Style: BOUNCE" << std::endl;
		glutPostRedisplay();
		break;
	case 'q': // Quit with either 'q' or ESC
	case 27:  // ESCAPE key
		PlaySound(NULL, 0, 0); // Stop any playing sound
		exit(0);
		break;
	default:
		break;
	}
}

// Special key input handling function (arrow keys and function keys)
// Arrow keys move/rotate the camera
// Function keys toggle views and display options
static void SpecialInput(int key, int x, int y) {
	std::cout << "[Debug][Special] Key pressed: ";
	switch (key) {
	case GLUT_KEY_F1:
		std::cout << "F1" << std::endl;
		showRear = !showRear;
		std::cout << "[Flag] showRear: " << (showRear ? "ON" : "OFF") << std::endl;
		break;

	case GLUT_KEY_F2:
		std::cout << "F2" << std::endl;
		showBird = !showBird;
		std::cout << "[Flag] showBird: " << (showBird ? "ON" : "OFF") << std::endl;
		break;

	case GLUT_KEY_F3:
		std::cout << "F3" << std::endl;
		mainIsBird = !mainIsBird;
		std::cout << "[Flag] mainIsBird: " << (mainIsBird ? "ON" : "OFF") << std::endl;
		break;

	case GLUT_KEY_UP:
		std::cout << "ArrowUp" << std::endl;
		cam.MoveForward(0.5f);
		cam.PrintStatus();
		break;

	case GLUT_KEY_DOWN:
		std::cout << "ArrowDown" << std::endl;
		cam.MoveForward(-0.5f);
		cam.PrintStatus();
		break;

	case GLUT_KEY_LEFT:
		std::cout << "ArrowLeft" << std::endl;
		cam.RotateYaw(-5.0f);
		cam.PrintStatus();
		break;

	case GLUT_KEY_RIGHT:
		std::cout << "ArrowRight" << std::endl;
		cam.RotateYaw(5.0f);
		cam.PrintStatus();
		break;

	}
	glutPostRedisplay();
}

// Display Function
static void MyDisplay() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (clearScreen) {	// If screen is clear, skip drawing 
		glutSwapBuffers();
		return;
	}

	// Main view
	// Either first-person view (FPV) or bird's-eye view
	if (mainIsBird) RenderView(MAIN_BIRD);
	else RenderView(MAIN_FPV);

	// Miniviews
	if (showRear) RenderView(MINI_REAR);
	if (showBird && !mainIsBird) RenderView(MINI_BIRD);

	// Handled in RenderWorld()
	/*if (displayAxis) {
		DrawAxes();
	}

	glPushMatrix();
	DrawGround();
	glPopMatrix();*/
	// end drawing
	glFlush();
	glutSwapBuffers();
}

static void Reshape(int w, int h) {
	winW = w;
	winH = h;
	glViewport(0, 0, (GLsizei)winW,(GLsizei)winH);
	glutPostRedisplay();
}

// MAIN
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); 
	glutInitWindowSize(winW, winH); // window size
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Assignment 2 | Ricky Atkinson | ratkin10");

	glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D
	glClearColor(0.0, 0.0, 0.0, 1.0); // clear the window screen

	PrintInstructions();

	// Initialize camera
	cam.position = Vec3(0.0f, 5.0f, 6.0f);
	cam.yaw = 0.0f;
	InitScene();

	// Callback functions
	glutDisplayFunc(MyDisplay);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(KeyboardInput);
	glutSpecialFunc(SpecialInput);

	glutTimerFunc(16, UpdateAnim, 0);

	glutMainLoop();
	return 0;
}
