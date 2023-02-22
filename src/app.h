#pragma once

#include <memory>

namespace flow {

struct application
{
    application(int argc, char** argv);
    void close();
    void render();
};

}