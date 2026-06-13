#ifndef physics_hpp
#define physics_hpp

#include <SFML/Graphics.hpp>

struct Thrust {
    sf::Vector2f point; //point where the force is applied, relative to the CoM
    sf::Vector2f vector; //force vector
};
class Object {
public:
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
    void reset () {
        this->thrusts = std::vector<Thrust>(); //clear all forces acting on the object
        this->torques = std::vector<float>(); //clear all torques acting on the object
    }

};

//forward delcarations
class Ball;
sf::Vector3f resultOfForces(Ball& obj);
void BallCollision(Ball& ballA, Ball& ballB, sf::Vector2f ptA, sf::Vector2f ptB);

class Ball : public Object {
public: 
    float radius;
    float radiusPixels; //for rendering, the radius in pixels
    float MoI; //moment of inertia, for physics calculations
    bool calcPhysics; //whether to calculate physics for this object or not (ex. a background decoration may not need physics calculations)
    std::vector<Ball*>* allBalls; //list of balls in the system
        //allballs is a pointer the list of balls
    //constructor

    bool isDragged; //whether this ball is currently being dragged
    sf::Vector2f prevDragPos; //mouse position (meters) last frame, for throw velocity


    Ball(int id, float radius,float mass, sf::Vector2f CoM, sf::Color color, float angle=0.0f, bool hasGravity=true, bool canCollide = true, bool checkCollisions = true, bool fixed=false, bool calcPhysics = true) :         Object(id, mass, CoM, angle, color, hasGravity, canCollide, checkCollisions, fixed, std::nullopt) {
        //calls the parent constructor to initialize the ball object
        this->radius = radius;
        this->radiusPixels = meterToPixels(radius);
        this->MoI = 0.4f * mass * radius * radius; //moment of inertia for a solid sphere
        this->calcPhysics = calcPhysics;
        this->allBalls = nullptr; //will be set later after all balls are created
        this->isDragged = false;
        this->prevDragPos = sf::Vector2f(0.0f, 0.0f);
    }

    // checks if a click (in meters) lands on this ball; if so, starts dragging
    bool tryStartDrag(sf::Vector2f clickMeters) {
        sf::Vector2f diff = this->CoM - clickMeters;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist < this->radius) {
            this->isDragged = true;
            this->velocity = sf::Vector2f(0.0f, 0.0f);
            this->angularVelocity = 0.0f;
            this->fixed = true; // ignore forces while dragging
            this->prevDragPos = clickMeters;
            return true;
        }
        return false;
    }
    // call every frame while mouse button held, with current mouse position in meters
    void updateDrag(sf::Vector2f mouseMeters) {
        if (!this->isDragged) return;
        sf::Vector2f delta = mouseMeters - this->prevDragPos;
        this->velocity = delta / setTimeStep(); // store for throw velocity on release
        this->prevDragPos = mouseMeters;
    }
    // call on mouse release
    void endDrag() {
        if (!this->isDragged) return;
        this->isDragged = false;
        this->fixed = false; // gravity/forces resume, using last-set velocity as throw speed
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
            sf::Vector3f result = resultOfForces(*this);

            //apply new velocities
            this->velocity = sf::Vector2f(result.x, result.y);
            this->angularVelocity = result.z;

            //update position and angle with new velocities
            this->CoM += this->velocity * setTimeStep();
            this->angle += this->angularVelocity * setTimeStep();
            if (this->isDragged) {
                this->CoM = this->prevDragPos;
            } else {
                //it has collided with ground
                if (this->CoM.y - this->radius < 0.0f) {
                    this->CoM.y = this->radius; //make the ball sit on the ground, not go through it
                    this->velocity.y *= -0.8f; //reverse and reduce velocity
                }

                //it has collided with ceiling
                float ceilingMeters = HEIGHT / 100.0f; //px to meters
                if (this->CoM.y + this->radius > ceilingMeters) {
                    this->CoM.y = ceilingMeters - this->radius; //make the ball sit on the ceiling, not go through it
                    this->velocity.y *= -0.8f; //reverse and reduce velocity
                }

                //collided with left or right wall
                float wallMeters = WIDTH / 100.0f; //px to meters
                if (this->CoM.x - this->radius < 0.0f) {
                    this->CoM.x = this->radius;
                    this->velocity.x *= -0.8f;
                }
                if (this->CoM.x + this->radius > wallMeters) {
                    this->CoM.x = wallMeters - this->radius;
                    this->velocity.x *= -0.8f;
                }
            }
            checkBallCollisions();

        }
    }
    void checkBallCollisions(){
        if (!this->allBalls || !this->canCollide) {
            return; //no other balls to check collisions with
        }
        for (Ball* other : *this->allBalls) {
            // don't check against self
            if (other->id == this->id) continue;
            // don't check against non-collidable balls
            if (!other->canCollide) continue;
            // only check if other ball has higher id, to avoid resolving same pair twice
            if (other->id < this->id) continue;

            sf::Vector2f diff = other->CoM - this->CoM;
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

            if (dist < this->radius + other->radius && dist > 0.0f) {
                // push apart
                float overlap = (this->radius + other->radius) - dist;
                sf::Vector2f normal = diff / dist;
                this->CoM -= normal * (overlap / 2.0f);
                other->CoM += normal * (overlap / 2.0f);

                // collision points
                sf::Vector2f ptA = this->CoM + normal * this->radius;
                sf::Vector2f ptB = other->CoM - normal * other->radius;

                BallCollision(*this, *other, ptA, ptB);
            }
        }
    }
};
class Spring : public Object {
    public: 
        Object* objA;
        Object* objB;
        float strength; //spring constant
        int widthPx;
        float restLength; //length of spring at rest

    Spring (int id, Object* objA, Object* objB, float strength, sf::Color color, float restLength) 
        : Object(id, mass=0.0f, CoM=sf::Vector2f(0.0f, 0.0f), angle=0.0f, color=color, hasGravity=false, canCollide=false, checkCollisions=false, fixed=true, std::nullopt) {
        //calls the parent constructor to initialize the spring object
        //a spring is massless and fixed in place, it just applies forces to the objects it connects
        this->objA = objA;
        this->objB = objB;
        this->strength = strength;
        this->widthPx = 15;
        this->restLength = restLength;
    }

    void getForces() {
        //distance between the two objects
        sf::Vector2f diff = objB->CoM - objA->CoM;
        float length = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (length == 0) return; //to avoid division by 0

        float extension = length - restLength; //how much is the spring extended
        float forceMagnitude = strength * extension;

        //unit vector A to B
        sf::Vector2f dirAtoB = diff / length;

        //thrust on A, or the pull towards B
        Thrust thrustA;
        thrustA.vector = dirAtoB * forceMagnitude;
        thrustA.point = sf::Vector2f(0.0f, 0.0f); // the force is applied at CoM

        //thrust on B, or the pull towards A
        Thrust thrustB;
        thrustB.vector = -dirAtoB * forceMagnitude; //equal and opposite force on B
        thrustB.point = sf::Vector2f(0.0f, 0.0f); // the force is applied at CoM
        
        objA->thrusts.push_back(thrustA);
        objB->thrusts.push_back(thrustB);
    }
    //no update function bc the spring itself wont move

    void draw(sf::RenderWindow& window) {
        //rendering code using SFML, draws a line between the two objects it connects
        
        sf::Vector2f pixelA = metersToScreenCoords(objA->CoM.x, objA->CoM.y, HEIGHT);
        sf::Vector2f pixelB = metersToScreenCoords(objB->CoM.x, objB->CoM.y, HEIGHT);

        //we are making the spring spiral
        sf::Vector2f delta = pixelB - pixelA; //direction
        float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (length < 1.0f) return;

        sf::Vector2f along = delta / length; //unit vector
        sf::Vector2f perp(-along.y, along.x); //perpendicular unit vector
        
        const int   coils      = 10;    // number of coil loops
        const float amplitude  = 8.0f; // half-width of the coil in pixels
        const int   segments   = coils * 16; // smoothness
        const float endcap     = 12.0f; // straight lead-in/out in pixels

        auto makeVertex = [&](sf::Vector2f pos) {
            sf::Vertex v;
            v.position = pos;
            v.color = this->color;
            return v;
        };

        std::vector<sf::Vertex> verts;
        verts.reserve(segments + 3);

        // start point of the coil, after the straight endcap
        sf::Vector2f coilStart = pixelA + along * endcap;
        float coilLength = std::max(length - 2.0f * endcap, 1.0f);

        verts.push_back(makeVertex(pixelA));

        //for each segment, calculate the position along the line and add a perpendicular offset to create the coil effect
        for (int i = 0; i <= segments; ++i) {
            float t    = static_cast<float>(i) / segments;
            float sine = std::sin(t * coils * 2.0f * 3.14159265f);
            sf::Vector2f pt = coilStart + along * (t * coilLength)
                                        + perp  * (sine * amplitude);
            verts.push_back(makeVertex(pt));
        }

        verts.push_back(makeVertex(pixelB));

        window.draw(verts.data(), verts.size(), sf::PrimitiveType::LineStrip);


    }


};


float crossProduct2DVectors (sf::Vector2f a, sf::Vector2f b);
sf::Vector3f operator+(sf::Vector3f a, sf::Vector3f b);
sf::Vector3f operator*(float s, sf::Vector3f v);
sf::Vector3f operator*(sf::Vector3f v, float s);
sf::Vector3f rb(sf::Vector3f y, float t, float g, float m, std::vector<Thrust> thrusts, std::vector<float> torques, float I, float k, bool fixed);
sf::Vector3f resultOfForces(Ball& obj);
void BallCollision(Ball& ballA, Ball& ballB, sf::Vector2f ptA, sf::Vector2f ptB);


#endif // physics_hpp