/* sierpinski gasket with vertex arrays */

#include "Angel.h"
#include<GL/glut.h>
#include<windows.h>
#include<iostream>
#include<stdlib.h>

using namespace std;

GLint timeParam, windowWidth =512, windowHeight=512;
GLint sinAngle, cosAngle, angleDegrees;
GLint transform;
GLfloat xRotated, yRotated, zRotated;
GLdouble radius = 1;
//gluSphere();
int Index = 0;
bool keyPressedUp = false, keyPressedDown = false, keyPressedLeft = false, keyPressedRight = false;


int loadBMP(char *filename)
{
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

  /*
  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  */
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
/*
GLuint loadTexture(Image* image) {
  GLuint textureId;
  glGenTextures(1, &textureId); //Make room for our texture
  glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
  //Map the image to the texture
  glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
         0,                            //0 for now
         GL_RGB,                       //Format OpenGL uses for image
         image->width, image->height,  //Width and height
         0,                            //The border of the image
         GL_RGB, //GL_RGB, because pixels are stored in RGB format
         GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                           //as unsigned numbers
         image->pixels);               //The actual pixel data
  return textureId; //Returns the id of the texture
}
*/
GLuint sunTexture, mercuryTexture, venusTexture, earthTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture, backgroundTexture; //The id of the textur
GLUquadric *quad, *sun, *mercury, *venus, *earth, *mars, *jupiter, *saturn, *uranus, *neptune;
GLfloat rotate;
GLfloat mercuryOrbital = 1, venusOrbital = 2, earthOrbital = 3, marsOrbital = 4, jupiterOrbital = 5, saturnOrbital = 6, uranusOrbital = 7, neptuneOrbital = 8;
GLdouble fovy = 38.0, znear =1, zfar =20;
GLdouble eyex = 0, eyey = 2, eyez = 4, centerx=0, centery=0, centerz=0, upx=0, upy=1, upz=0;
int mouse_state, mouse_button;

void LookAt()
{
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
    backgroundTexture = loadBMP("background.bmp");
    //glEnable(GL_DEPTH);
}

void sunRendering(){
    sun = gluNewQuadric();
    sunTexture = loadBMP("sun.bmp");
}

void mercuryRendering(){
    mercury = gluNewQuadric();
    mercuryTexture = loadBMP("mercury.bmp");

}

void venusRendering(){
    venus = gluNewQuadric();
    venusTexture = loadBMP("venus.bmp");

}

void earthRendering(){

    earth = gluNewQuadric();
    earthTexture = loadBMP("earth.bmp");
}

void marsRendering(){
    mars = gluNewQuadric();
    marsTexture = loadBMP("mars.bmp");

}

void jupiterRendering(){
    jupiter = gluNewQuadric();
    jupiterTexture = loadBMP("jupiter.bmp");

}

void saturnRendering(){
    saturn = gluNewQuadric();
    saturnTexture = loadBMP("saturn.bmp");
}

void uranusRendering(){
    uranus = gluNewQuadric();
    uranusTexture = loadBMP("uranus.bmp");

}

void neptuneRendering(){
    neptune = gluNewQuadric();
    neptuneTexture = loadBMP("neptune.bmp");

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

void displayBackground(){
    glDisable(GL_DEPTH);
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( -(GLdouble)1366/(GLdouble)768, -1.0f,10.f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( -(GLdouble)1366/(GLdouble)768, 1.0f,10.f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( (GLdouble)1366/(GLdouble)768, 1.0f,10.f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( (GLdouble)1366/(GLdouble)768, -1.0f,10.f);
    glEnd();
    glEnable(GL_DEPTH);
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
    gluSphere(sun,0.05,20,20);
    glPopMatrix();
}

void displayMercury(){
    glPushMatrix();
    AmbientLighting();
    GLfloat mercuryOrbitalSpeed = rotate/mercuryOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, mercuryTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(mercuryOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(0.387, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(mercury,1);
    gluSphere(mercury,0.008,20,20);
    glPopMatrix();
}

void displayVenus(){
    glPushMatrix();
    AmbientLighting();
    GLfloat venusOrbitalSpeed = rotate/venusOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, venusTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(venusOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(0.723, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(venus,1);
    gluSphere(venus,0.016,20,20);
    glPopMatrix();
}

void displayEarth(){
    glPushMatrix();
    AmbientLighting();
    GLfloat earthOrbitalSpeed = rotate/earthOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(earthOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(0.9, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(earth,1);
    gluSphere(earth,0.018,20,20);
    glPopMatrix();
}

void displayMars(){
    glPushMatrix();
    AmbientLighting();
    GLfloat marsOrbitalSpeed = rotate/marsOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, marsTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(marsOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(1.1, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(mars,1);
    gluSphere(mars,0.009,20,20);
    glPopMatrix();
}

void displayJupiter(){
    glPushMatrix();
    AmbientLighting();
    GLfloat jupiterOrbitalSpeed = rotate/jupiterOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, jupiterTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(jupiterOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(1.3, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(jupiter,1);
    gluSphere(jupiter,0.03,20,20);
    glPopMatrix();
}

void displaySaturn(){
    glPushMatrix();
    AmbientLighting();
    GLfloat saturnOrbitalSpeed = rotate/saturnOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, saturnTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(saturnOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(1.7, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(saturn,1);
    gluSphere(saturn,0.025,20,20);
    glPopMatrix();
}

void displayUranus(){
    glPushMatrix();
    AmbientLighting();
    GLfloat uranusOrbitalSpeed = rotate/uranusOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, uranusTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(uranusOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(2.1, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(uranus,1);
    gluSphere(uranus,0.022,20,20);
    glPopMatrix();
}

void displayNeptune(){
    glPushMatrix();
    AmbientLighting();
    GLfloat neptuneOrbitalSpeed = rotate/neptuneOrbital;
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glBindTexture(GL_TEXTURE_2D, neptuneTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(neptuneOrbitalSpeed,0.0f,0.0f,1.0f);
    glTranslatef(2.6, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(neptune,1);
    gluSphere(neptune,0.020,20,20);
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

void Draw_Skybox(float x, float y, float z, float width, float height, float length)
{
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

void
display( void )
{
    glLoadIdentity();
    LookAt();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    Draw_Skybox(0,0,0, 15, 15, 15);
    //    displayBackground();
    //displayMercury();
    //displayVenus();


    displayObject();
//    perspectiveGL(99.0, 1.2,0.6,100);
//gluLookAt(20, 20, 10, 2, 0, 0, 0, 1, 0);
    glutSwapBuffers();

}

void update(int value){
    rotate+=2.0f;
    glutPostRedisplay();
    glutTimerFunc(25,update,0);
}

void idle (void){
    rotate = 0.001*glutGet(GLUT_ELAPSED_TIME)*180/3.1416;
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
    if (keyPressedRight) {
        eyex +=0.0005;
        centerx+=0.0005;
    }
    //printf("%f\n", rotate);
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 'q':
            exit(0);
            break;
    case '0':
            eyex = 0;
            eyey = 1;
            eyez = 2;
            centerx=0;
            centery=0;
            centerz=-1;
            upx=0;
            upy=1;
            upz=0;
            break;
    case '1':
            eyex = -2.5;
            eyez = 1;
            eyez = 2;
            break;
    case '2':
            exit(0);
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

void motion(int x, int y){
    int deltaX, deltaY;
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
    }
}

void mouse(int button, int state, int x, int y){
    mouse_state = state;
    mouse_button = button;
    if ((button == 3) || (button == 4))    {
       if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
       if (button == 3 && fovy>0)fovy-=2;
       else fovy+=2;
       printf("%f\n", fovy);
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

 //   init();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc(SpecialInput);
    glutSpecialUpFunc(SpecialInputUp);
//    LookAt();
    glutMainLoop();
    return 0;
}
