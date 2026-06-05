#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include "physics.hpp"
#include <optional>
#include <cmath>


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

void BallCollision(Ball& ballA, Ball& ballB, sf::Vector2f ptA, sf::Vector2f ptB) {

    sf::Vector2f AcomToPt = ptA - ballA.CoM; //vector from CoM of A to collision point
    sf::Vector2f BcomToPt = ptB - ballB.CoM; //vector from CoM of B to collision point

    //total velocity of point is:
        //translational velocity, CoM moving
        //rotational velocity, object spinning around CoM

    //rotational speed of any point is its distance from center times angular velocity
    float pointArotationalSpeed = std::sqrt(std::pow(AcomToPt.x, 2) + std::pow(AcomToPt.y, 2)) * ballA.angularVelocity; //v = r * omega
    sf::Vector2f perpendicularRotationVector = sf::Vector2f(-AcomToPt.y, AcomToPt.x); //perpendicular to the vector from CoM to point, direction of rotation velocity
    float prependicularLen = std::sqrt(std::pow(perpendicularRotationVector.x, 2) + std::pow(perpendicularRotationVector.y, 2));
    if (prependicularLen != 0) { //to avoid division by 0
        perpendicularRotationVector /= prependicularLen; //normalize the perpendicular vector
    }
    sf::Vector2f ptArotationVector = pointArotationalSpeed * perpendicularRotationVector; //rotational velocity vector at the point

    float pointBrotationalSpeed = std::sqrt(std::pow(BcomToPt.x, 2) + std::pow(BcomToPt.y, 2)) * ballB.angularVelocity; //v = r * omega
    sf::Vector2f perpendicularRotationVectorB = sf::Vector2f(-BcomToPt.y, BcomToPt.x); //perpendicular to the vector from CoM to point, direction of rotation velocity
    float perpendicularLenB = std::sqrt(std::pow(perpendicularRotationVectorB.x, 2) + std::pow(perpendicularRotationVectorB.y, 2));
    if (perpendicularLenB != 0) { //to avoid division by 0
        perpendicularRotationVectorB /= perpendicularLenB; //normalize the perpendicular vector
    }
    sf::Vector2f ptBrotationVector = pointBrotationalSpeed * perpendicularRotationVectorB; //rotational velocity vector at the point

    //now need to figure out relative velocity
    //this is the velocity of A relative to B, so we subtract B's velocity from A's velocity
    sf::Vector2f relPtV =(
        (ballA.velocity + ptArotationVector) -
        (ballB.velocity + ptBrotationVector)
    );

    //unit vector from A to B
    sf::Vector2f AtoB = ptB - ptA;
    float AtoBLen = std::sqrt(std::pow(AtoB.x, 2) + std::pow(AtoB.y, 2));
    if (AtoBLen != 0) { //to avoid division by 0
        AtoB.x /= AtoBLen; //normalize the vector from A to B
        AtoB.y /= AtoBLen;
    }

    //rotational resistance terms
    float rA = crossProduct2DVectors(AcomToPt, AtoB) / ballA.MoI; //perpendicular distance from CoM of A to the line of action of the collision force
    float rB = crossProduct2DVectors(BcomToPt, AtoB) / ballB.MoI; //perpendicular distance from CoM of B to the line of action of the collision force

    //impulse scalar, which tells us how much force to apply to each object to resolve the collision
    float impulse = (
        crossProduct2DVectors(relPtV, AtoB) *
        -(1+ELASTICITY) / //coefficient of restitution, how bouncy the collision is
        (1/ballA.mass + 1/ballB.mass + rA + rB)
    );

    //update velocities
    ballA.velocity += (AtoB * impulse) / ballA.mass;
    ballB.velocity += (AtoB * -impulse) / ballB.mass; //B pushed the opposite direction of A

    ballA.angularVelocity += crossProduct2DVectors(AcomToPt, AtoB * impulse) / ballA.MoI; //torque = r x F
    ballB.angularVelocity -= crossProduct2DVectors(BcomToPt, AtoB * impulse) / ballB.MoI; //torque = r x F, B gets opposite torque of A

}