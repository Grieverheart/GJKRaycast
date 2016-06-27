#ifndef EDMD_SHAPE_DISK_H
#define EDMD_SHAPE_DISK_H

#include "convex.h"

namespace shape{

    class Disk: public Convex{
    public:
        clam::Vec3d support(const clam::Vec3d& dir)const{
            double length2 = dir[0] * dir[0] + dir[2] * dir[2];
            return (length2 > 0.0)? clam::Vec3d(dir[0], 0.0, dir[2]) / sqrt(length2): clam::Vec3d(0.0);
        }

        double out_radius(void)const{
            return 1.0;
        }

        double volume(void)const{
            return 0.0;
        }

    };
}

#endif
