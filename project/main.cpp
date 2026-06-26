#include <Windows.h>
#include "MyGame.h"
#include <scene/Framework.h>

using namespace MyEngine;
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Framework* game = new MyGame();
    game->Run();
    delete game;
    return 0;
}
