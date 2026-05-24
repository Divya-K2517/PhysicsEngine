#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include "physics.hpp"


int main() {

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
