
/*
	Connect 4 AI in C

	Uses an efficient 128bit representation of the board
	based on John Trump's Fhourstones benchmark.

	By bit shifting the board we can line up all the
	possible sets of 4 lines in one direction all the
	same time, allowing us to check for victory in O(1).

	Original code: https://tromp.github.io/c4/Connect4.java
	
	FCUP-CC

	Luís Afonso Brandão
	João Mário Carvalhinho
											                */
#include <stdio.h>   //IO
#include <stdlib.h> //rand()
#include <time.h>  //time
#include <math.h> //exp()

typedef enum { false, true } bool;

//Guarantees 64bit integer representantion. We will need 48bits
typedef long unsigned int UI;

//Board sizes, cannot be changed 
#define BOARD_HEIGHT 6
#define BOARD_WIDTH  7

//Recursion limit on the MINMAX function
#define MAX_DEPTH    10

void printBoard_(UI x, UI y, UI t) {
	if((t+1)%BOARD_WIDTH)printf("%c", (x&1)?'X':((y&1)?'O':'.'));
	printf("%c",(!((t+1)%(BOARD_WIDTH)))?'\n':'|');
	if(t<48)printBoard_(x >> 1, y >> 1, ++t);
}
//-----------------------------------------------------------------------------
/* 
	
 EXAMPLE:

 -------
        |
        |
   O    |
   O    |       This board has a vertical win by the 'O' player.
  XO    |     This is the process to check the board for victory.
 XXO    |
 -------
 0123456


                  Less significant bit
                  |
                  v

                  col 1   col 2   col 3   col 4   col 5   col 6   col 7
 	boardO      = 000000  000000  111100  000000  000000  000000  000000

 	boardO >> 1 = 000000  000001  111000  000000  000000  000000  000000

	boardO >> 2 = 000000  000011  110000  000000  000000  000000  000000

 	boardO >> 3 = 000000  000111  100000  000000  000000  000000  000000

                                                          BITWISE AND(&)
	____________________________________________________________________

    result 		= 000000  000000  100000  000000  000000  000000  000000


Result != 0 => There is at least one segment of 4 connected 'O's, therefore there is a victory.

This process works for all directions, using different bit offsets.	
							                                                */
bool fourTogether(UI board) {
    UI y = board & (board >> 6);
    if (y & (y >> 2 * 6))     // check \  diagonal
        return true;
    y = board & (board >> 7);
    if (y & (y >> 2 * 7))     // check -- horizontal
        return true;
    y = board & (board >> 8);
    if (y & (y >> 2 * 8))     // check /  diagonal
        return true;
    y = board & (board >> 1);
    if (y & (y >> 2))         // check |  vertical
        return true;
    return false;
}

//SUBTRACTING 1 FROM A NUMBER AND "&ing" IT WITH ITSELF GIVES US
//THE SAME BISTRING WITHOUT THE LAST SET BIT. THE WHILE CYCLE WILL
//RUN FOR HOWEVER MANY ONES ARE IN THE BITSTRING. 
int countBits(int n) {
	/*
    int count = 0;
    while (n) {
    	n &= (n-1) ;
    	count++;
    }
    return count;
    */

    return __builtin_popcount(n); //The GCC builtin version is O(1).
}

int heuristic(UI board) {
  int lDiag = (board & (board >> 6) & (board >> 12));
  int rDiag = (board & (board >> 8) & (board >> 16));   
  int horiz = (board & (board >> 7) & (board >> 14));
  int verti = (board & (board >> 1) & (board >>  2));
  return countBits(lDiag) + countBits(rDiag) + countBits(horiz) + countBits(verti);
}

int util(UI board[]) { return heuristic(board[1]) - heuristic(board[0]); };

int getMove(char c) {
	int m;
	printf("%c's turn. Insert move\n", c);
	scanf("%d", &m);
	return m;
}

//WE USE POW() INSTEAD OF >> BECAUSE IN C/C++ BIT SHIFTS 
//OF MORE THAN 30 BITS ARE UNDEFINED BEHAVIOUR
UI makeMove(UI board, int height[], int move){ return board | ((UI) pow(2, (BOARD_HEIGHT + 1) * move + height[move])); }

UI undoMove(UI board, int height[], int move){ return board & (~(UI) pow(2, (BOARD_HEIGHT + 1) * move + height[move])); }

int minF(int x, int y){ if(x < y) return x; else return y; };
int maxF(int x, int y){ if(x > y) return x; else return y; };

// Initial values of Aplha and Beta
const int MAX =  1000;
const int MIN = -1000;
int k;
int MOVE;
int minimax(int depth, UI board[], int height[], int activePlayer, int alpha, int beta) {
	int moveBonus;
	k++;
	/*
 	if(depth == MAX_DEPTH) {

 		if(fourTogether(board[activePlayer])){
 			if(activePlayer == 1)return 1000; else return -1000;
 		}

 		if(activePlayer == 1) moveBonus = 1;else moveBonus = -1;
		return util(board) + moveBonus;
	};*/

	if(fourTogether(board[activePlayer])){
 		if(activePlayer == 1)return 1000; else return -1000;
 	}

	if(depth == MAX_DEPTH) {
		if(activePlayer == 1) moveBonus = 1;else moveBonus = -1;
		return util(board) + moveBonus;
	}

 	int max, min, val;
 	max = -10000;
 	min =  10000;
	for(int i = 0 ; i < BOARD_WIDTH ; i++){
		if(height[i] < BOARD_HEIGHT - 1) {	
			//printf("Checking branch %d at depth %d\n", i, depth);
			height[i]++;
			board[activePlayer] = makeMove(board[activePlayer], height, i);
			val = minimax(depth + 1, board, height, !activePlayer, alpha, beta);
			//if(depth == 0) printf("Col no %d has score %d\n", i, val);
			if(activePlayer == 1){
				if(val > max){
					max = val;
					if(depth == 0)MOVE = i;
				}
				alpha = maxF(val, alpha);
			}else{
				if(val < min){
					min = val;
					if(depth == 0)MOVE = i;
				}
				beta  = minF(val, beta);
			}
			
			//Undo move
			board[activePlayer] = undoMove(board[activePlayer], height, i);
			height[i]--;

			if(beta < alpha) break;
			
		}
	}
	if(activePlayer == 1) return max;
	return min;
}

int selectMove(UI board[], int height[], int activePlayer, int bestVal) {
 	int moveBonus, val;
	for(int i = 0 ; i < BOARD_WIDTH ; i++){
		if(height[i] < BOARD_HEIGHT - 1) {	
			height[i]++;
			board[activePlayer] = makeMove(board[activePlayer], height, i);
			if(fourTogether(board[activePlayer])){
				if(activePlayer == 1) val = -1000; else val = 1000;
			}else {
				if(activePlayer == 1) moveBonus = -1;else moveBonus = 1; 
				val =  util(board) + moveBonus;
			}
			
			printf("val = %d\n", val);
			if(val == bestVal) return i;

			board[activePlayer] = undoMove(board[activePlayer], height, i);
			height[i]--;
			
		}
	}
	return -1;
}

void play() {
    int turn;
    printf("Quem começa? 1-pessoa 0-pc");
    scanf("%d", &turn);

	int height[BOARD_WIDTH], tempHeight[BOARD_WIDTH];
	for(int i = 0 ; i < BOARD_WIDTH ; i++) height[i] = -1;
	UI board[2], tempBoard[2];
	board[1]    = board[0] = 0;
	int move    = 0;
	//int turn    = (int)rand() % 2;
	int bestVal;
	while(true){
		char c;
		if(turn == 1) c = 'X'; else c = 'O';
		if(turn == 1){
			printf("Human's turn\n");
			while(height[move = getMove(c)] >= BOARD_HEIGHT - 1) printf("Invalid move!\n");	
		}else {
			printf("AI's turn\n");
			for(int i = 0 ; i < BOARD_WIDTH ; i++) tempHeight[i] = height[i];
			tempBoard[0] = board[0];
			tempBoard[1] = board[1];

			bestVal = minimax(0, tempBoard, tempHeight, turn, MIN, MAX);
			move = MOVE;
			printf("AI's move is collum %d\n", MOVE);
			//printf("kmovs=%d",k);
		}
		height[move]++;
		board[turn] = makeMove(board[turn], height, move);
		printBoard_(board[1], board[0], 0);

		printf("Util: %d\n", util(board));

		if(fourTogether(board[turn])){printf("WINNER IS %c\n", c);break;}
		turn = !turn; //pass the turn
		printf("\n===========================\n");
	}
}

int main() {
	srand(time(NULL)); //Seed the random generator function
	play();
	return 0;
}
