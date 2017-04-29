#include <cstdio>
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

int main(int argc, char *argv[]){

    int width = 1800;
    int height = 1800;
    int n_pixels = width * height;

    double screen_width = 12.0;
    double screen_height = 12.0;

    double sun_dir[] = {0.2, -1.0, -1.5};
    {
        double length = ntcd__vec3_length(sun_dir);
        for(int i = 0; i < 3; ++i) sun_dir[i] /= length;
    }

    u32* screen = new u32[n_pixels];
    for(int p = 0; p < n_pixels; ++p) screen[p] = rgba(255, 255, 255, 0);

    ntcd_line line;
    ntcd_sphere sphere;

    ntcd_line_initialize(&line);
    ntcd_sphere_initialize(&sphere);

    ntcd_minkowski_sum msum;
    {
        auto xform = ntcd_transform{
            {0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0, 1.0},
            3.0
        };

        ntcd_minkowski_sum_initialize(&msum, &xform, &sphere, &line);
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

    double ray_dir[] = {0.0, -0.2, -1.0};

    for(int h = 0; h < height; ++h){
        for(int w = 0; w < width; ++w){

            double pos[] = {(double(w) / width - 0.5) * screen_width, (double(h) / height - 0.5) * screen_height, 5.0};
            memcpy(xform_a.pos, pos, 3 * sizeof(double));
            double normal[3];
            double distance = 1000000.0;
            int ray_hit = ntcd_gjk_raycast(&distance, normal, &xform_a, &point, &xform_b, &msum, ray_dir);

            if(ray_hit){
                double reflected[3];
                ntcd__vec3_fmadd(reflected, -2.0 * ntcd__vec3_dot(sun_dir, normal), normal, sun_dir);
                double spec = ntcd__vec3_dot(reflected, ray_dir);
                double factor = fmax(ntcd__vec3_dot(sun_dir, normal), 0.0) + 4.0 * (ntcd__vec3_dot(sun_dir, normal) > 0.0) * pow(spec, 64.0);
                screen[h * width + w] = rgba(fmin(255.0, factor * 255.0), fmin(255.0, factor * 255.0), fmin(255.0, factor * 255.0), 255);
            }
        }
    }

    save_bmp(screen, width, height, "temp.bmp");

    delete[] screen;

    return 0;
}
