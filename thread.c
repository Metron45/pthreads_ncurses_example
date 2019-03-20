#include <stdio.h> 
#include <stdlib.h> 
#include <ncurses.h>
#include <unistd.h>

void main(int argc, char *argv[]){ 
int threads;
sscanf (argv[1],"%d",&threads);

int x,y;
x = 0;
y = 5;

char c = '0';

initscr();
noecho();
curs_set(FALSE);

printw("o");


while(true){

  sleep(1);

  if(x < 20){
    x++;
  }else{
    x=0;
  }
  clear();
  move(y,x);
  printw("o");
  refresh();
}

endwin();
} 


	
	
	
	