#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define SIZE 20
#define X 0
#define Y 1
#define Option 2

int *cursor_x, *cursor_y;
int counter;

void move_position(int * data){
    switch(data[Option]){
        case 0:
            data[X]++;
            if(data[X] >= SIZE){
                data[Option]=(data[Option]+4)%8;
            }
        break; 
        case 1:
            data[X]++;
            data[Y]--;
            if(data[X] >= SIZE || data[Y] <= 0){
                data[Option]=(data[Option]+4)%8;
            }
        break;
        case 2:
            data[Y]--;
            if(data[Y] <= 0){
                data[Option]=(data[Option]+4)%8;
            }
        break; 
        case 3:
            data[X]--;
            data[Y]--;
            if(data[X] <= 0 || data[Y] <= 0){
                data[Option]=(data[Option]+4)%8;
            }
        break;
        case 4:
            data[X]--;
            if(data[X] <= 0){
                data[Option]=(data[Option]+4)%8;
            }
        break; 
        case 5:
            data[X]--;
            data[Y]++;
            if(data[X] <= 0 || data[Y] >= SIZE){
                data[Option]=(data[Option]+4)%8;
            }
        break;
        case 6:
            data[Y]++;
            if(data[Y] >= SIZE){
                data[Option]=(data[Option]+4)%8;
            }
        break; 
        case 7:
            data[X]++;
            data[Y]++;
            if(data[X] >= SIZE || data[Y] >= SIZE){
                data[Option]=(data[Option]+4)%8;
            }
        break;
    }
}

void *thread_function( void * ptr ){
    int pos[] = {SIZE/2,SIZE/2,0};
    pos[Option] = (int)(rand() % 8);
    int thread_id = atoi((char*) ptr);
    for(int i = 0 ; i <= counter ; i++){
        sleep(1);
        //movement function
        move_position(pos);
        //write to critical section
        cursor_x[thread_id] = pos[X];
        cursor_y[thread_id] = pos[Y];
    }
}

void main(int argc, char *argv[]){ 
    pthread_t thread;
    int error;
    //number of threads
    int threads = atoi(argv[1]); 
    //amount of repeats
    counter = atoi(argv[2]);
    //initialize critical ball position
    cursor_x = malloc(threads * sizeof(int));
    cursor_y = malloc(threads * sizeof(int));
    for(int i = 0;i < threads; i++){
        cursor_x[i] = -1;
        cursor_y[i] = -1;
    }
    //initialize thread_ids
    char **message = malloc(threads * sizeof(char));
     for(int i = 0;i < threads; i++){
        message[i] = malloc(2 * sizeof(char));
        sprintf(message[i], "%d", i);
    }
    //initialize random for direction
    srand(time(NULL));
    //initialize ncurses screen
    initscr();
    noecho();
    curs_set(FALSE);
    //initialize threads
    for(int i=0; i < threads ; i++){
        error = pthread_create( &thread, NULL, thread_function, (void*) message[i]);
        if(error){
            fprintf(stderr,"Error creating threads code: %d\n",error);
            exit(EXIT_FAILURE);
        }
    }
    //refresh board
    for(int i=0;i <= counter;i++){
        sleep(1);
        clear();
        move(SIZE + 1, 0);
        printw("Threads: %d Size: %d Count: %d", threads, SIZE, counter);
        for(int y = 0 ; y < threads ; y++){
            move(SIZE + 2 + y, 0);
            printw("Thread: %d Position %d %d", y, cursor_y[y], cursor_x[y]);
            if(cursor_x[y] != -1 && cursor_y[y] != -1){
                move(cursor_y[y],cursor_x[y]);
                printw("o");
                cursor_x[y] = -1;
                cursor_y[y] = -1;
            }
        }
        refresh();
    }
    //finish threads
    for(int i=0; i < threads ; i++){
        pthread_join( thread, NULL);
    }
    //ending ncurses
    endwin();
    exit(EXIT_SUCCESS);
} 