#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>

//using SI units
float meterToPixels (float meters) {
    return meters * 100.0f; //for now, assuming 1 meter equals 100 pixels
}
std::array<float, 2> metersToScreenCoords (float xMeters, float yMeters, float windowHeightPx) { //for 2d positions/vectors
    std::array<float, 2> pixels;
    //computer pixels have the y-axis inverted, so we need to flip the y value
    pixels[0] = meterToPixels(xMeters);
	pixels[1] = windowHeightPx - meterToPixels(yMeters);
		//meterToPixels(yMeters) gives the y coord on a normal graph
		//but then subtract from the height bc the y axis is downwards on coordinate scale
	return pixels;
}

int main() {
	const unsigned int WIDTH = 1280;
	const unsigned int HEIGHT = 720;

	sf::RenderWindow window( sf::VideoMode( { WIDTH, HEIGHT } ), "SFML works!" );
	sf::CircleShape shape( 100.f );
	shape.setFillColor( sf::Color::Green );
	window.setFramerateLimit(60);

	std::cout << "Window size: " << WIDTH << " x " << HEIGHT << " pixels\n";
    std::cout << "Simulation space: " << WIDTH / 100.0f << " x " << HEIGHT / 100.0f << " meters\n";

	while ( window.isOpen() )
	{
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}

		window.clear();
		window.draw( shape );
		window.display();
	}
}


//references:
//https://medium.com/@www.seymour/coding-a-2d-physics-engine-from-scratch-and-using-it-to-simulate-a-pendulum-clock-964b4ac2107a
//https://github.com/SFML/SFML
//https://github.com/SFML/cmake-sfml-project/tree/master
