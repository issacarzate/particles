#include "cParticle.hpp"

cParticle::cParticle()
{
    restitutionCoefficient = 0.8f;
    radius = 10.0f + 5.0f*( (float)rand()/(float)RAND_MAX);
    mass = radius;
    pos[0] = 1000.0f*((float)rand()/(float)RAND_MAX);
    pos[1] = 200.0f + 100.0f*((float)rand()/(float)RAND_MAX);
    pos[2] = 1000.0f*((float)rand()/(float)RAND_MAX);
    
    oldPos[0] = pos[0];
    oldPos[1] = pos[1];
    oldPos[2] = pos[2];
    
    forces[0] = -10 + 20.0f * ((float)rand()/(float)RAND_MAX);;
    forces[1] = 0.0;
    forces[2] = -10 + 20.0f * ((float)rand()/(float)RAND_MAX);
    
    diffuse[0] = 0.8f*((float)rand()/(float)RAND_MAX);
    diffuse[1] = ((float)rand()/(float)RAND_MAX);
    diffuse[2] = ((float)rand()/(float)RAND_MAX);
    
    oDiffuse[0] = diffuse[0];
    oDiffuse[1] = diffuse[1];
    oDiffuse[2] = diffuse[2];
    
    specular[0] = 0.9;
    specular[1] = 0.9;
    specular[2] = 0.9;
    
    shininess=100;
    
    dragForce[0] = 0.0;
    dragForce[1] = 0.0;
    dragForce[2] = 0.0;
    
    alive = true;
    
}

cParticle::cParticle(float m, float *pos0)
{
    mass = m;
    pos[0] = pos0[0];
    pos[1] = pos0[1];
    pos[2] = pos0[2];
    oldPos[0] = pos[0];
    oldPos[1] = pos[1];
    oldPos[2] = pos[2];
    
    dragForce[0] = 0.0;
    dragForce[1] = 0.0;
    dragForce[2] = 0.0;
    
    alive = true;
    
}

cParticle::~cParticle()
{
}

void cParticle::draw()
{
    glPushMatrix();
    {
        glEnable(GL_COLOR_MATERIAL);
        {
            glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
            glMaterialf(GL_FRONT, GL_SHININESS, shininess);
            glColor3fv(diffuse);
            glTranslatef(pos[0], pos[1], pos[2]);
            glutSolidSphere(radius, 20, 20);
        }
        glDisable(GL_COLOR_MATERIAL);
    }
    glPopMatrix();
}

void cParticle::stepSimulation(float dt)
{
    integrateVerlet(dt);
}

void cParticle::clearForces()
{
    forces[0] = 0;
    forces[1] = 0;
    forces[2] = 0;
}

void cParticle::addForce(float *force)
{
    forces[0]+=force[0];
    forces[1]+=force[1];
    forces[2]+=force[2];
    
    dragForce[0]=forces[0];
    dragForce[1]=forces[1];
    dragForce[2]=forces[2];
    
    unitVector(dragForce);
}

void cParticle::integrateVerlet(float dt)
{
    float accel[3], posAux[3];
    
    // Apply 2nd Newton law: f = ma ->  a = f / m
    accel[0] = forces[0]/mass;
    accel[1] = forces[1]/mass;
    accel[2] = forces[2]/mass;
    
    posAux[0] = pos[0];
    posAux[1] = pos[1];
    posAux[2] = pos[2];
    
    pos[0] += pos[0] - oldPos[0] + accel[0]*dt*dt - dragForce[0];
    pos[1] += pos[1] - oldPos[1] + accel[1]*dt*dt - dragForce[1];
    pos[2] += pos[2] - oldPos[2] + accel[2]*dt*dt - dragForce[2];
    
    oldPos[0] = pos[0];
    oldPos[1] = pos[1];
    oldPos[2] = pos[2];
}

//Actual distance and sum of both radius are compared to know if max Distance from both centers before collision (radius1 + radius2) is passed
bool cParticle::inCollision(cParticle* other)
{
    float radius1 = radius;
    float radius2 = other->radius;
    float distance = sqrtf(((other->pos[2]-pos[2])*(other->pos[2]-pos[2]))+((other->pos[1]-pos[1])*(other->pos[1]-pos[1]))+((other->pos[0]-pos[0])*(other->pos[0]-pos[0])));
    
    if ((distance) < (radius1 + radius2)) {
        return true;
    }
    return false;
}

void cParticle::checkFloorCollisions()
{
    if(pos[1] < radius)
    {
        pos[1] = radius;
        forces[1] = -restitutionCoefficient * forces[1];  // rC = 0.8f
        
        float dirX = pos[0] - oldPos[0];
        float dirZ = pos[2] - oldPos[2];
        
        float dX2 = dirX * dirX;
        float dZ2 = dirZ * dirZ;
        float mag = sqrt(dX2 + dZ2);
        
        dirX /= mag;
        dirZ /= mag;
        
        dragForce[0] = dirX;
        dragForce[1] = forces[1];
        dragForce[2] = dirZ;
        unitVector(dragForce);
    }
}

