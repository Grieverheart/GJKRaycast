#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "shape/shapes.h"
#include "overlap/overlap.h"
#include "overlap/gjk.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

inline u32 rgba(u8 r, u8 g, u8 b, u8 a = 0){
    return u32(b) | (u32(g) << 8) | (u32(r) << 16) | (u32(a) << 24);
}

#pragma pack(1)
struct BMPHeader{
    u8 signature[2] = {'B', 'M'};
    u32 file_size;
    u32 reserved = 0;
    u32 poffset;
};

#pragma pack(1)
struct BMPDIBHeader{
    u32 header_size = 40;
    u32 width;
    u32 height;
    u16 num_color_planes = 1;
    u16 bpp = 32;
    u32 compression = 0; //BI_RGB
    u32 image_size = 0;
    u32 x_ppm = 2835;
    u32 y_ppm = 2835;
    u32 num_colors = 0;
    u32 num_important_colors = 0;
};

inline void save_bmp(const u32* pixels, int width, int height, const char* filename){
    int num_pixels = width * height;
    BMPHeader bmp_header;
    bmp_header.file_size = sizeof(BMPHeader) + sizeof(BMPDIBHeader) + sizeof(u32) * num_pixels;
    bmp_header.poffset = sizeof(BMPHeader) + sizeof(BMPDIBHeader);

    BMPDIBHeader dbi_header;
    dbi_header.width = width;
    dbi_header.height = height;
    dbi_header.image_size = num_pixels * sizeof(u32);

    FILE* fp = fopen(filename, "wb");

    fwrite(&bmp_header, sizeof(BMPHeader), 1, fp);
    fwrite(&dbi_header, sizeof(BMPDIBHeader), 1, fp);
    fwrite(pixels, sizeof(u32), num_pixels, fp);

    fclose(fp);
}

constexpr double tolerance = 1.0e-12;

bool conservative_advancement(const Transform& pa, const shape::Convex& a, const Transform& pb, const shape::Convex& b, const clam::Vec3d& ray_dir, double& distance, clam::Vec3d& normal){

    double distance_ = 0.0;
    double max_advance = 0.0;
    Transform temp_pb = pb;
    Transform temp_pa = pa;

    temp_pb.pos_ = temp_pb.pos_ - temp_pa.pos_;
    temp_pa.pos_ = 0.0;
    clam::Vec3d dir = -ray_dir;

    while(true){
        clam::Vec3d shortest_dist = overlap::gjk_distance(temp_pa, a, temp_pb, b);
        double shortest_distance = shortest_dist.length();

        if(shortest_distance < 2.0 * tolerance){
            int iter = 0;
            while(shortest_distance < tolerance){
                temp_pb.pos_ -= dir * 0.001 * distance_;
                distance_ *= 0.999;
                shortest_dist = overlap::gjk_distance(temp_pa, a, temp_pb, b);
                shortest_distance = shortest_dist.length();
                //NOTE: This should alsmost never happen.
                if(iter++ > 1000){
                    printf("Error!\n");
                    return false;
                }
            }

            normal = -shortest_dist / shortest_distance;
            distance = distance_;
            return true;
        }

        /* Conservative advancement by Mirtich 1996 PhD Thesis */
        double max_vel = clam::dot(shortest_dist, dir);
        if(max_vel <= 0.0) break;
        max_vel /= shortest_distance;

        max_advance = (shortest_distance - tolerance) / max_vel;
        distance_ += max_advance;
        if(distance_ <= 0.0 || distance_ > 100.0) break;
        temp_pb.pos_ += dir * max_advance;
    }

    return false;
}

int main(int argc, char *argv[]){

    int width = 1200;
    int height = 1200;
    int n_pixels = width * height;

    double screen_width = 8.0;
    double screen_height = 8.0;

    clam::Vec3d sun_dir(0.2, -1.0, -0.5);
    sun_dir /= sun_dir.length();

    u32* screen = new u32[n_pixels];
    for(int p = 0; p < n_pixels; ++p) screen[p] = rgba(255, 255, 255, 0);

    auto cone = shape::Cone(1.0, 5.0);
    auto sphere = shape::Cone(1.0, 5.0);

    auto xform_cone = Transform{
        clam::Vec3d(0.0),
        clam::Quatd(0.0, 0.0, 0.0, 1.0),
        1.0,
    };

    auto xform_sphere = Transform{
        clam::Vec3d(0.0),
        clam::Quatd(0.5, 0.5, 0.5, 0.5),
        0.5,
    };

    auto ray_dir = clam::Vec3d(0.0, 0.0, -1.0);

    for(int h = 0; h < height; ++h){
        for(int w = 0; w < width; ++w){
            xform_sphere.pos_ = clam::Vec3d((double(w) / width - 0.5) * screen_width, (double(h) / height - 0.5) * screen_height, 15.0);
            double distance = 1000000.0;
            clam::Vec3d normal;
            //bool ray_hit = overlap::gjk_raycast(xform_sphere, sphere, xform_cone, cone, ray_dir, distance, normal);
            bool ray_hit = conservative_advancement(xform_sphere, sphere, xform_cone, cone, ray_dir, distance, normal);
            if(ray_hit){
                clam::Vec3d reflected = sun_dir - 2.0 * clam::dot(sun_dir, normal) * normal;
                double spec = clam::dot(reflected, ray_dir);
                double factor = std::min(1.0, std::max(clam::dot(sun_dir, normal), 0.0) + 2.0 * (clam::dot(sun_dir, normal) > 0.0) * pow(spec, 32.0));
                screen[h * width + w] = rgba(factor * 255, factor * 120, factor * 50, 255);
            }
        }
    }

    save_bmp(screen, width, height, "temp.bmp");

    return 0;
}
