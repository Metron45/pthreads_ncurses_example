## pthreads_ncurses_example
Simply example of multithreading use in C using ncurses and pthreads.

## Running
To build the program simply run the:
```
make
```
command to start the makefile to build the program. Currently makefile itself includes solely:
```
gcc main.c -pthread -lncurses -o main
```
To run the program run the main file with command:
```
./main {amount of ball threads} {max amount of active threads} {amount of obstacles}
```
In case of any parameter missing it will use default values of 6,3,4. 
