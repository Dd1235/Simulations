#include <SFML/Graphics.hpp>

// g++ -o main main.cpp -I/opt/homebrew/opt/sfml/include -L/opt/homebrew/opt/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system

// ./main
class Ball
{
public:
    sf::CircleShape shape;
    float vx, vy;
    float radius;

    Ball(float x, float y, float radius, float vx, float vy) : vx(vx), vy(vy), radius(radius)
    {
        shape.setPosition(x, y);
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::Red);
        shape.setOrigin(radius, radius);
    }

    void update(float dt, sf::FloatRect boxBounds)
    {
        sf::Vector2f position = shape.getPosition();

        position.x += vx * dt;
        position.y += vy * dt;

        // collision with left wall
        if (position.x - radius < boxBounds.left)
        {
            position.x = boxBounds.left + radius;
            vx = -vx;
        }
        // collision with right wall
        else if (position.x + radius > boxBounds.left + boxBounds.width)
        {
            position.x = boxBounds.left + boxBounds.width - radius;
            vx = -vx;
        }
        // collision with top wall
        if (position.y - radius < boxBounds.top)
        {
            position.y = boxBounds.top + radius;
            vy = -vy;
        }
        // collision with bottom wall
        else if (position.y + radius > boxBounds.top + boxBounds.height)
        {
            position.y = boxBounds.top + boxBounds.height - radius;
            vy = -vy;
        }
        shape.setPosition(position);
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Ball in a Box - Elastic Collision");

    // discrete collision detection, to take care of tunneling
    window.setFramerateLimit(60);

    sf::FloatRect boxBounds(100, 100, 600, 400);

    sf::RectangleShape boxOutline;
    boxOutline.setPosition(boxBounds.left, boxBounds.top);
    boxOutline.setSize(sf::Vector2f(boxBounds.width, boxBounds.height));
    boxOutline.setFillColor(sf::Color::Transparent);
    boxOutline.setOutlineThickness(5);
    boxOutline.setOutlineColor(sf::Color::White);

    Ball ball(400, 400, 20, 200, 150);

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();
        ball.update(dt, boxBounds);

        window.clear(sf::Color::Black);

        window.draw(boxOutline);
        window.draw(ball.shape);
        window.display();
    }
    return 0;
}