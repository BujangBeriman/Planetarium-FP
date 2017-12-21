/* sierpinski gasket with vertex arrays */

#include "Angel.h"
#include<GL/glut.h>
#include<windows.h>
#include<iostream>
#include<stdlib.h>

using namespace std;

int Index = 0;
bool keyPressedUp = false, keyPressedDown = false, keyPressedLeft = false, keyPressedRight = false, keyPressedPgUp = false, keyPressedPgDwn = false, aboutPopup = false, varputer = false;
GLuint sunTexture, mercuryTexture, venusTexture, earthTexture, marsTexture, jupiterTexture, saturnTexture, saturnRingTexture, uranusTexture, neptuneTexture, backgroundTexture; //The id of the textur
GLUquadric *quad, *sun, *mercury, *venus, *earth, *mars, *jupiter, *saturn, *saturnring, *uranus, *neptune;
GLfloat rotate;
GLfloat mercuryOrbital = 1, venusOrbital = 2, earthOrbital = 3, marsOrbital = 4, jupiterOrbital = 5, saturnOrbital = 6, uranusOrbital = 7, neptuneOrbital = 8;
GLdouble fovy = 38.0, znear =1, zfar =20, l=0, phi = 0, theta = 0;
GLdouble eyex = 0, eyey = 2, eyez = 4, centerx=0, centery=0, centerz=0, upx=0, upy=1, upz=0;
int mouse_state, mouse_button, flagrotate = 0;
GLfloat speedFactor = 1;
int oldX, oldY;

int loadBMP(char *filename){
    FILE * file;
    char temp;
    long i;

    BITMAPINFOHEADER infoheader;
    unsigned char *infoheader_data;

    GLuint num_texture;

    if( (file = fopen(filename, "rb"))==NULL) return (-1); // Open the file for reading

    fseek(file, 18, SEEK_CUR);  /* start reading width & height */
    fread(&infoheader.biWidth, sizeof(int), 1, file);

    fread(&infoheader.biHeight, sizeof(int), 1, file);

    fread(&infoheader.biPlanes, sizeof(short int), 1, file);
    if (infoheader.biPlanes != 1) {
      printf("Planes from %s is not 1: %u\n", filename, infoheader.biPlanes);
      return 0;
    }

    // read the bpp
    fread(&infoheader.biBitCount, sizeof(unsigned short int), 1, file);
    if (infoheader.biBitCount != 24) {
      printf("Bpp from %s is not 24: %d\n", filename, infoheader.biBitCount);
      return 0;
    }

    fseek(file, 24, SEEK_CUR);

    // read the data
    if(infoheader.biWidth<0){
  infoheader.biWidth = -infoheader.biWidth;
    }
    if(infoheader.biHeight<0){
  infoheader.biHeight = -infoheader.biHeight;
    }
    infoheader_data = (unsigned char *) malloc(infoheader.biWidth * infoheader.biHeight * 3);
    if (infoheader_data == NULL) {
      printf("Error allocating memory for color-corrected image data\n");
      return 0;
    }

    if ((i = fread(infoheader_data, infoheader.biWidth * infoheader.biHeight * 3, 1, file)) != 1) {
      printf("Error reading image data from %s.\n", filename);
      return 0;
    }

    for (i=0; i<(infoheader.biWidth * infoheader.biHeight * 3); i+=3) { // reverse all of the colors. (bgr -> rgb)
      temp = infoheader_data[i];
      infoheader_data[i] = infoheader_data[i+2];
      infoheader_data[i+2] = temp;
    }


    fclose(file); // Closes the file stream

    glGenTextures(1, &num_texture);
    glBindTexture(GL_TEXTURE_2D, num_texture); // Bind the ID texture specified by the 2nd parameter

    // The next commands sets the texture parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // If the u,v coordinates overflow the range 0,1 the image is repeated
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // The magnification function ("linear" produces better results)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); //The minifying function

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Finally we define the 2d texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, infoheader.biWidth, infoheader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, infoheader_data);

    // And create 2d mipmaps for the minifying function
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, infoheader.biWidth, infoheader.biHeight, GL_RGB, GL_UNSIGNED_BYTE, infoheader_data);

    free(infoheader_data); // Free the memory we used to load the texture

    return (num_texture); // Returns the current texture OpenGL ID
}
//----------------------------------------------------------------------------
void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar ){
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;

    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

void AmbientLighting(){
  glEnable(GL_LIGHTING);

  float amb = .3;
  GLfloat global_ambient[] = {amb,amb,amb, 0.1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
}

void PointLight(const float x, const float y, const float z, const float amb, const float diff, const float spec){
  glEnable(GL_LIGHTING);

  GLfloat light_ambient[] = { amb,amb,amb, 1.0 };
  GLfloat light_diffuse[] = {diff, diff, diff, 1.0 };
  GLfloat light_specular[] = {spec, spec, spec, 1.0 };

  GLfloat light_position[] = {x,y,z, 0.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_LIGHT0); //enable the light after setting the properties

}

void LookAt(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    perspectiveGL(fovy,(double)glutGet(GLUT_WINDOW_WIDTH)/(double)glutGet(GLUT_WINDOW_HEIGHT),znear,zfar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
}

void backgroundRendering(){
    //glDisable(GL_DEPTH);
    quad = gluNewQuadric();
    backgroundTexture = loadBMP("texture/background.bmp");
    //glEnable(GL_DEPTH);
}

void sunRendering(){
    sun = gluNewQuadric();
    sunTexture = loadBMP("texture/sun.bmp");
}

void mercuryRendering(){
    mercury = gluNewQuadric();
    mercuryTexture = loadBMP("texture/mercury.bmp");

}

void venusRendering(){
    venus = gluNewQuadric();
    venusTexture = loadBMP("texture/venus.bmp");

}

void earthRendering(){

    earth = gluNewQuadric();
    earthTexture = loadBMP("texture/earth.bmp");
}

void marsRendering(){
    mars = gluNewQuadric();
    marsTexture = loadBMP("texture/mars.bmp");

}

void jupiterRendering(){
    jupiter = gluNewQuadric();
    jupiterTexture = loadBMP("texture/jupiter.bmp");

}

void saturnRendering(){
    saturn = gluNewQuadric();
    saturnTexture = loadBMP("texture/saturn.bmp");
    saturnring = gluNewQuadric();
    saturnRingTexture = loadBMP("texture/saturnring.bmp");
}

void uranusRendering(){
    uranus = gluNewQuadric();
    uranusTexture = loadBMP("texture/uranus.bmp");

}

void neptuneRendering(){
    neptune = gluNewQuadric();
    neptuneTexture = loadBMP("texture/neptune.bmp");

}

void renderObject(){
    mercuryRendering();
    venusRendering();
    earthRendering();
    marsRendering();
    jupiterRendering();
    saturnRendering();
    uranusRendering();
    neptuneRendering();
    sunRendering();
}

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    backgroundRendering();
    renderObject();
}

void displaySun(){
    glPushMatrix();
    AmbientLighting();
    PointLight(0,0,0, 1, 0, 0);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(rotate,0.0f,0.0f,1.0f);
    gluQuadricTexture(sun,1);
    gluSphere(sun,0.1,20,20);
    glPopMatrix();
}

void displayMercury(){
    glPushMatrix();
    AmbientLighting();
    GLfloat mercuryOrbitalSpeed = rotate/mercuryOrbital;
    float x = cos((mercuryOrbitalSpeed)*3.14159265/180);
    float z = sin((mercuryOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, mercuryTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(mercuryOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(0.387, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(mercury,1);
    gluSphere(mercury,0.016,20,20);
    glPopMatrix();
}

void displayVenus(){
    glPushMatrix();
    AmbientLighting();
    GLfloat venusOrbitalSpeed = rotate/venusOrbital;
    float x = cos((venusOrbitalSpeed)*3.14159265/180);
    float z = sin((venusOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, venusTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(venusOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(0.723, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(venus,1);
    gluSphere(venus,0.032,20,20);
    glPopMatrix();
}

void displayEarth(){
    glPushMatrix();
    AmbientLighting();
    GLfloat earthOrbitalSpeed = rotate/earthOrbital;
    float x = cos((earthOrbitalSpeed)*3.14159265/180);
    float z = sin((earthOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(earthOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(0.9, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(earth,1);
    gluSphere(earth,0.036,20,20);
    glPopMatrix();
}

void displayMars(){
    glPushMatrix();
    AmbientLighting();
    GLfloat marsOrbitalSpeed = rotate/marsOrbital;
    float x = cos((marsOrbitalSpeed)*3.14159265/180);
    float z = sin((marsOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, marsTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(marsOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(1.1, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(mars,1);
    gluSphere(mars,0.018,20,20);
    glPopMatrix();
}

void displayJupiter(){
    glPushMatrix();
    AmbientLighting();
    GLfloat jupiterOrbitalSpeed = rotate/jupiterOrbital;
    float x = cos((jupiterOrbitalSpeed)*3.14159265/180);
    float z = sin((jupiterOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, jupiterTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(jupiterOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(1.3, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(jupiter,1);
    gluSphere(jupiter,0.06,20,20);
    glPopMatrix();
}

void displaySaturn(){
    glPushMatrix();
    AmbientLighting();
    GLfloat saturnOrbitalSpeed = rotate/saturnOrbital;
    float x = cos((saturnOrbitalSpeed)*3.14159265/180);
    float z = sin((saturnOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, saturnTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(saturnOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(1.7, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(saturn,1);
    gluSphere(saturn,0.05,20,20);
    glDisable(GL_LIGHTING);
    glRotatef(45, 0, 1,0);
    glBindTexture(GL_TEXTURE_2D, saturnRingTexture);
    gluDisk(saturnring, 0.07, 0.09, 100, 1);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void displayUranus(){
    glPushMatrix();
    AmbientLighting();
    GLfloat uranusOrbitalSpeed = rotate/uranusOrbital;
    float x = cos((uranusOrbitalSpeed)*3.14159265/180);
    float z = sin((uranusOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, uranusTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(uranusOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(2.1, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(uranus,1);
    gluSphere(uranus,0.044,20,20);
    glPopMatrix();
}

void displayNeptune(){
    glPushMatrix();
    AmbientLighting();
    GLfloat neptuneOrbitalSpeed = rotate/neptuneOrbital;
    float x = cos((neptuneOrbitalSpeed)*3.14159265/180);
    float z = sin((neptuneOrbitalSpeed+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, neptuneTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(neptuneOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(2.6, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(neptune,1);
    gluSphere(neptune,0.040,20,20);
    glPopMatrix();
}

void displayObject(){
    displaySun();
    displayMercury();
    displayVenus();
    displayEarth();
    displayMars();
    displayJupiter();
    displaySaturn();
    displayUranus();
    displayNeptune();
}

void Draw_Skybox(float x, float y, float z, float width, float height, float length){
	// Center the Skybox around the given x,y,z position
//	glDisable(GL_DEPTH);
    glDisable(GL_LIGHTING);
	x = x - width  / 2;
	y = y - height / 2;
	z = z - length / 2;


	// Draw Front side
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,		z+length);
	glEnd();

	// Draw Back side
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z);
	glEnd();

	// Draw Left side
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height,	z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z);
	glEnd();

	// Draw Right side
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height,	z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height,	z);
	glEnd();

	// Draw Up side
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height,	z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z);
	glEnd();

	// Draw Down side
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y,		z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y,		z);
	glEnd();
    glEnable(GL_LIGHTING);
}

void display( void ){
    glLoadIdentity();
    LookAt();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    Draw_Skybox(0,0,0, 15, 15, 15);
    displayObject();
    glRasterPos2i(centerx + 0, centery + 1);
    const unsigned char* t = reinterpret_cast<const unsigned char *>("Dibuat Oleh :\n1. M. Illham Hanafi    5115100117\n2. Ariya Wildan Depanto    5115100123\n3. Arya Wiranata   5115100163\n4. Dias Adhi P.   5115100173");
    glPushAttrib( GL_CURRENT_BIT );
    glDisable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
    if (aboutPopup) glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);
    glEnable(GL_TEXTURE_2D);
    glPopAttrib();
    l = sqrt(pow(eyex, 2) + pow(eyey,2)+ pow(eyez,2));
    glutSwapBuffers();

}

void idle (void){
    if (flagrotate) rotate += (speedFactor*0.001*180/3.1416);
    if (keyPressedUp) {
        eyey +=0.0005;
        centery+=0.0005;
    }
    else if (keyPressedDown) {
        eyey -=0.0005;
        centery -=0.0005;
    }
    else if (keyPressedLeft) {
        eyex -=0.0005;
        centerx -=0.0005;
    }
    else if (keyPressedRight) {
        eyex +=0.0005;
        centerx+=0.0005;
    }
    else if (keyPressedPgUp) {
        speedFactor += 0.01;
    }
    else if (keyPressedPgDwn) {
        speedFactor -= 0.01;
    }
    //printf("%f\n", rotate);
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y ){
    switch ( key ) {
    case 'q':
            exit(0);
            break;
    case '0':
            eyex = 0;
            eyey = 2;
            eyez = 4;
            centerx=0;
            centery=0;
            centerz=0;
            break;
   case '1':
            eyex = -4;
            eyey = 0;
            eyez = 0;
            centerx=0;
            centery=0;
            centerz=0;
            break;
    case '2':
            eyex = 4;
            eyey = 0;
            eyez = 0;
            centerx=0;
            centery=0;
            centerz=0;
            break;
    case '3':
            eyex = 0;
            eyey = 4;
            eyez = 0.001;
            centerx=0;
            centery=0;
            centerz=0;
            break;
    case '4':
            eyex = 0;
            eyey = -4;
            eyez = 0.001;
            centerx=0;
            centery=0;
            centerz=0;
            break;
    case '5':
            eyex = 0;
            eyey = 0;
            eyez = 4;
            centerx=0;
            centery=0;
            centerz=0;
            break;
    case '6':
            eyex = 0;
            eyey = 0;
            eyez = -4;
            centerx=0;
            centery=0;
            centerz=0;
            break;
    case 'r':
            if (flagrotate == 1) flagrotate = 0;
            else flagrotate = 1;
            break;
    case 'a':
        if (aboutPopup) aboutPopup = false;
            else aboutPopup = true;
            break;
    }
}

void SpecialInput(int key, int x, int y){
    switch(key){
            case GLUT_KEY_UP:
                keyPressedUp = true;
                break;
            case GLUT_KEY_DOWN:
                keyPressedDown = true;
                break;
            case GLUT_KEY_LEFT:
                keyPressedLeft = true;
                break;
            case GLUT_KEY_RIGHT:
                keyPressedRight = true;
                break;
            case GLUT_KEY_PAGE_UP:
                keyPressedPgUp = true;
                break;
            case GLUT_KEY_PAGE_DOWN:
                keyPressedPgDwn = true;
                break;
            }
glutPostRedisplay();
}

void SpecialInputUp(int key, int x, int y){
    switch(key){
            case GLUT_KEY_UP:
                keyPressedUp = false;
                break;
            case GLUT_KEY_DOWN:
                keyPressedDown = false;
                break;
            case GLUT_KEY_LEFT:
                keyPressedLeft = false;
                break;
            case GLUT_KEY_RIGHT:
                keyPressedRight = false;
                break;
            case GLUT_KEY_PAGE_UP:
                keyPressedPgUp = false;
                break;
            case GLUT_KEY_PAGE_DOWN:
                keyPressedPgDwn = false;
                break;
            }
}

void reshape(int x, int y){
    if (y == 0 || x == 0) return;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    perspectiveGL(20.0,(double)glutGet(GLUT_WINDOW_WIDTH)/(double)glutGet(GLUT_WINDOW_HEIGHT),1,5.0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0,0,x,y);  //Use the whole window for rendering
}

GLdouble deltaZ, deltaY, deltaX;

void motion(int x, int y){
/*    int deltaX, deltaY;
    if (mouse_button == GLUT_LEFT_BUTTON && mouse_state == GLUT_DOWN){
        deltaX = x - deltaX;
        deltaY = y - deltaY;
        printf ("%f\n",eyex);
        GLdouble dummyeyex, dummyeyey;
        dummyeyex =sqrt((eyex*eyex) + (eyez*eyez))*sin(deltaX/500);
        eyez = sqrt((eyex*eyex) + (eyez*eyez))*cos(deltaX/500);
        eyex = dummyeyex;
        dummyeyey =sqrt((eyey*eyey) + (eyez*eyez))*sin(deltaY/500);
        eyez = sqrt((eyey*eyey) + (eyez*eyez))*cos(deltaY/500);
        eyey = dummyeyey;
    }
    if (mouse_button == GLUT_LEFT_BUTTON && mouse_state == GLUT_UP){
        deltaX = x;
        deltaY = y;
    }*/
    // this will only be true when the left button is down
      //you might need to adjust this multiplier(0.01)
      x = x-(glutGet(GLUT_WINDOW_WIDTH)/2);
      y = y-(glutGet(GLUT_WINDOW_HEIGHT)/2);
      theta += (x-oldX)*0.001f;
      phi   += (y-oldY)*0.001f;
    eyex = l*cos(phi)*sin(theta);
    eyey = l*sin(phi)*sin(theta);
    eyez = l*cos(theta);
    oldX = x;
    oldY = y;
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){
    mouse_state = state;
    mouse_button = button;
    varputer = false;
    if ((button == 3) || (button == 4))    {
       if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
       if (button == 3 && fovy>0)fovy-=2;
       else fovy+=2;
       //printf("%f\n", fovy);
   }
   if (button == GLUT_LEFT_BUTTON) {
            oldX = x-(glutGet(GLUT_WINDOW_WIDTH)/2);
            oldY = y-(glutGet(GLUT_WINDOW_HEIGHT)/2);
            varputer = true;
		}

}

enum MENU_TYPE{
    VIEW_UTAMA,
    VIEW_DEPAN,
    VIEW_BELAKANG,
    VIEW_ATAS,
    VIEW_BAWAH,
    VIEW_KIRI,
    VIEW_KANAN,
    TRANSLATE_ATAS,
    TRANSLATE_BAWAH,
    TRANSLATE_KIRI,
    TRANSLATE_KANAN,
    ZOOM_IN,
    ZOOM_OUT,
    SPEED_INCREASE,
    SPEED_DECREASE,
    QUIT,
    ABOUT,
    TOGGLE_ROTATE,
};

void mainMenu(int item){
    switch (item){
        case QUIT:
            keyboard('q', 0, 0);
            break;
        case TOGGLE_ROTATE:
            keyboard('r', 0, 0);
            break;
        case ABOUT:
            keyboard('a', 0, 0);
            break;
    }
}

void viewSubMenu(int item){
    switch (item){
        case VIEW_UTAMA:
            keyboard('0', 0, 0);
            break;
        case VIEW_DEPAN:
            keyboard('1', 0, 0);
            break;
        case VIEW_BELAKANG:
            keyboard('2', 0, 0);
            break;
        case VIEW_ATAS:
            keyboard('3', 0, 0);
            break;
        case VIEW_BAWAH:
            keyboard('4', 0, 0);
            break;
        case VIEW_KIRI:
            keyboard('5', 0, 0);
            break;
        case VIEW_KANAN:
            keyboard('6', 0, 0);
            break;
    }
}

void translateSubMenu(int item){
    switch (item){
        case TRANSLATE_ATAS:
            eyey +=0.5;
            centery+=0.5;
            break;
        case TRANSLATE_BAWAH:
            eyey -=0.5;
            centery-=0.5;
            break;
        case TRANSLATE_KANAN:
            eyex +=0.5;
            centerx+=0.5;
            break;
        case TRANSLATE_KIRI:
            eyex -=0.5;
            centerx-=0.5;
            break;
    }
}

void zoomSubMenu(int item){
    switch (item){
        case ZOOM_IN:
            if (fovy<10)fovy-=0;
            else fovy-=10;
            break;
        case ZOOM_OUT:
            fovy+=10;
            break;
    }
}

void speedSubMenu(int item){
    switch (item){
        case SPEED_INCREASE:
            speedFactor +=1;
            break;
        case SPEED_DECREASE:
            speedFactor -=1;
            break;
    }
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
//    glutInitDisplayMode( GLUT_RGBA ); Bukan Double Buffering
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize( 1000, 500 );
    glutCreateWindow("Planetarium - FP");
    initRendering();
//    glutTimerFunc(25,update,0);
    glutIdleFunc(*idle);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
//    glutFullScreen();

    glewInit();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc(SpecialInput);
    glutSpecialUpFunc(SpecialInputUp);

    int viewMenu =glutCreateMenu(viewSubMenu);
    glutAddMenuEntry("View Utama (0)", VIEW_UTAMA);
    glutAddMenuEntry("View Kiri (1)", VIEW_KIRI);
    glutAddMenuEntry("View Kanan (2)", VIEW_KANAN);
    glutAddMenuEntry("View Atas (3)", VIEW_ATAS);
    glutAddMenuEntry("View Bawah (4)", VIEW_BAWAH);
    glutAddMenuEntry("View Depan (5)", VIEW_DEPAN);
    glutAddMenuEntry("View Belakang (6)", VIEW_BELAKANG);

    int translateMenu = glutCreateMenu(translateSubMenu);
    glutAddMenuEntry("Geser Atas (Up Button)", TRANSLATE_ATAS);
    glutAddMenuEntry("Geser Bawah (Down Button)", TRANSLATE_BAWAH);
    glutAddMenuEntry("Geser Kiri (Left Button)", TRANSLATE_KIRI);
    glutAddMenuEntry("Geser Kanan (Right Button)", TRANSLATE_KANAN);

    int zoomMenu = glutCreateMenu(zoomSubMenu);
    glutAddMenuEntry("Zoom In (Scroll Up)", ZOOM_IN);
    glutAddMenuEntry("Zoom Out (Scroll Down)", ZOOM_OUT);

    int speedMenu = glutCreateMenu(speedSubMenu);
    glutAddMenuEntry("Speed Increase (Page Up)", SPEED_INCREASE);
    glutAddMenuEntry("Speed Decrease (Page Down)", SPEED_DECREASE);

    glutCreateMenu(mainMenu);
    glutAddSubMenu("Ubah View Kamera", viewMenu);
    glutAddSubMenu("Geser Kamera", translateMenu);
    glutAddSubMenu("Zoom Kamera", zoomMenu);
    glutAddSubMenu("Ubah Kecepatan Rotasi", speedMenu);
    glutAddMenuEntry("Toggle Rotasi (R)", TOGGLE_ROTATE);
    glutAddMenuEntry("Tentang Kami (A)", ABOUT);
    glutAddMenuEntry("Quit (Q)", QUIT);

    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
