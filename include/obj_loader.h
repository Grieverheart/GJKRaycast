#ifndef EDMD_OBJ_LOADER_H
#define EDMD_OBJ_LOADER_H

#include <vector>

bool load_obj(const char* filepath, std::vector<double>& vertices, std::vector<std::vector<unsigned int>>& faces);

#endif
