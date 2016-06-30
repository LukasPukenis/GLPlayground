#include <iostream>
#include "Game.h"

int main()
{
	Game game;
	game.setup("map/maps/q3dm2.bsp", 800, 600);
	game.run();

	return 0;
}