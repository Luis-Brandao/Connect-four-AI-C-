/*
	Connect 4 AI in C

	Uses an efficient 128bit representation of the board
	based on John Trump's Fhourstones benchmark.

	By bit shifting the board we can line up all the
	possible sets of 4 lines in one direction all the
	same time, allowing us to check for victory in O(1).

	Original code: https://tromp.github.io/c4/Connect4.java
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

//THIS IS A VERY BAD FUNCTION, WE NEED TO FIX IT
//ALSO MAYBE ROTATE THE BOARD -90º SO WE CAN ACTUALLY READ IT
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

bool threeTogether(UI board) {
  if (board & (board >> 6) & (board >> 12) != 0) return true;
  if (board & (board >> 8) & (board >> 16) != 0) return true;   
  if (board & (board >> 7) & (board >> 14) != 0) return true;
  if (board & (board >> 1) & (board >>  2) != 0) return true;
  return false;
}

//SUBTRACTING 1 FROM A NUMBER AND "&ing" IT WITH ITSELF GIVES US
//THE SAME BISTRING WITHOUT THE LAST SET BIT. THE WHILE CYCLE WILL
//RUN FOR HOWEVER MANY ONES ARE IN THE BITSTRING. 

//WE CAN MAKE THIS O(1) WITH LOOKUP TABLES
//FOR MORE INFO: 
//http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
int countBits(int n) {
    int count = 0;
    while (n) {
      n &= (n-1) ;
      count++;
    }
    return count;
}

/* 
bool heuristc(UI board) {
  int lDiag = (board & (board >> 6) & (board >> 12) != 0);
  int rDiag = (board & (board >> 8) & (board >> 16) != 0);   
  int horiz = (board & (board >> 7) & (board >> 14) != 0);
  int verti = (board & (board >> 1) & (board >>  2) != 0);
  return countBits(lDiag) + countBits(rDiag) + 
}
*/
//-----------------------------------------------------------------------------

int getMove(char c) {
	int m;
	printf("%c's turn. Insert move", c);
	scanf("%d", &m);
	return m;
}

//WE USE POW() INSTEAD OF >> BECAUSE IN C/C++ BIT SHIFTS 
//OF MORE THAN 30 BITS ARE UNDEFINED BEHAVIOUR

//TRUE IF COLLUM IS FREE
bool isFree(int col, UI board[]){ (board[0] | board[1]) | }

//UI makeMove(UI board, int height[], int move){ return board | ((UI) pow(2, (BOARD_HEIGHT + 1) * move + height[move])); }

void play() {
	int height[BOARD_WIDTH];
	for(int i = 0 ; i < BOARD_WIDTH ; i++) height[i] = -1;

	UI board[2];
	board[1]    = board[0] = 0;
	int move    = 0;

	int turn    = (int)rand() % 2;

	while(true){
		char c;
		if(turn == 1) c = 'X'; else c = 'O';

		while(height[move = getMove(c)] >= BOARD_HEIGHT - 1)printf("Invalid move!\n");

		height[move]++;
		board[turn] = makeMove(board[turn], height, move);

		printBoard_(board[1], board[0], 0);
		
		if(fourTogether(board[turn])){printf("WINNER IS %c\n", c);break;}
			
		turn = !turn; //pass the turn
	}
}
/*
int minmax(UI board[], int depth, int activePlayer){
	if(depth = MAX_DEPTH)
}
*/

//-----------------------------------------------------------------------------

int main() {
	srand(time(NULL)); //Seed the random generator function
	play();
	return 0;
}
