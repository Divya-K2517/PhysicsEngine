#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include "physics.hpp"
#include <optional>

float crossProduct2DVectors (sf::Vector2f a, sf::Vector2f b) {
    return a.x * b.y - a.y * b.x;
}
//to add vectors
sf::Vector3f operator+(sf::Vector3f a, sf::Vector3f b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
//to multiply vector by scalar
sf::Vector3f operator*(float s, sf::Vector3f v) {
    return {s * v.x, s * v.y, s * v.z};
}
sf::Vector3f operator*(sf::Vector3f v, float s) { return s * v; }

sf::Vector3f rb(sf::Vector3f y, float t, float g, float m, std::vector<Thrust> thrusts, std::vector<float> torques, float I, float k, bool fixed) {
        // y is the state vector, containing the velocity in x and y directions and the angle of the object
        float vx = y.x; //new velocity in x direction
        float vy = y.y; //new velocity in y direction
        float theta = y.z; //angular acceleration

        //operate on the forces and torques
        if (!fixed) {
            float sum = 0.0f; //sum of forces in x direction
            for (Thrust thrust : thrusts) {
                sum += thrust.vector.x;
            }
            //F = ma -> a = F/m, also adding a damping term proportional to velocity to prevent infinite acceleration
            vx = (sum - k * vx) / m; 
            sum = 0.0f; //sum of forces in y direction
            for (Thrust thrust : thrusts) {
                sum += thrust.vector.y;
            }
            vy = (sum - m*g - k * vy)/ m; //subtracting gravity force and damping term
        }
        
        float torqueFromThrusts = 0.0f;
        for (Thrust thrust : thrusts) {
            //do the cross product of the thrust vector and the vector from the CoM to the point where the force is applied
            torqueFromThrusts += crossProduct2DVectors(thrust.point, thrust.vector);
        }
        float sumTorques = 0.0f;
        for (float torque : torques) {
            sumTorques += torque;
        }

        //torque = I * alpha -> alpha = torque / I, also adding a damping term proportional to angular velocity to prevent infinite angular acceleration
        //k is damping coefficient
        theta = (torqueFromThrusts + sumTorques - k*theta) / I;

        return sf::Vector3f(vx, vy, theta);
}

sf::Vector3f resultOfForces(Ball& obj) {
    //this function does rb() 4 times to figure out where the object ends up
    float m = obj.mass;
    float I = obj.MoI;
    float dt = setTimeStep();
    bool fixed = obj.fixed;

    //y0 is the initial state vector
    sf::Vector3f y0 = sf::Vector3f(obj.velocity.x, obj.velocity.y, obj.angularVelocity);
        
    //the result of rb is the new velocity and angular velocity after applying the forces and torques for one time step
    //we will integrate this result of rb ([dvx/dt, dvy/dt, dalpha/dt]) over time to get the new velocity and angle of the object

    float g = obj.hasGravity ? GRAVITY : 0.0f;

    //dydt is a wrapper that takes in y and calls rb() on it 
    //[&] captures all the local variables
    auto dydt = [&](sf::Vector3f y) {
        return rb(y, 0.0f, g, m, obj.thrusts, obj.torques, I, 0.1f, fixed);
    };

    //4 evaluations of derivative
    sf::Vector3f k1 = dydt(y0); //where do we end up if we apply forces for one time step from initial state
    sf::Vector3f k2 = dydt(y0 + (dt/2.0f) * k1); //k1 is used to figure out where we are at the midpoint, then recalculate acceleration there
    sf::Vector3f k3 = dydt(y0 + (dt/2.0f) * k2); //use k2 for an even better midpoint estimate
    sf::Vector3f k4 = dydt(y0 + dt * k3); //use k3 to get acceleration at the end of timestamp

    //weighted avg of the 4 accelerations
    sf::Vector3f y1 = y0 + (dt / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
    return y1;
}