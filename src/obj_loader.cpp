#include "obj_loader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace{
    using uchar = unsigned char;

    inline bool isSeparator( char token ){
        return (token == ' ' ||
                token == '\n' ||
                token == '\f' ||
                token == '\r' ||
                token == '\t' );
    }

    //NOTE: Not safe, can go beyond end
    inline const char* getNextWord(const char* text, char* buff){
        const char* temp = text;
        while(!isSeparator(*temp)){
            *buff = *temp;
            ++temp; ++buff;
        }
        *buff = '\0';
        return temp + 1;
    }

    //NOTE: Not safe, can go beyond end
    inline uchar findCharAndTerminate(char* text, char endchar){
        uchar i = 0;
        while(text[i] != endchar && text[i] != '\0') ++i;
        text[i] = '\0';
        return i + 1;
    }
}

bool load_obj(const char* filepath, std::vector<double>& vertices, std::vector<std::vector<unsigned int>>& faces){
    FILE* file;
    char line[128];

    file = fopen(filepath, "r");
    if(!file) return false;

    while(fgets(line, sizeof(line), file) != NULL){
        switch(line[0]){
        case 'v':
            switch(line[1]){
            case ' ':{
                const char* next = line + 2;
                int i;
                for(i = 0; *next != '\0'; ++i){
                    char buff[32];
                    next = getNextWord(next, buff);
                    vertices.push_back(atof(buff));
                }
                assert(i == 3);
                break;
            }
            default:
                break;
            }
            break;
        case 'f':{
            unsigned int f;
            const char* next = line + 2;
            std::vector<unsigned int> face;
            for(; *next != '\0';){
                char buff[32];
                next = getNextWord(next, buff);
                findCharAndTerminate(buff, '/');
                f = (unsigned int)atoi(buff);
                face.push_back(--f);
            }
            faces.push_back(face);
            break;
        }
        default:
            break;
        }
    }

    return true;
}
