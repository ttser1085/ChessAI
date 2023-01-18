#include "ChessHandler.h"

bool color(Figure figure)
{
	return (int)figure < 6;
}

uint8_t fscore(Figure figure)
{
	switch (figure % 6)
	{
	case 0:
		return 255;
	case 1:
		return 9;
	case 2:
		return 5;
	case 3:
		return 3;
	case 4:
		return 3;
	case 5:
		return 1;
	}
}

char to_char(Figure figure)
{
	switch (figure % 6)
	{
	case 0:
		return 'K';
	case 1:
		return 'Q';
	case 2:
		return 'R';
	case 3:
		return 'B';
	case 4:
		return 'N';
	case 5:
		return 'P';
	}
}

std::string to_string(Ending ending)
{
	switch (ending)
	{
	case WHITE_WIN:
		return "White win";
	case BLACK_WIN:
		return "Black win";
	case STALEMATE:
		return "Stalemate";
	case IMPOSSIBILITY:
		return "Impossibility of checkmate";
	default:
		return "";
	}
}

std::string to_string(Move move)
{
	std::string str;

	if (move.short_castling)
		str = "0-0";
	else if (move.long_castling)
		str = "0-0-0";
	else
	{
		if (move.figure != wP && move.figure != bP)
			str += to_char(move.figure);

		str += std::string(1, (char)(move.from.x + 'a')) + std::to_string(move.from.y + 1) + std::string(1, (move.capture ? 'x' : '-')) + std::string(1, (char)(move.to.x + 'a')) + std::to_string(move.to.y + 1);
	}

	if (move.checkmate)
		str += '#';
	else if (move.draw)
		str += '=';
	else if (move.check)
		str += '+';

	return str;
}

bool operator== (Move move1, Move move2)
{
	return (move1.figure == move2.figure) && (move1.from == move2.from) && (move1.to == move2.to);
}

//------------------------------------------------------------------------------------

Move::Move(Figure figure, pos_t from, pos_t to)
{
	this->figure = figure;
	this->from = from;
	this->to = to;
}

//------------------------------------------------------------------------------------

void CastlingData::updateData(Move& move)
{
	if (move.short_castling || move.long_castling)
	{
		if (color(move.figure))
			this->white_castling_done = true;
		else
			this->black_castling_done = true;
	}

	if (move.figure == wK)
		this->wk_moved = true;
	else if (move.figure == bK)
		this->bk_moved = true;

	if (move.from == pos_t(7, 0))
		this->white_rook_moved[0] = true;
	else if (move.from == pos_t(0, 0))
		this->white_rook_moved[1] = true;
	else if (move.from == pos_t(7, 7))
		this->black_rook_moved[0] = true;
	else if (move.from == pos_t(0, 7))
		this->black_rook_moved[1] = true;
}

//------------------------------------------------------------------------------------

ChessHandler::ChessHandler(sf::Vector2f board_pos, float square_size)
{
	this->data.board = Board(board_pos, square_size);

	this->turn = true;
	this->is_selected = false;

	this->active = true;

	this->player_color = true;
	this->frame_paused = false;

	ChessHandler::findAttackedPoses(&this->data);

	this->font.loadFromFile("C:/Windows/Fonts/Arial.ttf");

	this->moves_text.setFont(this->font);
	this->moves_text.setPosition(700.0f, 20.0f);
	this->moves_text.setFillColor(sf::Color::Black);
	this->moves_text.setCharacterSize(22);

	this->ending_text.setFont(this->font);
	this->ending_text.setPosition(50.0f, 735.0f);
	this->ending_text.setFillColor(sf::Color::Black);
	this->ending_text.setCharacterSize(38);
}

void ChessHandler::checkEvents(sf::Event& event, sf::RenderWindow& window)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		sf::Vector2f mpos = sf::Vector2f(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);

		if (event.mouseButton.button == sf::Mouse::Left)
		{
			if (this->data.board.getRect().contains(mpos) && this->active && this->turn == this->player_color)
				this->boardClick(mpos);
		}
	}
}

void ChessHandler::move(Board* board, Move move)
{
	board->moveFigure(move.from, move.to);

	bool clr = color(move.figure);

	if (move.en_passant)
		board->delFigure(pos_t(move.to.x, move.to.y - (color(move.figure) * 2 - 1)));
	else if (move.promotion)
		board->setFigure(move.to, (Figure)(move.figure - 4));
	else if (move.short_castling)
		board->moveFigure(pos_t(7, clr ? 0 : 7), pos_t(5, clr ? 0 : 7));
	else if (move.long_castling)
		board->moveFigure(pos_t(0, clr ? 0 : 7), pos_t(3, clr ? 0 : 7));
}

float ChessHandler::calcScore(ChessGameData* data, bool active, Ending ending)
{
	float score = 0.0f;

	if (active)
	{
		for (uint8_t y = 0; y < 8; y++)
		{
			for (uint8_t x = 0; x < 8; x++)
			{
				Figure figure;
				if (data->board.getFigure(pos_t(x, y), figure))
				{
					score += (color(figure) ? fscore(figure) : -fscore(figure)) * 11.2f;

					if (figure == wP)
						score += y * y / 17.2f;
					else if (figure == bP)
						score -= (7 - y) * (7 - y) / 17.2f;
				}
			}
		}

		int wcount = data->white_attacked_poses.size();
		int bcount = data->black_attacked_poses.size();

		score += wcount * sqrt(wcount) / 4.7f;
		score -= bcount * sqrt(bcount) / 4.7f;

		score += (float)data->castling.white_castling_done / sqrt(data->moves.size()) * 11.0f;
		score -= (float)data->castling.black_castling_done / sqrt(data->moves.size()) * 11.0f;

		if (score > 200.0f)
			score = 200.0f;
		else if (score < -200.0f)
			score = -200.0f;
	}
	else
	{
		if (ending == WHITE_WIN)
			score = 300.0f;
		else if (ending == BLACK_WIN)
			score = -300.0f;
		else
			score = 0.0f;
	}

	return score;
}

bool ChessHandler::checkMove(ChessGameData* data, Move& move, bool conside_defend)
{
	if ((move.from.x >= 8) ||
		(move.from.y >= 8) ||
		(move.to.x >= 8) ||
		(move.to.y >= 8))
		return false;

	Figure target;
	move.capture = data->board.getFigure(move.to, target);

	if (move.capture)
	{
		if (color(target) == color(move.figure) && !conside_defend)
			return false;
	}

	int8_t dx = move.to.x - move.from.x;
	int8_t dy = move.to.y - move.from.y;

	if (dx == 0 && dy == 0)
		return false;

	if (move.figure == wK || move.figure == bK)
	{
		if (std::max(std::abs(dx), std::abs(dy)) == 1)
			return true;
		if (dy == 0 && !data->prev_move.check)
		{
			if (color(move.figure))
			{
				if (dx == 2)
				{
					if (!data->castling.wk_moved && !data->castling.white_rook_moved[0] && !data->castling.white_castling_done &&
						!data->board.getFigure(pos_t(5, 0)) && !data->board.getFigure(pos_t(6, 0)) &&
						std::find(data->black_attacked_poses.begin(), data->black_attacked_poses.end(), pos_t(5, 0)) == data->black_attacked_poses.end())
					{
						move.short_castling = true;
						return true;
					}
					return false;
				}
				else if (dx == -2)
				{
					if (!data->castling.wk_moved && !data->castling.white_rook_moved[1] && !data->castling.white_castling_done &&
						!data->board.getFigure(pos_t(3, 0)) && !data->board.getFigure(pos_t(2, 0)) && !data->board.getFigure(pos_t(1, 0)) &&
						std::find(data->black_attacked_poses.begin(), data->black_attacked_poses.end(), pos_t(3, 0)) == data->black_attacked_poses.end())
					{
						move.long_castling = true;
						return true;
					}
					return false;
				}
			}
			else
			{
				if (dx == 2)
				{
					if (!data->castling.bk_moved && !data->castling.black_rook_moved[0] && !data->castling.black_castling_done &&
						!data->board.getFigure(pos_t(5, 7)) && !data->board.getFigure(pos_t(6, 7)) &&
						std::find(data->white_attacked_poses.begin(), data->white_attacked_poses.end(), pos_t(5, 7)) == data->white_attacked_poses.end())
					{
						move.short_castling = true;
						return true;
					}
					return false;
				}
				else if (dx == -2)
				{
					if (!data->castling.bk_moved && !data->castling.black_rook_moved[1] && !data->castling.black_castling_done &&
						!data->board.getFigure(pos_t(3, 7)) && !data->board.getFigure(pos_t(2, 7)) && !data->board.getFigure(pos_t(1, 7)) &&
						std::find(data->white_attacked_poses.begin(), data->white_attacked_poses.end(), pos_t(3, 7)) == data->white_attacked_poses.end())
					{
						move.long_castling = true;
						return true;
					}
					return false;
				}
			}
		}
	}
	else if (move.figure == wQ || move.figure == bQ)
	{
		/*if ((dx == 0) || (dy == 0) ||
			(std::abs(dx) == std::abs(dy)))
		{
			int8_t stepx = (dx == 0 ? 0 : (dx / std::abs(dx)));
			int8_t stepy = (dy == 0 ? 0 : (dy / std::abs(dy)));

			pos_t pos;
			pos.x = move.from.x + stepx;
			pos.y = move.from.y + stepy;
			while (pos != move.to)
			{
				if (data->board.getFigure(pos))
					return false;

				pos.x += stepx;
				pos.y += stepy;
			}
			return true;
		}
		return false;*/

		int8_t stepx = (dx == 0 ? 0 : (dx / std::abs(dx)));
		int8_t stepy = (dy == 0 ? 0 : (dy / std::abs(dy)));

		pos_t pos;
		pos.x = move.from.x + stepx;
		pos.y = move.from.y + stepy;
		while (pos != move.to)
		{
			if (data->board.getFigure(pos))
				return false;

			pos.x += stepx;
			pos.y += stepy;
		}
		return true;
	}
	else if (move.figure == wR || move.figure == bR)
	{

		/*if ((dx == 0) || (dy == 0))
		{
			int8_t stepx = (dx == 0 ? 0 : (dx / std::abs(dx)));
			int8_t stepy = (dy == 0 ? 0 : (dy / std::abs(dy)));

			pos_t pos;
			pos.x = move.from.x + stepx;
			pos.y = move.from.y + stepy;
			while (pos != move.to)
			{
				if (data->board.getFigure(pos))
					return false;

				pos.x += stepx;
				pos.y += stepy;
			}
			return true;
		}
		return false;*/

		int8_t stepx = (dx == 0 ? 0 : (dx / std::abs(dx)));
		int8_t stepy = (dy == 0 ? 0 : (dy / std::abs(dy)));

		pos_t pos;
		pos.x = move.from.x + stepx;
		pos.y = move.from.y + stepy;
		while (pos != move.to)
		{
			if (data->board.getFigure(pos))
				return false;

			pos.x += stepx;
			pos.y += stepy;
		}
		return true;
	}
	else if (move.figure == wB || move.figure == bB)
	{

		/*if ((std::abs(dx) == std::abs(dy)))
		{
			int8_t stepx = (dx == 0 ? 0 : (dx / std::abs(dx)));
			int8_t stepy = (dy == 0 ? 0 : (dy / std::abs(dy)));

			pos_t pos;
			pos.x = move.from.x + stepx;
			pos.y = move.from.y + stepy;
			while (pos != move.to)
			{
				if (data->board.getFigure(pos))
					return false;

				pos.x += stepx;
				pos.y += stepy;
			}
			return true;
		}
		return false;*/

		int8_t stepx = (dx == 0 ? 0 : (dx / std::abs(dx)));
		int8_t stepy = (dy == 0 ? 0 : (dy / std::abs(dy)));

		pos_t pos;
		pos.x = move.from.x + stepx;
		pos.y = move.from.y + stepy;
		while (pos != move.to)
		{
			if (data->board.getFigure(pos))
				return false;

			pos.x += stepx;
			pos.y += stepy;
		}
		return true;
	}
	else if (move.figure == wN || move.figure == bN)
	{
		return true /*dx * dx + dy * dy == 5*/;
	}
	else if (move.figure == wP)
	{
		move.promotion = (move.to.y == 7);

		if (move.capture || conside_defend)
		{
			return std::abs(dx) == 1 && dy == 1;
		}
		else
		{
			if (data->prev_move.figure == bP && data->prev_move.to - data->prev_move.from == pos_t(0, -2) && std::abs(dx) == 1 && dy == 1 && move.to == pos_t(data->prev_move.to.x, data->prev_move.to.y + 1))
			{
				move.capture = true;
				move.en_passant = true;
				return true;
			}

			if (dx != 0)
				return false;

			if (move.from.y == 1)
			{
				return (dy == 1) || (dy == 2 && !data->board.getFigure(pos_t(move.from.x, 2)) && !data->board.getFigure(pos_t(move.from.x, 3)));
			}
			else
			{

				return dy == 1;
			}
		}
	}
	else if (move.figure == bP)
	{
		move.promotion = (move.to.y == 0);

		if (move.capture || conside_defend)
		{
			return std::abs(dx) == 1 && dy == -1;
		}
		else
		{
			if (data->prev_move.figure == wP && data->prev_move.to - data->prev_move.from == pos_t(0, 2) && std::abs(dx) == 1 && dy == -1 && move.to == pos_t(data->prev_move.to.x, data->prev_move.to.y - 1))
			{
				move.capture = true;
				move.en_passant = true;
				return true;
			}

			if (dx != 0)
				return false;

			if (move.from.y == 6)
			{
				return (dy == -1) || (dy == -2 && !data->board.getFigure(pos_t(move.from.x, 5)) && !data->board.getFigure(pos_t(move.from.x, 4)));
			}
			else
			{
				return dy == -1;
			}
		}
	}
	return false;
}

bool ChessHandler::checkCheck(ChessGameData* data, bool& wk_check, bool& bk_check)
{
	pos_t wpos = data->board.findKing(true);
	pos_t bpos = data->board.findKing(false);

	wk_check = std::find(data->black_attacked_poses.begin(), data->black_attacked_poses.end(), wpos) != data->black_attacked_poses.end();
	bk_check = std::find(data->white_attacked_poses.begin(), data->white_attacked_poses.end(), bpos) != data->white_attacked_poses.end();
	
	return wk_check || bk_check;
}

bool ChessHandler::isAvailable(ChessGameData* data, Move& move)
{
	ChessGameData temp = *data;
	temp.board = data->board.copy();
	temp.prev_move = data->prev_move;
	temp.castling = data->castling;

	ChessHandler::move(&temp.board, move);
	ChessHandler::findAttackedPoses(&temp);

	bool wk_check, bk_check;
	move.check = ChessHandler::checkCheck(&temp, wk_check, bk_check);

	if (move.check)
	{
		if ((color(move.figure) && wk_check) || (!color(move.figure) && bk_check))
			return false;
	}
	return true;
}

std::vector<Move> ChessHandler::findPossibleMoves(ChessGameData* data, pos_t pos, bool conside_defend)
{
	std::vector<Move> moves;

	Figure figure;
	data->board.getFigure(pos, figure);

	/*for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			Move move(figure, pos, pos_t(x, y));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}*/

	if (figure == wP)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			Move move(figure, pos, pos_t(pos.x + i % 3 - 1, pos.y + i / 3 + 1));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}
	else if (figure == bP)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			Move move(figure, pos, pos_t(pos.x + i % 3 - 1, pos.y + i / 3 - 2));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}
	else if (figure == wB || figure == bB)
	{
		for (int8_t i = -7; i < 8; i++)
		{
			Move move(figure, pos, pos_t(pos.x + i, pos.y + i));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}

			move = Move(figure, pos, pos_t(pos.x + i, pos.y - i));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}
	else if (figure == wN || figure == bN)
	{
		pos_t poses[] = 
		{
			pos_t(pos.x + 1, pos.y + 2),
			pos_t(pos.x + 2, pos.y + 1),

			pos_t(pos.x - 1, pos.y + 2),
			pos_t(pos.x - 2, pos.y + 1),

			pos_t(pos.x + 1, pos.y - 2),
			pos_t(pos.x + 2, pos.y - 1),

			pos_t(pos.x - 1, pos.y - 2),
			pos_t(pos.x - 2, pos.y - 1)
		};

		for (uint8_t i = 0; i < 8; i++)
		{
			Move move(figure, pos, poses[i]);
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}
	else if (figure == wR || figure == bR)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			Move move(figure, pos, pos_t(x, pos.y));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}

		for (uint8_t y = 0; y < 8; y++)
		{
			Move move(figure, pos, pos_t(pos.x, y));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}
	else if (figure == wQ || figure == bQ)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			Move move(figure, pos, pos_t(x, pos.y));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}

		for (uint8_t y = 0; y < 8; y++)
		{
			Move move(figure, pos, pos_t(pos.x, y));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}

		for (int8_t i = -7; i < 8; i++)
		{
			Move move(figure, pos, pos_t(pos.x + i, pos.y + i));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}

			move = Move(figure, pos, pos_t(pos.x + i, pos.y - i));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}
	}
	else if (figure == wK || figure == bK)
	{
		for (uint8_t i = 0; i < 9; i++)
		{
			Move move(figure, pos, pos_t(pos.x + i % 3 - 1, pos.y + i / 3 - 1));
			if (conside_defend)
			{
				if (checkMove(data, move, conside_defend))
					moves.push_back(move);
			}
			else
			{
				if (checkMove(data, move, conside_defend) && isAvailable(data, move))
					moves.push_back(move);
			}
		}

		Move move(figure, pos, pos + pos_t(2, 0));
		if (conside_defend)
		{
			if (checkMove(data, move, conside_defend))
				moves.push_back(move);
		}
		else
		{
			if (checkMove(data, move, conside_defend) && isAvailable(data, move))
				moves.push_back(move);
		}

		move = Move(figure, pos, pos - pos_t(2, 0));
		if (conside_defend)
		{
			if (checkMove(data, move, conside_defend))
				moves.push_back(move);
		}
		else
		{
			if (checkMove(data, move, conside_defend) && isAvailable(data, move))
				moves.push_back(move);
		}
	}

	return moves;
}

void ChessHandler::findAttackedPoses(ChessGameData* data)
{
	data->white_attacked_poses.clear();
	data->black_attacked_poses.clear();

	for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			Figure figure;
			if (data->board.getFigure(pos_t(x, y), figure))
			{
				std::vector<Move> moves = findPossibleMoves(data, pos_t(x, y), true);

				if (color(figure))
				{
					for (auto& move : moves)
					{
						pos_t pos = move.to;

						if (std::find(data->white_attacked_poses.begin(), data->white_attacked_poses.end(), pos) == data->white_attacked_poses.end())
							data->white_attacked_poses.push_back(pos);
					}
				}
				else
				{
					for (auto& move : moves)
					{
						pos_t pos = move.to;

						if (std::find(data->black_attacked_poses.begin(), data->black_attacked_poses.end(), pos) == data->black_attacked_poses.end())
							data->black_attacked_poses.push_back(pos);
					}
				}
			}
		}
	}
}

bool ChessHandler::checkEnding(ChessGameData* data, Ending& ending)
{
	unsigned int mnum = 0;

	bool imp = true;
	bool wb[] { false, false };
	bool bb[] { false, false };

	uint8_t wknights = 0;
	uint8_t bknights = 0;

	for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			Figure figure;
			if (data->board.getFigure(pos_t(x, y), figure))
			{
				uint8_t f = figure % 6;

				if (f == 5 || f == 1 || f == 2)
					imp = false;

				if (figure == wB)
					wb[(x % 2 + y % 2 + 1) % 2] = true;
				else if (figure == bB)
					bb[(x % 2 + y % 2 + 1) % 2] = true;
				else if (figure == wN)
					wknights++;
				else if (figure == bN)
					bknights++;

				if (color(figure) != color(data->prev_move.figure))
					mnum += findPossibleMoves(data, pos_t(x, y)).size();
			}
		}
	}

	if (data->prev_move.check)
	{
		if (mnum == 0)
		{
			ending = (Ending)((uint8_t)(!color(data->prev_move.figure)));
			return true;
		}
	}
	else
	{
		if (mnum == 0)
		{
			ending = STALEMATE;
			return true;
		}
		if (imp)
		{
			bool wcw = false;
			bool bcw = false;

			if (wknights != 0)
			{
				if (wknights == 1)
					wcw = (wb[0] || wb[1]);
				else
					wcw = true;
			}
			else
				wcw = (wb[0] && wb[1]);

			if (bknights != 0)
			{
				if (bknights == 1)
					bcw = (bb[0] || bb[1]);
				else
					bcw = true;
			}
			else
				bcw = (bb[0] && bb[1]);

			if (!wcw && !bcw)
			{
				ending = IMPOSSIBILITY;
				return true;
			}
		}
	}

	return false;
}

void ChessHandler::select(pos_t pos)
{
	this->selected_possible_moves.clear();

	this->is_selected = true;
	this->selected_pos = pos;
	this->selected_possible_moves = this->findPossibleMoves(&this->data, pos);
}

void ChessHandler::unselect()
{
	this->is_selected = false;
	this->selected_possible_moves.clear();
}

void ChessHandler::update()
{
	if (this->turn != this->player_color && this->frame_paused && this->active)
	{
		ChessGameData cdata = this->data;
		cdata.board = this->data.board.copy();
		cdata.prev_move = this->data.prev_move;
		cdata.castling = this->data.castling;
		cdata.white_attacked_poses.resize(this->data.white_attacked_poses.size());
		cdata.black_attacked_poses.resize(this->data.black_attacked_poses.size());
		cdata.moves = this->data.moves;

		auto t1 = std::chrono::high_resolution_clock::now();

		Move ai_move = ChessHandler::minimax(&cdata, 2, turn, -5000.0f, 5000.0f).move;

		auto t2 = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		std::cout << duration << '\n';

		this->update(ai_move);
	}

	this->frame_paused = true;
}

void ChessHandler::update(Move move)
{
	ChessHandler::move(&this->data.board, move);

	this->data.moves.push_back(move);
	this->data.prev_move = move;

	this->turn = !this->turn;

	this->unselect();

	ChessHandler::findAttackedPoses(&this->data);

	this->data.castling.updateData(move);

	this->active = !ChessHandler::checkEnding(&this->data, this->ending);

	if (!this->active)
	{
		if (this->ending == WHITE_WIN || this->ending == BLACK_WIN)
		{
			this->data.moves[this->data.moves.size() - 1].checkmate = true;
			this->data.prev_move.checkmate = true;
		}
		else
		{
			this->data.moves[this->data.moves.size() - 1].draw = true;
			this->data.prev_move.draw = true;
		}
	}

	this->score = ChessHandler::calcScore(&this->data, this->active, this->ending);
}

EvalMove ChessHandler::minimax(ChessGameData* cdata, int depth, bool turn, float alpha, float beta)
{
	EvalMove best;

	Ending ending = IMPOSSIBILITY;
	if (depth == 0)
	{
		best.score = calcScore(cdata, true, ending);
		return best;
	}

	if (checkEnding(cdata, ending))
	{
		best.score = calcScore(cdata, false, ending);
		return best;
	}

	std::vector<Move> moves;

	for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			Figure figure;
			if (cdata->board.getFigure(pos_t(x, y), figure) && color(figure) == turn)
			{
				std::vector<Move> pmoves = ChessHandler::findPossibleMoves(cdata, pos_t(x, y));
				moves.insert(moves.end(), pmoves.begin(), pmoves.end());
			}
		}
	}

	//best.move = moves[0];

	if (turn)
	{
		best.score = -5000.0f;

		for (auto& move : moves)
		{
			EvalMove emove;
			emove.move = move;

			ChessGameData* temp = new ChessGameData;
			temp->board = cdata->board.copy();
			temp->moves = cdata->moves;
			temp->castling = cdata->castling;

			ChessHandler::move(&temp->board, emove.move);

			ChessHandler::findAttackedPoses(temp); // доделать типа функции update
			temp->castling.updateData(move);

			emove.score = minimax(temp, depth - 1, false, alpha, beta).score;
			delete temp;

			if (emove.score >= best.score)
				best = emove;

			/*if (emove.score < alpha)
			{
				return best;
			}
			if (emove.score < beta)
			{
				beta = emove.score;
			}*/

			//------------------------------

			if (emove.score > beta)
			{
				return best;
			}
			if (emove.score > alpha)
			{
				alpha = emove.score;
			}

			/*alpha = std::max(alpha, best.score);

			if (beta <= alpha)
				return best;*/
		}
	}
	else
	{
		best.score = 5000.0f;

		for (auto& move : moves)
		{
			EvalMove emove;
			emove.move = move;

			ChessGameData* temp = new ChessGameData;
			temp->board = cdata->board.copy();
			temp->moves = cdata->moves;
			temp->castling = cdata->castling;

			ChessHandler::move(&temp->board, emove.move);

			ChessHandler::findAttackedPoses(temp); // доделать типа функции update
			temp->castling.updateData(move);

			emove.score = minimax(temp, depth - 1, true, alpha, beta).score;
			delete temp;

			if (emove.score <= best.score)
				best = emove;

			/*if (emove.score > beta)
			{
				return best;
			}
			if (emove.score > alpha)
			{
				alpha = emove.score;
			}*/

			//------------------------------

			if (emove.score < alpha)
			{
				return best;
			}
			if (emove.score < beta)
			{
				beta = emove.score;
			}

			/*beta = std::min(beta, best.score);

			if (beta <= alpha)
				return best;*/
		}
	}

	return best;
}

void ChessHandler::boardClick(sf::Vector2f mpos)
{
	pos_t pos;
	pos.x = (mpos.x - this->data.board.getRect().left) / this->data.board.getSquareSize();
	pos.y = 8 - (mpos.y - this->data.board.getRect().top) / this->data.board.getSquareSize();

	if (this->is_selected)
	{
		Figure figure, selected_figure;
		this->data.board.getFigure(this->selected_pos, selected_figure);
		bool attack = this->data.board.getFigure(pos, figure);

		Move move(selected_figure, this->selected_pos, pos);
		auto iterator = std::find(this->selected_possible_moves.begin(), this->selected_possible_moves.end(), move);

		if (iterator != this->selected_possible_moves.end())
		{
			this->update(this->selected_possible_moves[iterator - this->selected_possible_moves.begin()]);
			this->frame_paused = false;
		}
		else if (attack && color(figure) == this->turn)
			this->select(pos);
		else
			this->unselect();
	}
	else
	{
		Figure figure;
		if (this->data.board.getFigure(pos, figure) && color(figure) == this->turn)
			this->select(pos);
	}
}

void ChessHandler::render(sf::RenderWindow& window)
{
	this->data.board.render(window);

	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(this->data.board.getSquareSize(), this->data.board.getSquareSize()));
	rect.setFillColor(sf::Color(0, 0, 0, 0));
	rect.setPosition(this->data.board.getRect().left + this->data.board.getSquareSize() * this->selected_pos.x, this->data.board.getRect().top + this->data.board.getSquareSize() * (7 - this->selected_pos.y));
	rect.setOutlineColor(sf::Color::Red);
	rect.setOutlineThickness(2.0f);
	if (this->is_selected)
		window.draw(rect);

	sf::CircleShape circle;
	circle.setRadius(this->data.board.getSquareSize() * 0.2f);
	circle.setFillColor(sf::Color(255, 0, 0, 200));

	for (auto& move : this->selected_possible_moves)
	{
		pos_t pos = move.to;
		circle.setPosition(this->data.board.getRect().left + pos.x * this->data.board.getSquareSize(), this->data.board.getRect().top + (7 - pos.y) * this->data.board.getSquareSize());
		circle.move(this->data.board.getSquareSize() / 2 - circle.getRadius(), this->data.board.getSquareSize() / 2 - circle.getRadius());
		window.draw(circle);
	}

	sf::RectangleShape score_rect;
	score_rect.setSize(sf::Vector2f(200.0f, 7.0f));
	score_rect.setFillColor(sf::Color::White);
	score_rect.setPosition(50.0f, 720.0f);
	score_rect.setOutlineColor(sf::Color::Black);
	score_rect.setOutlineThickness(1.5f);

	window.draw(score_rect);

	sf::RectangleShape black_score_rect;
	black_score_rect.setSize(sf::Vector2f(-this->score * 0.5f + 100, 7.0f));
	black_score_rect.setFillColor(sf::Color::Black);
	black_score_rect.setPosition(50.0f, 720.0f);

	window.draw(black_score_rect);

	std::string str;
	for (int i = 0; i < this->data.moves.size(); i += 2)
	{
		if (i + 1 < this->data.moves.size())
			str += std::to_string(i / 2 + 1) + ".  " + to_string(this->data.moves[i]) + "  " + to_string(this->data.moves[i + 1]) + '\n';
		else
			str += std::to_string(i / 2 + 1) + ".  " + to_string(this->data.moves[i]);
	}

	this->moves_text.setString(str);
	window.draw(this->moves_text);

	if (!this->active)
		this->ending_text.setString(to_string(this->ending));
	window.draw(this->ending_text);
}