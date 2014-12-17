#include "stdafx.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <windows.h>

int const windowWidth{1800};
int const windowHeight{1100};
float const ballRadius = 3.0;

///////////////////////////////////////////////////////////////////////////////////////////////////
struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    Ball() {}
    Ball(float x, float y, float vx, float vy)
    {
        velocity = sf::Vector2f(vx, vy);
        shape.setPosition(x, y);
        shape.setRadius(ballRadius);
        shape.setFillColor(sf::Color(rand()%255, rand()%255, rand()%255));
        shape.setOrigin(ballRadius, ballRadius);
        isVisible = true;
    }
    void Update() 
    { 
        if (left() < 0) { velocity.x = abs(velocity.x); }
        else if (right() > windowWidth) { velocity.x = -abs(velocity.x); }

        if (top() < 0) { velocity.y = abs(velocity.y); }
        else if (bottom() > windowHeight) { velocity.y = -abs(velocity.y); }

        shape.move(velocity);
    }
    bool isVisible;

    float x()       { return shape.getPosition().x; }
    float y()       { return shape.getPosition().y; }
    float left()    { return x() - shape.getRadius(); }
    float right()   { return x() + shape.getRadius(); }
    float top()     { return y() - shape.getRadius(); }
    float bottom()  { return y() + shape.getRadius(); }
};

bool IsTouching(Ball& b1, Ball& b2)
{
    //return sqrt(pow(abs(b1.x() - b2.x()), 2) + pow(abs(b1.y() - b2.y()), 2)) <= 2*ballRadius;
    auto const R2 = 2*ballRadius;
    auto const xx = abs(b1.x() - b2.x());
    auto const yy = abs(b1.y() - b2.y());
    return xx <= R2 && yy <= R2 && sqrt(pow(xx, 2) + pow(yy, 2)) <= R2;
}

/*
///////////////////////////////////////////////////////////////////////////////////////////////////
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "MvD");
    window.setFramerateLimit(60);

    std::vector<Ball> balls;
    for (int i=0; i<1000; ++i)
    {
        balls.push_back(Ball(rand()%windowWidth, rand()%windowHeight, rand()%11-5, rand()%11-5));
    }
    
    while (window.isOpen())
    {
        // windows message pump
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) { window.close(); }
        }

        window.clear(sf::Color::Black);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) { window.close(); }
        for (auto& ball : balls) 
        { 
            if (ball.isVisible)
            {
                ball.Update(); 
                for (auto& ball2 : balls)
                {
                    if (ball2.isVisible && &ball != &ball2 && IsTouching(ball, ball2))
                    {
                        ball.isVisible = false;
                        ball2.isVisible = false;
                    }
                    else
                    {
                        window.draw(ball.shape);
                    }
                }
            }
        }
        window.display();
    }
    return 0;
}
*/

void UpdateImage(sf::Image& im, unsigned char DEFINITION)
{
    if (true)
    {
        im.setPixel(windowWidth/2, 0, sf::Color::Green);
    }
    else
    {
        for (int x=0; x<windowWidth; ++x)
        {
            im.setPixel(x, 0, rand() % 2 ? sf::Color::Black : sf::Color::Green);
        }
    }

    for (int y=1; y<windowHeight; ++y)
    {
        for (int x=1; x<(windowWidth-1); ++x)
        {
            int a = im.getPixel(x-1, y-1) == sf::Color::Black ? 0 : 1;
            int b = im.getPixel(x,   y-1) == sf::Color::Black ? 0 : 1;
            int c = im.getPixel(x+1, y-1) == sf::Color::Black ? 0 : 1;
            int d = 4*a + 2*b + c;
            im.setPixel(x, y, ((DEFINITION & (1<<d)) == 0) ? sf::Color::Black : sf::Color::Green);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    srand(static_cast<unsigned int>(time(0)));
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "MvD");
    window.setFramerateLimit(10);

    sf::Image im;
    im.create(windowWidth, windowHeight);

    unsigned char DEFINITION = 0x10;

    sf::Texture tex;
    tex.loadFromImage(im);
    sf::Sprite s;
    s.setTexture(tex, true);

    while (window.isOpen())
    {
        // process keyboard
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) { window.close(); }

        // windows message pump
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) { window.close(); }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            ++DEFINITION;
            window.clear(sf::Color::Black);
            UpdateImage(im, DEFINITION);
            tex.loadFromImage(im);
            s.setTexture(tex, true);
            s.setScale(2.0, 2.0);
            window.setTitle("definition = " + std::to_string(DEFINITION));
            window.draw(s);
            window.display();
            Sleep(100);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            --DEFINITION;
            UpdateImage(im, DEFINITION);
            tex.loadFromImage(im);
            s.setTexture(tex, true);
            s.setScale(2.0, 2.0);
            window.clear(sf::Color::Black);
            window.draw(s);
            window.setTitle("definition = " + std::to_string(DEFINITION));
            window.display();
            Sleep(200);
        }
    }
    return 0;
}


