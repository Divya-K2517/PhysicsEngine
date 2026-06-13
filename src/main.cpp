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

	//this is a fixed point
	Ball anchor (0, 0.05f, 1.0f, sf::Vector2f(6.4f, 6.0f), sf::Color::White,
	             0.0f, /*hasGravity=*/false, /*canCollide=*/false, /*checkCollisions=*/false, /*fixed=*/true);

	// first pendulum bob
	Ball bob1 (1, 0.2f, 1.0f, sf::Vector2f(8.4f, 6.0f), sf::Color::Red,
	           0.0f, true, false, false, false);
	// second pendulum bob
	Ball bob2 (2, 0.2f, 1.0f, sf::Vector2f(10.4f, 6.0f), sf::Color::Blue,
	           0.0f, true, false, false, false);

	balls.push_back(&anchor);
	balls.push_back(&bob1);
	balls.push_back(&bob2);

	for (Ball* b : balls) b->allBalls = &balls; // canCollide=false on all, so this is harmless

	//stiff rods
	Spring rod1(3, &anchor, &bob1, 5000.0f, sf::Color::White, 2.0f); // rest length = initial distance
	Spring rod2(4, &bob1, &bob2, 5000.0f, sf::Color::White, 2.0f);

	Ball* draggedBall = nullptr; 

	while ( window.isOpen() )
	{
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();

			//checking if user is dragging a ball 
			if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mousePressed->button == sf::Mouse::Button::Left) {
					sf::Vector2f clickPx(static_cast<float>(mousePressed->position.x), static_cast<float>(mousePressed->position.y));
					sf::Vector2f clickMeters(clickPx.x / 100.0f, (HEIGHT - clickPx.y) / 100.0f);

					for (Ball* b : {&bob1, &bob2}) {
						if (b->tryStartDrag(clickMeters)) { //initiate dragging
							draggedBall = b;
							break;
						}
					}
				}
			}
			//when dragging stops
			if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
				if (mouseReleased->button == sf::Mouse::Button::Left && draggedBall) {
					draggedBall->endDrag();
					draggedBall = nullptr;
				}
			}
			
		}

		if (draggedBall) {
			sf::Vector2i mousePx = sf::Mouse::getPosition(window);
			sf::Vector2f mouseMeters(static_cast<float>(mousePx.x) / 100.0f, (HEIGHT - static_cast<float>(mousePx.y)) / 100.0f);
			draggedBall->updateDrag(mouseMeters);
		}

		rod1.getForces();
		rod2.getForces();

        bob1.update();
        bob2.update();
        bob1.reset();
        bob2.reset();

        window.clear(sf::Color::Black);
		rod1.draw(window);
		rod2.draw(window);
        anchor.draw(window);
        bob1.draw(window);
        bob2.draw(window);
        window.display();
	}
}


//references:
//https://medium.com/@www.seymour/coding-a-2d-physics-engine-from-scratch-and-using-it-to-simulate-a-pendulum-clock-964b4ac2107a
//https://github.com/SFML/SFML
//https://github.com/SFML/cmake-sfml-project/tree/master
