#ifndef utils_hpp
#define utils_hpp

#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>

//constants
extern const unsigned int WIDTH;
extern const unsigned int HEIGHT;
extern const unsigned int TICK;
extern const unsigned int SIM_MULTIPLIER; //how many times the simulation runs per render tick
extern const float SPEED;
extern const float GRAVITY; //gravitational acceleration in m/s^2
extern const float ELASTICITY; //how bouncy the collisions are, between 0 and 1, where 1 is perfectly elastic and 0 is perfectly inelastic


float meterToPixels (float meters);
sf::Vector2f metersToScreenCoords (float xMeters, float yMeters, float windowHeightPx);
float setTimeStep();


#endif // utils_hpp