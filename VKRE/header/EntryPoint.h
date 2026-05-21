#pragma once

#include <Application.h>

int main() {
    auto app = VKRE::CreateApplication();
    std::println("Here");
    app->Run();
    delete app;

    return 0;
}
