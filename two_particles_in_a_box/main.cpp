// not working yet

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

// Particle structure
struct Particle
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float mass;

    Particle(float radius, float mass, sf::Vector2f position, sf::Vector2f velocity, sf::Color color)
    {
        this->mass = mass;
        this->velocity = velocity;
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setOrigin(radius, radius); // Set origin to center for easier positioning
        shape.setPosition(position);
    }
};

void handleWallCollision(Particle &particle, const sf::FloatRect &bounds)
{
    sf::Vector2f position = particle.shape.getPosition();
    float radius = particle.shape.getRadius();

    // Left or right wall
    if ((position.x - radius <= bounds.left && particle.velocity.x < 0) ||
        (position.x + radius >= bounds.left + bounds.width && particle.velocity.x > 0))
    {
        particle.velocity.x = -particle.velocity.x;
    }

    // Top or bottom wall
    if ((position.y - radius <= bounds.top && particle.velocity.y < 0) ||
        (position.y + radius >= bounds.top + bounds.height && particle.velocity.y > 0))
    {
        particle.velocity.y = -particle.velocity.y;
    }
}

void handleParticleCollision(Particle &p1, Particle &p2)
{
    sf::Vector2f delta = p2.shape.getPosition() - p1.shape.getPosition();
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    float sumRadius = p1.shape.getRadius() + p2.shape.getRadius();

    // Check for collision
    if (distance < sumRadius)
    {
        // Normalize the delta vector
        sf::Vector2f mtd = delta * ((sumRadius - distance) / distance);

        // Push particles apart to avoid overlap
        p1.shape.move(-mtd * (p2.mass / (p1.mass + p2.mass)));
        p2.shape.move(mtd * (p1.mass / (p1.mass + p2.mass)));

        // Relative velocity
        sf::Vector2f v = p1.velocity - p2.velocity;

        // Collision normal
        sf::Vector2f collisionNormal = delta / distance;

        // Velocity along the normal
        float vn = v.x * collisionNormal.x + v.y * collisionNormal.y;

        // If velocities are separating, do nothing
        if (vn > 0.0f)
            return;

        // Compute impulse scalar
        float restitution = 1.0f; // Perfectly elastic collision
        float i = -(1.0f + restitution) * vn / (1.0f / p1.mass + 1.0f / p2.mass);

        // Impulse vector
        sf::Vector2f impulse = collisionNormal * i;

        // Change velocities
        p1.velocity += impulse / p1.mass;
        p2.velocity -= impulse / p2.mass;
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Two Particles in a Box");

    sf::FloatRect boxBounds(50.f, 50.f, 700.f, 500.f); // Left, Top, Width, Height
    sf::RectangleShape box;
    box.setPosition(sf::Vector2f(boxBounds.left, boxBounds.top));
    box.setSize(sf::Vector2f(boxBounds.width, boxBounds.height));
    box.setFillColor(sf::Color::Transparent);
    box.setOutlineThickness(2.f);
    box.setOutlineColor(sf::Color::White);

    // Create two particles
    Particle particle1(20.f, 2.f, sf::Vector2f(200.f, 300.f), sf::Vector2f(500.f, -300.f), sf::Color::Red);
    Particle particle2(30.f, 3.f, sf::Vector2f(200.f, 400.f), sf::Vector2f(-650.f, 270.f), sf::Color::Blue);

    // Start the game loop
    sf::Clock clock;
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window : exit
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear screen
        window.clear();

        // Time since last frame
        float dt = clock.restart().asSeconds();

        // Update particle positions
        particle1.shape.move(particle1.velocity * dt);
        particle2.shape.move(particle2.velocity * dt);

        // Handle wall collisions
        handleWallCollision(particle1, boxBounds);
        handleWallCollision(particle2, boxBounds);

        // Handle particle collision
        handleParticleCollision(particle1, particle2);

        // Draw the box
        window.draw(box);

        // Draw the particles
        window.draw(particle1.shape);
        window.draw(particle2.shape);

        // Update the window
        window.display();
    }

    return 0;
}
