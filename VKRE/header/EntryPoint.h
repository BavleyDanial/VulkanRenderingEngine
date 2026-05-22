#pragma once

#include <Application.h>

int main() {
    auto app = VKRE::CreateApplication();
    app->Run();
    delete app;

    return 0;
}
