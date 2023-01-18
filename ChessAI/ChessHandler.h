#pragma once

#include "Board.h"
#include <chrono>

struct Move
{
	Move(Figure figure, pos_t from, pos_t to);

	Figure figure;
	pos_t from;
	pos_t to;

	bool capture = false;
	bool en_passant = false;
	bool promotion = false;
	bool short_castling = false;
	bool long_castling = false;

	bool check = false;
	bool checkmate = false;
	bool draw = false;

	friend bool operator== (Move move1, Move move2);
};

struct EvalMove
{
	float score;
	Move move = Move(wK, pos_t(0, 0), pos_t(0, 0));
};

struct CastlingData
{
	bool white_castling_done = false;
	bool black_castling_done = false;

	bool wk_moved = false;
	bool white_rook_moved[2] = { false, false };

	bool bk_moved = false;
	bool black_rook_moved[2] = { false, false };

	void updateData(Move& move);
};

struct ChessGameData
{
	Board board;
	std::vector<Move> moves;
	Move prev_move = Move(wK, pos_t(0, 0), pos_t(0, 0));

	CastlingData castling;

	std::vector<pos_t> white_attacked_poses;
	std::vector<pos_t> black_attacked_poses;
};

enum Ending : uint8_t
{
	WHITE_WIN,
	BLACK_WIN,
	STALEMATE,
	IMPOSSIBILITY
};

class ChessHandler
{
public:
	ChessHandler(sf::Vector2f board_pos, float square_size);

	void checkEvents(sf::Event& event, sf::RenderWindow& window);

	void update();
	void render(sf::RenderWindow& window);

private:
	ChessGameData data;

	Ending ending;
	bool active;

	bool turn;
	bool player_color;
	bool frame_paused;

	float score;

	bool is_selected;
	pos_t selected_pos;
	std::vector<Move> selected_possible_moves;

	sf::Font font;
	sf::Text moves_text;
	sf::Text ending_text;

	static void move(Board* board, Move move);

	static float calcScore(ChessGameData* data, bool active, Ending ending);

	static bool checkMove(ChessGameData* data, Move& move, bool conside_defend);
	static bool checkCheck(ChessGameData* data, bool& wk_check, bool& bk_check);
	static bool isAvailable(ChessGameData* data, Move& move);

	static std::vector<Move> findPossibleMoves(ChessGameData* data, pos_t pos, bool conside_defend = false);
	static void findAttackedPoses(ChessGameData* data);

	static bool checkEnding(ChessGameData* data, Ending& ending);

	void select(pos_t pos);
	void unselect();

	void update(Move move);

	static EvalMove minimax(ChessGameData* data, int depth, bool turn, float alpha, float beta);

	void boardClick(sf::Vector2f mpos);
};
