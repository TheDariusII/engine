#include <iostream>
#include <UI.h>

#include "Application.h"
#include "GameState.h"

Camera GameState::cam;
MouseState GameState::ms;
//KeyboardState GameState::ks;

int main()
{
	std::cout << "------\n";
	Application* app = new Application((char*)"Dean", 1920, 1080);
	app->Run();
	std::cout << "------\n";
    return 0;
}

