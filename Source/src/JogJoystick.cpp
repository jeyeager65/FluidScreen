#include "JogJoystick.h"

JogJoystick::JogJoystick(byte xPin, byte yPin) 
{
    this->xPin = xPin;
    this->yPin = yPin;
    init();
}

void JogJoystick::init() 
{
    this->lastReadTime = 0;
    this->readDelay = 200;

    // Determine joystick center
    int xTemp[3], yTemp[3];
	
    for(short i=0; i<3; i++) {
        delay(100);
        xTemp[i] = analogRead(this->xPin);
        yTemp[i] = analogRead(this->yPin);
    }

    this->positionCenter.x = (int)((xTemp[0]+xTemp[1]+xTemp[2])/3);
    this->positionCenter.y = (int)((yTemp[0]+yTemp[1]+yTemp[2])/3);	
}

Position JogJoystick::getPosition()
{
    return this->position;
}

void JogJoystick::update() {
    this->positionRaw.x = analogRead(this->xPin);
    this->positionRaw.y = analogRead(this->yPin);

    int xPos, yPos, xPosMax, xPosCurr, yPosMax, yPosCurr;
    int xFix, yFix;

    if(this->positionRaw.x > this->positionCenter.x) {
        xFix = 1;
        xPosMax = 4095 - this->positionCenter.x;
        xPosCurr = this->positionRaw.x - this->positionCenter.x;
        xPos = ((float)xPosCurr/(float)xPosMax) * 1000 * xFix; // Normalize to 1000
    } else {
        xFix = -1;
        xPosMax = this->positionCenter.x;
        xPosCurr = this->positionCenter.x - this->positionRaw.x;
        xPos = ((float)xPosCurr/(float)xPosMax) * 1000 * xFix; // Normalize to 1000
    }

    if(this->positionRaw.y > this->positionCenter.y) {
        yFix = -1;
        yPosMax = 4095 - this->positionCenter.y;
        yPosCurr = this->positionRaw.y - this->positionCenter.y;
        yPos = ((float)yPosCurr/(float)yPosMax) * 1000 * yFix; // Normalize to 1000
    } else {
        yFix = 1;
        yPosMax = this->positionCenter.y;
        yPosCurr = this->positionCenter.y - this->positionRaw.y;
        yPos = ((float)yPosCurr/(float)yPosMax) * 1000 * yFix; // Normalize to 1000
    }

    // Add 2% deadzone
    if(abs(xPos) <= 20) {
        xPos = 0;
    }
    if(abs(yPos) <= 20) {
        yPos = 0;
    }

    // Add curve for more control at low end
    if(xPos != 0) {
        xPos = 0.001 * pow(xPos, 2) * xFix;
    }
    if(yPos != 0) {
        yPos = 0.001 * pow(yPos, 2) * yFix;
    }

    // Normalize to 100
    this->position.x = xPos/10;
    this->position.y = yPos/10;
}

void JogJoystick::loop() {
    if((millis() - lastReadTime) > readDelay) {
        update();
        lastReadTime = millis();
    }
}