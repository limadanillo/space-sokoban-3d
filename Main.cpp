/*
Program Title: Main.cpp
Author: Darren Glyn Roberts
Date: 07/03/2013
Version: 1.0
*/

#include <stdlib.h>         // Header file for standard library
#include <windows.h>		// Header File For Windows
#include <mmsystem.h>       // Header file to use mciSendString()
#include <stdio.h>          // Header file for standard input/output
#include <stdarg.h>         // Header file for standard arguments
#include <gl/gl.h>			// Header File For The OpenGL32 Library
#include <gl/glu.h>			// Header File For The GLu32 Library
#include <gl/glaux.h>		// Header File For The Glaux Library
#include "Game.h"           // Header file for Game class
#define GL_CLAMP_TO_EDGE 0x812F    // Clamp textures to edge, used when texture mapping
#define MAX_PARTICLES   3000 // Number of particles in each particle system

using namespace std;        // Namespace for std

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;	// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

game spaceSokoban;          // Game object

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag. Toggle for fullscreen
bool    started = FALSE;    // Flag to signify game has been started
short   control = 0;        // Control speed of key presses
bool    colour = TRUE;      // Multi colour
bool    cycleColour;        // Stay on colour
bool    reset;              // Change back to colours
bool    light;              // Lighting ON / OFF
bool    fog;                // Fog ON / OFF
bool    lightToggle;        // Toggle light
bool    fogToggle;          // Toggle fog

GLfloat fogColor[4]= {0.5f, 0.5f, 0.5f, 1.0f};      // Fog Color
GLfloat	resistance = 2.0f;				            // Resistance of particle velcotiy
GLfloat	xVelocity;						            // Base X Speed (To Allow Keyboard Direction Of Tail)
GLfloat	yVelocity;						            // Base Y Speed (To Allow Keyboard Direction Of Tail)
GLfloat	particleZoom = -40.0f;				        // Initial Particle system zoom
GLuint	col;						                // Current Color Selection
GLuint	delay;						                // Rainbow Effect Delay

GLYPHMETRICSFLOAT gmf[256]; // Contains information about the placement and orientation of a glyph in a character cell.

GLuint texture[13];         // Number of textures
GLuint base;                // Storage For 256 Characters
GLuint moves = 0;           // Number of player moves
GLuint pushes = 0;          // Number of player pushes
GLfloat time = 0.0f;        // Time elapsed
GLfloat move_x = 0.0f;      // Translation of maze along x-axis
GLfloat move_y = 0.0f;      // Translation of maze along y-axis
GLfloat move_z = 0.0f;      // Translation of maze along z-axis
GLfloat rotationX = 0.0f;   // Camera rotation along x-axis
GLfloat rotationY = 0.0f;   // Camera rotation along y-axis
GLfloat rotationZ = 0.0f;   // Camera rotation along z-axis
GLfloat positionX = 0.0f;   // Camera position along x-axis
GLfloat positionY = 0.0f;   // Camera position along y-axis
GLfloat positionZ = 0.0f;   // Camera position along z-axis

typedef struct			    // Create A Structure For Particle
{
	bool	active;			// Active (Yes/No)
	GLfloat	life;			// Particle Life
	GLfloat	fade;			// Fade Speed
	GLfloat	red;			// Red Value
	GLfloat	green;			// Green Value
	GLfloat	blue;			// Blue Value
	GLfloat	xLocation;		// X Position
	GLfloat	yLocation;		// Y Position
	GLfloat	zLocation;		// Z Position
	GLfloat	xDirection;		// X Direction
	GLfloat	yDirection;		// Y Direction
	GLfloat	zDirection;		// Z Direction
	GLfloat	xGravity;		// X Gravity
	GLfloat	yGravity;		// Y Gravity
	GLfloat	zGravity;		// Z Gravity
} ps_one;					// Particles Structure

ps_one ps_one_particles[MAX_PARTICLES];	// Particle Array (Room For Particle Info)
ps_one ps_two_particles[MAX_PARTICLES];	// Particle Array (Room For Particle Info)

static GLfloat colours[12][3]=		// Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	base = glGenLists(256);								// Storage For 256 Characters
	font = CreateFont( -8,							    // Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Comic Sans MS");				// Font Name, change for different font type

	SelectObject(hDC, font);							// Selects The Font We Created
	wglUseFontOutlines(	hDC,							// Select The Current DC
						0,								// Starting Character
						255,							// Number Of Display Lists To Build
						base,							// Starting Display Lists
						0.0f,							// Deviation From The True Outlines
						0.0f,							// Font Thickness In The Z Direction
						WGL_FONT_POLYGONS,				// Use Polygons, Not Lines
						gmf);							// Address Of Buffer To Recieve Data
}

GLvoid KillFont(GLvoid)                                 // Delete The Font List
{
	glDeleteLists(base, 256);		                    // Delete All 256 Characters
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	float		length = 0;								// Used To Find The Length Of The Text
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)                                    // If There's No Text
    {									
		return;                                         // Do Nothing
    }									

	va_start(ap, fmt);									// Parses The String For Variables
    vsprintf(text, fmt, ap);						    // And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	for (unsigned int loop = 0; loop < (strlen(text)); loop++)	// Loop To Find Text Length
	{
		length += gmf[text[loop]].gmfCellIncX;			// Increase Length By Each Characters Width
	}

	glTranslatef(-length, 0.0f, 0.0f);					// Center Our Text On The Screen
	
	glPushAttrib(GL_LIST_BIT);							    // Pushes The Display List Bits
        glListBase(base);									// Sets The Base Character to 0
	    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										    // Pops The Display List Bits
}

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}
	return NULL;										// If Load Failed Return NULL
}

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
	int status = FALSE;                                 // Status Indicator
    AUX_RGBImageRec *TextureImage[13];                  // Create Storage Space For The Texture
    memset(TextureImage, 0, sizeof(void *)*13);         // Set The Pointer To NULL
  
    // Load The Bitmaps, Check For Errors, If Bitmap's Not Found Quit
    TextureImage[0] = LoadBMP("textures/square_wall.bmp");
    TextureImage[1] = LoadBMP("textures/cbox.bmp");
    TextureImage[2] = LoadBMP("textures/floor.bmp");
    TextureImage[3] = LoadBMP("textures/player.bmp");
    TextureImage[4] = LoadBMP("textures/mbox.bmp");
    TextureImage[5] = LoadBMP("textures/cboxr.bmp");
    TextureImage[6] = LoadBMP("textures/small_particle.bmp");
    TextureImage[7] = LoadBMP("textures/space_behind.bmp");
    TextureImage[8] = LoadBMP("textures/space_left.bmp");
    TextureImage[9] = LoadBMP("textures/space_front.bmp");
    TextureImage[10] = LoadBMP("textures/space_right.bmp");
    TextureImage[11] = LoadBMP("textures/space_above.bmp");
    TextureImage[12] = LoadBMP("textures/space_below.bmp");
 
    status = TRUE; // Bitmaps loaded succesfully
    glGenTextures(13, &texture[0]);   // Create The Textures
 
    for(int i = 0; i < 13; i++) // Loop through all textures
    {
       glBindTexture(GL_TEXTURE_2D, texture[i]);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Used when texture is drawn bigger than original texture size
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Used when texture is drawn smaller than original texture size
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
       glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->sizeX, TextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[i]->data);
       
        if (TextureImage[i])                    // If Texture Exists
        {
            if (TextureImage[i]->data)           // If Texture Image Exists
            {
                free(TextureImage[i]->data);     // Free The Texture Image Memory
            }
            free(TextureImage[i]);              // Free The Image Structure
        }
    }
	return status;										// Return The status
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0) // Prevent A Divide By Zero By Making Height Equal One
	{
        height=1;
    }					
    	
	glViewport(0, 0, width, height);					// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 5000.0f); // Calculate The Aspect Ratio Of The Window
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{   
    mciSendString("play sounds/music.wav repeat", NULL, 0, NULL);
    
    if(!LoadGLTextures())                               // If textures not loaded
    {
        return FALSE;                                   // return FALSE
    }
    
    GLfloat light_Diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };    // diffuse : color where light hit directly the object's surface, alpha = transparency
    GLfloat light_Ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };    // ambient : color applied everywhere
    GLfloat light_Specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // specular higlights
    GLfloat light_Position[] = { 0.0f, 0.0f, -20.0f, 1.0f }; // Light position

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_Ambient); //Ambient light component
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_Diffuse); //Diffuse light component
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_Specular); //Specular light component
    glLightfv(GL_LIGHT0, GL_POSITION, light_Position); // Light position

    glEnable(GL_NORMALIZE);                             // Normals
    glEnable(GL_LIGHTING);                              // Enable lighting
    glEnable(GL_LIGHT0);                                // Enable light 0
    
    glShadeModel(GL_SMOOTH);                            // Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Background colour
	glClearDepth(1.0f);									// Depth Buffer Setup
	
	glEnable(GL_BLEND);                                 // Enable Blending, see through textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);                   // Type Of Blending To Perform
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);             // Really Nice Point Smoothing

    glEnable(GL_TEXTURE_2D);                            // Enable 2D textures
    
    for (int i = 0; i < MAX_PARTICLES; i++)				// Initials All The Textures
	{
		ps_one_particles[i].active = TRUE;								            // Make All The Particles Active
		ps_one_particles[i].life = 1.0f;								            // Give All The Particles Full Life
		ps_one_particles[i].fade = float(rand() % 100) / 1000.0f + 0.003f;	        // Random Fade Speed
		ps_one_particles[i].red = colours[i * (12 / MAX_PARTICLES)][0];	            // Select Red Color
		ps_one_particles[i].green = colours[i * (12 / MAX_PARTICLES)][1];	        // Select Green  Color
		ps_one_particles[i].blue = colours[i * (12 / MAX_PARTICLES)][2];	        // Select Blue  Color
		ps_one_particles[i].xDirection = float((rand() % 50)- 26.0f) * 10.0f;		// Random Speed On X Axis
		ps_one_particles[i].yDirection = float((rand() % 50)- 25.0f) * 10.0f;		// Random Speed On Y Axis
		ps_one_particles[i].zDirection = float((rand() % 50)- 25.0f) * 10.0f;		// Random Speed On Z Axis
		ps_one_particles[i].xGravity = 0.0f;									    // Set Horizontal Pull To Zero
		ps_one_particles[i].yGravity = -0.8f;								        // Set Vertical Pull Downward
		ps_one_particles[i].zGravity = 0.0f;
		
		ps_two_particles[i].active = TRUE;								            // Make All The Particles Active
		ps_two_particles[i].life = 1.0f;								            // Give All The Particles Full Life
		ps_two_particles[i].fade = float(rand() % 100) / 1000.0f + 0.003f;	        // Random Fade Speed
		ps_two_particles[i].red = colours[i * (12 / MAX_PARTICLES)][0];	            // Select Red Color
		ps_two_particles[i].green = colours[i * (12 / MAX_PARTICLES)][1];	        // Select Green  Color
		ps_two_particles[i].blue = colours[i * (12 / MAX_PARTICLES)][2];	        // Select Blue Color
		ps_two_particles[i].xDirection = float((rand() % 50) - 26.0f) * 10.0f;		// Random Speed On X Axis
		ps_two_particles[i].yDirection = float((rand() % 50) - 25.0f) * 10.0f;		// Random Speed On Y Axis
		ps_two_particles[i].zDirection = float((rand() % 50) - 25.0f) * 10.0f;		// Random Speed On Z Axis
		ps_two_particles[i].xGravity = 0.0f;									    // Set Horizontal Pull To Zero
		ps_two_particles[i].yGravity = -0.8f;								        // Set Vertical Pull Downward
		ps_two_particles[i].zGravity = 0.0f;                                        // Set Pull On Z Axis To Zero
	}
	
     glFogi(GL_FOG_MODE, GL_LINEAR);        // Fog Mode
     glFogfv(GL_FOG_COLOR, fogColor);       // Set Fog Color
     glFogf(GL_FOG_DENSITY, 0.35f);         // How Dense Will The Fog Be
     glHint(GL_FOG_HINT, GL_DONT_CARE);     // Fog Hint Value
     glFogf(GL_FOG_START, -270.0f);         // Fog Start Depth
     glFogf(GL_FOG_END, 270.0f);            // Fog End Depth
     
	 BuildFont();		                    // Build the font
	 return TRUE;							// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear the screen and the depth buffer
    
    glLoadIdentity();                                   // Load the identity matrix, reset the view
    glRotatef(rotationX, 1.0, 0.0, 0.0);                // Rotate camera around x-axis
    glRotatef(rotationY, 0.0, 1.0, 0.0);                // Rotate camera around y-axis
    glRotatef(rotationZ, 0.0, 0.0, 1.0);                // Rotate camera around z-axis
    glTranslatef(positionX, positionY, positionZ);      // Translate camera
    
    glDisable(GL_LIGHTING);                             // Disable lighting
    spaceSokoban.drawSkyBox(texture[7], texture[8], texture[9], texture[10], texture[11], texture[12]); // Draw skybox
    glBindTexture(GL_TEXTURE_2D, texture[6]);                            // Particle texture
        
     for (int i = 0; i < MAX_PARTICLES; i++)					        // Loop Through All The Particles
	{
		if (ps_one_particles[i].active)							        // If The Particle Is Active
		{
			GLfloat x = ps_one_particles[i].xLocation;					// Grab Our Particle X Position
			GLfloat y = ps_one_particles[i].yLocation;					// Grab Our Particle Y Position
			GLfloat z = ps_one_particles[i].zLocation + particleZoom;	// Particle Z Position + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(ps_one_particles[i].red, ps_one_particles[i].green, ps_one_particles[i].blue, ps_one_particles[i].life);

			glBegin(GL_TRIANGLE_STRIP);						          // Build Quad From A Triangle Strip
			    glTexCoord2d(1,1); glVertex3f(x + 0.5f, y + 0.5f, z); // Top Right
				glTexCoord2d(0,1); glVertex3f(x - 0.5f, y + 0.5f, z); // Top Left
				glTexCoord2d(1,0); glVertex3f(x + 0.5f, y - 0.5f, z); // Bottom Right
				glTexCoord2d(0,0); glVertex3f(x - 0.5f, y - 0.5f, z); // Bottom Left
			glEnd();										          // Done Building Triangle Strip

			ps_one_particles[i].xLocation += ps_one_particles[i].xDirection / (resistance * 1000);// Move On The X Axis By X Speed
			ps_one_particles[i].yLocation += ps_one_particles[i].yDirection / (resistance * 1000);// Move On The Y Axis By Y Speed
			ps_one_particles[i].zLocation += ps_one_particles[i].zDirection / (resistance * 1000);// Move On The Z Axis By Z Speed
			ps_one_particles[i].xDirection += ps_one_particles[i].xGravity;			              // Take Pull On X Axis Into Account
			ps_one_particles[i].yDirection += ps_one_particles[i].yGravity;			              // Take Pull On Y Axis Into Account
			ps_one_particles[i].zDirection += ps_one_particles[i].zGravity;			              // Take Pull On Z Axis Into Account
			ps_one_particles[i].life -= ps_one_particles[i].fade;		                          // Reduce Particles Life By 'Fade'

			if (ps_one_particles[i].life < 0.0f)					                              // If Particle Is Burned Out
			{
				ps_one_particles[i].life = 1.0f;					                              // Give It New Life
				ps_one_particles[i].fade = float(rand() % 100) / 1000.0f+0.003f;	              // Random Fade Value
				ps_one_particles[i].xLocation = -23.0f;						                      // Center On X Axis
				ps_one_particles[i].yLocation = 7.0f;						                      // Center On Y Axis
				ps_one_particles[i].zLocation = 0.0f;						                      // Center On Z Axis
				ps_one_particles[i].xDirection = xVelocity + float((rand() % 60) -32.0f);	      // X Axis Speed And Direction
				ps_one_particles[i].yDirection = yVelocity + float((rand() % 60) -30.0f);	      // Y Axis Speed And Direction
				ps_one_particles[i].zDirection = float((rand() % 60) -30.0f);	                  // Z Axis Speed And Direction
				ps_one_particles[i].red = colours[col][0];			                              // Select Red From Color Table
				ps_one_particles[i].green = colours[col][1];		                              // Select Green From Color Table
				ps_one_particles[i].blue = colours[col][2];			                              // Select Blue From Color Table
			}

			if (keys[VK_NUMPAD8] && (ps_one_particles[i].yGravity < 1.5f)) ps_one_particles[i].yGravity += 0.01f;  // If Number Pad 8 And Y Gravity Is Less Than 1.5 Increase Pull Upwards
			if (keys[VK_NUMPAD2] && (ps_one_particles[i].yGravity > -1.5f)) ps_one_particles[i].yGravity -= 0.01f; // If Number Pad 2 And Y Gravity Is Greater Than -1.5 Increase Pull Downwards
			if (keys[VK_NUMPAD6] && (ps_one_particles[i].xGravity < 1.5f)) ps_one_particles[i].xGravity += 0.01f;  // If Number Pad 6 And X Gravity Is Less Than 1.5 Increase Pull Right
			if (keys[VK_NUMPAD4] && (ps_one_particles[i].xGravity > -1.5f)) ps_one_particles[i].xGravity -= 0.01f; // If Number Pad 4 And X Gravity Is Greater Than -1.5 Increase Pull Left

			if (keys['N'])										                        // 'N' Key Causes A Burst of particles
			{
				ps_one_particles[i].xDirection = float((rand() % 50) -26.0f) * 10.0f;	// Random Speed On X Axis
				ps_one_particles[i].yDirection = float((rand() % 50) -25.0f) * 10.0f;	// Random Speed On Y Axis
				ps_one_particles[i].zDirection = float((rand() % 50) -25.0f) * 10.0f;	// Random Speed On Z Axis
			}
		}
		
		if (ps_two_particles[i].active)							             // If The Particle Is Active
		{
			float x = ps_two_particles[i].xLocation;						// Grab Our Particle X Position
			float y = ps_two_particles[i].yLocation;						// Grab Our Particle Y Position
			float z = ps_two_particles[i].zLocation + particleZoom;			// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(ps_two_particles[i].red, ps_two_particles[i].green, ps_two_particles[i].blue, ps_two_particles[i].life);

			glBegin(GL_TRIANGLE_STRIP);						          // Build Quad From A Triangle Strip
			    glTexCoord2d(1,1); glVertex3f(x + 0.5f, y + 0.5f, z); // Top Right
				glTexCoord2d(0,1); glVertex3f(x - 0.5f, y + 0.5f, z); // Top Left
				glTexCoord2d(1,0); glVertex3f(x + 0.5f, y - 0.5f, z); // Bottom Right
				glTexCoord2d(0,0); glVertex3f(x - 0.5f, y - 0.5f, z); // Bottom Left
			glEnd();										          // Done Building Triangle Strip

			ps_two_particles[i].xLocation += ps_two_particles[i].xDirection / (resistance * 1000); // Move On The X Axis By X Speed
			ps_two_particles[i].yLocation += ps_two_particles[i].yDirection / (resistance * 1000); // Move On The Y Axis By Y Speed
			ps_two_particles[i].zLocation += ps_two_particles[i].zDirection / (resistance * 1000); // Move On The Z Axis By Z Speed
			ps_two_particles[i].xDirection += ps_two_particles[i].xGravity;			               // Take Pull On X Axis Into Account
			ps_two_particles[i].yDirection += ps_two_particles[i].yGravity;			               // Take Pull On Y Axis Into Account
			ps_two_particles[i].zDirection += ps_two_particles[i].zGravity;			               // Take Pull On Z Axis Into Account
			ps_two_particles[i].life -= ps_two_particles[i].fade;		                           // Reduce Particles Life By 'Fade'

			if (ps_two_particles[i].life < 0.0f)					                               // If Particle Is Burned Out
			{
				ps_two_particles[i].life = 1.0f;					                               // Give It New Life
				ps_two_particles[i].fade = float(rand() % 100) / 1000.0f+0.003f;	               // Random Fade Value
				ps_two_particles[i].xLocation = 23.0f;						                       // Center On X Axis
				ps_two_particles[i].yLocation = 7.0f;						                       // Center On Y Axis
				ps_two_particles[i].zLocation = 0.0f;						                       // Center On Z Axis
				ps_two_particles[i].xDirection = xVelocity + float((rand() % 60) -32.0f);	       // X Axis Speed And Direction
				ps_two_particles[i].yDirection = yVelocity + float((rand() % 60) -30.0f);	       // Y Axis Speed And Direction
				ps_two_particles[i].zDirection = float((rand() % 60) -30.0f);	                   // Z Axis Speed And Direction
				ps_two_particles[i].red = colours[col][0];			                               // Select Red From Color Table
				ps_two_particles[i].green = colours[col][1];			                           // Select Green From Color Table
				ps_two_particles[i].blue = colours[col][2];			                               // Select Blue From Color Table
			}

			if (keys[VK_NUMPAD8] && (ps_two_particles[i].yGravity < 1.5f)) ps_two_particles[i].yGravity += 0.01f;  // If Number Pad 8 And Y Gravity Is Less Than 1.5 Increase Pull Upwards
			if (keys[VK_NUMPAD2] && (ps_two_particles[i].yGravity > -1.5f)) ps_two_particles[i].yGravity -= 0.01f; // If Number Pad 2 And Y Gravity Is Greater Than -1.5 Increase Pull Downwards
			if (keys[VK_NUMPAD6] && (ps_two_particles[i].xGravity < 1.5f)) ps_two_particles[i].xGravity += 0.01f;  // If Number Pad 6 And X Gravity Is Less Than 1.5 Increase Pull Right
			if (keys[VK_NUMPAD4] && (ps_two_particles[i].xGravity > -1.5f)) ps_two_particles[i].xGravity -= 0.01f; // If Number Pad 4 And X Gravity Is Greater Than -1.5 Increase Pull Left

			if (keys['M'])										                        // Tab Key Causes A Burst of particles
			{
				ps_two_particles[i].xDirection = float((rand() % 50) -26.0f) * 10.0f;	// Random Speed On X Axis
				ps_two_particles[i].yDirection = float((rand() % 50) -25.0f) * 10.0f;	// Random Speed On Y Axis
				ps_two_particles[i].zDirection = float((rand() % 50) -25.0f )* 10.0f;	// Random Speed On Z Axis
			}
		}
    }
        
    if(!started) // if game not started
    {
        if(!spaceSokoban.loadlevel()) // if level not loaded
        {
            MessageBox(NULL, "Selected level does not exist", "ERROR", MB_OK | MB_ICONSTOP); // Error message
        }
        started = true; // start the game
    }
    
    glColor3f(0.0f, 1.0f, 0.0f); // text colour
    time += 0.02;                // Increment game time
    
    glDisable(GL_TEXTURE_2D); // Print text
    glTranslatef(-9.0f, -9.0f, -24.0f); glPrint("Level  %2i", spaceSokoban.lvl()); glTranslatef(9.0f, 9.0f, 0.0f);
    glTranslatef(-2.0f, -9.0f, 0.0f);   glPrint("Moves %4i", moves);               glTranslatef(2.0f, 9.0f, 0.0f);
    glTranslatef(5.0f, -9.0f, 0.0f);    glPrint("Pushes %4i", pushes);             glTranslatef(-5.0f, 9.0f, 0.0f);
    glTranslatef(12.0f, -9.0f, 0.0f);   glPrint("Time %6.0f", time);               glTranslatef(-2.0f, 9.0f, 0.0f);   
    
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    
    glTranslatef(-20.0f, 9.0f, 0.0f); // Translate maze position
    glDisable(GL_BLEND);
    
 	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable(GL_ALPHA_TEST);
    spaceSokoban.draw(texture, move_x, move_y, move_z); // Draw maze
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    
    if (spaceSokoban.finished()) // Level completed
    { 
        PlaySound(TEXT("sounds\\complete.wav"), NULL, SND_FILENAME | SND_ASYNC);  
        spaceSokoban.nextlvl(); // Load next level
        
        if (!spaceSokoban.loadlevel())
        {
            MessageBox(NULL, "Selected level does not exist", "ERROR", MB_OK | MB_ICONSTOP); // Error message
        }
        moves = 0;
        pushes = 0;
        time =0;
    }
	return TRUE; // Everything Went OK
}

GLvoid KillGLWindow(GLvoid)	// Properly Kill The Window
{
	if(fullscreen) // Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);	// If So Switch Back To The Desktop
		ShowCursor(TRUE); // Show Mouse Pointer
    }
    
	if(hRC)	// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
        	MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);	// Are We Able To Release The DC And RC Contexts?
        }
        
		if (!wglDeleteContext(hRC))
        {
        	MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION); // Are We Able To Delete The RC?
        }
		hRC = NULL;	// Set RC To NULL
    }
    
	if (hDC && !ReleaseDC(hWnd,hDC)) // Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.","SHUTDOWN ERROR" ,MB_OK | MB_ICONINFORMATION);
		hDC = NULL;	// Set DC To NULL
    }
    
	if (hWnd && !DestroyWindow(hWnd)) // Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL; // Set hWnd To NULL
    }
    
	if (!UnregisterClass("OpenGL", hInstance)) // Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL; // Set hInstance To NULL
    }
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			   // Holds The Results After Searching For A Match
	WNDCLASS	wc;						   // Windows Class Structure
	DWORD		dwExStyle;				   // Window Extended Style
	DWORD		dwStyle;				   // Window Style
	RECT		WindowRect;				   // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long) 0;			   // Set Left Value To 0
	WindowRect.right = (long )width;	   // Set Right Value To Requested Width
	WindowRect.top = (long) 0;			   // Set Top Value To 0
	WindowRect.bottom = (long) height;	   // Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;		   // Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
    }
    	
	if (fullscreen)	// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN)!= DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "OpenGL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen=FALSE; // Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP); // Pop Up A Message Box Letting User Know The Program Is Closing.
				return FALSE; // Return FALSE
			}
		}
	}

	if (fullscreen)	// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW; // Window Extended Style
		dwStyle = WS_POPUP;          // Windows Style
		ShowCursor(FALSE);           // Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;                  // Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right - WindowRect.left,	// Calculate Window Width
								WindowRect.bottom - WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))	// Did We Get A Device Context?
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
    }
    
	if (!(PixelFormat=ChoosePixelFormat(hDC, &pfd))) // Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
    }
    
	if(!SetPixelFormat(hDC, PixelFormat, &pfd))	// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
	}
	
	if (!(hRC=wglCreateContext(hDC))) // Are We Able To Get A Rendering Context?
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
    }
    
	if(!wglMakeCurrent(hDC, hRC)) // Try To Activate The Rendering Context
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
    }
    
	ShowWindow(hWnd, SW_SHOW);    // Show The Window
	SetForegroundWindow(hWnd);    // Slightly Higher Priority
	SetFocus(hWnd);	              // Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height); // Set Up Our Perspective GL Screen
	
	if (!InitGL()) // Initialize Our Newly Created GL Window
	{
		KillGLWindow();	// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE; // Return FALSE
    }
	return TRUE; // Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg) // Check For Windows Messages
	{
		case WM_ACTIVATE: // Watch For Window Activate Message
		{
			if (!HIWORD(wParam)) // Check Minimization State
			{
				active = TRUE; // Program Is Active
			}
			else
			{
				active = FALSE;	// Program Is No Longer Active
			}
			return 0; // Return To The Message Loop
		}

		case WM_SYSCOMMAND:	// Intercept System Commands
		{
			switch (wParam)	// Check System Calls
			{
				case SC_SCREENSAVE:	// Screensaver Trying To Start?
				case SC_MONITORPOWER: // Monitor Trying To Enter Powersave?
				return 0; // Prevent From Happening
			}
			break; // Exit
		}

		case WM_CLOSE: // Did We Receive A Close Message?
		{
			PostQuitMessage(0);	// Send A Quit Message
			return 0; // Jump Back
		}

		case WM_KEYDOWN: // Is A Key Being Held Down?
		{
			keys[wParam] = TRUE; // If So, Mark It As TRUE
			return 0; // Jump Back
		}

		case WM_KEYUP: // Has A Key Been Released?
		{
			keys[wParam] = FALSE; // If So, Mark It As FALSE
			return 0; // Jump Back
		}

		case WM_SIZE: // Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0; // Jump Back
		}
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam); // Pass All Unhandled Messages To DefWindowProc
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg; // Windows Message Structure
	BOOL	done = FALSE; // Bool Variable To Exit Loop

	// Create Our OpenGL Window
     if (!CreateGLWindow("Space Sokoban 3D", ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), 32, fullscreen))
     {
         return 0; // Quit If Window Was Not Created
     } 
            
    // Recreate Our OpenGL Window ( Modified )    
	while(!done) // Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT) // Have We Received A Quit Message?
			{
				done = TRUE; // If So done=TRUE
			}
			else // If Not, Deal With Window Messages
			{                
				TranslateMessage(&msg);	// Translate The Message
				DispatchMessage(&msg);	// Dispatch The Message
			}
		}
		else // If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)	// Program Active?
			{
				if (keys[VK_ESCAPE]) // Was ESC Pressed?
				{
                     { 
                            PlaySound(TEXT("sounds\\gameover.wav"), NULL, SND_FILENAME | SND_ASYNC); // play block sound effect
                            Sleep(4000);   
                            done = TRUE;	// ESC Signalled A Quit
                     }  
				}
				else // Not Time To Quit, Update Screen
				{
					DrawGLScene(); // Draw The Scene
					SwapBuffers(hDC); // Swap Buffers (Double Buffering)
					
					if (keys[VK_ADD] && (resistance > 1.0f))
                    {
                        resistance -= 0.1f;		// Speed Up Particles
                    }
                    
    				if (keys[VK_SUBTRACT] && (resistance < 4.0f))
                    { 
                        resistance += 0.1f;	// Slow Down Particles
                    }
    
    				if (keys[VK_PRIOR])
                    {
                        particleZoom += 0.1f;		// Zoom In particles
                    }
                    
    				if (keys[VK_NEXT])
                    {
                        particleZoom -= 0.1f;		// Zoom Out particles
                    }
    
    				if (keys[VK_RETURN] && !reset)			// Return Key Pressed
    				{
    					reset = TRUE;						// Set Flag Telling Us It's Pressed
    					colour =! colour;				    // Toggle colour Mode On / Off
    				}
    				
    				if (!keys[VK_RETURN]) reset = FALSE;		// If Return Is Released Clear Flag
    				
    				if ((keys[' '] && !cycleColour) || (colour && (delay > 25)))	// Space Or colour Mode
    				{
    					if (keys[' '])
                        {	
                            colour = FALSE;	// If Spacebar Is Pressed Disable colour Mode
                        }
    					
    					cycleColour = TRUE;						// Set Flag Telling Us Space Is Pressed
    					delay = 0;						        // Reset The Color Cycling Delay
    					col++;
                        						                // Change The Particle Color
    					if (col > 11)
                        {	
                            col = 0;				            // If Color Is To High Reset It
                        }
    				}
    				
    				if (!keys[' '])
                    {
                        cycleColour = FALSE; // If Spacebar Is Released Clear Flag
                    }
 
    				if (keys[VK_UP] && (yVelocity < 200)) // If Up Arrow And Y Speed Is Less Than 200 Increase Upward Speed
                    {
                        yVelocity += 1.0f;
                    }
    
    				if (keys[VK_DOWN] && (yVelocity > -200)) // If Down Arrow And Y Speed Is Greater Than -200 Increase Downward Speed
                    {
                        yVelocity -= 1.0f;
                    }
    
    				if (keys[VK_RIGHT] && (xVelocity < 200)) // If Right Arrow And X Speed Is Less Than 200 Increase Speed To The Right
                    {
                        xVelocity += 1.0f;
                    }
    				
    				if (keys[VK_LEFT] && (xVelocity > -200)) // If Left Arrow And X Speed Is Greater Than -200 Increase Speed To The Left
                    {
                        xVelocity -= 1.0f;
                    }
    
    				delay++;							// Increase colour Mode Color Cycling Delay Counter
                    
                    if(keys['R']) 
                    {
                        if(!spaceSokoban.loadlevel())
                        { 
                            MessageBox(NULL, "Selected level does not exist", "ERROR", MB_OK | MB_ICONSTOP); // Error message
                        }
                        
                        moves = 0;
                        pushes = 0;
                        time = 0;
                    }
                        
                    // Camera translation
                    if(keys['D']) { positionX -= 1.5f; }
                    if(keys['A']) { positionX += 1.5f; }
                    if(keys['W']) { positionY -= 1.5f; }
                    if(keys['S']) { positionY += 1.5f; }
                    if(keys['R']) { positionZ += 1.5f; }
                    if(keys['F']) { positionZ -= 1.5f; }
                        
                    // Maze translation
                    if(keys['L']) { move_x += 0.3f; }
                    if(keys['J']) { move_x -= 0.3f; }
                    if(keys['K']) { move_y -= 0.3f; }
                    if(keys['I']) { move_y += 0.3f; }
                    if(keys['Y']) { move_z += 0.3f; }
                    if(keys['H']) { move_z -= 0.3f; }
                        
                    //Camera orientation
                    if(keys['C']) { rotationX -=  1.0f;}
                    if(keys['V']) { rotationX +=  1.0f;}
                    if(keys['Z']) { rotationY -=  1.0f;}
                    if(keys['X']) { rotationY +=  1.0f;}
                        
                    if (keys['O'] && !lightToggle)
    				{
    					lightToggle = TRUE;
    					light = !light;
    					
    					if (!light)
    					{
    						glDisable(GL_LIGHT0); // Enable GL_LIGHT0
    					}
    					else
    					{
    						glEnable(GL_LIGHT0); // Disable GL_LIGHT0
    					}
    				}
    				
    				if (!keys['O'])
    				{
    					lightToggle = FALSE;
    				}
				
                    if (keys['B'] && !fogToggle)
    				{
    					fogToggle = TRUE;
    					fog =! fog;
    					
    					if (!fog)
    					{
    						glEnable(GL_FOG);                   // Enables GL_FOG
                            glFogi (GL_FOG_MODE, GL_LINEAR);   // Fog Mode
    					}
    					else
    					{
    						glDisable(GL_FOG); // Disables GL_FOG
    					}
    				}
    				
    				if (!keys['B'])
    				{
    					fogToggle = FALSE;
    				}
				
                    if(keys[VK_F3] && control > 10)
                    {
                        spaceSokoban.nextlvl(); // Go to next level
                                     
                        if(!spaceSokoban.loadlevel())
                        {
                            MessageBox(NULL, "Selected level does not exist", "ERROR", MB_OK | MB_ICONSTOP); // Error message
                        }
                        
                        moves = 0; // reset variables for next level
                        pushes = 0;
                        time = 0;
                        control = 0;         
                    }
    
                    if(keys[VK_F2] && control > 10)
                    {
                        spaceSokoban.prevlvl();  // Go to previous level
                        
                        if(!spaceSokoban.loadlevel())
                        { 
                            MessageBox(NULL, "Selected level does not exist", "ERROR", MB_OK | MB_ICONSTOP);  // Error message
                        }
                        
                        moves = 0;  // reset variables for next level
                        pushes = 0;
                        time = 0;
                        control = 0;         
                    }
    
                    if(keys[VK_DOWN] && control > 10 && spaceSokoban.block(0, 1) != 1)
                    {
                        keys[VK_DOWN] = FALSE;
                        
                        if(spaceSokoban.block(0, 1) == 2) // if there is a block
                        {
                            if(spaceSokoban.block(0, 2) != 1 && spaceSokoban.block(0, 2) != 2) // if there is no block and no wall
                            {           
                                pushes++; // Increment pushes and moves
                                moves++;            
                                PlaySound(TEXT("sounds\\photon.wav"), NULL, SND_FILENAME | SND_ASYNC); // play block sound effect
                                spaceSokoban.setblock(0, 1, 6); // Set old block position to a tile block
                                spaceSokoban.setblock(0, 2, 2); // Set destination tile to a block
                                spaceSokoban.setxy(0, 1); // Save new player coordinates
                            }
                        }
                        else 
                        {
                            spaceSokoban.setxy(0, 1); // Save new player coordinates
                            moves++; // Increment moves
                        }
                        control = 0;                  
                    }
    
                    if(keys[VK_UP] && control > 10 && spaceSokoban.block(0, -1) != 1)
                    {        
                        keys[VK_UP] = FALSE;
                        
                        if(spaceSokoban.block(0, -1) == 2) // if there is a block
                        {
                            if(spaceSokoban.block(0, -2) != 1 && spaceSokoban.block(0, -2) != 2)  // if there is no block and no wall
                            {
                                pushes++; // Increment pushes and moves
                                moves++;
                                PlaySound(TEXT("sounds\\photon.wav"), NULL, SND_FILENAME | SND_ASYNC); // play block sound effect
                                spaceSokoban.setblock(0, -1, 6); // Set old block position to a tile block
                                spaceSokoban.setblock(0, -2, 2); // Set destination tile to a block
                                spaceSokoban.setxy(0, -1);  // Save new player coordinates
                            }
                        }
                        else 
                        {
                            spaceSokoban.setxy(0, -1); // Save new player coordinates
                            moves++; // Increment moves
                        }
                        control = 0;
                    }
    
                    if(keys[VK_LEFT] && control > 10 && spaceSokoban.block(-1, 0) != 1)
                    {
                        keys[VK_LEFT] = FALSE;
                        
                        if (spaceSokoban.block(-1, 0) == 2) // if there is a block
                        {
                            if (spaceSokoban.block(-2, 0) != 1 && spaceSokoban.block(-2, 0) != 2) // if there is no block and no wall
                            {
                                pushes++;  // Increment pushes and moves
                                moves++;
                                PlaySound(TEXT("sounds\\photon.wav"), NULL, SND_FILENAME | SND_ASYNC); // play block sound effect
                                spaceSokoban.setblock(-1, 0, 6); // Set old block position to a tile block
                                spaceSokoban.setblock(-2, 0, 2); // Set destination tile to a block
                                spaceSokoban.setxy(-1, 0);  // Save new player coordinates
                            }
                        }
                        else 
                        {
                            spaceSokoban.setxy(-1, 0); // Save new player coordinates
                            moves++; // Increment moves
                        }
                        control = 0;
                    }
    
                    if (keys[VK_RIGHT] && control > 10 && spaceSokoban.block(1, 0) != 1)
                    {
                        keys[VK_RIGHT] = FALSE;
                        
                        if (spaceSokoban.block(1, 0) == 2) // if there is a block
                        {
                            if (spaceSokoban.block(2, 0) != 1 && spaceSokoban.block(2, 0) != 2) // if there is no block and no wall
                            {
                                pushes++; // Increment pushes and moves
                                moves++;
                                PlaySound(TEXT("sounds\\photon.wav"), NULL, SND_FILENAME | SND_ASYNC); // play block sound effect
                                spaceSokoban.setblock(1, 0, 6); // Set old block position to a tile block
                                spaceSokoban.setblock(2, 0, 2); // Set destination tile to a block
                                spaceSokoban.setxy(1, 0);  // Set destination tile to a block
                            }
                        }
                        else 
                        {
                            spaceSokoban.setxy(1, 0); // Save new player coordinates
                            moves++; // Increment moves
                        }
                        control = 0;
                    }
    
                    if (control < 11)
                    { 
                        control++;
                    }
				}
			}
        }
    }
    
	KillGLWindow();	// Kill The Window
	return (msg.wParam); // Exit The Program
}
