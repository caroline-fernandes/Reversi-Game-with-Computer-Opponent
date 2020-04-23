/*
 * Sample OpenGL Program to Draw a Grid and Select Squares
 *
 * VIZA 653            Donald H. House         1/27/05
 * VIST 271            Michael Ringham         2/2/2011
 * VIST 271	       Caroline Fernandes      2/8/2020
 *
 * To compile Linux: g++ -o glsquare glsquare.cpp -lGL -lGLU -lglut 
 * To compile Mac:   g++ -o glsquare glsquare.cpp -framework Carbon -framework OpenGL -framework GLUT
 */

// red goes first

#ifdef __APPLE__
#  include <OpenGL/gl.h>  // definitions for GL graphics routines
#  include <OpenGL/glu.h> // definitions for GL input device handling
#  include <GLUT/glut.h>  // deginitions for the GLUT utility toolkit
#else
#  include <GL/gl.h>      // definitions for GL graphics routines
#  include <GL/glu.h>     // definitions for GL input device handling
#  include <GL/glut.h>    // deginitions for the GLUT utility toolkit
#endif

#include <iostream>       // definitions for standard I/O routines
#include <cmath>
//#include <chrono>
//#include <thread>

using namespace std;

#define EMPTY 0
#define RED -1
#define BLUE 1
#define pass (void)0
#define timer time(NULL)

#define WIDTH    600    /* initial window dimensions */
#define HEIGHT   600

#define N_ROWS   8      /* number of rows in grid */
#define N_COLS   8      /* number of columns in grid */

#define PI 3.1415926536 // mathematical pi

static int Width;       /* global variable window width in pixels*/
static int Height;      /* global variable window height in pixels */

int prow=0, pcol=0;

/*
Get row and column value
*/

#define BOARDSIZE 8   /* define board size to generalize solution */

#define EMPTY 0   /* constants for indicating board square contents */
#define RED -1	// my black
#define BLUE 1

int turn;
int numcaptured[3][3]; // used for user's turn
int aicaptured[3][3]; // used for computer's turn
int challenger;

int board[BOARDSIZE][BOARDSIZE];  /* make board global since all routines use it */

/*
numberOfPlayers asks for user input to decide how to structure following gameplay
aka whether I should call the computersTurn function or not 
*/
int numberOfPlayers() {
  
  char answer;

  cout << "Do you want to play against the computer?(y/n)" << endl;
  cin >> answer;

  if (answer == 'y')
	return 1;
  return 0;
}

void drawCircle(int x, int y) {
 
  float ainc = 2 * PI / 35;
  float angle;

  glBegin(GL_POLYGON);
  for (int k = 0; k < 35; k++) {
    angle = k * ainc;
    glVertex2f(x+0.5+0.5*cos(angle),y+0.5+0.5*sin(angle));
  }
  glEnd(); 

  glPopMatrix();
  
  return;
}

/*
  Clear the board and place the initial 4 checkers in the center
*/
void initboard(){
  int i, j;
  
  for(i = 0; i < BOARDSIZE; i++)
    for(j = 0; j < BOARDSIZE; j++)
      board[i][j] = EMPTY;
      
  board[BOARDSIZE / 2 - 1][BOARDSIZE / 2 - 1] = BLUE;
  board[BOARDSIZE / 2][BOARDSIZE / 2] = BLUE;
  board[BOARDSIZE / 2 - 1][BOARDSIZE / 2] = RED;
  board[BOARDSIZE / 2][BOARDSIZE / 2 - 1] = RED;
}

/*
Function to print board in the command line
*/
void printboard(){
  int i, j;
  
  // double for loop to print board right side up
  for(i = BOARDSIZE-1; i >=0 ; i--) {
    for(j = 0; j < BOARDSIZE; j++) {
      cout << board[i][j] <<"\t";
    }
    cout << endl;
  }   

}

/*
  Check if (i, j) is a location on the board. Return TRUE if it is, FALSE otherwise
*/
int onboard(int i, int j){
  return (i >= 0 && i < BOARDSIZE && j >= 0 && j < BOARDSIZE);
}

/*
  Function that checks if the click was on the board and that the pieces isn't equal to the color or empty
  then changes the color on the board to the correct one 
*/
int flip(int i, int j, int di, int dj, int color) {
  int k = 1;

  while (onboard(i+k*dj, j+k*dj) && board[i+ k* di][j+k*dj] != color && board[i+ k* di][j+k*dj] != EMPTY) {	
	board[i+ k* di][j+k*dj] = color;
	k++;
    
  }
  
}
/*
  Checks captures in all directions from (i, j) if specified color is placed at 
  this location. Returns total pieces captured, and returns an array of the 
  number of pieces captured in each direction.
  
  If the position (i, j) is not on the board, or if the space isn't empty, 
  returns 0 pieces captured.
*/
void flipcaptured(int i, int j, int color){ // , int numcaptured[3][3])
  int k, l;
  int di,dj;
 
  for(k = 0, di=-1; di<= 1; di++, k++){
    for(l = 0, dj=-1; dj<= 1; dj++,l++){
      if (challenger == 0 || color == -1) {
        if (numcaptured[k][l] > 0) {
      	  flip(i, j, di,dj, color);
        }
      }
      else if(challenger == 1 && color == 1) {
	if(aicaptured[k][l] > 0) {
	  flip(i,j,di,dj,color);
	}
      }
    }
  }
 
}

/*
  Starting from position (i, j) on board, moving in direction indicated by (di, dj),
  count the number of pieces that would be captured by placing a piece of the
  indicated color on square (i, j).
  
  This routine assumes that (i, j) is a valid empty board position and that
  di and dj are both either -1, 0, or 1.
*/
int countcaptured(int i, int j, int di, int dj, int color){
  int k;
  int capturecount;
  
  if(di == 0 && dj == 0)  /* make sure at least one of di, dj is non-zero */
    return 0;
  
  /*
     move in direction indicated by (di, dj) until either a match for color is
     found, or the square is empty, or we are off the board. If a match for the 
     color is found, return a count of the opposite color pieces captured.
     If off the board or an empty space, return a count of 0.
  */
  capturecount = 0;
  for(k = 1; k < BOARDSIZE; k++){
    if(!onboard(i + k * di, j + k * dj) || board[i + k * di][j + k * dj] == EMPTY)
      return 0;
    if(board[i + k * di][j + k * dj] == color)
      return capturecount;
    capturecount++;
  }
  
  cout << "error, should never get here!\n";
  return 0;
}

/*
  Checks captures in all directions from (i, j) if specified color is placed at 
  this location. Returns total pieces captured, and returns an array of the 
  number of pieces captured in each direction.
  
  If the position (i, j) is not on the board, or if the space isn't empty, 
  returns 0 pieces captured.
*/
int checkplacement(int i, int j, int color){
  int di, dj;
  int k, l;
  int total;
  
  // intialized numcaptured 2d array to zero
  for(k = 0; k < 3; k++) {
    for(l = 0; l < 3; l++) {
      numcaptured[k][l] = 0;
    }
  }
      
  if(!onboard(i, j) || board[i][j] != EMPTY)
    return 0;
  
  total = 0;

  // double for loop counting the number of pieces captured 
  // at position i,j which is returned from the function
  for(k = 0, di = -1; di <= 1; di++, k++){
    for(l = 0, dj = -1; dj <= 1; dj++, l++){
        numcaptured[k][l] = countcaptured(i, j, di, dj, color);
        total += numcaptured[k][l];
    }
  }
  return total;
}

/*
function passes pointer to row and col and updates them to get their position 
*/
void getRowCol(int *row, int *col, int x, int y)
{
    float dx, dy;

    y = Height - y;

    dy = (float)Height / N_ROWS;
    dx = (float)Width / N_COLS;

    *row = (int)(y / dy);
    *col = (int)(x / dx);
}

/*
Draws the hilighted square 
*/
void drawHilight (int col, int row)
{
    /* enable blending with color already stored in the framebuffer */
    glEnable (GL_BLEND);
        glPushAttrib(GL_CURRENT_BIT);         /* save current drawing color     */
        glPushAttrib(GL_POLYGON_BIT);         /* prepare for glPolygonMode      */
            glPolygonMode(GL_FRONT, GL_FILL); /* front facing polygon is filled */
            glColor4f(0,0,.6,.4);             /* transparent shade of blue      */
            glRecti(col, row, col+1, row+1);
        glPopAttrib();
        glPopAttrib();
    glDisable(GL_BLEND);

}

/*
Display Callback Routine: clear the screen and draw a M x N grid
This routine is called every time the window on the screen needs
to be redrawn, such as when the window is iconized and then reopened
It is also called when the window is first created.
*/
void drawGrid(){
    int row, col;

    /* clear the window to the background color */
    glClear(GL_COLOR_BUFFER_BIT);

    /* draw the grid using thick red lines */
    glColor3f(1, 0, 0);
    glLineWidth(2);

    /* draw horizontal grid lines */
    glBegin(GL_LINES);
    for(row = 0; row <= N_ROWS; row++){
        glVertex2i(0, row);
        glVertex2i(N_COLS, row);
    }
    glEnd();

    /* draw vertical grid lines */
    glBegin(GL_LINES);
    for(col = 0; col <= N_COLS; col++){
        glVertex2i(col, 0);
        glVertex2i(col, N_ROWS);
    }
    glEnd();

    // 2 for loops drawing the circles according to
    // their values stored in the board array
    for(int p = 0; p<8; p++){
	for (int q = 0; q<8; q++) {
		if (board[p][q] == RED){
			glColor3f(1,0,0);
			drawCircle(q,p);
		}
		else if (board[p][q] == BLUE){
			glColor3f(0,0,1);
			drawCircle(q,p);
		}
	}
    }  
    
    drawHilight(pcol,prow);

    glutSwapBuffers();
}

/*
PassiveMotionFunc callback. Watch mouse movement 
*/
void p_Motion(int x, int y)
{
    static int oldrow, oldcol;

    getRowCol(&prow, &pcol, x, y);

    if ((prow == oldrow)&&(pcol == oldcol)) return;

    oldrow=prow; oldcol=pcol;  

    glutPostRedisplay();
}

/*
teamScore Function prints the score after a red and blue circle are placed
*/
void teamScore() {
    int redTeam=0;
    int blueTeam=0;

    for(int i = 0; i < 8; i++){
	for(int j = 0; j < 8; j++){
		if (board[i][j] == RED){
			redTeam++;
		} else if (board[i][j] == BLUE) {
			blueTeam++;
		}
	}
    }
    cout << "Score ---------------" << endl;
    cout << "Red Team: " << redTeam << endl;
    cout << "Blue Team: " << blueTeam << endl;
    
    return;
}

/*
computersTurn function calculates the best position for blue's turn

It calls checkplacement to find valid moves and then stores the 
row and col with the highest number of moves. The first if statement
is a sort of scoring system for squares placed in the corner
of the board. Since corners cannot be flipped they have a higher value.

The function also places a piece at the best position, flips 
the corresponding pieces, and changes the turn back to red. 

I called computersTurn inside handleButtons
*/
void computersTurn () {
       
    int row = 0, col = 0, moves = 0, largest_move = 0, largest_row = 0, largest_col = 0;        

    cout << "turn: " << turn << endl;
    for (int i = 0; i < 8; ++i) {
	for (int j = 0; j < 8; ++j) {
	    moves = checkplacement(i,j,1);
		if (moves != 0) {

		    if (i == 0 && j == 0 || i == 0 && j == 7 || i == 7 && j == 0 || i == 7 && j == 7) {
		
                        for (int c = 0; c < 3; c++) {
		            for (int d = 0; d < 3; d++) {
                                aicaptured[c][d] = numcaptured[c][d];
  			    }
                        } 
                        board[i][j] = BLUE;
                        flipcaptured(i,j,1);
			turn*= -1;
			
			return;
                    } 
		    else if (moves > largest_move) {
                        for (int a = 0; a < 3; a++) {
		            for (int b = 0; b < 3; b++) {
                                aicaptured[a][b] = numcaptured[a][b];
  			    }
                        }
			largest_move = moves;
			largest_row = i;
			largest_col = j;
		    }
		}
	}
    }
    for (int k = 0; k < 8; ++k) {
	for (int l = 0; l < 8; ++l) {
	    if (k == largest_row && l == largest_col) {
		board[k][l] = turn;
		flipcaptured(k,l, 1);
		turn*= -1;
	
                return;
            }
	}
    }
    
    return;
}


void handleComputerTurn(int value) {
    computersTurn();
    glutPostRedisplay();
}

/*
Mouse button event handler, called when button pressed or released
*/
void handleButtons(int button, int state, int x, int y){
    int row, col, moves, large_row = 0, large_col = 0; 


    if(button != GLUT_LEFT_BUTTON || state != GLUT_UP)
        return;

    getRowCol(&row, &col, x, y);

    moves = checkplacement(row, col, turn);
   
    if (moves == 0) {
    	cout << "Invalid move. Try again." << endl;	
    }
    else {
        board[row][col] = turn;
	flipcaptured(row,col,turn);
	cout << "turn: " << turn << endl;
        printboard();
       
        glutPostRedisplay();
        
        turn*= -1;
        
	// check to see if user chose to play computer or not
	if (challenger == 1) {
                glutTimerFunc(1200,handleComputerTurn,0);
		//computersTurn();     
        }
        //glutPostRedisplay();
	cout << "turn: " << turn << endl;
       
	teamScore();

	
    }
    printboard();
    
    cout << "button click in cell (" << row << ", " << col << ")\n";
}

/*
Reshape event handler, called whenever window is resized by user
*/
void handleResize(int w, int h){

    Width = w;
    Height = h;

    /* lower left of window is (0, 0), upper right is (WIDTH, HEIGHT) */
    glViewport(0, 0, Width, Height);

    /* drawing coordinates are always in terms of number of rows/cols */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, N_COLS, 0, N_ROWS);
    glMatrixMode(GL_MODELVIEW);
}
/*
Main program to draw the grid, and accept mouse clicks
*/
int main(int argc, char* argv[]){

    turn = RED;
    initboard();  
    printboard(); 

    int numPlayers = 0;
    numPlayers = numberOfPlayers();
    if (numPlayers == 1) {
    	cout << "Ok you will play versus the computer. You are red and play first." << endl;
	challenger = 1;
    } else {
	cout << "Ok you will play as both red and blue. Red goes first." << endl;
	challenger = 0;
    } 

    /* start up the glut utilities */
    glutInit(&argc, argv);

    /* create the graphics window, giving width, height, and title text */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Caroline's Reversi Game");


    /* define the operation of blending when enabled */
    /* transparency is best implemented with the following 2 parameters */
    glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /* drawing callback routine */
    glutDisplayFunc(drawGrid);

    /* mouse button callback routine */
    glutMouseFunc(handleButtons);
    glutPassiveMotionFunc(p_Motion);

    /* window resize callback routine */
    glutReshapeFunc(handleResize);

    /* specify window clear (background) color to be white */
    glClearColor(1, 1, 1, 1);

    glutMainLoop();
    return 0;
}
