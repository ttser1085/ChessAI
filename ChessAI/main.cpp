#include "ChessHandler.h"

const int WIDTH = 1080, HEIGHT = 840;

int main()
{
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close);

	sf::Clock clock;

	ChessHandler chess(sf::Vector2f(20.0f, 20.0f), 80.0f);

	while (window.isOpen())
	{
		float dt = clock.restart().asSeconds();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			chess.checkEvents(event, window);
		}

		chess.update();

		window.clear(sf::Color::White);

		chess.render(window);

		window.display();
	}

	return 0;
}