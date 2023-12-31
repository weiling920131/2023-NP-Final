#include "slither.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <stack>
#include <iostream>
#include <stdio.h>

Player State::current_player() const {
  	return (Player)((turn_ % 6) / 3);
}
Player State::get_winner() const {
  	return winner_;
}
std::string State::get_board() const {
	std::string board = "";
	for (int i=0; i<kNumOfGrids; i++) {
		board += std::to_string(board_[i]);
	}
	return board;
}
void State::set_board(std::string input) {
	char* board;
	int turn;
	sscanf(input.c_str(), "%s %d\n", board, &turn);
	for (int i=0; i<kNumOfGrids; i++) {
		board_[i] = board[i] - '0';
	}
	turn_ = turn;
	return;
}
int State::get_turn() {
	return turn_;
}
std::vector<Action> State::legal_actions() const {
	std::vector<Action> actions;
	if(turn_ % 3 == 0) {// empty
		// actions.push_back(empty_index);
	}
	else if(turn_ % 3 == 1 && skip_ == 1){
		actions.push_back(empty_index);
		return actions;
	}
	for (uint32_t i = 0; i <= kNumOfGrids; i++) {
		if (is_legal_action(i)) {
			actions.push_back(i);
		}
	}
	if (actions.empty()) {
		actions.push_back(empty_index);
	}

	return actions;
}
void State::apply_action(const Action &action) {
    //std::cout << "current turn:" << turn_ << "\n"; 
    //std::cout << "action:" << action << '\n';
    if (turn_ % 3 == 0) {  // choose
		if (action == empty_index) skip_ = 1;
		++turn_;
		history_.push_back(action);
		//std::cout << "success\n";
	} 
	else if (turn_ % 3 == 1) { // move
		Player player = current_player();
		const int src = history_[history_.size() - 1];
		if(src != empty_index && action != empty_index){
			std::swap(board_[src], board_[action]);
			board_[src] = EMPTY;
		} else if (action == empty_index) {
			skip_ = 1;
			history_[history_.size() - 1] = empty_index;
		}
		++turn_;
		history_.push_back(action);
	} 
	else {  
		// place new piece
		Player player = current_player();
		board_[action] = player;
		++turn_;
		history_.push_back(action);
		// check if valid
		if (skip_ == 1) { // only play a new piece
			skip_ = 0;
			if (!is_valid(action)) {
				if(player == BLACK)
					winner_ = WHITE;
				else 
					winner_ = BLACK;
			}
			// check if win
			if(winner_ == -1 && have_win(action))
				winner_ = player;
		}
		else {
			const int src_empty = history_[history_.size() - 3]; // piece -> empty
			const int src_new_piece = history_[history_.size() - 2]; // empty -> piece
			if (!is_valid(action) || !is_valid(src_new_piece) || (action != src_empty && !is_empty_valid(src_empty) ) ) {
				if(player == BLACK)
					winner_ = WHITE;
				else 
					winner_ = BLACK;
			}
			// check if win
			if(winner_ == -1){
				if(have_win(action) || have_win(src_new_piece))
					winner_ = player;
			}
		}
	}
}
bool State::is_terminal() const {
  return (winner_ != -1);
}
std::string State::to_string() const {
    std::stringstream ss;
    const std::vector<std::string> chess{"x", "o", "·"};

    for (int i = 0; i < kBoardSize; ++i) {
        ss << std::setw(2) << std::setfill(' ') << kBoardSize - i;
        for (int j = 0; j < kBoardSize; ++j) {
            ss << ' ' << chess[board_[i * kBoardSize + j]];
        }
        ss << std::endl;
    }
    ss << "  ";
    for (int i = 0; i < kBoardSize; ++i) ss << ' ' << static_cast<char>('A' + i);
    return ss.str();
}

std::vector<int> State::get_restrictions(const Action src, const Action action, const Player player, std::array<short, kNumOfGrids>* bptr) const {
	// return empty vector if there is no restriction
	const std::vector<int> kValid = {};
	// empty_index is used to mark the existence of restriction
	std::vector<int> constrained_points = {empty_index};
	const std::vector<int>& kInvalid = constrained_points;

	if (src < 0 || src > kNumOfGrids) return kInvalid;
	if (action < 0 || action >= kNumOfGrids) return kInvalid;


	std::array<short, kNumOfGrids> mimic_board = (!bptr) ? board_ : *bptr;
	mimic_board[src] = EMPTY;
	mimic_board[action] = player;
	
	/*
	0x1001	0x0001	0x0011
	0x1000	 pos	0x0010
	0x1100	0x0100	0x0110
	*/

	auto CheckPostion = [=](Action pos, Action check) -> bool{ 
		if (check < 0 || check >= empty_index) return false;

		int rpos = pos / kBoardSize;
		int cpos = pos % kBoardSize;
		int rchk = check / kBoardSize;
		int cchk = check % kBoardSize;
		if (abs(rpos - rchk) > 1) return false;
		if (abs(cpos - cchk) > 1) return false;
		return true;
	};

	const int Cross[4] = {0x0001, 0x0010, 0x0100, 0x1000};
	const int Corner[4] = {0x1001, 0x0011, 0x0110, 0x1100};
	
	// check if dst is valid
	int ul = action + MoveDirection[0];
	int u = action + MoveDirection[1];
	int ur = action + MoveDirection[2];
	int l = action + MoveDirection[3];
	int r = action + MoveDirection[4];
	int bl = action + MoveDirection[5];
	int b = action + MoveDirection[6];
	int br = action + MoveDirection[7];
	const int PointsCrossDst[4] = {u, r, b, l};
	const int PointsCornerDst[4] = {ul, ur, br, bl};

	int place_cross = 0x0000;
	for(int i = 0; i < 4; i++){
		int chkpt = PointsCrossDst[i];
		if (CheckPostion(action, chkpt) == false) continue;
		if (mimic_board[chkpt] == player) {
			place_cross = place_cross | Cross[i];
		}
	}
	int restrictions = 0x1111;
	for (int i = 0; i < 4; i++){
		int chkpt = PointsCornerDst[i];
		if (CheckPostion(action, chkpt) == false) continue;
		if (mimic_board[chkpt] == player) {
			if (!(place_cross & Corner[i])){
				restrictions = restrictions & Corner[i];
			}
		}
	}

	// if there is no feasible restriction
	if (restrictions == false) return kInvalid;

	if (src == empty_index) {
		// placing action but have restrictions
		if (restrictions ^ 0x1111) return kInvalid;
		else return kValid;
	}

	std::vector<int> valid_points_dst;
	std::vector<int> valid_points_src;

	if (restrictions ^ 0x1111){
		for (int i = 0; i < 4; i++){
			if(restrictions & 0x1) {
				int pt = PointsCrossDst[i];
				valid_points_dst.push_back(pt);
			}
			restrictions = restrictions >> 4;
		}
	} 

	if (!valid_points_dst.empty()){
		valid_points_dst.erase(std::remove_if(
			valid_points_dst.begin(), valid_points_dst.end(), 
			[&](const Action& p) {
				return !is_placing_valid(p, player, &mimic_board);
			}), valid_points_dst.end());
		if (valid_points_dst.empty()) return kInvalid;
	}

	// check if src is valid
	ul = src + MoveDirection[0];
	u = src + MoveDirection[1];
	ur = src + MoveDirection[2];
	l = src + MoveDirection[3];
	r = src + MoveDirection[4];
	bl = src + MoveDirection[5];
	b = src + MoveDirection[6];
	br = src + MoveDirection[7];
	const int PointsCrossSrc[4] = {u, r, b, l};
	const int PointsCornerSrc[4] = {ul, ur, br, bl};

	place_cross = 0x0000;
	for(int i = 0; i < 4; i++){
		int chkpt = PointsCrossSrc[i];
		if (CheckPostion(src, chkpt) == false) continue;
		if(mimic_board[chkpt] == player){
			place_cross = place_cross | Cross[i];
		}
	}
	for(int i = 0; i < 4; i++){
		int chkpt = PointsCornerSrc[i];
		if (CheckPostion(src, chkpt) == false) continue;
		if (mimic_board[chkpt] != player){
			if(!(~(place_cross) & Corner[i])){
				valid_points_src.push_back(chkpt);
			}
		}
	}

	if (valid_points_src.size() == 1) {
		if (!is_placing_valid(valid_points_src[0], player, &mimic_board)) {
			valid_points_src.pop_back();
		}
		if (is_placing_valid(src, player, &mimic_board)) {
			valid_points_src.push_back(src);
		}
		if (valid_points_src.empty()) return kInvalid;
	// more than a corner must be placed
	} else if (valid_points_src.size() > 1) {
		if (is_placing_valid(src, player, &mimic_board)) {
			valid_points_src.clear();
			valid_points_src.push_back(src);
		} else {
			return kInvalid;
		}
	}

	if (valid_points_src.empty() && valid_points_dst.empty()) {
		return kValid;
	} else if (valid_points_dst.empty()) {
		constrained_points.insert(constrained_points.end(), valid_points_src.begin(), valid_points_src.end());
	} else if (valid_points_src.empty()) {
		constrained_points.insert(constrained_points.end(), valid_points_dst.begin(), valid_points_dst.end());
	} else {
		std::sort(valid_points_dst.begin(), valid_points_dst.end());
		std::sort(valid_points_src.begin(), valid_points_src.end());
		std::set_intersection(
			valid_points_dst.begin(), valid_points_dst.end(),
			valid_points_src.begin(), valid_points_src.end(),
			std::inserter(constrained_points, constrained_points.begin()));
	}

	constrained_points.erase(std::remove_if(
		constrained_points.begin(), constrained_points.end(), 
		[&](const Action& p) {
			if (p < 0 || p >= empty_index) return false;
			return (mimic_board[p] != EMPTY);
		}), constrained_points.end());

	return constrained_points;
}
bool State::is_selecting_valid(const Action action, const Player player) const {
	auto CheckPostion = [=](Action pos, Action check) -> bool{ 
		if (check < 0 || check >= empty_index) return false;

		int rpos = pos / kBoardSize;
		int cpos = pos % kBoardSize;
		int rchk = check / kBoardSize;
		int cchk = check % kBoardSize;
		if (abs(rpos - rchk) > 1) return false;
		if (abs(cpos - cchk) > 1) return false;
		return true;
	};

	if (action == kNumOfGrids) {
		return is_placing_valid(player);
	} else {
		for (auto &dir: MoveDirection) {
			const Action pt_surrounded = action + dir;
			if (CheckPostion(action, pt_surrounded) == false) continue;
			if (board_[pt_surrounded] != EMPTY) continue;

			std::vector<int> constrained_points = get_restrictions(action, pt_surrounded, player);
			std::array<short, kNumOfGrids> mimic_board = board_;
			mimic_board[action] = EMPTY;
			mimic_board[pt_surrounded] = player;
			if (constrained_points.empty()) {
				return is_placing_valid(player, &mimic_board);
			} else if (constrained_points.size() != 1) {
				for (auto pt: constrained_points) {
					if (is_placing_valid(pt, player, &mimic_board)) return true;
				}
			}
		}
	}

	return false;
}
bool State::is_moving_valid(const Action src, const Action action, const Player player, std::array<short, kNumOfGrids>* bptr) const {
	std::vector<int> constrained_points = get_restrictions(src, action, player, bptr);
	std::array<short, kNumOfGrids> mimic_board = board_;
	mimic_board[src] = EMPTY;
	mimic_board[action] = player;
	if (constrained_points.empty()) {
		return is_placing_valid(player, &mimic_board);
	}
	return get_restrictions(src, action, player, bptr).size() != 1;
}
bool State::is_placing_valid(const Action action, const Player player, std::array<short, kNumOfGrids>* bptr) const {
	return get_restrictions(empty_index, action, player, bptr).size() != 1;
}
bool State::is_placing_valid(const Player player, std::array<short, kNumOfGrids>* bptr) const {
	std::array<short, kNumOfGrids> mimic_board = (!bptr) ? board_ : *bptr;
	for (int i = 0; i < kNumOfGrids; ++i) {
		if (mimic_board[i] == EMPTY) {
			if (is_placing_valid(i, player, &mimic_board)) return true;
		}
	}
	return false;
}
bool State::is_legal_action(const Action &action) const {
	if (action < 0 || action > kNumOfGrids) return false;
	const Player player = current_player();
	if (turn_ % 3 == 0) {  // choose
		if ((board_[action] == player && have_move(action)) || action == kNumOfGrids) {
			// return true;
			return is_selecting_valid(action, player);
		}
	}
	else if (turn_ % 3 == 1) {  // move
		const Action &src = history_[history_.size() - 1];
		if (board_[action] == EMPTY) {
			if ((std::abs(action / kBoardSize - src / kBoardSize) <= 1) &&
				(std::abs(action % kBoardSize - src % kBoardSize) <= 1)){
				// return true;
				return is_moving_valid(src, action, player);
			}
		}
		return false;
	} else {  // new chess
		if(board_[action] == EMPTY) {
			// return true;
			Action src = history_[history_.size() - 2];
			Action dst = history_[history_.size() - 1];
			if (dst == empty_index) {
				return is_placing_valid(action, player);
			}

			std::vector<int> constrained_points = get_restrictions(src, dst, player);
			// if the moving action gets no restriction, or if placing action is a possible valid option
			if (constrained_points.empty() || std::find(constrained_points.begin(), constrained_points.end(), action) != constrained_points.end()) {
				return is_placing_valid(action, player);
			}
		}
	}
	return false;
}
bool State::have_move(const Action &action) const {
    for (int const &diff : MoveDirection) {
        int dest = action + diff;
        if ((dest >= 0 && dest < kNumOfGrids && board_[dest] == EMPTY) &&
            (std::abs(dest / kBoardSize - action / kBoardSize) <= 1) &&
            (std::abs(dest % kBoardSize - action % kBoardSize) <= 1)) {
        return true;
        }
    }
    return false;
}
bool State::have_win(const Action &action) const {
	if (action < 0 || action >= kNumOfGrids) return false;
	const Player player = board_[action];
	std::stack<int> pieces;
	pieces.push(int(action));
	int traversed[kNumOfGrids] = {0}; //const
	traversed[action] = 1;
	int head = 0, tail = 0; 
	while(!pieces.empty()){
		int cur = pieces.top();
		pieces.pop();
		int u = cur + MoveDirection[1];
		int l = cur + MoveDirection[3];
		int r = cur + MoveDirection[4];
		int b = cur + MoveDirection[6];
		int top_check = 1, bottom_check = 1, left_check = 1, right_check = 1;
		if(cur / kBoardSize == 0) top_check = 0;
		if(cur / kBoardSize == kBoardSize - 1) bottom_check = 0;
		if(cur % kBoardSize == 0) left_check = 0;
		if(cur % kBoardSize == kBoardSize - 1) right_check = 0;
		if(top_check && traversed[u] == 0 && board_[u] == player){
			traversed[u] = 1;
			pieces.push(u);
		}
		if(left_check && traversed[l] == 0 && board_[l] == player){
			traversed[l] = 1;
			pieces.push(l);
		}
		if(right_check && traversed[r] == 0 && board_[r] == player){
			traversed[r] = 1;
			pieces.push(r);
		}
		if(bottom_check && traversed[b] == 0 && board_[b] == player){
			traversed[b] = 1;
			pieces.push(b);
		}
	}
	if(player == BLACK){
		for(int i = 0; i < kBoardSize; i++){
			if(traversed[i] == 1)
				head = 1;
			if(traversed[kBoardSize * (kBoardSize - 1) + i] == 1)
				tail = 1;
		}
	}
	else{
		for(int i = 0; i < kBoardSize; i++){
			if(traversed[i*kBoardSize] == 1)
				head = 1;
			if(traversed[kBoardSize * (i+1) - 1] == 1)
				tail = 1;
		}
	}
	if(head && tail){
		return true;
	}
	return false;
}
bool State::is_valid(const Action &action) const {
	if (action < 0 || action >= kNumOfGrids) return false;
	const Player player = board_[action];
	int top_check = 1, bottom_check = 1, left_check = 1, right_check = 1;
	if(action / kBoardSize == 0) top_check = 0;
	if(action / kBoardSize == kBoardSize - 1) bottom_check = 0;
	if(action % kBoardSize == 0) left_check = 0;
	if(action % kBoardSize == kBoardSize - 1) right_check = 0;
	int ul = action + MoveDirection[0];
	int u = action + MoveDirection[1];
	int ur = action + MoveDirection[2];
	int l = action + MoveDirection[3];
	int r = action + MoveDirection[4];
	int bl = action + MoveDirection[5];
	int b = action + MoveDirection[6];
	int br = action + MoveDirection[7];
	// upper left check
	if(top_check && left_check && board_[ul] == player && board_[u] != player && board_[l] != player)
		return false;
	// upper right check
	if(top_check && right_check && board_[ur] == player && board_[u] != player && board_[r] != player)
		return false;
	// lower left check
	if(bottom_check && left_check && board_[bl] == player && board_[b] != player && board_[l] != player)
		return false;
	// lower right check
	if(bottom_check && right_check && board_[br] == player && board_[b] != player && board_[r] != player)
		return false;
	return true;
}
bool State::is_empty_valid(const Action &action) const {
	if (action < 0 || action >= kNumOfGrids) return false;
	if(board_[action] != EMPTY) return true;
	int top_check = 1, bottom_check = 1, left_check = 1, right_check = 1;
	if(action / kBoardSize == 0) top_check = 0;
	if(action / kBoardSize == kBoardSize - 1) bottom_check = 0;
	if(action % kBoardSize == 0) left_check = 0;
	if(action % kBoardSize == kBoardSize - 1) right_check = 0;
	int ul = action + MoveDirection[0];
	int u = action + MoveDirection[1];
	int ur = action + MoveDirection[2];
	int l = action + MoveDirection[3];
	int r = action + MoveDirection[4];
	int bl = action + MoveDirection[5];
	int b = action + MoveDirection[6];
	int br = action + MoveDirection[7];
	// upper left check
	if(top_check && left_check && board_[u] != EMPTY && board_[u] == board_[l] && board_[u] != board_[ul])
		return false;
	// upper right check
	if(top_check && right_check && board_[u] != EMPTY && board_[u] == board_[r] && board_[u] != board_[ur])
		return false;
	// lower left check
	if(bottom_check && left_check && board_[b] != EMPTY && board_[b] == board_[l] && board_[b] != board_[bl])
		return false;
	// lower right check
	if(bottom_check && right_check && board_[b] != EMPTY && board_[b] == board_[r] && board_[b] != board_[br])
		return false;
	return true;
}
std::string State::action_to_string(const Action &action) const {
    using namespace std::string_literals;
    std::stringstream ss;
    if(action == empty_index)
        ss << "X";
    else
        ss << "ABCDEFGHIJKLM"s.at(action % kBoardSize) << kBoardSize - (action / kBoardSize);
    return ss.str();
}
std::vector<Action> State::string_to_action(const std::string &str) const {
    std::string wopunc = str;
    wopunc.erase(std::remove_if(wopunc.begin(), wopunc.end(), ispunct),
                wopunc.end());
    std::stringstream ss(wopunc);
    std::string action;
    std::vector<Action> id;
    while (ss >> action) {
        if (action.size() == 1 && (action.at(0) == 'X' || action.at(0) == 'x')) {
        	id.push_back(empty_index);
        } else if (action.size() == 2 && (action.at(0) >= 'a' && action.at(0) < 'a' + kBoardSize) && 
										 (action.at(1) >= '1' && action.at(1) < '1' + kBoardSize)) {
        	int tmp = (kBoardSize - std::stoi(action.substr(1))) * kBoardSize +
                    (action.at(0) - 'a');
        	id.push_back(tmp);
        } else if (action.size() == 2 && (action.at(0) >= 'A' && action.at(0) < 'a' + kBoardSize) && 
										 (action.at(1) >= '1' && action.at(1) < '1' + kBoardSize)) {
        	int tmp = (kBoardSize - std::stoi(action.substr(1))) * kBoardSize +
                    (action.at(0) - 'A');
        	id.push_back(tmp);
        }
    }
    return id;
}