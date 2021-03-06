#include <cstdio>
#include <cstdint>
//#include "obj_loader.h"

#define NTCD_IMPLEMENTATION
#define NTCD_STATIC
#include "ntcd.h"

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

class PinholeCamera{
public:
    PinholeCamera(const double* position, const double* look_at, double fov, double aspect, double z_near, int width, int height, int n_samples):
        n_samples_(n_samples),
        z_near_(z_near),
        half_width_(width / 2), half_height_(height / 2),
        size_x_(tan((M_PI * fov * 0.5) / 180.0) * z_near_ / half_width_),
        size_y_(aspect * tan((M_PI * fov * 0.5) / 180.0) * z_near_ / half_height_)
    {
        ntcd__vec3_sub(dir_, look_at, position);
        double ilength = 1.0 / ntcd__vec3_length(dir_);
        ntcd__vec3_smul(dir_, ilength, dir_);

        memcpy(pos_, position, 3 * sizeof(double));

        up_vector_[0] = 0.0;
        up_vector_[1] = 1.0;
        up_vector_[0] = 0.0;

        ntcd__vec3_cross(right_vector_, up_vector_, dir_);
        ilength = -1.0 / ntcd__vec3_length(right_vector_);
        ntcd__vec3_smul(right_vector_, ilength, right_vector_);

        ntcd__vec3_cross(up_vector_, right_vector_, dir_);
        ilength = 1.0 / ntcd__vec3_length(up_vector_);
        ntcd__vec3_smul(up_vector_, ilength, up_vector_);
    }

    void shoot_ray(double* ray_dir, double* ray_pos, int x, int y, int s)const{
        int sx = s % n_samples_;
        int sy = s / n_samples_;

        ntcd__vec3_smul(ray_dir, (y - half_height_ + (double)sy / n_samples_) * size_y_, up_vector_);
        ntcd__vec3_fmadd(ray_dir, (x - half_width_ + (double)sx / n_samples_) * size_x_, right_vector_, ray_dir);
        ntcd__vec3_fmadd(ray_dir, z_near_, dir_, ray_dir);

        ntcd__vec3_smul(ray_dir, 1.0 / ntcd__vec3_length(ray_dir), ray_dir);
        memcpy(ray_pos, pos_, 3 * sizeof(double));
    }

private:
    double pos_[3];
    double dir_[3];
    int n_samples_;

    double z_near_;
    int half_width_, half_height_;
    double size_x_, size_y_;
    double up_vector_[3], right_vector_[3];
};

void quat_from_axis_angle(double* quat, double angle, const double* axis){
    double sina = sin(0.5 * angle);
    quat[0] = sina * axis[0];
    quat[1] = sina * axis[1];
    quat[2] = sina * axis[2];
    quat[3] = cos(0.5 * angle);
}

int main(int argc, char *argv[]){

    int width = 600;
    int height = 600;
    int n_pixels = width * height;

    //double screen_width = 12.0;
    //double screen_height = 12.0;

    double sun_dir[] = {0.2, -1.0, -1.5};
    {
        double length = ntcd__vec3_length(sun_dir);
        for(int i = 0; i < 3; ++i) sun_dir[i] /= length;
    }

    ntcd_disk disk;

    ntcd_disk_initialize(&disk);

    ntcd_hull hull;
    {
        auto xform_a = ntcd_transform{
            {-0.5, 0.0, 0.0},
            {0.0, 0.0, 0.0, 1.0},
            1.0
        };

        auto xform_b = ntcd_transform{
            {0.5, 0.0, 0.0},
            {0.0, 0.0, 0.0, 1.0},
            1.0
        };
        double axis[] = {1.0, 0.0, 0.0};
        quat_from_axis_angle(xform_b.rot, 0.5 * M_PI, axis);

        ntcd_hull_initialize(&hull, &xform_a, &disk, &xform_b, &disk);
    }

    ntcd_point point;
    ntcd_point_initialize(&point);

    //ntcd_mesh octahedron;
    //{
    //    std::vector<double> vertices;
    //    std::vector<std::vector<unsigned int>> faces;
    //    load_obj("obj/octahedron.obj", vertices, faces);
    //    std::vector<unsigned int> faces_;
    //    std::vector<unsigned int> face_start_;
    //    unsigned int i = 0;
    //    for(auto face: faces){
    //        face_start_.push_back(i);
    //        for(auto vid: face){
    //            faces_.push_back(vid);
    //            ++i;
    //        }
    //    }
    //    ntcd_mesh_initialize(&octahedron, vertices.size(), vertices.data(), faces.size(), face_start_.data(), faces_.data());
    //}

    auto xform_a = ntcd_transform{
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0},
        1.0
    };

    auto xform_b = ntcd_transform{
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0},
        1.0
    };

    double look_at[]  = {0.0, 0.0, 0.0};
    double position[] = {0.0, 0.0, 5.0};
    auto camera = PinholeCamera(position, look_at, 60.0, double(width) / height, 1.0, width, height, 1);

    for(int i = 0; i < 40; ++i){
        double axis[] = {1.0, 1.0, 0.0};
        ntcd__vec3_smul(axis, 1.0 / ntcd__vec3_length(axis), axis);
        quat_from_axis_angle(xform_b.rot, 2.0 * (i / 40.0) * M_PI, axis);

        u32* screen = new u32[n_pixels];
        for(int p = 0; p < n_pixels; ++p) screen[p] = rgba(255, 255, 255, 0);

        for(int h = 0; h < height; ++h){
            for(int w = 0; w < width; ++w){
                double ray_dir[3];
                camera.shoot_ray(ray_dir, position, w, h, 0);
                memcpy(xform_a.pos, position, 3 * sizeof(double));

                double normal[3];
                double distance = 1000000.0;
                int ray_hit = ntcd_gjk_raycast(&distance, normal, &xform_a, &point, &xform_b, &hull, ray_dir);

                if(ray_hit){
                    double reflected[3];
                    ntcd__vec3_fmadd(reflected, -2.0 * ntcd__vec3_dot(sun_dir, normal), normal, sun_dir);
                    double spec = ntcd__vec3_dot(reflected, ray_dir);
                    double factor = 255.0 * fmin(1.0, fmax(ntcd__vec3_dot(sun_dir, normal), 0.0) + 4.0 * (ntcd__vec3_dot(sun_dir, normal) > 0.0) * pow(spec, 64.0));
                    screen[h * width + w] = rgba(factor, factor, factor, 255);
                }
            }
        }

        char filepath[128];
        sprintf(filepath, "temp/temp%03d.bmp", i);
        save_bmp(screen, width, height, filepath);

        delete[] screen;
    }

    return 0;
}
