#ifndef CAR_H
#define CAR_H

class car
{
    public:
        car(double v, double a, double p, bool b);
        void set_velocity(double v);
        void set_acceleration(double a);
        void set_position(double p);
        void set_break_light(bool b);
        double get_velocity();
        double get_acceleration();
        double get_position();
        bool get_break_light();


    private:
        //in mph
        double velocity;
        double acceleration;
        // in feet
        double position;
        // 0 is off 1 is on
        bool break_light;
};
#endif // CAR_H
