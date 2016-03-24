#ifndef EDMD_SHAPE_POINT_H
#define EDMD_SHAPE_POINT_H

#include "convex.h"

namespace shape{

    class Point: public Convex{
    public:
        clam::Vec3d support(const clam::Vec3d& dir)const{
            return clam::Vec3d(0.0);
        }

        double volume(void)const{
            return 0.0;
        }

    };
}

#endif
