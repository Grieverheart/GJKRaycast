#include "shape/cylinder.h"
#include <algorithm>

namespace shape{
    //TODO: Move to separate header file
    template<typename T>
    static inline T sqr(T val){
        return val * val;
    }

    Cylinder::Cylinder(double base_radius, double height):
        base_radius_(base_radius), half_height_(0.5 * height),
        in_radius_(std::min(half_height_, base_radius_)), out_radius_(sqrt(sqr(half_height_) + sqr(base_radius_))),
        volume_(M_PI * sqr(base_radius) * height)
    {}

}
