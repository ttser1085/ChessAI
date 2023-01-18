#include "Board.h"

Board::Board() {}

Board::Board(sf::Vector2f pos, float square_size)
{
	this->pos = pos;
	this->square_size = square_size;

	this->figures[0][0] = wR;
	this->figures[0][1] = wN;
	this->figures[0][2] = wB;
	this->figures[0][3] = wQ;
	this->figures[0][4] = wK;
	this->figures[0][5] = wB;
	this->figures[0][6] = wN;
	this->figures[0][7] = wR;

	this->figures[1][0] = wP;
	this->figures[1][1] = wP;
	this->figures[1][2] = wP;
	this->figures[1][3] = wP;
	this->figures[1][4] = wP;
	this->figures[1][5] = wP;
	this->figures[1][6] = wP;
	this->figures[1][7] = wP;

	this->figures[7][0] = bR;
	this->figures[7][1] = bN;
	this->figures[7][2] = bB;
	this->figures[7][3] = bQ;
	this->figures[7][4] = bK;
	this->figures[7][5] = bB;
	this->figures[7][6] = bN;
	this->figures[7][7] = bR;

	this->figures[6][0] = bP;
	this->figures[6][1] = bP;
	this->figures[6][2] = bP;
	this->figures[6][3] = bP;
	this->figures[6][4] = bP;
	this->figures[6][5] = bP;
	this->figures[6][6] = bP;
	this->figures[6][7] = bP;

	this->white_king_pos = pos_t(4, 0);
	this->black_king_pos = pos_t(4, 7);

	for (uint8_t x = 0; x < 8; x++)
	{
		for (uint8_t y = 0; y < 8; y++)
			this->locations[y][x] = false;

		this->locations[0][x] = true;
		this->locations[1][x] = true;
		this->locations[7][x] = true;
		this->locations[6][x] = true;
	}

	this->lines = sf::VertexArray(sf::Lines, 36);

	for (int i = 0; i < 17; i += 2)
	{
		this->lines[i] = sf::Vertex(this->pos + sf::Vector2f(i / 2 * this->square_size, 0.0f), sf::Color::Black);
		this->lines[i + 1] = sf::Vertex(this->pos + sf::Vector2f(i / 2 * this->square_size, 8 * this->square_size), sf::Color::Black);
	}

	for (int i = 0; i < 17; i += 2)
	{
		this->lines[i + 18] = sf::Vertex(this->pos + sf::Vector2f(0.0f, i / 2 * this->square_size), sf::Color::Black);
		this->lines[i + 19] = sf::Vertex(this->pos + sf::Vector2f(8 * this->square_size, i / 2 * this->square_size), sf::Color::Black);
	}

	this->quads = sf::VertexArray(sf::Quads, 128);

	int n = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if ((j % 2 + i % 2) % 2 != 0)
			{
				this->quads[n] = sf::Vertex(this->pos + sf::Vector2f(j * this->square_size, i * this->square_size), sf::Color::Black);
				this->quads[n + 1] = sf::Vertex(this->pos + sf::Vector2f((j + 1) * this->square_size, i * this->square_size), sf::Color::Black);
				this->quads[n + 2] = sf::Vertex(this->pos + sf::Vector2f((j + 1) * this->square_size, (i + 1) * this->square_size), sf::Color::Black);
				this->quads[n + 3] = sf::Vertex(this->pos + sf::Vector2f(j * this->square_size, (i + 1) * this->square_size), sf::Color::Black);
				n += 4;
			}
		}
	}

	this->textures = new sf::Texture[12];
	this->sprites = new sf::Sprite[12];

	this->textures[0].loadFromFile("../Textures/chess24/wK.png");
	this->textures[1].loadFromFile("../Textures/chess24/wQ.png");
	this->textures[2].loadFromFile("../Textures/chess24/wR.png");
	this->textures[3].loadFromFile("../Textures/chess24/wB.png");
	this->textures[4].loadFromFile("../Textures/chess24/wN.png");
	this->textures[5].loadFromFile("../Textures/chess24/wP.png");

	this->textures[6].loadFromFile("../Textures/chess24/bK.png");
	this->textures[7].loadFromFile("../Textures/chess24/bQ.png");
	this->textures[8].loadFromFile("../Textures/chess24/bR.png");
	this->textures[9].loadFromFile("../Textures/chess24/bB.png");
	this->textures[10].loadFromFile("../Textures/chess24/bN.png");
	this->textures[11].loadFromFile("../Textures/chess24//bP.png");

	for (int i = 0; i < 12; i++)
	{
		this->textures[i].setSmooth(true);
		this->sprites[i].setTexture(this->textures[i]);
		this->sprites[i].scale(this->square_size / this->textures[i].getSize().x, this->square_size / this->textures[i].getSize().x);
	}
}

float Board::getSquareSize()
{
	return this->square_size;
}

sf::FloatRect Board::getRect()
{
	return sf::FloatRect(this->pos, sf::Vector2f(this->square_size, this->square_size) * 8.0f);
}

bool Board::getFigure(pos_t pos)
{
	return this->locations[pos.y][pos.x];
}

bool Board::getFigure(pos_t pos, Figure& figure)
{
	figure = this->figures[pos.y][pos.x];
	return this->locations[pos.y][pos.x];
}

void Board::setFigure(pos_t pos, Figure figure)
{
	this->figures[pos.y][pos.x] = figure;
	this->locations[pos.y][pos.x] = true;
}

void Board::moveFigure(pos_t from, pos_t to)
{
	Figure figure = this->figures[from.y][from.x];

	if (figure == wK)
		this->white_king_pos = to;
	else if (figure == bK)
		this->black_king_pos = to;

	this->figures[to.y][to.x] = figure;

	this->locations[from.y][from.x] = false;
	this->locations[to.y][to.x] = true;
}

void Board::delFigure(pos_t pos)
{
	this->locations[pos.y][pos.x] = false;
}

pos_t Board::findKing(bool color)
{
	if (color)
		return this->white_king_pos;
	return this->black_king_pos;
}

Board Board::copy()
{
	Board copy;

	for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			copy.figures[y][x] = this->figures[y][x];
			copy.locations[y][x] = this->locations[y][x];
		}
	}

	copy.white_king_pos = this->white_king_pos;
	copy.black_king_pos = this->black_king_pos;

	return copy;
}

void Board::render(sf::RenderWindow& window)
{
	window.draw(this->lines);
	window.draw(this->quads);

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (this->locations[y][x])
			{
				this->sprites[this->figures[y][x]].setPosition(this->pos + sf::Vector2f(x, 7 - y) * this->square_size);
				window.draw(this->sprites[this->figures[y][x]]);
			}
		}
	}
}