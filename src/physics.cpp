#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include "utils.hpp"
#include <optional>

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
    std::vector<sf::Vector2f> thrusts; 
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

        this->thrusts = std::vector<sf::Vector2f>(); //initially no forces acting on the object
        this->torques = std::vector<float>(); //initially no torques 

        this->previousDistances = std::vector<float>(); //for collision detection, stores the previous distances between this object and other objects
        this->parent = nullptr;
    }

    std::vector<sf::Vector2f> getThrusts() {
        return this->thrusts;
    }
    void reset () {
        this->thrusts = std::vector<sf::Vector2f>(); //clear all forces acting on the object
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
};