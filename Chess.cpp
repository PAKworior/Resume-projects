//main project
#include <regex>
#include <conio.h>
#include <algorithm>
#include <windows.h>
#include <iostream>
#include <thread>
#include <cctype>
#include <cstdlib>
#include <string>
#include <sstream>
#include <chrono>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
class chess {
private:
	
	
	std::string board[8][8];//stores the board configuration
	int valid[8][8];// used to display stuff in board
	int assign_id = 0;// used to assign id to the notifications in the notification bar
	std::string numbers[10][10] = {
		{ "     #####     ",
		  "  ###########  ",
		  " ####     #### ",
		  "####       ####",
		  "####       ####",
		  "####       ####",
		  "####       ####",
		  " ####     #### ",
		  "  ###########  ",
		  "     #####     " },
		{ "    ######     ",
		  "   #######     ",
		  " #########     ",
		  "##########     ",
		  " ### #####     ",
		  "     #####     ",
		  "     #####     ",
		  "     #####     ",
		  "###############",
		  "###############" },
		{ "    #######    ",
		  "  ############ ",
		  " #####    #####",
		  " #####    #####",
		  "        ###### ",
		  "      ######   ",
		  "   ######      ",
		  " ######        ",
		  "###############",
		  "###############" },
		{ "   #########   ",
		  " ############# ",
		  "#####    ######",
		  "        ###### ",
		  "      #######  ",
		  "        ###### ",
		  "######    #####",
		  "######    #####",
		  " ############# ",
		  "   #########   " },
		{ "      #######  ",
		  "     ########  ",
		  "    #### ####  ",
		  "   ####  ####  ",
		  "  ####   ####  ",
		  " ####    ####  ",
		  "###############",
		  "###############",
		  "         ####  ",
		  "         ####  " },
		{ "###############",
		  "###############",
		  "####           ",
		  "##########     ",
		  "############   ",
		  "        ###### ",
		  "###       #####",
		  "####      #####",
		  " ############# ",
		  "  ##########   " },
		{ "    ########   ",
		  "  ###########  ",
		  " ####     #### ",
		  "####           ",
		  "####  #####    ",
		  "############## ",
		  "#####     #####",
		  "#####     #####",
		  " ############# ",
		  "  ##########   " },
		{ "###############",
		  "###############",
		  "          #####",
		  "         ##### ",
		  "       #####   ",
		  "     #####     ",
		  "   #####       ",
		  "  #####        ",
		  "#####          ",
		  "#####          " },
		{ "    #######    ",
		  "  ####   ####  ",
		  " ####     #### ",
		  "  ##### #####  ",
		  "   #########   ",
		  " #####   ##### ",
		  "####       ####",
		  "####       ####",
		  " ############# ",
		  "    #######    " },
		{ "  ##########   ",
		  " ############# ",
		  "####       ####",
		  "####       ####",
		  " ##############",
		  "   ############",
		  "          #####",
		  "####      #####",
		  " ####    ##### ",
		  "  ##########   " }, };
	bool white_left_rook_can_castel = true;
	bool white_right_rook_can_castel = true;
	bool black_left_rook_can_castel = true;
	bool black_right_rook_can_castel = true;
	typedef struct notification {
		int id;
		std::string message;
	};
	std::vector<notification>notifications_on_board;// this sotres the notifications(composed of it's id and it's text)


	//for timer thread only !!!!!!!!!!!
	std::atomic<double> white_time = 10;
	std::atomic<double> black_time = 10;
	std::atomic<char> turn = 'W';
	typedef struct {
		int execution_time;
		std::string comand;
	}timestamp;
	std::mutex timer_mutex;
	std::queue<timestamp> timer_queue;
	std::vector<timestamp> timestamps;
	void add(timestamp t) {

		int low = 0, high = timestamps.size() - 1;
		int mid = 0;
		while (low <= high) {
			mid = low + (high - low) / 2;
			if (timestamps[mid].execution_time > t.execution_time) low = mid + 1; else high = mid - 1;
		}
		timestamps.insert(timestamps.begin() + low, t);
	}

	//input thread
	std::string str;
	std::mutex input_cv_mutex;
	std::condition_variable input_cv;
	std::atomic <int>pressed_char{ 10 };

	// output processing
	std::string _left_side(int h, int  k) {
		if (k == 3) {
			return "| |  |"+colours.coordinates + std::to_string(8 - h) + "\033[0m";
		}
		return "| |  | ";
	}
	std::string _right_side(int h, int  k) {
		if (k == 3) {
			return colours.coordinates + std::to_string(8 - h) + colours.borders+"|  | |";
		}
		return " |  | |";
	}
	struct theme_menu_index {
		int x = 0;
		int y = 0;
	};
	theme_menu_index theme_index;
	typedef struct Themecolours {// use to colour theme of the game.
		std::string black_pieces = "\033[38;5;130m";// colour of the black team
		std::string white_pieces = "\033[38;5;216m";// colour of the white team
		std::string white_boxes = "\033[0m";//
		std::string coordinates = "\033[32m";// those 1-8 and A-H numbers that shows the coordinates of the pieces
		std::string borders = "\033[0m";//border of the chess board default white
		std::string safe_spots = "\033[38;5;22m";//when you select a piece, it shows the places where it can move.default green
		std::string attack_spots = "\033[38;5;88m";// tells if the piece on that spot is attackable default dark red
	};
	Themecolours colours;
	std::string menu_array[3][3][7];
	std::string menu_select_array[3][3][7];
	Themecolours menu_colours_array[3][3];
	//chess logic
	bool _is_counterable(int x, int y) {//gets coordinates of the checkmater
		std::string piece = board[x][y];
		int enemy_can_move_to[8][8];
		clear(enemy_can_move_to);
		char enemy_team;
		piece[0] == 'B' ? enemy_team = 'W' : enemy_team = 'B';
		_All_possible_spots(enemy_can_move_to, piece[0]);// gives all places you can move
		if (enemy_can_move_to[x][y]) return true;
		int temp[8][8];
		clear(temp);
		if (piece[piece.length() - 1] == 'k') {
			int directions[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
			for (int* d : directions) {
				clear(temp);
				int dx = d[0], dy = d[1];
				int i;
				for (i = 1;_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy] == "null";i++) {
					temp[x + dx * i][y + dy * i] = 1;
				}
				if (_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy][0] != piece[0] && board[x + i * dx][y + i * dy][5] == 'k' && board[x + i * dx][y + i * dy][6] == 'i') {
					break;
				}
			}//this puts the spots that, if enemy_can_move_to, it will be considered a block
		}
		if (piece[piece.length() - 1] == 'n') {
			int directions[8][2] = { {1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1} };
			for (int* d : directions) {
				clear(temp);
				int dx = d[0], dy = d[1];
				int i;
				for (i = 1;_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy] == "null";i++) {
					temp[x + dx * i][y + dy * i] = 1;
				}
				if (_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy][0] != piece[0] && board[x + i * dx][y + i * dy][5] == 'k' && board[x + i * dx][y + i * dy][6] == 'i') {
					break;
				}
			}
		}
		if (piece[piece.length() - 1] == 'p') {
			int directions[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
			for (int* d : directions) {
				clear(temp);
				int dx = d[0], dy = d[1];
				int i;
				for (i = 1;_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy] == "null";i++) {
					temp[x + dx * i][y + dy * i] = 1;
				}
				if (_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy][0] != piece[0] && board[x + i * dx][y + i * dy][5] == 'k' && board[x + i * dx][y + i * dy][6] == 'i') {
					break;
				}
			}
		}
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (temp[i][j] && enemy_can_move_to[i][j]) {
					return true;
				}
			}
		}
		return false;
	}
	void _All_possible_spots(int team_can_move[8][8], char team) {// calculates all places enemy can move his pieces and stores in valid, team can be B= black or W = white
		int valid[8][8];
		clear(valid);
		clear(team_can_move);
		int temp[8][8];//this will store all places where i can attack
		clear(temp);
		char enemy;
		if (team == 'W') enemy = 'B';else enemy = 'W';
		//std::cout << "\nenemy: " << enemy << "                  ";
		//std::cout << "\nteam: " << team << "                    ";

		// puts where everyone except king can move
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (board[i][j][0] == team) {//will only run on our pieces 
					std::string name = board[i][j];
					clear(temp);
					if (name.back() == 'k') {//rook
						_Valid_Rook_moves(temp, i, j);
					}
					else if (name.back() == 't') {//knight
						_Valid_Knight_moves(temp, i, j);
					}
					else if (name.back() == 'p') {//bishop
						_Valid_Bishop_moves(temp, i, j);
					}
					else if (name.back() == 'n') {//queen
						_Valid_Queen_moves(temp, i, j);
					}
					else if (name.back() == 'r') {//soldier or pawn
						_Valid_Soldier_moves(temp, i, j);
					}
					_add_valid_moves(team_can_move, temp);// valid stores all places where i can move

				}
			}
		}
		//std::cout << "\nteam_can_move(before king):                         ";
		//print(team_can_move);
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (board[i][j][0] == enemy) {
					std::string name = board[i][j];
					clear(temp);
					if (name.back() == 'k') {//rook
						_Valid_Rook_moves(temp, i, j);
					}
					else if (name.back() == 't') {//knight
						_Valid_Knight_moves(temp, i, j);
					}
					else if (name.back() == 'p') {//bishop
						_Valid_Bishop_moves(temp, i, j);
					}
					else if (name.back() == 'n') {//queen
						_Valid_Queen_moves(temp, i, j);
					}
					else if (name.back() == 'r') {//soldier or pawn
						_Valid_Soldier_moves(temp, i, j);
					}
					_add_valid_moves(valid, temp);
				}
			}
		}
		//std::cout << "\nvalid(before king):                         ";
		//print(valid);
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (board[i][j][0] == team && board[i][j][board[i][j].length() - 1] == 'g') {// enemy king
					int directions[8][2] = { {1,1},{-1,1},{1,-1},{-1,-1},{0,1},{1,0},{0,-1},{-1,0} };

					for (int* dir : directions) {
						int dx = dir[0], dy = dir[1];
						if (_in_board(i + dx, j + dy) && !valid[i + dx][j + dy] && board[i + dx][j + dy][0] != team) {
							team_can_move[i + dx][j + dy] = 1;
						}
					}
				}
			}
		}
		//std::cout << "\nteam_can_move:                         ";
		//print(team_can_move);
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (board[i][j][0] != team && board[i][j][board[i][j].length() - 1] == 'g') {// our king

					int directions[8][2] = { {1,1},{-1,1},{1,-1},{-1,-1},{0,1},{1,0},{0,-1},{-1,0} };
					for (int* dir : directions) {
						int dx = dir[0], dy = dir[1];
						if (_in_board(i + dx, j + dy) && !team_can_move[i + dx][j + dy] && board[i + dx][j + dy][0] == team) {
							valid[i + dx][j + dy] = 1;
						}
					}
				}
			}
		}

		//std::cout << "\nvalid:                         ";
		//print(valid);
	}
	void _add_valid_moves(int arr1[8][8], int arr2[8][8]) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {

				arr1[i][j] || arr2[i][j] ? arr1[i][j] = 1 : arr1[i][j] = 0;
			}
		}

	}
	void _Valid_Bishop_moves(int valid[8][8], int x, int y) {
		std::string piece = board[x][y];

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				valid[i][j] = 0;
			}
		}
		int directions[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int* i : directions) {
			int dx = i[0], dy = i[1];
			int i;
			for (i = 1;_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy] == "null";i++) {
				valid[x + i * dx][y + i * dy] = 1;
			}
			if (_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy][0] != piece[0]) {
				valid[x + i * dx][y + i * dy] = 1;
			}
		}
	}
	void _Valid_Soldier_moves(int valid[8][8], int x, int y) {
		if (board[x][y] == "WhiteSoldier") {
			if (y == 6 && board[x][y - 2][0] != 'W' && board[x][y - 1][0] == 'n') valid[x][y - 2] = 1;
			if (_in_board(x - 1, y - 1) && board[x - 1][y - 1][0] == 'B') valid[x - 1][y - 1] = 1;// capture a piece on left
			if (_in_board(x + 1, y - 1) && board[x + 1][y - 1][0] == 'B') valid[x + 1][y - 1] = 1;// capture a piece on right
			if (_in_board(x, y - 1) && board[x][y - 1] == "null") valid[x][y - 1] = 1;
		}//
		else if (board[x][y] == "BlackSoldier") {
			if (y == 1 && board[x][y + 2][0] != 'B' && board[x][y + 1][0] == 'n') valid[x][y + 2] = 1;
			if (_in_board(x + 1, y + 1) && board[x + 1][y + 1][0] == 'W') valid[x + 1][y + 1] = 1;// capture a piece on left
			if (_in_board(x - 1, y + 1) && board[x - 1][y + 1][0] == 'W') valid[x - 1][y + 1] = 1;// capture a piece on right
			if (_in_board(x, y + 1) && board[x][y + 1] == "null") valid[x][y + 1] = 1;
		}
	}
	void _Valid_Queen_moves(int valid[8][8], int x, int y) {
		std::string piece = board[x][y];
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				valid[i][j] = 0;
			}
		}
		int directions[8][2] = { {1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1} };
		for (int* i : directions) {
			int dx = i[0], dy = i[1];
			int i;
			for (i = 1;_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy] == "null";i++) {
				valid[x + i * dx][y + i * dy] = 1;
			}
			if (_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy][0] != piece[0]) {
				valid[x + i * dx][y + i * dy] = 1;
			}
		}
	}
	void _Valid_Rook_moves(int valid[8][8], int x, int y) {
		std::string piece = board[x][y];
		int i;
		int directions[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };

		for (int* i : directions) {
			int dx = i[0], dy = i[1];
			int i;
			for (i = 1;_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy] == "null";i++) {
				valid[x + dx * i][y + dy * i] = 1;
			}
			if (_in_board(x + i * dx, y + i * dy) && board[x + i * dx][y + i * dy][0] != piece[0]) {
				valid[x + i * dx][y + i * dy] = 1;
			}
		}
	}
	void _Valid_Knight_moves(int valid[8][8], int x, int y) {
		char team = board[x][y][0];
		if (_in_board(x - 1, y - 2) && board[x - 1][y - 2][0] != team) {
			valid[x - 1][y - 2] = 1;
		}
		if (_in_board(x - 2, y - 1) && board[x - 2][y - 1][0] != team) {
			valid[x - 2][y - 1] = 1;
		}
		if (_in_board(x + 1, y - 2) && board[x + 1][y - 2][0] != team) {
			valid[x + 1][y - 2] = 1;
		}
		if (_in_board(x + 2, y - 1) && board[x + 2][y - 1][0] != team) {
			valid[x + 2][y - 1] = 1;
		}
		if (_in_board(x - 1, y + 2) && board[x - 1][y + 2][0] != team) {
			valid[x - 1][y + 2] = 1;
		}
		if (_in_board(x - 2, y + 1) && board[x - 2][y + 1][0] != team) {
			valid[x - 2][y + 1] = 1;
		}
		if (_in_board(x + 1, y + 2) && board[x + 1][y + 2][0] != team) {
			valid[x + 1][y + 2] = 1;
		}
		if (_in_board(x + 2, y + 1) && board[x + 2][y + 1][0] != team) {
			valid[x + 2][y + 1] = 1;
		}
	}
	int _Valid_King_moves(int valid[8][8], int x, int y) {
		// 0 = checkmate, 1 = statemate, 2 = none

		int safe[8][8];//0 = king can exist here 1 = king can't exist here    
		clear(safe);
		int danger[8][8];// just a temporary variable for processing.
		clear(danger);
		char team = board[x][y][0];//W || B
		// putting 1 wherever there is friendly piece in safe
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (board[i][j][0] == team) {
					safe[i][j] = 1;
				}
			}
		}
		safe[x][y] = 0;
		int safe_spots = 0;
		bool king_is_capturable = false;
		int checkmater_cord_x = -1;
		int checkmater_cord_y = -1;
		std::string str = board[x][y];
		board[x][y] = "null";
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				std::string piece = board[i][j];
				if (piece != "null" && piece[0] != team) {
					clear(danger);
					if (piece[5] == 'S') {
						_Valid_Soldier_moves(danger, x, y);
						if (danger[x][y] == 1) {
							king_is_capturable = true;
							checkmater_cord_x = i;
							checkmater_cord_y = j;
						}
					}
					else if (piece[5] == 'K' && piece[6] == 'n') {
						_Valid_Knight_moves(danger, i, j);
						if (danger[x][y] == 1) {
							king_is_capturable = true;
							checkmater_cord_x = i;
							checkmater_cord_y = j;
						}
					}
					else if (piece[5] == 'B') {
						_Valid_Bishop_moves(danger, i, j);
						if (danger[x][y] == 1) {
							king_is_capturable = true;
							checkmater_cord_x = i;
							checkmater_cord_y = j;
						}
					}
					else if (piece[5] == 'R') {
						_Valid_Rook_moves(danger, i, j);
						if (danger[x][y] == 1) {
							king_is_capturable = true;
							checkmater_cord_x = i;
							checkmater_cord_y = j;
						}
					}
					else if (piece[5] == 'Q') {
						_Valid_Queen_moves(danger, i, j);
						if (danger[x][y] == 1) {
							king_is_capturable = true;
							checkmater_cord_x = i;
							checkmater_cord_y = j;

						}
					}

					else if (piece[5] == 'K' && piece[6] == 'i') {
						int directions[8][2] = { {1,1},{-1,1},{1,-1},{-1,-1},{0,1},{1,0},{0,-1},{-1,0} };
						for (int* dir : directions) {
							if (_in_board(i + dir[0], j + dir[1]) && (board[i + dir[0]][j + dir[1]] == "null" || board[i + dir[0]][j + dir[1]][0] != piece[0])) {
								danger[i + dir[0]][j + dir[1]] = 1;
							}
						}
						if (danger[x][y] == 1) {
							king_is_capturable = true;
							checkmater_cord_x = i;
							checkmater_cord_y = j;
						}
					}
					_add_valid_moves(safe, danger);

				}
			}
		}
		board[x][y] = str;
		int directions[8][2] = { {1,1},{-1,1},{1,-1},{-1,-1},{0,1},{1,0},{0,-1},{-1,0} };
		for (int* dir : directions) {
			int dx = dir[0], dy = dir[1];

			if (_in_board(x + dx, y + dy) && !safe[x + dx][y + dy]) {
				safe_spots++;
			}
			if (_in_board(x + dx, y + dy))valid[x + dx][y + dy] = !safe[x + dx][y + dy];
		}
		clear(danger);
		_All_possible_spots(danger, board[x][y][0]);
		int sum = 0;// sum of all valid places where it can move to
		//std::cout << "\nking_is_capturable = " << king_is_capturable << "                      ";
		if (!king_is_capturable) {
			for (int i = 0;i < 8;i++) {
				for (int j = 0;j < 8;j++) {
					if (danger[i][j] == 1) {
						sum++;
					}
				}
			}
			//std::cout << "\nsum = " << sum << "                      ";
			if (sum == 0) {
				//std::cout << "\nstatemate!                 ";
				return 1;
			}

		}
		//std::cout << "\nsafe spots= " << safe_spots << "                      ";
		/*bool c = _is_counterable(checkmater_cord_x, checkmater_cord_y);
		std::cout << "\ncounteramle?= " << c<< "                      ";*/

		if (king_is_capturable && !safe_spots && !_is_counterable(checkmater_cord_x, checkmater_cord_y)) {
			//std::cout << "\ncheckmate!                 ";
			return 0;
		}
		//std::cout << "\nnone!                 ";
		return 2;//none
	}
	void Valid_Chess_Moves(int x, int y) {//given the coordinate of a chess piece it will return the places where can it move in 8x8 matrix of bool
		std::string piece = board[x][y];
		if (piece == "null") return;
		if (piece == "WhiteSoldier" || piece == "BlackSoldier") {
			_Valid_Soldier_moves(valid, x, y);
		}
		if (piece == "WhiteKnight" || piece == "BlackKnight") {
			_Valid_Knight_moves(valid, x, y);
		}
		if (piece == "WhiteBishop" || piece == "BlackBishop") {
			_Valid_Bishop_moves(valid, x, y);
		}
		if (piece == "WhiteRook" || piece == "BlackRook") {
			_Valid_Rook_moves(valid, x, y);
		}
		if (piece == "WhiteQueen" || piece == "BlackQueen") {
			_Valid_Queen_moves(valid, x, y);
		}
		if (piece == "WhiteKing" || piece == "BlackKing") {
			_Valid_King_moves(valid, x, y);
		}

	}
	//input pocessing
	void str_to_cords(std::string str, int& n1, int& n2) {
		n1 = str[0] - 'a';
		n2 = 8 - (str[1] - '0');
	}
	void print(int arr[8][8]) {
		std::cout << '\n';
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				std::cout << arr[j][i];

			}
			std::cout << '\n';
		}
	}

public:
	bool running = true;
	std::mutex output_mutex;//use it for output purposes

	//for main  thread only !!!!!!!!!!!
	std::condition_variable main_cv;
	std::queue <std::string> main_queue;
	std::mutex main_mutex;
	std::string chessboard_buffer[64];
	std::string timer_buffer[23];
	std::string _notification_buffer[22];
	std::string theme_menu_buffer[36];
	std::atomic<char> input_mode{ 's' };
	//timer thread
	void timer_thread() {

			main_mutex.lock();
			std::string temp_str = "-m 5000 \"Time thread # ";temp_str += std::to_string(GetCurrentProcessorNumber()); temp_str += '\"';
			main_queue.push(temp_str);
			main_mutex.unlock();
			int time_since_start = 0;// use it to know the time accurately
			const std::chrono::milliseconds interval(1);
			std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
			std::chrono::high_resolution_clock::time_point next_call_time = start_time;

			while (running) {

				std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::micro> elapsed = now - start_time;
				//start of the loop of every 1 milliseconds as long as load is not more time consuming then 1 ms
				if (turn == 'W') white_time = white_time - .001; else black_time = black_time - .001;
				timer_mutex.lock();
				while (!timer_queue.empty()) {

					timestamp s = timer_queue.front();
					add({ time_since_start + s.execution_time, s.comand });
					timer_queue.pop();
				}
				timer_mutex.unlock();
				while (!timestamps.empty() && timestamps[timestamps.size() - 1].execution_time <= time_since_start) {//executing comand after the delay
					timestamp t = timestamps.back();
						main_mutex.lock();
					main_queue.push(t.comand);

					timestamps.pop_back();
					main_mutex.unlock();
				}
				if (_kbhit()) {
					std::lock_guard<std::mutex> lock(input_cv_mutex);
					input_cv.notify_one();
				}
				main_mutex.lock();
				if (!main_queue.empty()) main_cv.notify_one();
				main_mutex.unlock();

				//end
				time_since_start += 1;
				next_call_time += interval;
				std::this_thread::sleep_until(next_call_time);
			}
		}
	//input thread
	void input() {
		main_mutex.lock();
		std::string temp_str = "-m 5000 \"Input thread # ";temp_str += std::to_string(GetCurrentProcessorNumber()); temp_str += '\"';
		main_queue.push(temp_str);
		main_mutex.unlock();

		std::regex pattern1(R"(^\s*([a-h][1-8])\s*[a-h]?$)");// f5
		std::regex pattern2(R"(^\s*([a-h][1-8])\s*([a-h][1-8])\s*$)");// f5 f6
		std::regex pattern3(R"(^\s*help\s*$)");
		std::regex pattern4(R"(.*theme\s*$)");
		std::smatch matches;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		const COORD origin = { csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y };//coordinates of cursor when it was called
		char prev = ' ';
		while (true) {
			char ch;
			{
				std::unique_lock<std::mutex> lock(input_cv_mutex);
				input_cv.wait(lock, [&] { return _kbhit();});
				ch = _getch();
			}
			/*if (mode == 'm') {
				if ((ch == 72 && prev == -32) || ch == 'k') {
					main_mutex.lock();
					main_queue.push("-m up");
					main_mutex.unlock();
				}
				else if ((ch == 80 && prev == -32) || ch == 'j') {
					main_mutex.lock();
					main_queue.push("-m down");
					main_mutex.unlock();
				}
				else if ((ch == 75 && prev == -32) || ch == 'h') {
					main_mutex.lock();
					main_queue.push("-m left");
					main_mutex.unlock();
				}
				else if ((ch == 77 && prev == -32) || ch == 'l') {
					main_mutex.lock();
					main_queue.push("-m right");
					main_mutex.unlock();
				}
				else if (ch == 13) {
					main_mutex.lock();
					main_queue.push("-m enter");
					main_mutex.unlock();
					return;
				}
			}*/
			if (input_mode == 't') {
				if (prev == -32) {
					if (ch == 72) {
						main_mutex.lock();
						main_queue.push("-i up");
						main_mutex.unlock();
					}
					else if (ch == 75) {
						main_mutex.lock();
						main_queue.push("-i left");
						main_mutex.unlock();
					}
					else if (ch == 77) {
						main_mutex.lock();
						main_queue.push("-i right");
						main_mutex.unlock();
					}
					else if (ch == 80) {
						main_mutex.lock();
						main_queue.push("-i down");
						main_mutex.unlock();
					}
				}
				else if (ch == 13) {
					main_mutex.lock();
					main_queue.push("-i enter");
					main_mutex.unlock();
				}
				else if (ch == 104) {
					main_mutex.lock();
					main_queue.push("-i left");
					main_mutex.unlock();
				}
				else if (ch == 108) {
					main_mutex.lock();
					main_queue.push("-i right");
					main_mutex.unlock();
				}
				else if (ch == 106) {
					main_mutex.lock();
					main_queue.push("-i down");
					main_mutex.unlock();
				}
				if (ch == 107) {
					main_mutex.lock();
					main_queue.push("-i up");
					main_mutex.unlock();
				}
			}
			else if (input_mode == 's') {
				output_mutex.lock();
				GetConsoleScreenBufferInfo(hConsole, &csbi);
				int x_cursor_index = csbi.dwCursorPosition.X - origin.X;
				if (prev == -32) {
					if (ch == 115) {
						int i;
						for (i = x_cursor_index - 1;i > 0;i--) {
							if (str[i - 1] == ' ' && str[i] != ' ') break;
						}
						if (i < 0) i = 0;

						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + i),static_cast<SHORT>(origin.Y) });
					}
					else if (ch == 116) {

						short i;
						for (i = x_cursor_index + 1;i < str.length();i++) {
							if (str[i] == ' ' && str[i - 1] != ' ') break;
						}
						if (i > str.length()) i = str.length();
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + i),static_cast<SHORT>(origin.Y) });
					}
					else if (ch == 71) {
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
					}
					else if (ch == 79) {
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + str.length()),static_cast<SHORT>(origin.Y) });
					}
					else if (ch == 75) {
						short i = x_cursor_index - 1;
						if (i < 0) i = 0;
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + i),static_cast<SHORT>(origin.Y) });
					}
					else if (ch == 77) {
						short i = x_cursor_index + 1;
						if (i > str.length()) i = str.length();
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + i),static_cast<SHORT>(origin.Y) });
					}
					else if (ch == 72) {

					}
				}
				else if (ch >= 32 && ch <= 126) {
					str.insert(x_cursor_index, 1, ch);

					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
					std::cout << str << "                                                                           ";
					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + 1 + x_cursor_index),static_cast<SHORT>(origin.Y) });
				}
				else if (ch == 13) {

					main_mutex.lock();
					main_queue.push(str);
					main_mutex.unlock();
					//SetConsoleCursorPosition(hConsole, { 0,0 });
					str = "";

					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
					std::cout << str << "                                                                           ";
					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
				}
				else if (ch == 8) {
					if (x_cursor_index != 0) {
						str.erase(x_cursor_index - 1, 1);
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });std::cout << str << "                                           ";
						SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + x_cursor_index - 1),static_cast<SHORT>(origin.Y) });
					}
				}
				else if (ch == -32) {
					//do nothing :)
				}
				else {
					std::cout << "\a";
				}
				output_mutex.unlock();
				if (std::regex_match(str, matches, pattern1)) {// f5
					std::string str;
					str = matches[1];
					main_mutex.lock();
					main_queue.push(str);
					main_mutex.unlock();
				}
				//else if (std::regex_match(str, matches, pattern2)) {// f5 a1
				//	std::string str = matches[1];
				//	str += " ";
				//	str += matches[2];
				//	main_mutex.lock();
				//	main_queue.push(str);
				//	main_mutex.unlock();
				//}
				else if (std::regex_match(str, matches, pattern3)) {// help
					main_mutex.lock();
					main_queue.push("help");
					main_mutex.unlock();
					str = "";

					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
					std::cout << str << "                                                                           ";
					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
				}
				else if (std::regex_match(str, matches, pattern4)) {
					main_mutex.lock();
					main_queue.push("-m 2000 \"Just showing a notification bar lol\"");
					main_queue.push("-m 4000 \"and this is another notification\"");
					main_queue.push("-m 5000 \"and another i guess\"");
					main_queue.push("-c input_mode: theme");
					main_mutex.unlock();
					str.resize(str.size() - 5);
					output_mutex.lock();
					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X),static_cast<SHORT>(origin.Y) });
					std::cout<< str << "                                                                           ";
					SetConsoleCursorPosition(hConsole, { static_cast<SHORT>(origin.X + str.size() ),static_cast<SHORT>(origin.Y) });
					output_mutex.unlock();
				}
			}
			prev = ch;
			/*std::string s = "-m 5000 \"current mode is ";
			s += input_mode;s += "\"";
			main_mutex.lock();
			main_queue.push(s);
			main_mutex.unlock();*/
		}
		running = false;
	}
	chess() {

		board[0][0] = "BlackRook";
		board[1][0] = "BlackKnight";
		board[2][0] = "BlackBishop";
		board[3][0] = "BlackQueen";
		board[4][0] = "BlackKing";
		board[5][0] = "BlackBishop";
		board[6][0] = "BlackKnight";
		board[7][0] = "BlackRook";
		for (int i = 0;i < 8;i++) {
			board[i][1] = "BlackSoldier";
			board[i][2] = "null";
			board[i][3] = "null";
			board[i][4] = "null";
			board[i][5] = "null";
			board[i][6] = "WhiteSoldier";
		}
		board[0][7] = "WhiteRook";
		board[1][7] = "WhiteKnight";
		board[2][7] = "WhiteBishop";
		board[3][7] = "WhiteQueen";
		board[4][7] = "WhiteKing";
		board[5][7] = "WhiteBishop";
		board[6][7] = "WhiteKnight";
		board[7][7] = "WhiteRook";
		board[4][4] = "BlackQueen";
		clear(valid);
		menu_colours_array[2][0].black_pieces = "\033[38;5;54m";// alien
		menu_colours_array[2][0].white_pieces = "\033[36m";
		menu_colours_array[2][0].white_boxes = "\x1b[38;5;117m";
		menu_colours_array[2][0].borders = "\x1b[38;5;56m";
		menu_colours_array[2][0].attack_spots = "\x1b[38;5;89m";
		menu_colours_array[2][0].safe_spots = "\x1b[38;5;77m";
		menu_colours_array[2][0].coordinates = "\x1b[38;5;42m";
		
		menu_colours_array[1][0].black_pieces = "\x1b[38;5;25m"; //snowy theme
		menu_colours_array[1][0].white_pieces = "\x1b[38;5;87m";
		menu_colours_array[1][0].white_boxes = "\x1b[38;5;153m";
		menu_colours_array[1][0].borders = "\x1b[38;5;195m";
		menu_colours_array[1][0].attack_spots = "\x1b[38;5;161m";
		menu_colours_array[1][0].safe_spots = "\x1b[38;5;82m";
		menu_colours_array[1][0].coordinates = "\x1b[38;5;33m";

		menu_colours_array[1][2].black_pieces = "\x1b[38;5;243m"; //space
		menu_colours_array[1][2].white_pieces = "\x1b[38;5;254m";
		menu_colours_array[1][2].white_boxes = "\x1b[38;5;236m";
		menu_colours_array[1][2].borders = "\x1b[38;5;254m";
		menu_colours_array[1][2].attack_spots = "\x1b[38;5;217m";
		menu_colours_array[1][2].safe_spots = "\x1b[38;5;194m";
		menu_colours_array[1][2].coordinates = "\x1b[38;5;249m";

		menu_colours_array[0][2].black_pieces = "\x1b[38;5;225m"; //arora
		menu_colours_array[0][2].white_pieces = "\x1b[38;5;159m";
		menu_colours_array[0][2].white_boxes = "\x1b[38;5;158m";
		menu_colours_array[0][2].borders = "\x1b[38;5;225m";
		menu_colours_array[0][2].attack_spots = "\x1b[38;5;217m";
		menu_colours_array[0][2].safe_spots = "\x1b[38;5;193m";
		menu_colours_array[0][2].coordinates = "\x1b[38;5;123m";

		menu_colours_array[1][1].black_pieces = "\x1b[38;5;2m"; //code
		menu_colours_array[1][1].white_pieces = "\x1b[38;5;46m";
		menu_colours_array[1][1].white_boxes = "\x1b[38;5;22m";
		menu_colours_array[1][1].borders = "\x1b[38;5;34m";
		menu_colours_array[1][1].attack_spots = "\x1b[38;5;1m";
		menu_colours_array[1][1].safe_spots = "\x1b[38;5;21m";
		menu_colours_array[1][1].coordinates = "\x1b[38;5;82m";

		menu_colours_array[2][1].black_pieces = "\x1b[38;5;226m"; //colours
		menu_colours_array[2][1].white_pieces = "\x1b[38;5;51m";
		menu_colours_array[2][1].white_boxes = "\x1b[38;5;46m";
		menu_colours_array[2][1].borders = "\x1b[38;5;20m";
		menu_colours_array[2][1].attack_spots = "\x1b[38;5;201m";
		menu_colours_array[2][1].safe_spots = "\x1b[38;5;202m";
		menu_colours_array[2][1].coordinates = "\x1b[38;5;42m";

		menu_colours_array[2][2].black_pieces = "\x1b[38;5;1m"; //rose
		menu_colours_array[2][2].white_pieces = "\x1b[38;5;3m";
		menu_colours_array[2][2].white_boxes = "\x1b[38;5;244m";
		menu_colours_array[2][2].borders = "\x1b[38;5;236m";
		menu_colours_array[2][2].attack_spots = "\x1b[38;5;52m";
		menu_colours_array[2][2].safe_spots = "\x1b[38;5;17m";
		menu_colours_array[2][2].coordinates = "\x1b[38;5;91m";

		menu_colours_array[0][0].black_pieces = "\x1b[38;5;1m"; //lapis
		menu_colours_array[0][0].white_pieces = "\x1b[38;5;20m";
		menu_colours_array[0][0].white_boxes = "\x1b[38;5;75m";
		menu_colours_array[0][0].borders	= "\x1b[38;5;124m";
		menu_colours_array[0][0].attack_spots = "\x1b[38;5;127m";
		menu_colours_array[0][0].safe_spots = "\x1b[38;5;44m";
		menu_colours_array[0][0].coordinates = "\x1b[38;5;226m";

		menu_colours_array[0][1].black_pieces = "\033[38;5;130m";// hello, world
		menu_colours_array[0][1].white_pieces = "\033[38;5;216m";
		menu_colours_array[0][1].white_boxes = "\033[0m";
		menu_colours_array[0][1].borders = "\033[0m";
		menu_colours_array[0][1].attack_spots = "\033[38;5;88m";
		menu_colours_array[0][1].safe_spots = "\033[38;5;22m";
		menu_colours_array[0][1].coordinates = "\033[32m";

		menu_array[0][0][0] = "\033[34m    ____       ____      ";
		menu_array[0][0][1] = "  \033[38;5;88m_\033[34m/\033[38;5;88m_\033[34m__/       \\__\033[38;5;88m_\033[34m\\\033[38;5;88m_\033[34m    ";
		menu_array[0][0][2] = "\033[34m /\033[38;5;88m\\__\\\033[36m-----------\033[38;5;88m/__/\033[34m\\   ";
		menu_array[0][0][3] = "\033[34m|  \033[38;5;88m___   \033[34mlapis   \033[38;5;88m___\033[34m  |  ";
		menu_array[0][0][4] = "\033[34m \\\033[38;5;88m/__/\033[36m-----------\033[38;5;88m\\__\\\033[34m/   ";
		menu_array[0][0][5] = "\033[34m   \\____      _____/     ";
		menu_array[0][0][6] = "                         ";

		menu_array[0][1][0] = "\x1b[38;5;249m    __________________   ";
		menu_array[0][1][1] = "\x1b[38;5;249m  _/\x1b[38;5;238m_____________/___    ";
		menu_array[0][1][2] = "\x1b[38;5;249m /-\x1b[38;5;238m/_/\x1b[38;5;51m///////////\x1b[38;5;238m/__/\\   ";
		menu_array[0][1][3] = "\x1b[38;5;249m/-\x1b[38;5;238m/_/\x1b[38;5;51m///\x1b[38;5;159msnowy\x1b[38;5;51m///\x1b[38;5;238m/__/  \\  ";
		menu_array[0][1][4] = "\x1b[38;5;249m\\-\x1b[38;5;238m\\_\\\x1b[38;5;51m\\\\\\\\\\\\\\\\\\\\\\\x1b[38;5;238m\\__\\  /  ";
		menu_array[0][1][5] = "\x1b[38;5;249m \\-\x1b[38;5;238m\\_\\-----------\\__\\/   ";
		menu_array[0][1][6] = "\x1b[38;5;249m   \\____------_____/     ";

		menu_array[0][2][0] = "\x1b[38;5;117m            _____________";
		menu_array[0][2][1] = "        \x1b[38;5;42mo\x1b[38;5;241m==\x1b[38;5;117m<_\x1b[38;5;54m/\x1b[38;5;117m_\x1b[38;5;54m/\x1b[38;5;117m_\x1b[38;5;54m/\x1b[38;5;117m_\x1b[38;5;54m/\x1b[38;5;117m_\x1b[38;5;54m/\x1b[38;5;117m_/ ";
		menu_array[0][2][2] = "                         ";
		menu_array[0][2][3] = "          \x1b[38;5;57mAlien          ";
		menu_array[0][2][4] = "\x1b[38;5;117m_____________             ";
		menu_array[0][2][5] = "\x1b[38;5;117m\\_\033[36m\\\x1b[38;5;117m_\033[36m\\\x1b[38;5;117m_\033[36m\\\x1b[38;5;117m_\033[36m\\\x1b[38;5;117m_\033[36m\\\x1b[38;5;117m_>\x1b[38;5;241m==\x1b[38;5;89mo\x1b[38;5;255m         ";
		menu_array[0][2][6] = "                         ";

		menu_array[1][0][0] = "\x1b[38;5;247m#include \x1b[38;5;223m<\x1b[38;5;216mstdio.h\x1b[38;5;223m>       ";
		menu_array[1][0][1] = "\x1b[38;5;31mvoid\033[0m \x1b[38;5;11mmain\x1b[38;5;249m() {            ";
		menu_array[1][0][2] = "   \x1b[38;5;213mfor\033[0m(\x1b[38;5;31mint\x1b[38;5;123m i\x1b[38;5;249m=\x1b[38;5;151m0\x1b[38;5;249m;\x1b[38;5;123mi\x1b[38;5;249m<\x1b[38;5;151m9\x1b[38;5;249m;\x1b[38;5;123mi\x1b[38;5;249m++){ ";
		menu_array[1][0][3] = "   \x1b[38;5;213mfor\033[0m(\x1b[38;5;31mint\x1b[38;5;123m j\x1b[38;5;249m=\x1b[38;5;151m0\x1b[38;5;249m;\x1b[38;5;123mj\x1b[38;5;249m<\x1b[38;5;151m9\x1b[38;5;249m;\x1b[38;5;123mj\x1b[38;5;249m++){ ";
		menu_array[1][0][4] = "      \x1b[38;5;29m//Hello, world!    ";
		menu_array[1][0][5] = "      \x1b[38;5;11mprintf\033[0m(\033[33m\"#\"\033[0m);}      ";
		menu_array[1][0][6] = "      \x1b[38;5;11mprintf\033[0m(\033[33m\"\\n\"\033[0m);}}    ";

		menu_array[1][1][0] = "\x1b[38;5;2m     1   1  \x1b[38;5;22m0\x1b[38;5;2m   0        ";
		menu_array[1][1][1] = "       \x1b[38;5;46m1\x1b[38;5;2m   0  1     0    ";
		menu_array[1][1][2] = " \x1b[38;5;22m0\x1b[38;5;2m   0 1 1  0   \x1b[38;5;46m1 1\x1b[38;5;2m  0   ";
		menu_array[1][1][3] = "\x1b[38;5;2m 1  1    <code> 0  \x1b[38;5;22m0\x1b[38;5;2m  1  ";
		menu_array[1][1][4] = "\x1b[38;5;46m     0  \x1b[38;5;2m0   0  1  \x1b[38;5;46m1\x1b[38;5;2m      ";
		menu_array[1][1][5] = "\x1b[38;5;2m    1    0\x1b[38;5;46m 1\x1b[38;5;2m  0       \x1b[38;5;22m0  ";
		menu_array[1][1][6] = "                         ";

		menu_array[1][2][0] = "    \x1b[38;5;21m| \x1b[38;5;226m|       \x1b[38;5;214m| \x1b[38;5;196m| \x1b[38;5;21m|   \x1b[38;5;197m|  ";
		menu_array[1][2][1] = "   \x1b[38;5;83m|\x1b[38;5;21m| \x1b[38;5;226m|     \x1b[38;5;2m| \x1b[38;5;214m|     \x1b[38;5;46m|    ";
		menu_array[1][2][2] = " \x1b[38;5;196m| \x1b[38;5;83m|    \x1b[38;5;208m|\x1b[38;5;43m|    \x1b[38;5;214m| \x1b[38;5;196m| \x1b[38;5;57m| \x1b[38;5;46m|   \x1b[38;5;226m|";
		menu_array[1][2][3] = " \x1b[38;5;196m| \x1b[38;5;83m|  \x1b[38;5;200m|  \x1b[38;5;43m|\x1b[38;5;129mc\x1b[38;5;93mo\x1b[38;5;21ml\x1b[38;5;82mo\x1b[38;5;226mu\x1b[38;5;202mr\x1b[38;5;196ms \x1b[38;5;57m|     \x1b[38;5;226m|";
		menu_array[1][2][4] = "   \x1b[38;5;83m|\x1b[38;5;196m| \x1b[38;5;200m|  \x1b[38;5;43m| \x1b[38;5;99m|   \x1b[38;5;213m|    \x1b[38;5;28m|   \x1b[38;5;226m|";
		menu_array[1][2][5] = "\x1b[38;5;21m|   \x1b[38;5;196m| \x1b[38;5;200m|  \x1b[38;5;43m| \x1b[38;5;99m|  \x1b[38;5;226m|  \x1b[38;5;21m|\x1b[38;5;200m| \x1b[38;5;28m|    ";
		menu_array[1][2][6] = "        \x1b[38;5;56m|    \x1b[38;5;56m|    \x1b[38;5;200m|\x1b[37m      ";

		menu_array[2][0][0] = "\x1b[38;5;231m  _\x1b[38;5;195m___\x1b[38;5;159m___\x1b[38;5;123m___\x1b[38;5;87m___\x1b[38;5;51m___       ";
		menu_array[2][0][1] = "\x1b[38;5;231m /_\x1b[38;5;225m___\x1b[38;5;219m___\x1b[38;5;213m___\x1b[38;5;212m___\x1b[38;5;211m___\x1b[38;5;210m___    ";
		menu_array[2][0][2] = "\x1b[38;5;231m/   \x1b[38;5;225m\\__\x1b[38;5;183m___\x1b[38;5;141m___   \x1b[38;5;87m___\x1b[38;5;51m___   ";
		menu_array[2][0][3] = "\x1b[38;5;231m\\_\x1b[38;5;194m___\x1b[38;5;157m___\x1b[38;5;121mAro\x1b[38;5;122mra_\x1b[38;5;123m_/  \x1b[38;5;75m __    ";
		menu_array[2][0][4] = "\x1b[38;5;231m \\\x1b[38;5;230m___\x1b[38;5;191m/  \\   \x1b[38;5;87m\\__\x1b[38;5;14m___\x1b[38;5;45m/\x1b[38;5;87m__\x1b[38;5;86m___ ";
		menu_array[2][0][5] = "\x1b[38;5;231m     \x1b[38;5;228m\\__ \x1b[38;5;190m\\__\x1b[38;5;184m___\x1b[38;5;214m___\x1b[38;5;209m___\x1b[38;5;203m__  ";
		menu_array[2][0][6] = "\x1b[38;5;231m       \x1b[38;5;227m\\__\x1b[38;5;226m___       \x1b[38;5;231m     ";

		menu_array[2][1][0] = "\x1b[38;5;255m                 *       ";
		menu_array[2][1][1] = "\x1b[38;5;255m           .      \x1b[38;5;252m\\  \x1b[38;5;255mo   ";
		menu_array[2][1][2] = "\x1b[38;5;255m     o          \x1b[38;5;255mo  \x1b[38;5;249m\\  \x1b[38;5;240m.  ";
		menu_array[2][1][3] = "\x1b[38;5;255m  \x1b[38;5;240m'\x1b[38;5;255m  .    \x1b[38;5;240m*\x1b[38;5;255mspace .  \x1b[38;5;246m\\\x1b[38;5;255m. * ";
		menu_array[2][1][4] = "\x1b[38;5;255m       \x1b[38;5;240m.   \x1b[38;5;255m.    *    \x1b[38;5;243m\\   ";
		menu_array[2][1][5] = "\x1b[38;5;255m   *  . \x1b[38;5;255m_\x1b[38;5;250m_\x1b[38;5;247m_\x1b[38;5;246m_\x1b[38;5;244m_\x1b[38;5;243m_\x1b[38;5;241m_\x1b[38;5;240m_\x1b[38;5;238m_\x1b[38;5;237m_    \\  ";
		menu_array[2][1][6] = "\x1b[38;5;255m      \x1b[38;5;240m.            \x1b[38;5;255m*  .  ";

		menu_array[2][2][0] = "\x1b[38;5;236m      /|   \x1b[38;5;160m/\\   \x1b[38;5;236m|\\       ";
		menu_array[2][2][1] = "\x1b[38;5;236m  \x1b[38;5;160m|\x1b[38;5;236m  / |  \x1b[38;5;160m/  \\  \x1b[38;5;236m| \\  \x1b[38;5;160m|   ";
		menu_array[2][2][2] = "\x1b[38;5;236m|\\\x1b[38;5;160m|\\ \x1b[38;5;236m|\x1b[38;5;3m|\x1b[38;5;236m\\ \x1b[38;5;160m|    |\x1b[38;5;236m /\x1b[38;5;3m|\x1b[38;5;236m| \x1b[38;5;160m/|\x1b[38;5;236m/| ";
		menu_array[2][2][3] = "\x1b[38;5;236m| \\\x1b[38;5;3m\\\x1b[38;5;160m\\\x1b[38;5;236m| \x1b[38;5;3m\\\x1b[38;5;236m\\\x1b[38;5;160m|\x1b[38;5;88mrose\x1b[38;5;160m|\x1b[38;5;236m/\x1b[38;5;3m/ \x1b[38;5;236m|\x1b[38;5;160m/\x1b[38;5;3m/\x1b[38;5;236m/ | ";
		menu_array[2][2][4] = "\x1b[38;5;236m|\x1b[38;5;3m\\\x1b[38;5;236m \\\x1b[38;5;3m\\\x1b[38;5;160m\\\x1b[38;5;236m\\_\x1b[38;5;3m\\ \x1b[38;5;160m\\  /\x1b[38;5;3m /\x1b[38;5;236m_/\x1b[38;5;160m/\x1b[38;5;3m/\x1b[38;5;236m/ \x1b[38;5;3m/\x1b[38;5;236m| ";
		menu_array[2][2][5] = "\x1b[38;5;236m \\\x1b[38;5;3m\\\x1b[38;5;236m \\\x1b[38;5;3m\\\x1b[38;5;236m__\\\x1b[38;5;3m\\_\x1b[38;5;160m\\/\x1b[38;5;3m_/\x1b[38;5;236m/__\x1b[38;5;3m/\x1b[38;5;236m/ \x1b[38;5;3m/\x1b[38;5;236m/  ";
		menu_array[2][2][6] = "\x1b[38;5;236m  \\___\\__________/___/   ";



		menu_select_array[0][0][0] = R"(    ____       ____      )";
		menu_select_array[0][0][1] = R"(  _/___/       \___\_    )";
		menu_select_array[0][0][2] = R"( /\__\-----------/__/\   )";
		menu_select_array[0][0][3] = R"(|  ___   lapis   ___  |  )";
		menu_select_array[0][0][4] = R"( \/__/-----------\__\/   )";
		menu_select_array[0][0][5] = R"(   \____      _____/     )";
		menu_select_array[0][0][6] = R"(                         )";

		menu_select_array[0][1][0] = R"(    __________________   )";
		menu_select_array[0][1][1] = R"(  _/_____________/___    )";
		menu_select_array[0][1][2] = R"( /-/_/////////////__/\   )";
		menu_select_array[0][1][3] = R"(/-/_////snowy////__/  \  )";
		menu_select_array[0][1][4] = R"(\-\_\\\\\\\\\\\\\__\  /  )";
		menu_select_array[0][1][5] = R"( \-\_\-----------\__\/   )";
		menu_select_array[0][1][6] = R"(   \____------_____/     )";

		menu_select_array[0][2][0] = R"(            _____________)";
		menu_select_array[0][2][1] = R"(        o==<_/_/_/_/_/_/ )";
		menu_select_array[0][2][2] = R"(                         )";
		menu_select_array[0][2][3] = R"(          Alien          )";
		menu_select_array[0][2][4] = R"(____________             )";
		menu_select_array[0][2][5] = R"(\_\_\_\_\_\_>==o         )";
		menu_select_array[0][2][6] = R"(                         )";

		menu_select_array[1][0][0] = R"(#include <stdio.h>       )";
		menu_select_array[1][0][1] = R"(void main() {            )";
		menu_select_array[1][0][2] = R"(   for(int i=0;i<9;i++){ )";
		menu_select_array[1][0][3] = R"(   for(int j=0;j<9;j++){ )";
		menu_select_array[1][0][4] = R"(      //Hello, world!    )";
		menu_select_array[1][0][5] = R"(      printf("#");}      )";
		menu_select_array[1][0][6] = R"(      printf("\n");}}    )";


		menu_select_array[1][1][0] = R"(     1   1  0   0        )";
		menu_select_array[1][1][1] = R"(       1   0  1     0    )";
		menu_select_array[1][1][2] = R"( 0   0 1 1  0   1 1  0   )";
		menu_select_array[1][1][3] = R"( 1  1    <code> 0  0  1  )";
		menu_select_array[1][1][4] = R"(     0  0   0  1  1      )";
		menu_select_array[1][1][5] = R"(    1    0 1  0       0  )";
		menu_select_array[1][1][6] = R"(                         )";

		menu_select_array[1][2][0] = R"(    | |       | | |   |  )";
		menu_select_array[1][2][1] = R"(   || |     | |     |    )";
		menu_select_array[1][2][2] = R"( | |    ||    | | | |   |)";
		menu_select_array[1][2][3] = R"( | |  |  |colours |     |)";
		menu_select_array[1][2][4] = R"(   || |  | |   |    |   |)";
		menu_select_array[1][2][5] = R"(|   | |  | |  |  || |    )";
		menu_select_array[1][2][6] = R"(        |    |    |      )";

		menu_select_array[2][0][0] = R"(  ________________       )";
		menu_select_array[2][0][1] = R"( /___________________    )";
		menu_select_array[2][0][2] = R"(/   \________   ______   )";
		menu_select_array[2][0][3] = R"(\_______Arora__/   __    )";
		menu_select_array[2][0][4] = R"( \___/  \   \_____/_____ )";
		menu_select_array[2][0][5] = R"(     \__ \_____________  )";
		menu_select_array[2][0][6] = R"(       \_____            )";

		menu_select_array[2][1][0] = R"(                 *       )";
		menu_select_array[2][1][1] = R"(           .      \  o   )";
		menu_select_array[2][1][2] = R"(     o          o  \  .  )";
		menu_select_array[2][1][3] = R"(  '  .    *space .  \. * )";
		menu_select_array[2][1][4] = R"(       .   .    *    \   )";
		menu_select_array[2][1][5] = R"(   *  . __________    \  )";
		menu_select_array[2][1][6] = R"(      .            *  .  )";

		menu_select_array[2][2][0] = R"(      /|   /\   |\       )";
		menu_select_array[2][2][1] = R"(  |  / |  /  \  | \  |   )";
		menu_select_array[2][2][2] = R"(|\|\ ||\ |    | /|| /|/| )";
		menu_select_array[2][2][3] = R"(| \\\| \\|rose|// |/// | )";
		menu_select_array[2][2][4] = R"(|\ \\\\_\ \  / /_//// /| )";
		menu_select_array[2][2][5] = R"( \\ \\__\\_\/_//__// //  )";
		menu_select_array[2][2][6] = R"(  \___\__________/___/   )";



		//menu_colours_array[0][0].attack_spots = "";
	}
	char data(std::string name, int i) {
		if (name[name.length() - 1] == 'k') {
			return "   ## ## ##      ########       ######        ######        ######       ########      ########   "[i];
		}
		if (name[name.length() - 1] == 't') {
			return "     ____         ######       /######       ### ###         #####\\      ########      ########   "[i];
		}
		if (name[name.length() - 1] == 'p') {
			return "      /\\            \\/           /##\\         /####\\        \\####/        /####\\       ########   "[i];
		}
		if (name[name.length() - 1] == 'g') {
			return "      <>          /\\/\\/\\        \\#\\/#/        <####>         |##|         /####\\       ########   "[i];
		}
		if (name[name.length() - 1] == 'n') {
			return "     VVVV          \\##/         <####>         |##|         /####\\       ########      ########   "[i];
		}
		if (name[name.length() - 1] == 'r') {
			return "                   /##\\          \\##/          |##|          /##\\         ######        ######    "[i];
		}
		if (name[name.length() - 1] == 'l') {
			return "                                                                                                  "[i];
		}

		return '@';
	}
	void clear(int board[8][8]) {
		for (int i = 0;i < 8;i++) {
			for (int j = 0;j < 8;j++) {
				board[i][j] = 0;
			}
		}
	}
	bool _in_board(int x, int y) {
		return (x >= 0 && x < 8 && y >= 0 && y < 8);
	}
	// output processing
	void refresh_chessboard_buffer() {
		for (int i = 0;i <= 63;i++)chessboard_buffer[i] = "";
		chessboard_buffer[0] += colours.borders + "   ________________________________________________________________________________________________________________________   ";
		chessboard_buffer[1] += colours.borders + "  /  ____________________________________________________________________________________________________________________  \\  ";
		chessboard_buffer[2] += colours.borders + " /  /  ________________________________________________________________________________________________________________  \\  \\ ";
		chessboard_buffer[3] += colours.borders + "/  /  /" + colours.coordinates + "      A              B             C             D             E             F             G             H      " + colours.borders + "\\  \\  \\";
		int line_no = 0;
		int index = 0;
		std::string current_colour = colours.borders;
		for (int row = 0;row < 8;row++) {
			for (int r = 0;r < 7;r++) {
				current_colour = colours.borders;
				chessboard_buffer[line_no + 4] += colours.borders;
				chessboard_buffer[line_no + 4] += _left_side(row, r);
				for (int collum = 0;collum < 8;collum++) {
					for (int c = 0;c < 14;c++) {
						char character = data(board[collum][row], c + index);
						if (character == ' ') {
							if ((collum + row) % 2 == 0) {
								if (valid[collum][row]) {
									if (board[collum][row][0] == 'n') {
										if (colours.safe_spots != current_colour) {//green shows this colour when the piece is been selected
											current_colour = colours.safe_spots;
											chessboard_buffer[line_no + 4] += colours.safe_spots;
										}
										chessboard_buffer[line_no + 4] += "X";
									}
									else
									{
										if (colours.attack_spots != current_colour) {//red shows this colour when the piece is attacking
											current_colour = colours.attack_spots;
											chessboard_buffer[line_no + 4] += colours.attack_spots;
										}
										chessboard_buffer[line_no + 4] += "X";
									}

								}
								else
								{
									if (colours.white_boxes != current_colour) {
										current_colour = colours.white_boxes;
										chessboard_buffer[line_no + 4] += colours.white_boxes;
									}
									chessboard_buffer[line_no + 4] += "X";
								}
							}
							else {
								if (valid[collum][row]) {
									if (board[collum][row][0] == 'n') {//green shows this colour when the piece is been selected
										if (colours.safe_spots != current_colour) {
											current_colour = colours.safe_spots;
											chessboard_buffer[line_no + 4] += colours.safe_spots;
										}
										chessboard_buffer[line_no + 4] += "~";
									}
									else
									{
										if (colours.attack_spots != current_colour) {//red shows this colour when the piece is attacking
											current_colour = colours.attack_spots;
											chessboard_buffer[line_no + 4] += colours.attack_spots;
										}
										chessboard_buffer[line_no + 4] += "~";
									}
								}
								else
								{
									chessboard_buffer[line_no + 4] += " ";
								}
							}
						}
						else {
							if (board[collum][row][0] == 'B') {//black pieces
								if (colours.black_pieces != current_colour) {
									current_colour = colours.black_pieces;
									chessboard_buffer[line_no + 4] += colours.black_pieces;
								}
								chessboard_buffer[line_no + 4] += character;
							}
							else
							{
								if (colours.white_pieces != current_colour) {//white pieces
									current_colour = colours.white_pieces;
									chessboard_buffer[line_no + 4] += colours.white_pieces;
								}
								chessboard_buffer[line_no + 4] += character;
							}

						}
					}
				}
				index += 14;
				if (colours.borders != current_colour) {//borders
					current_colour = colours.borders;
					chessboard_buffer[line_no + 4] += colours.borders;
				}
				chessboard_buffer[line_no + 4] += _right_side(row, r);
				line_no += 1;
			}
			index = 0;
		}
		chessboard_buffer[line_no + 4] += colours.borders + "| |  | " + colours.coordinates + "      A             B              C            D             E              F            G              H    " + colours.borders + "   |  | |";
		chessboard_buffer[line_no + 5] += colours.borders + "\\  \\  \\________________________________________________________________________________________________________________/  /  /";
		chessboard_buffer[line_no + 6] += colours.borders + " \\  \\____________________________________________________________________________________________________________________/  / ";
		chessboard_buffer[line_no + 7] += colours.borders + "  \\________________________________________________________________________________________________________________________/  \033[0m";

	}
	void refresh_timer_buffer(double btimer = 12 * 60 + 34, double wtimer = 12 * 60 + 34) {
		for (std::string str : timer_buffer) str = "";
		int b = btimer, w = wtimer;
		//23 * 74
		int bminutes = (b % 3600) / 60;
		int wminutes = (w % 3600) / 60;
		int bseconds = b % 60;
		int wseconds = w % 60;

		std::string bmin = (bminutes < 10 ? "0" : "") + std::to_string(bminutes);
		std::string wmin = (wminutes < 10 ? "0" : "") + std::to_string(wminutes);
		std::string bsec = (bseconds < 10 ? "0" : "") + std::to_string(bseconds);
		std::string wsec = (wseconds < 10 ? "0" : "") + std::to_string(wseconds);

		int bmin1 = bmin[0] - '0';
		int bmin2 = bmin[1] - '0';
		int bsec1 = bsec[0] - '0';
		int bsec2 = bsec[1] - '0';

		int wmin1 = wmin[0] - '0';
		int wmin2 = wmin[1] - '0';
		int wsec1 = wsec[0] - '0';
		int wsec2 = wsec[1] - '0';

		std::string colon[10] = { "    ","    ","####","####","    ",		"    ","####","####","    ","    " };

		for (int k = 0; k < 10; k++) {
			timer_buffer[k] = "  " + numbers[bmin1][k] + "  " + numbers[bmin2][k] + "  " + colon[k] + "  " + numbers[bsec1][k] + "  " + numbers[bsec2][k];
		}
		timer_buffer[10] = "                                                                          ";
		timer_buffer[11] = "                                                                          ";
		for (int k = 0; k < 10; k++) {
			timer_buffer[k + 12] = "  " + numbers[wmin1][k] + "  " + numbers[wmin2][k] + "  " + colon[k] + "  " + numbers[wsec1][k] + "  " + numbers[wsec2][k];
		}
	}
	void _notification_buffer_refresher() {
		int j = 0;
		for (int i = 0; i < 22; i++) {
			if (i >= notifications_on_board.size()) break;
			int size = notifications_on_board[i].message.length() / 32;
			for (int k = 0; k <= size; k++) {
				if (j >= 22) break;
				//_notification_buffer[j] = "\033[31m";
				_notification_buffer[j] = std::string("\033[31m") + notifications_on_board[i].message.substr(k * 32, 32);
				// pad with spaces if last chunk is shorter than 32
				if (k == size) {
					int len = 32 - _notification_buffer[j].length()+sizeof("\033[31m");
					_notification_buffer[j] += std::string(len, ' ');
				}
				j++;
			}
			// Add blank line between notifications if space allows
			if (j < 22) {
				_notification_buffer[j] = "                                   ";
				j++;
			}
			else {
				break;
			}
		}
		// Fill remaining lines with blanks
		for (int i = j; i < 22; i++) {
			_notification_buffer[i] = "                                   ";
		}
	}
	void refresh_theme_buffer() {
		for (int e = 0;e < 36;e++) theme_menu_buffer[e] = "";
		int itt = 0;
		for (int k = 0;k < 3;k++) {
			itt += 5;
			for (int j = 0;j < 7;j++) {
				for (int i = 0; i < 3; i++) {
					if (i == theme_index.x && k == theme_index.y) {
						theme_menu_buffer[itt] += "\x1b[38;5;87m        ";
						theme_menu_buffer[itt] += menu_select_array[k][i][j];
					}
					else {
						theme_menu_buffer[itt] += "        ";
						theme_menu_buffer[itt] += menu_array[k][i][j];
					}
				}
				itt++;
			}
		}
		for (int e = 0;e < 36;e++) {
			if (theme_menu_buffer[e] == "") {
				theme_menu_buffer[e] = "                                                                                                   ";
			}
		}
	}
	//input pocessing
	void comand_executer(std::string comand) {
		std::smatch matches;
		for (char& c : comand) c = std::tolower(c);
		std::regex pattern1(R"(^\s*c\s*(([a-h])(1|8))\s*$)");// c a8 : does casting on the board if applicable
		std::regex pattern2(R"(^\s*(c|clear)\s*$)");// c | clear
		std::regex pattern3(R"(^\s*(t|test)\s*$)");// t | test
		std::regex pattern4(R"(^\s*(r|restart|restore)\s*$)");// r | restart
		std::regex pattern5(R"(^\s*(((b|w)(p|b|q|r|k|ki))|(n))\s+([a-h][1-8])\s*$)");// bki f5 : this places the piece on the specified spot.
		std::regex pattern6(R"(^\s*([a-h][1-8])\s+([a-h][1-8])\s*$)");// a1 a3
		std::regex pattern7(R"(^\s*([a-h][1-8])\s*$)");// a1
		std::regex pattern8(R"(^\s*-m\s+(\d*)\s+"([^"]+)\"\s*$)");// this is for giving a command to run a mesage in notification bar.  -m 3000 "eee"
		std::regex pattern9(R"(^\s*-r\s+([0-9]+)\s*$)");// -r 1245 : removes the notification of id 1245
		std::regex pattern10(R"(\s*-i\s+([a-zA-Z0-9]+)\s*$)");//for input of the theme input modemode
		std::regex pattern11(R"(\s*-c\s+([^\s]+)\s+([^\s]+))");
		//filtering the comand, and executing it.
		if (std::regex_match(comand, matches, pattern1) ){
			int x, y;
			char enemy;
			if (matches[3] == '8') enemy = 'W'; else enemy = 'B';
			str_to_cords(matches[1], x, y);
			int spots[8][8];//places where enemy can attack
			clear(spots);
			_All_possible_spots(spots, enemy);
			//print(spots);

			std::cout << "\n" << enemy << "                           ";
			if (matches[1] == "a8" && black_left_rook_can_castel && board[1][0] == "null" && board[2][0] == "null" && board[3][0] == "null" && spots[2][0] == 0 && spots[3][0] == 0 && spots[4][0] == 0) {//a8 black
				board[4][0] = 'null';
				board[2][0] = "BlackKing";
				board[0][0] = 'null';
				board[3][0] = "BlackRook";
				black_left_rook_can_castel = false;
				black_right_rook_can_castel = false;
			}
			else if (matches[1] == "h8" && black_right_rook_can_castel && board[5][0] == "null" && board[6][0] == "null" && spots[4][0] == 0 && spots[5][0] == 0 && spots[6][0] == 0) {
				board[4][0] = 'null';
				board[6][0] = "BlackKing";
				board[7][0] = 'null';
				board[5][0] = "BlackRook";
				black_left_rook_can_castel = false;
				black_right_rook_can_castel = false;
			}
			else if (matches[1] == "a1" && white_right_rook_can_castel && board[1][7] == "null" && board[2][7] == "null" && board[3][7] == "null" && spots[4][7] == 0 && spots[3][7] == 0 && spots[2][7] == 0) {
				board[4][7] = "null";
				board[0][7] = "null";
				board[2][7] = "WhiteKing";
				board[3][7] = "WhiteRook";
				white_left_rook_can_castel = false;
				white_right_rook_can_castel = false;

			}
			else if (matches[1] == "h1" && white_right_rook_can_castel && board[5][7] == "null" && board[6][7] == "null" && spots[4][7] == 0 && spots[5][7] == 0 && spots[6][7] == 0) {
				board[4][7] = 'null';
				board[6][7] = "WhiteKing";
				board[7][7] = 'null';
				board[5][7] = "WhiteRook";
				white_left_rook_can_castel = false;
				white_right_rook_can_castel = false;
			}
		}
		else if (std::regex_match(comand, matches, pattern2)) {
			clear(valid);
			for (int i = 0;i < 8;i++) {
				for (int j = 0;j < 8;j++) {
					board[i][j] = "null";
				}
			}

		}
		else if (std::regex_match(comand, matches, pattern3)) {
			int temp[8][8];
			clear(temp);
			_All_possible_spots(temp, 'B');
			print(temp);
			/*clear(valid);
			for (int i = 0;i < 8;i++) {
				for (int j = 0;j < 8;j++) {
					board[i][j] = "null";
				}
			}
			board[7][7] = "WhiteKing";
			board[6][5] = "BlackKing";
			board[1][6] = "BlackQueen";*/
		}
		else if (std::regex_match(comand, matches, pattern4)) {
			clear(valid);
			board[0][0] = "BlackRook";
			board[1][0] = "BlackKnight";
			board[2][0] = "BlackBishop";
			board[3][0] = "BlackQueen";
			board[4][0] = "BlackKing";
			board[5][0] = "BlackBishop";
			board[6][0] = "BlackKnight";
			board[7][0] = "BlackRook";
			for (int i = 0;i < 8;i++) {
				board[i][1] = "BlackSoldier";//BlackSoldier
				board[i][2] = "null";
				board[i][3] = "null";
				board[i][4] = "null";
				board[i][5] = "null";
				board[i][6] = "WhiteSoldier";//WhiteSoldier
			}

			board[0][7] = "WhiteRook";
			board[1][7] = "WhiteKnight";
			board[2][7] = "WhiteBishop";
			board[3][7] = "WhiteQueen";
			board[4][7] = "WhiteKing";
			board[5][7] = "WhiteBishop";
			board[6][7] = "WhiteKnight";
			board[7][7] = "WhiteRook";
			white_left_rook_can_castel = true;
			white_right_rook_can_castel = true;
			black_left_rook_can_castel = true;
			black_right_rook_can_castel = true;


		}
		else if (std::regex_match(comand, matches, pattern5)) {
			int x, y;
			str_to_cords(matches[6], x, y);
			std::string str;
			if (matches[1] == "n") {
				str = "null";
			}
			else {
				if (matches[3] == 'w') {
					str = "White";
				}
				else {
					str = "Black";
				}
				if (matches[4] == "ki") {
					str += "King";
				}
				else if (matches[4] == 'k') {
					str += "Knight";
				}
				else if (matches[4] == 'r') {
					str += "Rook";
				}
				else if (matches[4] == 'p') {
					str += "Soldier";
				}
				else if (matches[4] == 'b') {
					str += "Bishop";
				}
				else if (matches[4] == 'q') {
					str += "Queen";
				}
			}
			board[x][y] = str;

		}
		else if (std::regex_match(comand, matches, pattern6)) {
			int x1, y1, x2, y2;
			str_to_cords(matches[1], x1, y1);
			str_to_cords(matches[2], x2, y2);
			clear(valid);
			Valid_Chess_Moves(x1, y1);
			if (valid[x2][y2] == 1) {
				int last_ch = board[x1][y1][board[x1][y1].length() - 1];
				if (last_ch == 'k' || last_ch == 'g') {//rook, king
					if (last_ch == 'g') {//king
						if (board[x1][y1][0] == 'B') {
							black_right_rook_can_castel = false;
							black_left_rook_can_castel = false;
						}
						else
						{
							white_right_rook_can_castel = false;
							white_left_rook_can_castel = false;
						}
					}
					if (last_ch == 'k') {//rook
						if (x1 == 0 && y1 == 0)      black_left_rook_can_castel = false;
						else if (x1 == 7 && y1 == 0) black_right_rook_can_castel = false;
						else if (x1 == 0 && y1 == 7) white_left_rook_can_castel = false;
						else if (x1 == 7 && y1 == 7) white_right_rook_can_castel = false;
					}
				}
				board[x2][y2] = board[x1][y1];
				board[x1][y1] = "null";
				clear(valid);
			}
		}
		else if (std::regex_match(comand, matches, pattern7)) {
			clear(valid);
			int n1 = 1, n2 = 0;
			str_to_cords(matches[1], n1, n2);
			Valid_Chess_Moves(n1, n2);
		}
		else if (std::regex_match(comand, matches, pattern8)) {// (^\s*(-m)\s+([0-9]*)\s+\"([ a-zA-Z0-9]+)\"\s*$)
			int id = assign_id++;
			std::string time = matches[1];
			std::string message = matches[2];
			int actiual_time = 2000;
			std::string command_for_timer_thread = "-r " + std::to_string(id);
			if (time != "") actiual_time = std::stoi(time);
			notifications_on_board.push_back({id,message});
			timestamp n;
			n.execution_time= actiual_time;
			n.comand= command_for_timer_thread;
			timer_mutex.lock();
			timer_queue.push(n);
			timer_mutex.unlock();
		}
		else if (std::regex_match(comand, matches, pattern9)) {
			int id = std::stoi(matches[1]);
			for (size_t i = 0; i < notifications_on_board.size(); ) {
				if (notifications_on_board[i].id == id)
					notifications_on_board.erase(notifications_on_board.begin() + i);
				else
					++i;
			}
		}
		else if (std::regex_match(comand, matches, pattern10)) {
			std::string ch = matches[1];
			if (input_mode == 't') {
				if (ch == "enter") input_mode = 's';
				else if (ch == "up") theme_index.y--;
				else if (ch =="down")theme_index.y++;
				else if (ch == "left")theme_index.x--;
				else if (ch == "right")theme_index.x++;
				if (theme_index.y == -1)theme_index.y = 2;
				if (theme_index.y == 3)theme_index.y = 0;
				if (theme_index.x == -1) theme_index.x = 2;
				if (theme_index.x == 3) theme_index.x = 0;
				colours.black_pieces= menu_colours_array[theme_index.x][theme_index.y].black_pieces;
				colours.white_pieces= menu_colours_array[theme_index.x][theme_index.y].white_pieces;
				colours.white_boxes= menu_colours_array[theme_index.x][theme_index.y].white_boxes;
				colours.coordinates= menu_colours_array[theme_index.x][theme_index.y].coordinates;
				colours.borders= menu_colours_array[theme_index.x][theme_index.y].borders;
				colours.safe_spots= menu_colours_array[theme_index.x][theme_index.y].safe_spots;
				colours.attack_spots = menu_colours_array[theme_index.x][theme_index.y].attack_spots;

			}
		}
		else if (std::regex_match(comand, matches, pattern11)) {
			input_mode = 't';
		}
	}
};
int main() {
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	chess Chess;
	std::thread timer_thread(&chess::timer_thread, &Chess);
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	Chess.main_mutex.lock();
	std::string temp_str = "-m 5000 \"Main thread # ";temp_str += std::to_string(GetCurrentProcessorNumber()); temp_str += '\"';
	Chess.main_queue.push(temp_str);
	Chess.main_mutex.unlock();
	Chess._notification_buffer_refresher();
	Chess.refresh_chessboard_buffer();
	Chess.refresh_timer_buffer();
	//Chess.refresh_theme_buffer();
	for (int i = 0;i <= 63;i++) {
		std::lock_guard<std::mutex> lock(Chess.output_mutex);
		std::cout << Chess.chessboard_buffer[i];
		if (i >= 0 && i <= 21) std::cout << Chess.timer_buffer[i] + Chess._notification_buffer[i];
		std::cout << "\n";
	}
	std::thread input_thread(&chess::input, &Chess);
	std::vector<std::string> commands;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	while (Chess.running) {
		{
			std::unique_lock<std::mutex>lock(Chess.main_mutex);
			Chess.main_cv.wait(lock, [&] {return !Chess.main_queue.empty();});
			while (!Chess.main_queue.empty()) {
				//transfering strings from main_queue to commands
				commands.push_back(Chess.main_queue.front());
				
				Chess.main_queue.pop();
			}
		}
		for (std::string st : commands) Chess.comand_executer(st);


		commands.clear();
		Chess._notification_buffer_refresher();
		Chess.refresh_chessboard_buffer();
		Chess.refresh_timer_buffer();
		Chess.refresh_theme_buffer();
		std::lock_guard<std::mutex> lock(Chess.output_mutex);
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		SetConsoleCursorPosition(hConsole, { 0,0 });
		for (int i = 0;i <= 63;i++) {
			std::cout << Chess.chessboard_buffer[i];
			if (i >= 0 && i <= 21) std::cout << Chess.timer_buffer[i] + "  " + Chess._notification_buffer[i];
			else if(i<57){
				if(Chess.input_mode == 't')std::cout << Chess.theme_menu_buffer[i - 21];
				else {
					std::cout << "                                                                                                   ";
				}
			}
			

			std::cout << "\n";

		}
		SetConsoleCursorPosition(hConsole, { csbi.dwCursorPosition.X,csbi.dwCursorPosition.Y });
	}
	input_thread.join();
	timer_thread.join();
	return 0;
}
