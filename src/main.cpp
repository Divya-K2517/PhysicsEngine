#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include "physics.hpp"


int main() {

	sf::RenderWindow window( sf::VideoMode( { WIDTH, HEIGHT } ), "SFML works!" );
	window.setFramerateLimit(60);

	std::cout << "Window size: " << WIDTH << " x " << HEIGHT << " pixels\n";
    std::cout << "Simulation space: " << WIDTH / 100.0f << " x " << HEIGHT / 100.0f << " meters\n";

	//test ball
	Ball ball (0, 0.5f, 10.0f, sf::Vector2f(3.0f, 4.0f), sf::Color::Red); //id, radius in meters, mass in kg, CoM in meters, color
	ball.describe();



	while ( window.isOpen() )
	{
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}

		std::cout << "pos=(" << ball.CoM.x << ", " << ball.CoM.y << ") "
                  << "vel=(" << ball.velocity.x << ", " << ball.velocity.y << ")\n";

        ball.update();
        ball.reset(); // clear forces for next frame

        window.clear(sf::Color::Black);
        ball.draw(window);
        window.display();
	}
}


//references:
//https://medium.com/@www.seymour/coding-a-2d-physics-engine-from-scratch-and-using-it-to-simulate-a-pendulum-clock-964b4ac2107a
//https://github.com/SFML/SFML
//https://github.com/SFML/cmake-sfml-project/tree/master
