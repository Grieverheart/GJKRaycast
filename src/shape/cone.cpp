#include "shape/cone.h"

namespace shape{
    //TODO: Move to separate header file
    template<typename T>
    static inline T sqr(T val){
        return val * val;
    }

    Cone::Cone(double base_radius, double height):
        base_radius_(base_radius), half_height_(0.5 * height),
        sintheta_(sin(base_radius_ / sqrt(sqr(base_radius) + sqr(height)))),
        in_radius_(sintheta_ * half_height_), out_radius_(sqrt(sqr(base_radius_) + sqr(half_height_))),
        volume_(M_PI * sqr(base_radius) * height / 3.0)
    {}

}

