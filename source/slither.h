#pragma once

#include <vector>
#include <array>
#include <string>

using Player = int;
using Action = int;

constexpr int kBoardSize = 5;
constexpr int kNumOfGrids = kBoardSize * kBoardSize;
constexpr int empty_index = kNumOfGrids; // 25 in 5 * 5 board
constexpr std::array<int, 8> MoveDirection = {-kBoardSize-1, -kBoardSize, -kBoardSize+1, -1, +1, +kBoardSize-1, +kBoardSize, +kBoardSize+1}; //ul, u, ur, l, r, bl, b, br

/** Maximum number of steps */
constexpr int kMaxTurn = INT16_MAX;

class State {
public:
    State() : turn_(0), skip_(0), winner_(-1) {
        std::fill(std::begin(board_), std::end(board_), EMPTY);
        history_.clear();
    }
    State(const State &) = default;
    ~State() = default;

    Player current_player() const;
    Player get_winner() const;
    std::string get_board() const;
    void set_board(std::string);
    int get_turn();
    std::vector<Action> legal_actions() const;
    void apply_action(const Action &);
    bool is_terminal() const;
    std::string to_string() const;
    std::string action_to_string(const Action &) const;
    std::vector<Action> string_to_action(const std::string &) const;

private:
    std::vector<int> get_restrictions(const Action src, const Action action, const Player player, std::array<short, kNumOfGrids>* bptr = nullptr) const;
    bool is_selecting_valid(const Action action, const Player player) const;
    bool is_moving_valid(const Action src, const Action action, const Player player, std::array<short, kNumOfGrids>* bptr = nullptr) const;
    bool is_placing_valid(const Action action, const Player player, std::array<short, kNumOfGrids>* bptr = nullptr) const;
    bool is_placing_valid(const Player player, std::array<short, kNumOfGrids>* bptr = nullptr) const;
    bool is_legal_action(const Action &action) const;
    bool have_move(const Action &action) const;
    bool have_win(const Action &action) const;
    bool is_valid(const Action &action) const;
    bool is_empty_valid(const Action &action) const;

    /** 0 -> B (black)
     * 1 -> W (white)
     * 2 -> E (empty)*/
    enum Piece { BLACK, WHITE, EMPTY };

    /** index
     * 0  1  2  3  4  
     * 5  6  7  8  9
     * 10 11 12 13 14
     * 15 16 17 18 19
     * 20 21 22 23 24*/
    std::array<short, kNumOfGrids> board_;
    std::vector<Action> history_;
    int turn_;
    int skip_;

    Player winner_;
};