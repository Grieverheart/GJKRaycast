#ifndef FBMC_SHAPE_HULL_H
#define FBMC_SHAPE_HULL_H

#include "variant_fwd.h"
#include "convex.h"
#include "transform.h"
#include "clam.h"
#include <memory>
#include <vector>
#include <boost/variant/polymorphic_get.hpp>

namespace shape{

    struct ShapeDefinition{
        Transform xform_;
        std::shared_ptr<Variant> shape_;
    };

    class Hull: public Convex{
    public:
        Hull(const std::vector<ShapeDefinition>& shapes, double volume = 1.0);

        const std::vector<ShapeDefinition>& shape_definitions(void)const{
            return shapes_;
        }

        clam::Vec3d support(const clam::Vec3d& dir)const;

        double out_radius(void)const;

        double volume(void)const;

    private:
        std::vector<ShapeDefinition> shapes_;
        double out_radius_;
        double volume_;
    };

}

#endif
