#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

const double g = 9.81;
const double L1 = 200;
const double L2 = 200;
const double m1 = 10.0;
const double m2 = 10.0;
double theta1 = M_PI / 36;
double theta2 = M_PI / 36;
double omega1 = 0.0;
double omega2 = 0.0;
double dt = 0.01;

void derivatives(double &theta1, double &omega1, double &theta2, double &omega2, double &omega1_dot, double &omega2_dot)
{
    double delta = theta2 - theta1;
    double denominator = (2 * m1 + m2) - m2 * cos(2 * delta);
    double numerator1 = -g * (2 * m1 + m2) * sin(theta1) - m2 * g * sin(theta1 - 2 * theta2) - 2 * sin(delta) * m2 * (omega2 * omega2 * L2 + omega1 * omega1 * L1 * cos(delta));
    double denom1 = L1 * denominator;
    omega1_dot = numerator1 / denom1;
    double numerator2 = 2 * sin(delta) * (omega1 * omega1 * L1 * (m1 + m2) + g * (m1 + m2) * cos(theta1) + omega2 * omega2 * L2 * m2 * cos(delta));
    double denom2 = L2 * denominator;
    omega2_dot = numerator2 / denom2;
}

void rk4_step(double &theta1, double &omega1, double &theta2, double &omega2, double dt)
{
    double k1_theta1 = omega1;
    double k1_theta2 = omega2;
    double k1_omega1, k1_omega2;
    derivatives(theta1, omega1, theta2, omega2, k1_omega1, k1_omega2);

    double theta1_mid = theta1 + 0.5 * dt * k1_theta1;
    double omega1_mid = omega1 + 0.5 * dt * k1_omega1;
    double theta2_mid = theta2 + 0.5 * dt * k1_theta2;
    double omega2_mid = omega2 + 0.5 * dt * k1_omega2;

    double k2_theta1 = omega1_mid;
    double k2_theta2 = omega2_mid;
    double k2_omega1, k2_omega2;
    derivatives(theta1_mid, omega1_mid, theta2_mid, omega2_mid, k2_omega1, k2_omega2);

    theta1_mid = theta1 + 0.5 * dt * k2_theta1;
    omega1_mid = omega1 + 0.5 * dt * k2_omega1;
    theta2_mid = theta2 + 0.5 * dt * k2_theta2;
    omega2_mid = omega2 + 0.5 * dt * k2_omega2;

    double k3_theta1 = omega1_mid;
    double k3_theta2 = omega2_mid;
    double k3_omega1, k3_omega2;
    derivatives(theta1_mid, omega1_mid, theta2_mid, omega2_mid, k3_omega1, k3_omega2);

    double theta1_end = theta1 + dt * k3_theta1;
    double omega1_end = omega1 + dt * k3_omega1;
    double theta2_end = theta2 + dt * k3_theta2;
    double omega2_end = omega2 + dt * k3_omega2;

    double k4_theta1 = omega1_end;
    double k4_theta2 = omega2_end;
    double k4_omega1, k4_omega2;
    derivatives(theta1_end, omega1_end, theta2_end, omega2_end, k4_omega1, k4_omega2);

    theta1 += (dt / 6.0) * (k1_theta1 + 2.0 * k2_theta1 + 2.0 * k3_theta1 + k4_theta1);
    omega1 += (dt / 6.0) * (k1_omega1 + 2.0 * k2_omega1 + 2.0 * k3_omega1 + k4_omega1);
    theta2 += (dt / 6.0) * (k1_theta2 + 2.0 * k2_theta2 + 2.0 * k3_theta2 + k4_theta2);
    omega2 += (dt / 6.0) * (k1_omega2 + 2.0 * k2_omega2 + 2.0 * k3_omega2 + k4_omega2);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Double Pendulum Simulation");
    sf::Vector2f origin(400, 300);
    std::vector<sf::Vertex> trajectory;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        rk4_step(theta1, omega1, theta2, omega2, dt);

        double x1 = origin.x + L1 * sin(theta1);
        double y1 = origin.y + L1 * cos(theta1);
        double x2 = x1 + L2 * sin(theta2);
        double y2 = y1 + L2 * cos(theta2);

        trajectory.emplace_back(sf::Vector2f(x2, y2), sf::Color::Red);
        if (trajectory.size() > 1000)
            trajectory.erase(trajectory.begin());

        window.clear();

        if (trajectory.size() > 1)
            window.draw(&trajectory[0], trajectory.size(), sf::LinesStrip);

        sf::VertexArray rods(sf::LinesStrip, 3);
        rods[0].position = origin;
        rods[1].position = sf::Vector2f(x1, y1);
        rods[2].position = sf::Vector2f(x2, y2);
        window.draw(rods);

        sf::CircleShape mass1(10);
        mass1.setOrigin(10, 10);
        mass1.setPosition(x1, y1);
        mass1.setFillColor(sf::Color::Blue);
        window.draw(mass1);

        sf::CircleShape mass2(10);
        mass2.setOrigin(10, 10);
        mass2.setPosition(x2, y2);
        mass2.setFillColor(sf::Color::Green);
        window.draw(mass2);

        window.display();
    }

    return 0;
}