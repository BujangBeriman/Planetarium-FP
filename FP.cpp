/* sierpinski gasket with vertex arrays */

#include "Angel.h"
#include<GL/glut.h>
#include"ImageLoader.h"
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

  double amb = .2;
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

GLuint earthTexture, sunTexture, backgroundTexture; //The id of the textur
GLUquadric *quad;
GLfloat rotate;

void LookAt()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  perspectiveGL(20.0,(double)glutGet(GLUT_WINDOW_WIDTH)/(double)glutGet(GLUT_WINDOW_HEIGHT),1,5);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
    gluLookAt(0, 2, 1, 0, 0, -1, 0, 1, 0);
}

void backgroundRendering(){
    glDisable(GL_DEPTH);
    quad = gluNewQuadric();
    Image* image = loadBMP("background.bmp");
    backgroundTexture = loadTexture(image);
    delete image;
    glEnable(GL_DEPTH);
}

void earthRendering(){

    quad = gluNewQuadric();
    Image* image = loadBMP("earth.bmp");
    earthTexture = loadTexture(image);
    delete image;
}

void sunRendering(){
    quad = gluNewQuadric();
    Image* image = loadBMP("sun.bmp");
    sunTexture = loadTexture(image);
    delete image;
}

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
//    backgroundRendering();
    sunRendering();
    earthRendering();
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

void displayEarth(){
    glPushMatrix();
    AmbientLighting();
    float x = cos((rotate)*3.14159265/180);
    float z = sin((rotate+180)*3.14159265/180);
    PointLight(x,0,z, 0, 1, 1);
    glRotatef(180,0.0,0.0,1.0);
    glTranslatef(0.0f, 0.0f, -1.0f);
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(rotate,0.0f,0.0f,1.0f);
    glTranslatef(0.6, 0.0, 0.0);
    glRotatef(rotate,0.0,0.0,1.0);
    gluQuadricTexture(quad,1);
    gluSphere(quad,0.01,20,20);
    glPopMatrix();
}

void displaySun(){
    glPushMatrix();
    AmbientLighting();
    PointLight(0,0,0, 1, 0, 0);
    glRotatef(180,0.0,0.0,1.0);
    glTranslatef(0.0f, 0.0f, -1.0f);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(rotate,0.0f,0.0f,1.0f);
    gluQuadricTexture(quad,1);
    gluSphere(quad,0.05,20,20);
    glPopMatrix();
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
//    displayBackground();

    displayEarth();
    displaySun();
//    perspectiveGL(99.0, 1.2,0.6,100);
//    gluLookAt(20, 20, 10, 2, 0, 0, 0, 1, 0);
    glutSwapBuffers();

}

void update(int value){
    rotate+=2.0f;
    if(rotate>360.f)
    {
        rotate-=360;
    }
    glutPostRedisplay();
    glutTimerFunc(25,update,0);
}

void idle (void){
    rotate = 0.001*glutGet(GLUT_ELAPSED_TIME)*180/3.1416;
    //printf("%f\n", rotate);
    if(rotate>360){rotate-=360;}
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
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

void moveClick(int x, int y){
    GLfloat xmove = x*1000/windowWidth;
    xmove = xmove/1000;
    GLfloat ymove = y*1000/windowHeight;
    ymove = ymove/1000;
    printf("%i dan %i serta %i dan %i maka %f dan %f\n", x, y, windowWidth, windowHeight, xmove, ymove);
    mat4 view = Translate((0.0+xmove-0.5)*2, (0.0-ymove+0.5)*2, -0.9);
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
    glutMotionFunc(moveClick);
//    glutFullScreen();

    glewInit();

 //   init();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
//    LookAt();
    glutMainLoop();
    return 0;
}
