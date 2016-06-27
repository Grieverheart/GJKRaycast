#include "shape/hull.h"
#include "shape/variant.h"

namespace shape{

    Hull::Hull(const std::vector<ShapeDefinition>& shapes, double volume):
        shapes_(shapes),
        out_radius_(0.0), volume_(volume)
    {
        for(auto shape_def: shapes_){
            double outradius = shape_def.xform_.pos_.length() + shape_def.xform_.size_ * shape_outradius(*shape_def.shape_);
            if(outradius > out_radius_) out_radius_ = outradius;
        }
    }

    clam::Vec3d Hull::support(const clam::Vec3d& dir)const{
        clam::Vec3d ret = shapes_[0].xform_.pos_ + shapes_[0].xform_.rot_.rotate(shapes_[0].xform_.size_ * boost::polymorphic_get<Convex>(*shapes_[0].shape_).support(shapes_[0].xform_.rot_.inv().rotate(dir)));
        double max = clam::dot(ret, dir);
        for(size_t i = 1; i < shapes_.size(); ++i){
            auto sup = shapes_[i].xform_.pos_ + shapes_[i].xform_.rot_.rotate(shapes_[i].xform_.size_ * boost::polymorphic_get<Convex>(*shapes_[i].shape_).support(shapes_[i].xform_.rot_.inv().rotate(dir)));
            double dot = clam::dot(sup, dir);
            if(dot > max){
                max = dot;
                ret = sup;
            }
        }

        return ret;
    }

    double Hull::out_radius(void)const{
        return out_radius_;
    }

    double Hull::volume(void)const{
        return volume_;
    }
}
