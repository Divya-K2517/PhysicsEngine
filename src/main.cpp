#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include "physics.hpp"


int main() {

	sf::RenderWindow window( sf::VideoMode( { WIDTH, HEIGHT } ), "SFML works!" );
	window.setFramerateLimit(60);

	 std::vector<Ball*> balls;

	std::cout << "Window size: " << WIDTH << " x " << HEIGHT << " pixels\n";
    std::cout << "Simulation space: " << WIDTH / 100.0f << " x " << HEIGHT / 100.0f << " meters\n";

	//test ball
	Ball ballA (0, 0.5f, 10.0f, sf::Vector2f(2.0f, 4.0f), sf::Color::Red); //id, radius in meters, mass in kg, CoM in meters, color
	ballA.describe();
	Ball ballB (1, 0.5f, 10.0f, sf::Vector2f(10.0f, 4.0f), sf::Color::Blue); //id, radius in meters, mass in kg, CoM in meters, color
	ballB.describe();

	//give velocities so they will collide
	ballA.velocity = sf::Vector2f(5.0f, 0.0f); //1 m/s to the right
	ballB.velocity = sf::Vector2f(-5.0f, 0.0f); //1 m/s to the left

	balls.push_back(&ballA);
	balls.push_back(&ballB);

	for (Ball* b : balls) b->allBalls = &balls;

	//this spring will connect balls A and B
	//balls start 4m apart so the spring will bring them tg
	Spring spring(2, &ballA, &ballB, 20.0f, sf::Color::White, 3.0f);

	while ( window.isOpen() )
	{
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}

		//apply thr forces from spring onto ball
		spring.getForces();

        ballA.update();
        ballB.update();
        ballA.reset(); // clear forces for next frame
        ballB.reset(); // clear forces for next frame

        window.clear(sf::Color::Black);
		spring.draw(window);
        ballA.draw(window);
        ballB.draw(window);
        window.display();
	}
}


//references:
//https://medium.com/@www.seymour/coding-a-2d-physics-engine-from-scratch-and-using-it-to-simulate-a-pendulum-clock-964b4ac2107a
//https://github.com/SFML/SFML
//https://github.com/SFML/cmake-sfml-project/tree/master
