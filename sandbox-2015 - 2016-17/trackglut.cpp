#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <quat/quat.h>
#include <vrpn/vrpn_Tracker.h>

//-----------------------------------------------------------------------------
// This is a simple example of tracking some objects and displaying
// them using OpenGL. The program uses the GLUT library.
// The viewing system is just fixed for a particular volume (might
// not be appropriate for all setups), but is easily alterable.
// J.Ward 24/06/05
//
// HW converted to use VRPN Aug 2014
// HW converted to use freeglut3.0.0 Aug 2015
// HW converted to use GLEW 1.13.0 and vertex buffer objects Aug 2015
// As with my grandfather's axe (look it up), little now remains of James's
// original save the overall structure comprising callbacks and a main loop,
// plus the central idea of drawing frames for the glasses moving above a ground plane.
// 
// This non-object-oriented VE teaching program is written for understanding and ease-of-alteration 
// rather than style and is therefore strictly 
// NOT FOR DISTRIBUTION OUTSIDE THE UNIVERSITY OF HULL
//
//-----------------------------------------------------------------------------

#pragma region user-modifiable

// user-modifiable object sizing and placing variables - used in OnDisplay
double screenheight = 0.243; //// *** first stage - put actual measured height of your window's drawing area (metres!).
double cubefacesize[2]; //// *** first stage -  choose a value to fit in your window;
double screendist = 2.5;   //// *** second stage -  choose a positive value for origin-to-screen distance along OpenGl -z according to the spec
double screenup = (screenheight / 2.0) + 1.727;     //// *** second stage -  choose a positive value for ground-to-screen-bottom-edge distance along OpenGl +y according to the spec
// end user-modifiable section

#pragma endregion

#pragma region user-unmodifiable

int	width;  // viewport in pixels, set up in main
int	height;
float viewaspect; // width / height
bool bViewchanged = false; // forces a call to the look and projection setter
bool bShowData = false;  // output glasses positions or not
bool bInfront = true; // two simple static camera presets, infront (default on startup) and oblique
bool bOblique = false; 
bool bInperson = false; // first-person viewing - TODO
bool bSideon = false; // for checking cube placement -TODO
bool bStereo = false; // stereo - TODO
int numeyes = 1; // mono initially and (TODO) if stereo toggled off

// tracker globals
const int BUFRSIZE=512;
static int track_data_waiting = 0;
vrpn_Tracker_Remote *device = NULL;
char tracker_name[BUFRSIZE];

// vector for positions, and capturing glasses' Y for later stereo
double Yaxis[3], Pos[3];

// pipeline matrices
qgl_matrix_type togl = Q_ID_MATRIX; // togl axial transform done in OnCreate
qgl_matrix_type local; // e.g. some object translation or, for the glasses, tracked data in real-time
// two other matrices, the projection and camera, are local to ViewSystem

// shader and variable locations
GLuint shader, PositionLoc, RGBLoc, toglLoc, localLoc, lookLoc, projLoc;

// vertex buffer and object handles
GLuint vertexbufferid;
#define FRAMES 0
#define FRAMESIZE 6 // 6 vertices make 3 lines
#define GROUND 6
#define GROUNDSIZE 24 // next 24 vertices make 12 lines (2 sets of 6 each)
#define CUBE 30
#define CUBESIZE 24 // next 24 vertices make 12 lines (12 edges of cube)
#define MONITOR 54
#define MONITORSIZE 12 // final 12 vertices make 6 lines (4 edges and 2 diagonals of monitor)

#pragma endregion

void	VRPN_CALLBACK handle_tracker_pos_quat(void *, const vrpn_TRACKERCB object)
{
	q_type tmp;
	q_normalize(tmp, object.quat);

	qgl_to_matrix(local, tmp);
	// the logic here is that the direction cosines off the tracker rotate 
	// the vanilla glasses frame pointing along tracker X, Y, Z 

	// add in translation, i.e. object.pos, and the frames now move as the glasses do in the real world
	// copy Pos for debugging output (key 'd') and the glasses' Y axis for later stereo calculations
	for (int i = 0; i < 3; i++) {
		Yaxis[i] = local[1][i]; 
		Pos[i] = local[3][i] = object.pos[i];
	} // X component is in element 0, Y in 1, Z in 2

	// bingo!
	track_data_waiting = 1;

}//Handle tracker callback

void rotateObject(qgl_matrix_type m, const q_vec_type axis, const double rotation){
	qgl_matrix_type mtmp, rot;
	// copy source matrix to mtmp
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) mtmp[i][j] = m[i][j];
	q_type qtmp;
	double angle = Q_DEG_TO_RAD(rotation);
	q_from_axis_angle(qtmp, axis[0], axis[1], axis[2], angle);
	qgl_to_matrix(rot, qtmp);
	// multiply
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
		m[i][j] = 0.0f;
		for (int k = 0; k < 4; k++)
			m[i][j] += mtmp[k][j] * rot[i][k];
		}
} // rotate an (already rotated) object via its local transform, translation unchanged

void translateObject(qgl_matrix_type m, const q_vec_type translation){
	// add in translation
	for (int i = 0; i < 3; i++) m[3][i] += translation[i];
} // translate an (already translated) object via its local transform, orientation unchanged

void scaleObject(qgl_matrix_type m, const q_vec_type scale){
	qgl_matrix_type mtmp;
	// copy the source matrix to mtmp
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) mtmp[i][j] = m[i][j];
	qgl_matrix_type s = Q_ID_MATRIX;
	for (int i = 0; i < 3; i++) s[i][i] = scale[i];
	// multiply 
	for (int i = 0; i < 4; i++) 
		for (int j = 0; j < 4; j++) {
			m[i][j] = 0.0f;
			for (int k = 0; k < 4; k++)
				m[i][j] += mtmp[k][j] * s[i][k];
		}
} // scale an object's 3 dimensions independently via its local transform, translation and orientation unchanged

void AsymmetricFrustum(qgl_matrix_type dest, const double left, const double right, const double bottom, const double top, const double znear, const double zfar){
	// follow process on glFrustum man page
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)	dest[i][j] = 0.0f;
	dest[0][0] = 2.0 * znear / (right - left);
	dest[1][1] = 2.0 * znear / (top - bottom);
	dest[2][0] = (right + left) / (right - left);
	dest[2][1] = (top + bottom) / (top - bottom);
	dest[2][2] = (znear + zfar) / (znear - zfar);
	dest[2][3] = -1.0;
	dest[3][2] = 2.0*zfar*znear / (znear - zfar);
}

void SymmetricFrustum(qgl_matrix_type dest, const double vFOV, const double aspect, const double znear, const double zfar){
	// follow process on gluPerspective man page
	double f = 1.0 / tan(Q_DEG_TO_RAD(vFOV) / 2.0);
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)	dest[i][j] = 0.0f;
	dest[0][0] = f / aspect;
	dest[1][1] = f;
	dest[2][2] = (znear + zfar) / (znear - zfar);
	dest[2][3] = -1.0;
	dest[3][2] = 2.0*zfar*znear / (znear - zfar);
} // SymmetricFrustum

void Camera(qgl_matrix_type dest, const q_vec_type eye, const q_vec_type focus, const q_vec_type up) {
	// follow double cross process on gluLookat man page
	qgl_matrix_type rot = Q_ID_MATRIX, trans = Q_ID_MATRIX;
	q_vec_type F, UP, S, U;
	q_vec_subtract(F, focus, eye);
	q_vec_normalize(F, F);
	q_vec_normalize(UP, up);
	q_vec_cross_product(S, F, UP);
	q_vec_normalize(S, S);
	q_vec_cross_product(U, S, F);
	// rotation part
	for (int i = 0; i < 3; i++){
		rot[i][0] = S[i];
		rot[i][1] = U[i];
		rot[i][2] = -F[i];
	}
	// translation part
	trans[3][0] = -eye[0]; trans[3][1] = -eye[1]; trans[3][2] = -eye[2];
	// multiply to get final modelview matrix
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			dest[i][j] = 0.0f;
			for (int k = 0; k < 4; k++)
				dest[i][j] += rot[k][j] * trans[i][k];
			}
}// Camera

//-----------------------------------------------------------------------------

void ViewSystem(const bool righteye) {
	bViewchanged = false;
	if (numeyes == 2) {// stereo, i.e. two trips through render loop, right eye first, then left
		if (righteye)
			{
				q_vec_scale(Yaxis, 0.034, Yaxis);
				q_vec_subtract(Pos, Pos, Yaxis);
			} ////*** fifth stage - use quat's vector methods to move Pos along (global) Yaxis - but in which direction and by how much?
		else 
			{
				q_vec_add(Pos, Pos, Yaxis);
				q_vec_add(Pos, Pos, Yaxis);
			} ////*** fifth stage -  move Pos back to centre then along Y in opposite direction
	}

	qgl_matrix_type proj;  // perspective matrix
	if (bInperson) {   // first-person moving view - stages three and four
		////*** third stage - static view mimics symmetric frustum calculations
		//AsymmetricFrustum(proj, -(((2.155 * 2) / 4) * 0.1), ((2.155 * 2) / 4) * 0.1, -(((1.155 * 2) / 4) * 0.1), ((1.155 * 2) / 4) * 0.1, 0.1, 10.0); //numerical values for left, right etc to make look similar to the original
		//SymmetricFrustum(proj, 55.0, viewaspect, 0.1, 10.0); // not implemented yet so make same as static for now in case 'p' is pressed

		////*** fourth (+subsequent) stage(s) - 5 variables needed for cube fixed on the screen, moving view. Global Pos, with tracker X component in element 0, Y in 1, Z in 2
		double unsignedleft = ((screenheight * viewaspect) / 2.0) - Pos[1]; // calculation here using eye position Pos, screen height and viewaspect
		double unsignedright = ((screenheight * viewaspect) / 2.0) + Pos[1]; // calculation here using eye position Pos, screen height and viewaspect
		double unsignedbottom = Pos[2] - screenup - (screenheight / 2.0); // calculation here using eye position Pos and screenup
		double unsignedtop = screenup - (screenheight / 2.0) - Pos[2]; // calculation here using eye position Pos, screen height and screenup
		double S = 0.1 / (screendist - Pos[0]); // calculation here using eye position Pos, near (= 0.1) and screendist
		AsymmetricFrustum(proj, (unsignedleft * -S), unsignedright * S, (unsignedbottom * -S), unsignedtop * S, 0.1, 10.0); // uses above, scaled AND with appropriate l, r, t, b signs practised at stage 3
	}
	else { // static 
		SymmetricFrustum(proj, 55.0, viewaspect, 0.1, 10.0);  // the vertical (OpenGL axes) angle of the frustum, aspect ratio, near and far - don't change
	}
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

	qgl_matrix_type look;  // modelview matrix - the 'camera'
	q_vec_type eyepos, focus, up;
	if (bInfront) {
		q_vec_set(eyepos, 0.0, 2.0, 4.0);   // where we are looking from (OpenGL axes)
		q_vec_set(focus, 0.0, 0.0, 0.0); // and to 
		q_vec_set(up, 0.0, 1.0, 0.0);    // with up towards the ceiling
	}
	else if (bOblique) {
		q_vec_set(eyepos, 2.5, 2.0, -2.5);  // sit above the far right corner of the ground plane as seen from infront
		q_vec_set(focus, -2.5, 0.0, 2.5);// looking at the near left corner of the ground plane
		q_vec_set(up, 0.0, 1.0, 0.0);
	}
	else if (bInperson) { // not impemented yet, so make same as infront
		q_vec_set(eyepos, -Pos[1], Pos[2], -Pos[0]);  //// *** fourth (+subsequent) stage(s) - the TRANSFORMED eye position Pos (NB this is OpenGl space)
		q_vec_set(focus, -Pos[1], Pos[2], -screendist); //// *** fourth (+subsequent) stage(s) -  the TRANSFORMED looked-at position opposite Pos (NB this is OpenGl space)
		q_vec_set(up, 0.0, 1.0, 0.0);
	}
	else if (bSideon) { // not impemented yet, so make same as infront
		q_vec_set(eyepos, 4.0, screenup, -screendist);  //// *** second stage - same plane as screen at around glasses height, right-hand edge of the ground plane (NB this is OpenGl space)
		q_vec_set(focus, -4.0, screenup, -screendist); //// *** second stage - opposite side of scene to eye (reflect in gl's x=0 plane)
		q_vec_set(up, 0.0, 1.0, 0.0);
	}
	else { // shouldn't arrive here unless the camera preset logic is faulty
		q_vec_set(eyepos, 0.0, 1.0, 0.0);
		q_vec_set(focus, 0.0, 0.0, -4.0);
		q_vec_set(up, 0.0, 1.0, 0.0);
	}
	Camera(look, eyepos, focus, up);
	glUniformMatrix4fv(lookLoc, 1, GL_FALSE, &look[0][0]);

}//ViewSystem

//-----------------------------------------------------------------------------

// construct the objects 
void ConstructFrames() {
	// one-metre axis object
	float vertices[] = {    0.0f, 0.0f, 0.0f,   // origin
							1.0f, 0.0f, 0.0f,   // red
							1.0f, 0.0f, 0.0f,   // Endpt X axis
							1.0f, 0.0f, 0.0f,   // red
							0.0f, 0.0f, 0.0f,   // origin
							0.0f, 0.8f, 0.0f,   // green
							0.0f, 1.0f, 0.0f,   // Endpt Y axis
							0.0f, 0.8f, 0.0f,   // green
							0.0f, 0.0f, 0.0f,   // origin
							0.0f, 0.0f, 1.0f,   // blue
							0.0f, 0.0f, 1.0f,    // Endpt Z axis
							0.0f, 0.0f, 1.0f }; // blue

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

}//ConstructFrames

void ConstructGround() {
	int nCols=5, nRows=5;
	float vertices[144];
	for (int i = 0; i < 144; i++) vertices[i] = 1.0f; //initialise all white but overwrite with posns

    // specify nRows by nCols one metre square tiles of the ground plane (Tracker Z=0)
	int i = -1;
	float x2 = (float)nCols / 2.0f, x1 = -x2;
	float y2 = (float)nRows / 2.0f, y1 = -y2;
	for (int n=0; n<=nRows; n++) {
		float y = y1 + (float)n;
		// draw lines on the ground from x1 to x2, i.e. parallel to the screen, at separation y
		vertices[++i] = x1; vertices[++i] = y; vertices[++i] = 0.0f;
		i += 3; // preserve this vertex's colour entry
		vertices[++i] = x2; vertices[++i] = y; vertices[++i] = 0.0f;
		i += 3;
	}
	for (int n=0; n<=nCols; n++) {
		float x = x1 + (float)n;
		// draw lines on the ground from y1 to y2, i.e. perpendicular to the screen, at separation x
		vertices[++i] = x; vertices[++i] = y1; vertices[++i] = 0.0f;
		i += 3;
		vertices[++i] = x; vertices[++i] = y2; vertices[++i] = 0.0f;
		i += 3;
	}
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 36, sizeof(vertices), vertices);  // frames are already in first portion

}//ConstructGround

void ConstructCube() {
	float vertices[144];  

	for (int i = 0; i < 144; i += 3){ vertices[i] = 1.0f; vertices[i + 1] = 1.0f; vertices[i + 2] = 0.0f; } //initialise all yellow but overwrite with posns and 1 white face
	float x2 = 0.5f, x1 = -x2;  // 1m cube centred round the origin
	float y2 = x2, y1 = -y2;
	float z2 = x2, z1 = -z2;
	int i = -1;
	for (int n = 0; n < 2; n++) {
		float y = y1 + (float)n;
		float x = x1 + (float)n;
		// edges parallel to the X axis
		vertices[++i] = x1; vertices[++i] = y; vertices[++i] = z1;
		i += 3; // preserve this vertex's colour entry
		vertices[++i] = x2; vertices[++i] = y; vertices[++i] = z1;
		i += 3;
		vertices[++i] = x1; vertices[++i] = y; vertices[++i] = z2;
		vertices[++i] = 1.0; vertices[++i] = 1.0; vertices[++i] = 1.0; // plane z = 0.5 is white
		vertices[++i] = x2; vertices[++i] = y; vertices[++i] = z2;
		vertices[++i] = 1.0; vertices[++i] = 1.0; vertices[++i] = 1.0;
		// edges parallel to the Z axis
		vertices[++i] = x1; vertices[++i] = y; vertices[++i] = z1;
		i += 3;
		vertices[++i] = x1; vertices[++i] = y; vertices[++i] = z2;
		i += 3;
		vertices[++i] = x2; vertices[++i] = y; vertices[++i] = z1;
		i += 3;
		vertices[++i] = x2; vertices[++i] = y; vertices[++i] = z2;
		i += 3;
		// edges parallel to the Y axis
		vertices[++i] = x; vertices[++i] = y1; vertices[++i] = z1;
		i += 3;
		vertices[++i] = x; vertices[++i] = y2; vertices[++i] = z1;
		i += 3;
		vertices[++i] = x; vertices[++i] = y1; vertices[++i] = z2;
		vertices[++i] = 1.0; vertices[++i] = 1.0; vertices[++i] = 1.0;
		vertices[++i] = x; vertices[++i] = y2; vertices[++i] = z2;
		vertices[++i] = 1.0; vertices[++i] = 1.0; vertices[++i] = 1.0;
	}
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 180, sizeof(vertices), vertices);  // frames and ground are already in first portion

}//ConstructCube

void ConstructScreen() {// a 1 metre by 1 metre vertical (in tracker space) screen of zero x thickness
	float vertices[72];
	for (int i = 0; i < 72; i++) vertices[i] = 0.0f; //initialise all black but overwrite with posns

	float y2 = 0.5f, y1 = -y2; // vertical (in Tracker space) screen, centred round the origin
	float z2 = 0.5f, z1 = -z2;
	int i = -1;
	for (int n = 0; n < 2; n++){
		float y = y1 + (float)n;
		float z = z1 + (float)n;
		// vertical edges
		vertices[++i] = 0; vertices[++i] = y; vertices[++i] = z1;
		i += 3; // skip colour
		vertices[++i] = 0; vertices[++i] = y; vertices[++i] = z2;
		i += 3;
		// horizontal edges
		vertices[++i] = 0; vertices[++i] = y1; vertices[++i] = z;
		i += 3;
		vertices[++i] = 0; vertices[++i] = y2; vertices[++i] = z;
		i += 3;
	}
	// diagonals
	vertices[++i] = 0; vertices[++i] = y1; vertices[++i] = z1;
	i += 3;
	vertices[++i] = 0; vertices[++i] = y2; vertices[++i] = z2;
	i += 3;
	vertices[++i] = 0; vertices[++i] = y1; vertices[++i] = z2;
	i += 3;
	vertices[++i] = 0; vertices[++i] = y2; vertices[++i] = z1;
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 324, sizeof(vertices), vertices); // frames, ground and cube are already in first portion

}//ConstructScreen

void ReadyShaders(){
	// compile a simple vertex shader
	const GLchar *vshadersource[1] = { "#version 330\n"
		"in vec3 Position;\n"
		"in vec3 RGB;\n"
		"uniform mat4 local;\n"
		"uniform mat4 togl;\n"
		"uniform mat4 look;\n"
		"uniform mat4 proj;\n"
		"out vec4 Color;\n"
		"void main()\n"
		"{	 gl_Position = proj*look*togl*local*vec4(Position, 1.0);\n"
		"Color = vec4(RGB, 1.0);\n"
		"}\n\0" };
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, vshadersource, NULL);
	glCompileShader(vshader);

	// compile a simple fragment shader
	const GLchar *fshadersource[1] = { "#version 330\n"
		"in vec4 Color;\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{ FragColor = Color;\n"
		"}\n\0" };
	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, fshadersource, NULL);
	glCompileShader(fshader);

	// attach, link and clean up
	shader = glCreateProgram();
	glAttachShader(shader, vshader);
	glAttachShader(shader, fshader);
	glLinkProgram(shader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	// attributes and uniforms accessed elsewhere
	glUseProgram(shader);
	PositionLoc = glGetAttribLocation(shader, "Position");
	RGBLoc = glGetAttribLocation(shader, "RGB");
	localLoc = glGetUniformLocation(shader, "local");
	toglLoc = glGetUniformLocation(shader, "togl");
	lookLoc = glGetUniformLocation(shader, "look");
	projLoc = glGetUniformLocation(shader, "proj");

} // simple embedded vertex and fragment shaders

void OnCreate() {

	glClearColor(0.7f, 0.7f, 0.7f, 1.0);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.2);
	glEnable(GL_DEPTH_TEST);
	glGenBuffers(1, &vertexbufferid);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferid);
	// 6 vertices of frames (3 lines), posns (3) and colours (3) each
	// plus 24 vertices of ground (2 sets of 6 lines), posns (3) and colours (3) each
	// plus 24 vertices of cube (3 sets of 4 lines), posns (3) and colours (3) each
	// pluse 12 vertices of 'screen' (4 edges, 2 diagonals), posns (3) and colours (3) each
	// = 396
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 396, NULL, GL_STATIC_DRAW);   
	ConstructFrames();  // put vertex values and colours into the currently bound buffer
	ConstructGround();
	ConstructCube();
	ConstructScreen();

	ReadyShaders();

	// make the just-constructed arrays available to the just-constructed shader
	glEnableVertexAttribArray(PositionLoc);  // positions
	glEnableVertexAttribArray(RGBLoc);  // RGB values
	glVertexAttribPointer(PositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *)0); // start at the beginning but stride over the colours (colours stride over positions)
	glVertexAttribPointer(RGBLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *)(sizeof(float) * 3));  // start beyond the 3 posns of the first vertex

	// axial transform - generally needed so do this as soon as the shader is in use
	q_vec_type axis;
	q_vec_set(axis, 0.0, 0.0, 1.0);  // opengl axis object rotates anticlockwise with z pointing towards
	rotateObject(togl, axis, 90.0);  // togl already initialised to the identity
	q_vec_set(axis, 0.0, 1.0, 0.0);  // resulting object rotates anticlockwise with its (new) y pointing towards
	rotateObject(togl, axis, 90.0);  // resulting tracker axial object's matrix is the required tracker-to-gl axial transform 
									 // Objects subsequently described in tracker space will render correctly in OpenGL frame of reference
	glUniformMatrix4fv(toglLoc, 1, GL_FALSE, &togl[0][0]);

}//OnCreate

void OnDestroy() {

	std::cerr << "OnDestroy: cleaning up\n";
	// shader clean-up
	glDisableVertexAttribArray(PositionLoc);
	glDisableVertexAttribArray(RGBLoc);
	glDeleteBuffers(1, &vertexbufferid);
	glDeleteProgram(shader);

	// unregister tracker handler and close tracked device
	if (device)
		device->unregister_change_handler(NULL, handle_tracker_pos_quat);
}//OnDestroy

// GLUT callbacks and callees
void OnDisplay() {
	bool bRightEye = true;
	q_vec_type scale, translation, axis;

	for (int i = 0; i < numeyes; i++) {
		if (bRightEye) {// stereo filters - don't change
			glDrawBuffer(GL_BACK_RIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//if (numeyes == 2) glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);  // right eye draws cyan
		} 
		else {
			glDrawBuffer(GL_BACK_LEFT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);  // left eye draws red
		}

		if (bInperson || bViewchanged) ViewSystem(bRightEye); // multi-function moving/static - don't change

		// rotate and translate 30cm frames according to the tracked data in matrix local - don't change
		q_vec_set(scale, 0.3, 0.3, 0.3);
		if (i == 0) scaleObject(local, scale); // only one scale-down of 'local' per mono/stereo draw
		glUniformMatrix4fv(localLoc, 1, GL_FALSE, &local[0][0]);
		if (!bInperson) glDrawArrays(GL_LINES, FRAMES, FRAMESIZE); // don't draw when viewing from the glasses' perspective

		// pop the rotn matrix out of localLoc for static objects - don't change
		qgl_matrix_type localtmp = Q_ID_MATRIX;
		glUniformMatrix4fv(localLoc, 1, GL_FALSE, &localtmp[0][0]);
		// draw the earth, unmoved :-)
		glDrawArrays(GL_LINES, GROUND, GROUNDSIZE);

		//// *** original - draws the tracker axes full-size on the ground and slightly shifted along the tile diagonal
		//// *** first stage - move out of the user's path according to the spec. 
		////     Numerical values OK
		q_vec_set(translation, 0.0, 1.0, 0.0); //why is the ground z=zero in tracker space?
		translateObject(localtmp, translation);

		glUniformMatrix4fv(localLoc, 1, GL_FALSE, &localtmp[0][0]);
		glDrawArrays(GL_LINES, FRAMES, FRAMESIZE);

		q_vec_set(translation, -0.0, -1.0, -0.0); //transformations accummulate so undo the most recent translation to re-use localtmp
		translateObject(localtmp, translation);

		// pop the togl matrix to draw an OpenGl object - don't change
		qgl_matrix_type tmp = Q_ID_MATRIX;
		glUniformMatrix4fv(toglLoc, 1, GL_FALSE, &tmp[0][0]);

		//// *** original - draws an OpenGL axial frame on the ground, half size and slightly shifted opposite to the tracker axes
		//// *** first stage - move out of the user's path according to the spec. 
		////     Numerical values OK
		q_vec_set(translation, 1.0, 0.0, 0.0); //why is the same ground y=zero in OpenGl space? Why is this shift opposite to 0.1, 0.1, 0.0?
		translateObject(localtmp, translation);

		q_vec_set(scale, 0.5, 0.5, 0.5);
		scaleObject(localtmp, scale);

		glUniformMatrix4fv(localLoc, 1, GL_FALSE, &localtmp[0][0]);
		glDrawArrays(GL_LINES, FRAMES, FRAMESIZE);

		q_vec_set(translation, -1.0, -0.0, -0.0); //transformations accummulate so undo the most recent translation to re-use localtmp
		translateObject(localtmp, translation);

		// push back the togl for remaining objects, defined in tracker space - don't change
		glUniformMatrix4fv(toglLoc, 1, GL_FALSE, &togl[0][0]);

		if (screenheight <= 0)
		{
			screenheight = 0.00243;
			cubefacesize[0] = screenheight - (screenheight / 4);
			cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);
		}

		//// *** important parametrisation
		////     object-sizing and object-placing variables - assign suitable values in the global section and use in the screen and cube sections below
		////     screenheight (width is got programmatically using viewaspect = width / height) and cubefacesize
		////     screendist and screenup
		//// *** once set, do not reassign here. These values are modified using keyboard keys in order to check your size-distance dependencies

		//// *** original - draws the monitor 30cm tall and 40cm wide, moved away from the origin and with its bottom edge on the ground 
		//// *** first stage - scale as the graphics window and move it out of the user's path according to the spec.
		////     Use above sizing variables wherever they occur, numerical values elsewhere
		//// *** second stage - move it to be in front of user according to the spec, look at it from 'side-on' (ViewSystem)
		////     Use above placing and sizing variables wherever they occur, numerical values elsewhere
		qgl_matrix_type tmp2 = Q_ID_MATRIX;

		q_vec_set(scale, 1.0, (viewaspect * screenheight), screenheight);            //how do you do this using only height and viewaspect?
		scaleObject(tmp2, scale);

		q_vec_set(translation, screendist, 0.0, screenup);  //what is the significance of the halved translation in the original?
		translateObject(tmp2, translation);

		glUniformMatrix4fv(localLoc, 1, GL_FALSE, &tmp2[0][0]);
		glDrawArrays(GL_LINES, MONITOR, MONITORSIZE);

		q_vec_set(scale, -1.0, -(viewaspect * screenheight), -screenheight);            //how do you do this using only height and viewaspect?
		scaleObject(tmp2, scale);

		q_vec_set(translation, -screendist, 0.0, -screenup);  //what is the significance of the halved translation in the original?
		translateObject(tmp2, translation);

		//// *** original - draws a 60cm wide cube moved so it rests on the ground with its white face on top
		//// *** first stage - move it out of the user's path according to the spec, and rotated as below.
		////     Use above sizing variables wherever they occur, numerical values elsewhere
		//// *** second stage - move it to be in front of the user, rotated as below, look at it from 'side-on' (ViewSystem)
		////     Use above sizing and placing variables wherever they occur, numerical values elsewhere
		//// *** fifth stage - change shape and position according to the spec, to demonstrate positive, zero and negative parallax
		////     Use above sizing and placing variables plus viewaspect wherever they occur, numerical values elsewhere
		qgl_matrix_type tmp3 = Q_ID_MATRIX;

		q_vec_set(scale, cubefacesize[0], cubefacesize[1], cubefacesize[0]);
		scaleObject(tmp3, scale);

		q_vec_set(translation, ((cubefacesize[0] / 4.0) + screendist), 0.0, screenup);  //what is the significance of the halved translation in the original?
		translateObject(tmp3, translation);

		//// *** original - rotated 45 degrees anticlockwise with tracker z pointing towards us
		//// *** first stage - rotate clockwise 10 degrees per 10cm of face size, with tracker z pointing towards us
		//// *** second (+subsequent) stage(s) - rotate so white face is facing the user, numerical values OK
		q_vec_set(axis, 0.0, 1.0, 0.0);
		rotateObject(tmp3, axis, -90.0);

		glUniformMatrix4fv(localLoc, 1, GL_FALSE, &tmp3[0][0]);
		glDrawArrays(GL_LINES, CUBE, CUBESIZE);

		q_vec_set(scale, -cubefacesize[0], -cubefacesize[0], -cubefacesize[0]);
		scaleObject(tmp3, scale);

		q_vec_set(translation, -((cubefacesize[0] / 4.0) + screendist), 0.0, -screenup);  //what is the significance of the halved translation in the original?
		translateObject(tmp3, translation);

		q_vec_set(axis, 0.0, 1.0, 0.0);
		rotateObject(tmp3, axis, 90.0);

		bRightEye = false;
		if (numeyes == 2) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // stereo filters - don't change
	}
	glutSwapBuffers();
}//OnDisplay

void OnReshape(int w, int h) {
	width=w;
	height=h;
	glViewport(0,0,w,h);
	viewaspect = h?(float)w/(float)h:1.0f;

	cubefacesize[0] = screenheight - (screenheight / 4);
	cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);

	bViewchanged = true;
	OnDisplay();
}//OnReshape

void UpdateTracker() {
	// purge old reports if we've been slow getting here
	device->mainloop();

	track_data_waiting = 0;
	while (!track_data_waiting)
		device->mainloop();

	// callback has activated, so data should be available
	if (bShowData) {
		std::cout << "X=(" << Pos[0] << ") ";
		std::cout << "Y=(" << Pos[1] << ") ";
		std::cout << "Z=(" << Pos[2] << ") \n";
	}
}//UpdateTracker

void OnKeyboard(unsigned char key, int x, int y) {
	switch (tolower(key)) {
	case 'd':	// toggle display of data
		bShowData = !bShowData;
		break;
	case 'i':	// infront camera 
		bInfront = true;
		bOblique = false;
		bSideon = false;
		bInperson = false; // only one camera active
		bViewchanged = true;
		break;
	case 'o':	// oblique camera
		bInfront = false;
		bOblique = true;
		bSideon = false;
		bInperson = false;
		bViewchanged = true;
		break;
	case 's':	// side-on camera
		bInfront = false;
		bOblique = false;
		bInperson = false;
		bSideon = true;
		bViewchanged = true;
		break;
	case 'p':	// first-person camera
		bInfront = false;
		bOblique = false;
		bInperson = true;
		bSideon = false;
		// ViewSystem now called per draw anyway
		break;
	case '2':	// toggle stereo
		bStereo = !bStereo;
		if (bStereo) numeyes = 2; else numeyes = 1;
		break;
		// functionality test keys (also see OnSpecial)
	case '+':  // inflate the cube and monitor height
		if (screenheight <= 0)
		{
			screenheight = 0.00243;
			cubefacesize[0] = screenheight - (screenheight / 4);
			cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);
		}

		screenheight += 0.01;  // 1cm increments
		cubefacesize[0] = screenheight - (screenheight / 4);
		cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);
		break;
	case '-':  // deflate the cube and monitor height
		screenheight -= 0.01;  // 1cm decrements
		cubefacesize[0] = screenheight - (screenheight / 4);
		cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);

		if (screenheight <= 0)
		{
			screenheight = 0.00243;
			cubefacesize[0] = screenheight - (screenheight / 4);
			cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);
		}
		break;
	case 27:	// ESC was pressed
		glutLeaveMainLoop();
		break;
	}
}//OnKeyboard

void OnSpecial(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP: // crank screen further away
		screendist += 0.1; // 10cm increments
		break;
	case GLUT_KEY_DOWN: // crank screen nearer
		screendist -= 0.1; // 10cm reductions
		break;
	case GLUT_KEY_RIGHT:  // up the wall
		screenup += 0.1;
		break;
	case GLUT_KEY_LEFT:  // down the wall
		screenup -= 0.1;
		break;
	}
} // monitor and screen moving keys

void OnIdle() {
	UpdateTracker();
	glutPostRedisplay();
}//OnIdle

int main (int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STEREO);

	// print usage information
	if (argc < 2) {
		std::cerr << "Usage: 08347ACW <object>@<machinename>\n";
		return 1;
	}

	// get the device and set it up for VRPN
	device = new vrpn_Tracker_Remote(argv[1]);
	if (device == NULL) {
		std::cerr << "Cannot open specified device\n";
	    return 1;
	} else {
		std::cerr << "Opened tracker " << argv[1] << "\n";
		strncpy(tracker_name, argv[1], BUFRSIZE-1);
		tracker_name[BUFRSIZE-1] = '\0';
	}

	// register VRPN pos/quat callback
	device->register_change_handler(NULL, handle_tracker_pos_quat);

	// nice big window
	width = glutGet(GLUT_SCREEN_WIDTH) - 100;
	height = glutGet(GLUT_SCREEN_HEIGHT) - 100;
	viewaspect = height ? (float)width / (float)height : 1.0f; // needed for symmetric frustum and screen-shaped objects
	glutInitWindowSize(width, height);
	glutCreateWindow(tracker_name);
	glutPositionWindow(0, 0);

	cubefacesize[0] = screenheight - (screenheight / 4);
	cubefacesize[1] = (screenheight * viewaspect) - ((screenheight * viewaspect) / 4);

	// register GLUT callbacks
	glutDisplayFunc(OnDisplay);
	glutReshapeFunc(OnReshape);
	glutKeyboardFunc(OnKeyboard);
	glutSpecialFunc(OnSpecial);
	glutIdleFunc(OnIdle);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	// initialise GLEW
	GLenum res;
	if ((res = glewInit()) != GLEW_OK){
		std::cerr << "Problems initialising GLEW" << glewGetErrorString(res) << "\n";
		return 1;
	}

	OnCreate();  // make the objects and make available to the shader, load the axial transform

	// hand over to GLUT mainloop
	glutMainLoop();

	OnDestroy();  // this is FreeGLUT, so we will get back

	return 0;
}//main

//-----------------------------------------------------------------------------
