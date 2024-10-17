#include <cmath>
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>

// assuming 100 pixels represent 1 meter, hence gravity is 980

class Particle
{
public:
    sf::Vector2f position;
    sf::Vector2f previousPosition;
    sf::Vector2f acceleration;
    bool isPinned;

    // verlet integration is used, hence the next position is a function of the previous position, current position and the acceleration
    // "pin" a particle to make its position fixed

    Particle(float x, float y, bool pinned = false)
        : position(x, y), previousPosition(x, y), acceleration(0, 0), isPinned(pinned) {}

    // since all "particles" have the same mass, you assume some arbitrary unit mass (as its the interaction between them that is of interest)

    void applyForce(const sf::Vector2f &force)
    {
        if (!isPinned)
        {
            acceleration += force;
        }
    }

    void update(float timeStep)
    {
        if (!isPinned)
        {
            sf::Vector2f velocity = position - previousPosition;
            previousPosition = position;
            position += velocity + acceleration * timeStep * timeStep;
            acceleration = sf::Vector2f(0, 0); // Reset acceleration after update
        }
        else
        {
            previousPosition = position;
        }
    }

    void constrainToBounds(float width, float height)
    {
        if (position.x < 0.0f)
            position.x = 0.0f;
        if (position.x > width)
            position.x = width;
        if (position.y < 0.0f)
            position.y = 0.0f;
        if (position.y > height - 1.0f)
            position.y = height - 1.0f;
    }

    // damping simulates energy loss
    void applyDamping(float damping)
    {
        if (!isPinned)
        {
            sf::Vector2f velocity = position - previousPosition;
            velocity *= damping;
            previousPosition = position - velocity;
        }
    }

    void handleGroundCollision(float groundY)
    {
        if (position.y > groundY)
        {
            position.y = groundY;
            if (!isPinned)
            {
                sf::Vector2f velocity = position - previousPosition;
                velocity.y *= -0.5f; // Bounce effect with damping
                previousPosition.y = position.y - velocity.y;
            }
        }
    }
};

// shear sprinsg (i,j) and (i+1, j+1)
// flexion springs (i,j) - (i+2, j) and (i,j+2)

class Constraint
{
    // implement the structural forces using constraints
public:
    Particle *particle1;
    Particle *particle2;
    float restLength;
    // make inactive if the constraint is broken
    bool isActive;

    // calculate the rest length between the two particles
    Constraint(Particle *p1, Particle *p2)
        : particle1(p1), particle2(p2), isActive(true)
    {
        sf::Vector2f delta = particle2->position - particle1->position;
        restLength = std::hypot(delta.x, delta.y);
    }

    // method is based on verlet integration and constraint relaxation

    // constraint projection is the key idea, instead of applying the forces, we directly use the positions of the particles to satisfy the constraints
    // this makes the method more stable

    // taken from the paper "Advanced Character Physics" by Thomas Jakobsen (specifically the function void ParticleSystem::SatisfyConstraints())

    // note: can optimize to approximate the square root which hasn't been implemented
    void satisfy()
    {
        if (!isActive)
            return;

        sf::Vector2f delta = particle2->position - particle1->position;
        float currentLength = std::hypot(delta.x, delta.y);

        // normalize the difference with the current length
        float diff = (currentLength - restLength) / currentLength;

        // scale the distance between them based on the difference, and multiply by 0.5 so that the correction is equally shared between the two particles
        sf::Vector2f correction = delta * 0.5f * diff;

        if (!particle1->isPinned)
            particle1->position += correction;
        if (!particle2->isPinned)
            particle2->position -= correction;
    }

    void deactivate() { isActive = false; }
};

// class for input handling
class InputHandler
{
public:
    static bool isDragging;
    static bool isPinMode;
    static sf::Vector2f dragStart;
    static std::vector<sf::Vector2f> dragPath;

    static void handleEvents(const sf::Event &event, std::vector<Constraint> &constraints, std::vector<Particle> &particles)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
        {
            isPinMode = !isPinMode;
        }

        if (isPinMode)
        {
            handlePinning(event, particles);
        }
        else
        {
            handleTearing(event, constraints);
        }
    }

    static void drawOverlay(sf::RenderWindow &window)
    {
        if (isPinMode)
        {
            drawPinCursor(window);
        }
        else if (isDragging && dragPath.size() > 1)
        {
            drawTearLine(window);
        }
    }

private:
    static void handleTearing(const sf::Event &event, std::vector<Constraint> &constraints)
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // set isDragging to true, store the start position of the drag
            isDragging = true;
            dragStart = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
            dragPath.clear();
            dragPath.push_back(dragStart);
        }
        // if the mouse is moving and isDragging is true, continue to add the current position to the dragPath
        else if (event.type == sf::Event::MouseMoved && isDragging)
        {
            dragPath.push_back(sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
        }
        // is mouse is released, and isDragging is true, process the tear
        else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && isDragging)
        {
            isDragging = false;
            processTear(constraints);
            dragPath.clear();
        }
    }

    static void handlePinning(const sf::Event &event, std::vector<Particle> &particles)
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            togglePin(mouseX, mouseY, particles);
        }
    }

    // currently brute force
    // similar optimization like in processTear need to be implemented
    static void togglePin(float mouseX, float mouseY, std::vector<Particle> &particles)
    {
        const float pinRadius = 10.0f; // 10 pixels
        const float pinRadiusSq = pinRadius * pinRadius;

        for (auto &particle : particles)
        {
            float dx = particle.position.x - mouseX;
            float dy = particle.position.y - mouseY;
            float distSq = dx * dx + dy * dy;
            if (distSq < pinRadiusSq)
            {
                particle.isPinned = !particle.isPinned;
                break;
            }
        }
    }

    // currently implemented by brute force
    // note: look into
    // 1) spatial partitioning (quadtree, octree, grid based methods)
    // 2) sweep and prune
    // 3) bounding box pre-checks (aabb)
    // 4) early exit checks
    // 5) parallelizaiton
    // 6) simplyfing the drag path maybe using Ramer-Douglas-Peucker algorithm
    static void processTear(std::vector<Constraint> &constraints)
    {
        for (auto &constraint : constraints)
        {
            if (!constraint.isActive)
                continue;
            for (size_t i = 0; i < dragPath.size() - 1; ++i)
            {
                if (lineIntersectsLine(
                        dragPath[i], dragPath[i + 1],
                        constraint.particle1->position, constraint.particle2->position))
                {
                    constraint.deactivate();
                    break; // Move to next constraint after deactivation
                }
            }
        }
    }

    // using Parametric Line Intersection
    // determines if two lines intersection in a 2D space
    static bool lineIntersectsLine(const sf::Vector2f &a1, const sf::Vector2f &a2,
                                   const sf::Vector2f &b1, const sf::Vector2f &b2)
    {

        // a1 a2 are the end points of the first line segment, b1 and b2 of the second line segment

        // calculate the determinant
        float d = (a2.x - a1.x) * (b2.y - b1.y) - (a2.y - a1.y) * (b2.x - b1.x);
        if (d == 0.0f)
            return false;

        float ua = ((b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x)) / d;
        float ub = ((a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x)) / d;

        return (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f);
    }

    static void drawTearLine(sf::RenderWindow &window)
    {
        sf::VertexArray lines(sf::LineStrip, dragPath.size());
        for (size_t i = 0; i < dragPath.size(); ++i)
        {
            lines[i].position = dragPath[i];
            lines[i].color = sf::Color::Red;
        }
        window.draw(lines);
    }

    static void drawPinCursor(sf::RenderWindow &window)
    {
        sf::CircleShape cursor(5);
        cursor.setFillColor(sf::Color::Blue);
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        cursor.setPosition(static_cast<float>(mousePos.x) - 5.0f, static_cast<float>(mousePos.y) - 5.0f);
        window.draw(cursor);
    }
};

const int WIDTH = 1080;
const int HEIGHT = 640;
const float GRAVITY = 980.0f;   // Adjusted gravity
const float TIME_STEP = 0.016f; // 60 FPS, the physics updates 60 times per second
const float DAMPING = 0.99f;    // reducing damping will make it more bouncy and less resistant to movement
const int ROWS = 30;
const int COLS = 30;
const float REST_DISTANCE = 10.0f;
const int CONSTRAINT_ITERATIONS = 15;

void resetSimulation(std::vector<Particle> &particles, std::vector<Constraint> &constraints)
{
    particles.clear();
    constraints.clear();

    // Create particles
    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            float x = col * REST_DISTANCE + WIDTH / 3.0f;
            float y = row * REST_DISTANCE + 50.0f;    // Start higher on the screen
            bool pinned = (row == 0 && col % 5 == 0); // Pin every 5th particle on the top row
            particles.emplace_back(x, y, pinned);
        }
    }

    // Create structural constraints (vertical and horizontal)
    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            int index = row * COLS + col;
            if (col < COLS - 1)
            {
                constraints.emplace_back(&particles[index], &particles[index + 1]);
            }
            if (row < ROWS - 1)
            {
                constraints.emplace_back(&particles[index], &particles[index + COLS]);
            }
        }
    }
}

// Initialize static members
bool InputHandler::isDragging = false;
bool InputHandler::isPinMode = false;
sf::Vector2f InputHandler::dragStart = sf::Vector2f(0, 0);
std::vector<sf::Vector2f> InputHandler::dragPath;

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Cloth Simulation with Verlet Integration");

    std::vector<Particle> particles;
    std::vector<Constraint> constraints;

    // Create particles
    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            float x = col * REST_DISTANCE + WIDTH / 3.0f;
            float y = row * REST_DISTANCE + 50.0f;    // Start higher on the screen
            bool pinned = (row == 0 && col % 5 == 0); // Pin every 5th particle on the top row
            particles.emplace_back(x, y, pinned);
        }
    }

    // Create structural constraints (vertical and horizontal)
    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            int index = row * COLS + col;
            if (col < COLS - 1)
            {
                constraints.emplace_back(&particles[index], &particles[index + 1]);
            }
            if (row < ROWS - 1)
            {
                constraints.emplace_back(&particles[index], &particles[index + COLS]);
            }
        }
    }

    // need to implement shear and bend constraints
    // // Create shear constraints (diagonals)
    // for (int row = 0; row < ROWS - 1; ++row)
    // {
    //     for (int col = 0; col < COLS - 1; ++col)
    //     {
    //         int index = row * COLS + col;
    //         constraints.emplace_back(&particles[index], &particles[index + COLS + 1]);
    //         constraints.emplace_back(&particles[index + 1], &particles[index + COLS]);
    //     }
    // }

    // // Create bend constraints (over two particles)
    // for (int row = 0; row < ROWS; ++row)
    // {
    //     for (int col = 0; col < COLS; ++col)
    //     {
    //         int index = row * COLS + col;
    //         if (col < COLS - 2)
    //         {
    //             constraints.emplace_back(&particles[index], &particles[index + 2]);
    //         }
    //         if (row < ROWS - 2)
    //         {
    //             constraints.emplace_back(&particles[index], &particles[index + 2 * COLS]);
    //         }
    //     }
    // }

    sf::RectangleShape resetButton(sf::Vector2f(100.0f, 40.0f));
    resetButton.setPosition(WIDTH - 120.0f, HEIGHT - 60.0f);
    resetButton.setFillColor(sf::Color::Red);

    // measures the amount of time between frames
    sf::Clock clock;

    // the accumulator is used to handle variable frame rates
    // helps in decoupling the physics from the rendering
    // tracks how much time remains that hasn't been simulated
    float accumulator = 0.0f;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                float mouseX = static_cast<float>(event.mouseButton.x);
                float mouseY = static_cast<float>(event.mouseButton.y);

                if (resetButton.getGlobalBounds().contains(mouseX, mouseY))
                {
                    resetSimulation(particles, constraints); // Reset the simulation
                }
            }
            InputHandler::handleEvents(event, constraints, particles);
        }

        // time between the last frame and the current frame
        float deltaTime = clock.restart().asSeconds();
        accumulator += deltaTime;
        while (accumulator >= TIME_STEP)
        {
            // Update particles
            // we are polling every single particle
            // brute force
            // note: look into
            // 1) spatial partitioning (quadtree, octree, grid based methods)
            // 2) broad phase and narrow phase collision detection
            // 3) variable time stepping
            // 4) parallelizaiton
            // 5) reducing constraint equations dynamically
            // 6) early exit constraint satisfaction
            // 7) perhaps a larger rest distance
            for (auto &particle : particles)
            {
                particle.applyForce(sf::Vector2f(0, GRAVITY));
                particle.update(TIME_STEP);
                particle.applyDamping(DAMPING);
                particle.handleGroundCollision(HEIGHT - 1.0f); // Ground at bottom of the window
            }

            // increasing makes more accurate, stiffer, and more stable but also increases computational cost
            for (int i = 0; i < CONSTRAINT_ITERATIONS; ++i)
            {
                for (auto &constraint : constraints)
                {
                    constraint.satisfy();
                }
            }

            accumulator -= TIME_STEP;
        }

        window.clear(sf::Color(50, 50, 50)); // Dark gray background

        // Draw constraints (cloth)
        for (const auto &constraint : constraints)
        {
            if (!constraint.isActive)
                continue;
            sf::Vertex line[] = {
                sf::Vertex(constraint.particle1->position, sf::Color::White),
                sf::Vertex(constraint.particle2->position, sf::Color::White),
            };
            window.draw(line, 2, sf::Lines);
        }

        // Draw particles
        for (const auto &particle : particles)
        {
            sf::CircleShape circle(3);
            circle.setPosition(particle.position - sf::Vector2f(3, 3));
            if (particle.isPinned)
            {
                circle.setFillColor(sf::Color::Blue);
            }
            else
            {
                circle.setFillColor(sf::Color(200, 200, 200)); // Light gray for unpinned particles
            }
            window.draw(circle);
        }

        // Draw tear line or pin cursor
        InputHandler::drawOverlay(window);

        // Display mode text
        sf::Font font;
        if (!font.loadFromFile("arial.ttf"))
        {
            // Handle error
        }
        window.draw(resetButton);

        sf::Text resetText;
        resetText.setFont(font);
        resetText.setString("Reset");
        resetText.setCharacterSize(18);
        resetText.setFillColor(sf::Color::White);
        resetText.setPosition(WIDTH - 100.0f, HEIGHT - 50.0f);
        window.draw(resetText);

        sf::Text modeText;
        modeText.setFont(font);
        modeText.setCharacterSize(18);
        modeText.setFillColor(sf::Color::Yellow);
        modeText.setPosition(10, 10);
        if (InputHandler::isPinMode)
        {
            modeText.setString("Mode: Pinning (Press 'P' to switch)");
        }
        else
        {
            modeText.setString("Mode: Normal (Press 'P' to switch)");
        }
        window.draw(modeText);

        window.display();
    }

    return 0;
}
