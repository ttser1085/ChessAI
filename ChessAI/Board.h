#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

enum Figure : uint8_t
{
	wK,
	wQ,
	wR,
	wB,
	wN,
	wP,

	bK,
	bQ,
	bR,
	bB,
	bN,
	bP
};

typedef sf::Vector2<uint8_t> pos_t;

class Board
{
public:
	Board();
	Board(sf::Vector2f pos, float square_size);

	float getSquareSize();
	sf::FloatRect getRect();

	bool getFigure(pos_t pos);
	bool getFigure(pos_t pos, Figure& figure);
	void setFigure(pos_t pos, Figure figure);

	void moveFigure(pos_t from, pos_t to);
	void delFigure(pos_t pos);

	pos_t findKing(bool color);

	Board copy();

	void render(sf::RenderWindow& window);

private:
	Figure figures[8][8];
	bool locations[8][8];

	pos_t white_king_pos, black_king_pos;

	sf::Vector2f pos;
	float square_size;
	sf::VertexArray lines, quads;

	sf::Texture* textures;
	sf::Sprite* sprites;
};