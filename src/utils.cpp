#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>

//constants
const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;
const unsigned int TICK = 60;
const unsigned int SIM_MULTIPLIER = 2; //how many times the simulation runs per render tick
const unsigned int SPEED = 0.5;

//conversion between SI units and pixels
float meterToPixels (float meters) {
    return meters * 100.0f; //for now, assuming 1 meter equals 100 pixels
}
sf::Vector2f metersToScreenCoords (float xMeters, float yMeters, float windowHeightPx) { //for 2d positions/vectors
    sf::Vector2f pixels;
    //computer pixels have the y-axis inverted, so we need to flip the y value
    pixels.x = meterToPixels(xMeters);
	pixels.y = windowHeightPx - meterToPixels(yMeters);
		//meterToPixels(yMeters) gives the y coord on a normal graph
		//but then subtract from the height bc the y axis is downwards on coordinate scale
	return pixels;
}

//set time step
float setTimeStep() {
	return 1 / TICK / SIM_MULTIPLIER * SPEED;
}
