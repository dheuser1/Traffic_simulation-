#include "car.h"

car::car(double v, double a, double p, bool b)
{
    velocity=v;
    acceleration=a;
    position=p;
    break_light=b;
}

double car::get_velocity()
{
    return velocity;
}

double car::get_acceleration()
{
    return acceleration;
}

double car::get_position()
{
    return position;
}

bool car::get_break_light()
{
    return break_light;
}

void car::set_velocity(double v)
{
    velocity=v;
}

void car::set_acceleration(double a)
{
    acceleration=a;
}
void car::set_position(double p)
{
    position=p;
}

void car::set_break_light(bool b)
{
    break_light=b;
}
