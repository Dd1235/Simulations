#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

struct Segment
{
    sf::Vector2f position;
    float angle;
    float length;

    // Constructor
    Segment(sf::Vector2f pos, float ang, float len)
        : position(pos), angle(ang), length(len) {}
};

class Snake
{
public:
    std::vector<Segment> segments;
    float amplitude = 20.f;
    float frequency = 0.5f;
    float speed = 100.f;

    Snake(int numSegments, float segmentLength)
    {
        for (int i = 0; i < numSegments; ++i)
        {
            segments.emplace_back(sf::Vector2f(400, 300 - i * segmentLength), 0, segmentLength);
        }
    }

    void update(float deltaTime, sf::Vector2f target)
    {
        // FK: Sinusoidal motion
        for (size_t i = 1; i < segments.size(); ++i)
        {
            segments[i].angle = amplitude * std::sin(frequency * i - speed * deltaTime);
            segments[i].position = segments[i - 1].position +
                                   sf::Vector2f(std::cos(segments[i].angle), std::sin(segments[i].angle)) * segments[i].length;
        }

        // IK: Move towards target
        segments.back().position = target;
        for (int i = segments.size() - 2; i >= 0; --i)
        {
            sf::Vector2f dir = segments[i + 1].position - segments[i].position;
            float distance = std::hypot(dir.x, dir.y);
            sf::Vector2f offset = (dir / distance) * segments[i].length;
            segments[i].position = segments[i + 1].position - offset;
        }
    }

    void render(sf::RenderWindow &window)
    {
        for (const auto &segment : segments)
        {
            sf::RectangleShape line(sf::Vector2f(segment.length, 5));
            line.setPosition(segment.position);
            line.setRotation(segment.angle * 180 / M_PI);
            window.draw(line);
        }
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Snake with FK and IK");

    Snake snake(10, 20.f);
    sf::Vector2f target(400, 300);
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        target = sf::Vector2f(sf::Mouse::getPosition(window));

        float deltaTime = clock.restart().asSeconds();
        snake.update(deltaTime, target);

        window.clear();
        snake.render(window);
        window.display();
    }

    return 0;
}
