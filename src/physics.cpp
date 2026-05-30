#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include <optional>

struct Thrust {
    sf::Vector2f point; //point where the force is applied, relative to the CoM
    sf::Vector2f vector; //force vector
};

class Object {
protected:
    int id;
    float mass;
    sf::Vector2f CoM; //center of mass
    float angle; //rotation in radians
    sf::Color color;
    bool hasGravity;
    bool canCollide;
    bool checkCollisions;
    bool fixed; //whether the object is immovable or not
    std::optional<sf::Vector2f> root; //a reference point for the object, ex. a rod swinging from one end may have CoM somewhere else but needs its root at the end its swinging from
    sf::Vector2f velocity;
    float angularVelocity;
    std::vector<Thrust> thrusts; 
    std::vector<float> torques;
    std::vector<float> previousDistances; //for collision detection, stores the previous distances between this object and other objects
    Object* parent;
    
public: 

    //constructor
    Object (int id, float mass, sf::Vector2f CoM, float angle, sf::Color color, bool hasGravity, bool canCollide, bool checkCollisions, bool fixed, std::optional<sf::Vector2f> root){ 
        //id: each object has unique id
        //mass: how heavy it is
        //CoM: 2D vector of the objects position
        //angle: rotation of the object in radians
        //color: object color
        //hasGravity: whether gravity affects this object
        //canCollide: whether this object will participate in a collisions(ex. bounce, stop, etc)
        //checkCollisions: whether this object activly checks for collisions with others (ex. a static wall would have checkCollisions=False)
        //fixed: wehther the object is immovable or not
        //root: a reference point for the object, ex. a rod swinging from one end may have CoM somewhere else but needs its root at the end its swinging from
        
        this->id = id;
        this->mass = mass;
        this->CoM = CoM;
        this->angle = angle;
        this->color = color;
        this->hasGravity = hasGravity;
        this->canCollide = canCollide;
        this->checkCollisions = checkCollisions;
        this->fixed = fixed;
        this->root = root;

        if (!this->root.has_value()) {
            this->root = this->CoM; //if no root provided, set root to CoM by default
        }

        this->velocity = sf::Vector2f(0.0f, 0.0f); //initial velocity is 0
        this->angularVelocity = 0.0f; //initial angular velocity is 0

        this->thrusts = std::vector<Thrust>(); //initially no forces acting on the object
        this->torques = std::vector<float>(); //initially no torques 

        this->previousDistances = std::vector<float>(); //for collision detection, stores the previous distances between this object and other objects
        this->parent = nullptr;
    }

    std::vector<Thrust> getThrusts() {
        return this->thrusts;
    }
    void reset () {
        this->thrusts = std::vector<Thrust>(); //clear all forces acting on the object
        this->torques = std::vector<float>(); //clear all torques acting on the object
    }

};

class Ball : public Object {
protected: 
    float radius;
    float radiusPixels; //for rendering, the radius in pixels
    float MoI; //moment of inertia, for physics calculations
    bool calcPhysics; //whether to calculate physics for this object or not (ex. a background decoration may not need physics calculations)
public:
    //constructor
    Ball(int id, float radius,float mass, sf::Vector2f CoM, sf::Color color, float angle=0.0f, bool hasGravity=true, bool canCollide = true, bool checkCollisions = true, bool fixed=false, bool calcPhysics = true) :         Object(id, mass, CoM, angle, color, hasGravity, canCollide, checkCollisions, fixed, std::nullopt) {
        //calls the parent constructor to initialize the ball object
        this->radius = radius;
        this->radiusPixels = meterToPixels(radius);
        this->MoI = 0.4f * mass * radius * radius; //moment of inertia for a solid sphere
        this->calcPhysics = calcPhysics;
    }

    void describe() {
        std::cout << "Ball " << this->id << ": radius=" << this->radius << ", mass=" << this->mass << std::endl;
    }
    void draw(sf::RenderWindow& window ) {
        //rendering code using SFML, converts CoM from meters to pixels and draws a circle with the appropriate radius and color
        sf::CircleShape shape(this->radiusPixels);
        shape.setFillColor(this->color);
        sf::Vector2f pixelCoords = metersToScreenCoords(this->CoM.x, this->CoM.y, HEIGHT);
        shape.setPosition(sf::Vector2f(pixelCoords.x - this->radiusPixels, pixelCoords.y - this->radiusPixels)); //set position of the top left corner of the bounding box of the circle
        //the CoM is at the center of the circle, so we need to offset by the radius to get the top left corner
        //if we set the position to be the CoM directly, it would draw the circle with its center at the CoM, which is not what we want
        //we want the CoM to be at the center of the circle, so we need to offset by the radius to get the top left corner
        //this is because SFML's CircleShape position is based on the top left corner of its bounding box, not its center
        //so we need to adjust for that when setting the position
        //if we didn't do this, the circle would be drawn with its center at (0,0) instead of at its CoM

        //drawing to window
        window.draw(shape);
    }
    void update() {
        //physics calculations to update the ball's position, velocity, angle, etc. based on the forces and torques acting on it
        //this function would be called every simulation tick to update the ball's state
        if(this->calcPhysics) {
            //apply the forces 
            for (Thrust thrust : this->thrusts) {
                //F = ma -> a = F/m
                sf::Vector2f acceleration = thrust.vector / this->mass;
                this->velocity += acceleration * setTimeStep(); //update velocity based on acceleration and time step
            }
            for (float torque : this->torques) {
                //torque = I * alpha -> alpha = torque / I
                float angularAcceleration = torque / this->MoI;
                this->angularVelocity += angularAcceleration * setTimeStep(); //update angular velocity based on angular acceleration and time step
            }
            //update position and angle based on velocity and angular velocity
            this->CoM += this->velocity * setTimeStep();
            this->angle += this->angularVelocity * setTimeStep();
        }
    }
};

float crossProduct2DVectors (sf::Vector2f a, sf::Vector2f b) {
    return a.x * b.y - a.y * b.x;
}


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

float resultOfForces(Object obj) {
    // will calculate acceleration based on all of the forces in the simulation

   
}