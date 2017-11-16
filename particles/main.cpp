#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include "freeglut.h"
#endif

#include <stdio.h>
#include <math.h>

#include "cPlane.hpp"
#include "Camera.hpp"
#include "misc.h"
#include "cParticle.hpp"

int mouseCoords[2], mouseMotionType = 0;
Camera sceneCam, lightCam;
Camera *draggedCamera, *currentCamera;
long frames, time_,timebase=0;
unsigned long frameCount;
float fps = 0.0;
char bufferFPS[11];

void displayText(int x, int y, char *txt);
cPlane *plane;

#define MAX_PARTICLES 500
cParticle *particles;
int particleCounter=0;

void display(void)
{
    int x;
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    sceneCam.setView();
    
    float colorMat[] = {228/255.0, 35/255.0 , 0/255.0};
    plane->display(64, colorMat);
    
    
    float g[]={0,-9.81,0.0};
    for (x = 0;x<MAX_PARTICLES;x++)
    {
        if (particles[x].alive)
        {
            particles[x].addForce(g);
        }
        particles[x].stepSimulation(1);
        particles[x].checkFloorCollisions();
        particles[x].draw();
    }
    
    displayText(5,20,bufferFPS);
    
    glutSwapBuffers ();
}

//Know if two particles are in collision to color them
void collisions()
{
    for (int x = 0; x < MAX_PARTICLES; x++) {
        bool crash = false;
        for (int y = 0; y < MAX_PARTICLES; y++) {
            if (x != y) {
                if (particles[x].inCollision(&particles[y])) {
                    crash = true;
                    break;
                }
            }
        }
        if (crash) {
            particles[x].diffuse[0] = 1;
            particles[x].diffuse[1] = 0;
            particles[x].diffuse[2] = 0;
        }else{
            particles[x].diffuse[0] = particles[x].oDiffuse[0];
            particles[x].diffuse[1] = particles[x].oDiffuse[1];
            particles[x].diffuse[2] = particles[x].oDiffuse[2];
        }
    }
}

void idle (void)
{
    frames++;
    time_=glutGet(GLUT_ELAPSED_TIME);
    if (time_ - timebase > 1000) {
        fps = frames*1000.0f/(time_-timebase);
        sprintf (bufferFPS,"FPS:%4.2f\n",fps);
        //printf ("FPS:%4.2f \n",fps);
        timebase = time_;
        frames = 0;
    }
    collisions();
    glutPostRedisplay ();
}

void init (void)
{
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_DITHER);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
    
    sceneCam.setPos(-46.02, 56.9, -88);
    sceneCam.setDirVec(0.62, -0.15, 0.77);
    sceneCam.setPivot(0,0,0);
    sceneCam.fov = 45;
    sceneCam.near_plane = 1;
    sceneCam.far_plane = 4096;
    
    lightCam.setPos(56,86,-0.5);
    lightCam.setDirVec(0.56, -0.4, 0.73);
    lightCam.setPivot(0,0,0);
    lightCam.fov = 45;
    lightCam.near_plane = 0.1;
    lightCam.far_plane = 2048;
    
    currentCamera = &sceneCam;
    
    glClearColor (0.0, 0.0, 0.3, 0.0);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Especificar color y posicion de las Luces
    GLfloat diffusel0[ ] = {1.0, 1.0, 1.0, 1.0};
    GLfloat ambientl0[ ] = {0.3, 0.3, 0.3, 1.0};
    GLfloat specularl0[ ] = {1.0, 1.0, 1.0, 1.0};
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientl0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffusel0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularl0);
    
    GLfloat position[ ] = {100.0, 200.0, -100.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    plane = new cPlane(2048,2048);
    
    
    
    particles = new cParticle[MAX_PARTICLES];
}

void mouse( int button, int state, int x, int y)
{
    int mods;
    mouseCoords[0] = x;
    mouseCoords[1] = y;
    
    if(state == GLUT_DOWN){
        if(button == GLUT_LEFT_BUTTON){
            draggedCamera = &sceneCam;
        } else if(button == GLUT_RIGHT_BUTTON){
            draggedCamera = &lightCam;
        } else {
            draggedCamera = 0;
        }
        mods = glutGetModifiers();
        if(mods & GLUT_ACTIVE_SHIFT){
            mouseMotionType = 2;
        } else if(mods & GLUT_ACTIVE_ALT){
            mouseMotionType = 3;
        } else if(mods & GLUT_ACTIVE_CTRL){
            mouseMotionType = 4;
        } else {
            mouseMotionType = 1;
        }
    } else {
        if(button == GLUT_LEFT_BUTTON)
            mouseMotionType = 0;
    }
}

void motion(int x, int y)
{
    if(!draggedCamera) return;
    if(mouseMotionType == 4) {
        //Type 4: Zoom
        draggedCamera->moveForward((mouseCoords[1] - y) * 0.05); // zTrans += (mouseCoords[1] - y) * 0.02;
        draggedCamera->rotate((mouseCoords[0] - x) * 0.2, 0, 1, 0);
        mouseCoords[0] = x;
        mouseCoords[1] = y;
    } else if(mouseMotionType == 2) {
        // Type 2: Translation on X / Y
        draggedCamera->rotate((mouseCoords[1] - y) * 0.1, 1, 0, 0); // xTrans += (x - mouseCoords[0]) * 0.02;
        draggedCamera->rotate((x - mouseCoords[0]) * 0.1, 0, 0, 1); // yTrans += (mouseCoords[1] - y) * 0.02;
        mouseCoords[0] = x;
        mouseCoords[1] = y;
    } else if(mouseMotionType == 3) {
        // Type 3: Tilt
        draggedCamera->moveUp((mouseCoords[1] - y) * 0.05);   // yTrans += (y - mouseCoords[1]) * 0.02;
        draggedCamera->moveLeft((mouseCoords[0] - x) * 0.05); // xTrans += (mouseCoords[0] - x) * 0.02;
        
        mouseCoords[0] = x;
        mouseCoords[1] = y;
    } else if(mouseMotionType == 1) {
        // Type 1: Rotate scene
        draggedCamera->moveAround((mouseCoords[1] - y) * 0.1, 1, 0, 0);   // yTrans += (y - mouseCoords[1]) * 0.02;
        draggedCamera->moveAround((mouseCoords[0] - x) * 0.1, 0, 1, 0); // xTrans += (mouseCoords[0] - x) * 0.02;
        
        mouseCoords[0] = x;
        mouseCoords[1] = y;
    }
}

void keys (unsigned char key, int x, int y)
{
    float force[] = {currentCamera->dir[0]*10,currentCamera->dir[1]*40,currentCamera->dir[2]*20};
    switch (key)
    {
        case 27:
            delete plane;
            delete [] particles;
            exit(1);
            break;
        case 'c':
            if(currentCamera == &sceneCam)
                currentCamera = &lightCam;
            else
                currentCamera = &sceneCam;
            break;
        case 'p':case 'P':
            particles[particleCounter%MAX_PARTICLES].addForce(force);
            particles[particleCounter%MAX_PARTICLES].alive = true;
            particleCounter++;
            
            break;
        case 's':case 'S':
            break;
    }
}

void reshape (int w, int h)
{
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/h, 0.01f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    
    currentCamera->setView();
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (800, 600);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("hello");
    glutReshapeFunc (reshape);
    init ();
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keys);
    glutDisplayFunc(display);
    glutIdleFunc (idle);
    glutMainLoop();
    return 0;   /* ANSI C requires main to return int. */
}

void displayText(int x, int y, char *txt) {
    GLboolean lighting;
    GLint viewportCoords[4];
    glColor3f(1.0,0.0,0.0);
    glGetBooleanv(GL_LIGHTING, &lighting);
    glGetIntegerv(GL_VIEWPORT, viewportCoords);
    if(lighting) glDisable( GL_LIGHTING );
    
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0.0, viewportCoords[2], 0.0, viewportCoords[3]);
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    
    glRasterPos2i( x, viewportCoords[3]-y );
    
    
    while (*txt) {glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, *txt ); txt++;}
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    
    if(lighting) glEnable( GL_LIGHTING );
}

