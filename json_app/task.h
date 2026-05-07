#pragma once
#include <string>

struct Task {
    int         id         = 0;
    std::string title;
    std::string tag;
    bool        done       = false;
    std::string created_at;
};
