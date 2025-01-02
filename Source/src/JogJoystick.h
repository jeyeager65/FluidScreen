#ifndef JOG_JOYSTICK_H
#define JOG_JOYSTICK_H

#include <Arduino.h>

struct Position { int x, y; };
struct Jog { int x, y, feedrate, timeMs; };

class JogJoystick
{
    private:
        byte xPin, yPin;
        Position positionCenter, positionRaw, position;
        const int readMinValue = 0;
        const int readMaxValue = 4095;
        const int deadzone = 10;
        unsigned long lastReadTime;
        unsigned long readDelay;
    public:
        JogJoystick(byte xPin, byte yPin);
        void init();
        Position getPosition();
        void update();
        void loop();
};
#endif