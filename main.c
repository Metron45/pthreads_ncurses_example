#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define MAX_SPEED 3
#define SIZE 20
#define X 0
#define Y 1
#define OPTION 2
#define SPEED 3

int *cursor_x, *cursor_y, *control;
int counter;
int size;
bool finish;

void move_position(int * data){
    switch(data[OPTION]){
        case 0:
            data[X]++;
            if(data[X] >= SIZE){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break; 
        case 1:
            data[X]++;
            data[Y]--;
            if(data[X] >= SIZE || data[Y] <= 0){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break;
        case 2:
            data[Y]--;
            if(data[Y] <= 0){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break; 
        case 3:
            data[X]--;
            data[Y]--;
            if(data[X] <= 0 || data[Y] <= 0){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break;
        case 4:
            data[X]--;
            if(data[X] <= 0){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break; 
        case 5:
            data[X]--;
            data[Y]++;
            if(data[X] <= 0 || data[Y] >= SIZE){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break;
        case 6:
            data[Y]++;
            if(data[Y] >= SIZE){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break; 
        case 7:
            data[X]++;
            data[Y]++;
            if(data[X] >= SIZE || data[Y] >= SIZE){
                data[OPTION]=(data[OPTION]+4)%8;
            }
        break;
    }
}

void *thread_function( void * ptr ){
    int pos[] = {SIZE/2,SIZE/2,0,0};
    pos[OPTION] = (int)(rand() % 8);
    pos[SPEED] = (int)(rand() % MAX_SPEED) + 1;
    int thread_id = atoi((char*) ptr);
    while(!finish){
        sleep(5 / pos[SPEED]);
        //movement function
        move_position(pos);
        //write to critical section
        while(control[thread_id] == -1){}
        control[thread_id] = -1;
        cursor_x[thread_id] = pos[X];
        cursor_y[thread_id] = pos[Y];
        control[thread_id] = 0;
    }
}

void *thread_getch_function( void * ptr ){
    getch();
    finish = true;
}

void main(int argc, char *argv[]){ 
    pthread_t thread;
    int error;
    finish = false;
    //number of threads
    int threads = 4;
    if(argc > 1){
        threads = atoi(argv[1]); 
    }
    //amount of repeats
    counter = 25;
    if(argc > 2){
        counter = atoi(argv[2]); 
    }
    //initialize critical ball position
    cursor_x = malloc(threads * sizeof(int));
    cursor_y = malloc(threads * sizeof(int));
    control = malloc(threads * sizeof(int));
    for(int i = 0;i < threads; i++){
        cursor_x[i] = -1;
        cursor_y[i] = -1;
        control[i] = 0;
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
            fprintf(stderr,"Error creating ball threads: %d\n",error);
            exit(EXIT_FAILURE);
        }
    }
    error = pthread_create( &thread, NULL, thread_getch_function, (void*) message[0]);
        if(error){
            fprintf(stderr,"Error creating getch thtread: %d\n",error);
            exit(EXIT_FAILURE);
        }
    //refresh board
    while(!finish){
        sleep(1);
        clear();
        move(SIZE + 1 + MAX_SPEED*2, 0);
        printw("Threads: %d Size: %d Count: %d", threads, SIZE, counter);
        for(int y = 0 ; y < threads ; y++){
            move(SIZE + 2 + y + MAX_SPEED*2, 0);
            printw("Thread: %d Position %d %d", y, cursor_y[y], cursor_x[y]);
            while(control[y] == -1){
                
            }
            if(cursor_x[y] != -1 && cursor_y[y] != -1){
                control[y] == -1;
                move(cursor_y[y] + MAX_SPEED,cursor_x[y] + MAX_SPEED);
                printw("o");
                control[y] == 0;
            }
        }
        refresh();
    }
    //finish threads
    for(int i=0; i < threads + 1 ; i++){
        pthread_join( thread, NULL);
    }
    //ending ncurses
    endwin();
    //freeing space
    free(cursor_x);
    free(cursor_y);
    free(control);
    free(message);
    exit(EXIT_SUCCESS);
} 