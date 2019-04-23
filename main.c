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
#define INSIDE 4

#define box_X1 11
#define box_X2 18
#define box_Y1 2
#define box_Y2 18

//int *cursor_x, *cursor_y, *control;
int *cursor_x, *cursor_y;
pthread_mutex_t *mutexLock;
pthread_cond_t *condLock;
int size;
bool finish;

int border_check(int pos_x, int pos_y){
    if(pos_x == box_X1 && (pos_y <= box_Y2 && pos_y >= box_Y1)){
         return 1;
    }
    if(pos_x == box_X2 && (pos_y <= box_Y2 && pos_y >= box_Y1)){
         return 2;
    }
    if(pos_y == box_Y2  && (pos_y <= box_X2 && pos_y >= box_X1)){
        return 3;
    }
    if(pos_y == box_Y1  && (pos_y <= box_X2 && pos_y >= box_X1)){
         return 4;
    }
    return 0;
}

void move_position(int * data){
    if(data[INSIDE] == 1){
        switch(border_check(data[X],data[Y])){
            case 0:
            break;
            case 1:
                if(data[OPTION] >=1 && data[OPTION] <=3){
                    data[INSIDE] = 0;
                }
            break;
            case 2:
                data[OPTION] = 8 - data[OPTION];
            break;
            case 3:
                data[OPTION] = (12 - data[OPTION])%8;
            break;
            case 4:
                data[OPTION] = (12 - data[OPTION])%8;
            break;
        }
    }else if(border_check(data[X],data[Y]) != 0) {
        data[INSIDE] = 1;
    }

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

    int pos[] = {SIZE/2,SIZE/2,0,0,0}; //X, Y, Option, Speed, Inside
    pos[OPTION] = (int)(rand() % 8);
    pos[SPEED] = (int)(rand() % MAX_SPEED) + 1;
    int thread_id = atoi((char*) ptr);
    /*
    pthread_mutex_lock(&mutexLock[thread_id]);
    pthread_cond_wait(&condLock[thread_id], &mutexLock[thread_id]);
    pthread_mutex_unlock(&mutexLock[thread_id]);
    */
    while(!finish){
        sleep(MAX_SPEED / pos[SPEED]);
        //movement function
        move_position(pos);
        //write to critical section
        pthread_mutex_lock(&mutexLock[thread_id]);
        cursor_x[thread_id] = pos[X];
        cursor_y[thread_id] = pos[Y];
        pthread_mutex_unlock(&mutexLock[thread_id]);
    }
}

void *thread_getch_function( void * ptr ){
    /*
    for(int i =0; i < 4; i++){
        getch();
        pthread_cond_signal(&condLock[i]);
    }
    */
    getch();
    finish = true;
}

int main(int argc, char *argv[]){ 
    pthread_t thread;
    int error;
    finish = false;
    //number of threads
    int threads = 4;
    if(argc > 1){
        threads = atoi(argv[1]); 
    }

    //allocate
    cursor_x = malloc(threads * sizeof(int));
    cursor_y = malloc(threads * sizeof(int));
    mutexLock = malloc(threads * sizeof(pthread_mutex_t));
    condLock = malloc(threads * sizeof(pthread_cond_t)); 

    //initialize
    for(int i = 0;i < threads; i++){
        cursor_x[i] = -1;
        cursor_y[i] = -1;
        if (pthread_mutex_init(&mutexLock[i], NULL) != 0)
        {
            printf("\n mutex init failed\n");
            return 1;
        }
        if (pthread_cond_init(&condLock[i], NULL) != 0)
        {
            printf("\n condi init failed\n");
            return 1;
        }
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
        //draw top info
        move(SIZE + 1 + MAX_SPEED*2, 0);
        printw("Threads: %d Size: %d", threads, SIZE);
        //draw boundries
        for(int i = 0; i <= SIZE; i++){
            move(SIZE + 1,i);
            printw("$");
        }
        for(int i = 0; i <= SIZE; i++){
            move(i,SIZE + 1);
            printw("$");
        }
        //draw box
        for(int i = box_X1; i <= box_X2; i++){
            move(i, box_Y1);
            printw("#");
        }
        for(int i = box_X1; i <= box_X2; i++){
            move(i, box_Y2);
            printw("#");
        }
        for(int i = box_Y1; i <= box_Y2; i++){
            move(box_X2, i);
            printw("#");
        }
        for(int i = box_Y1; i <= box_Y2; i++){
            move(box_X1, i);
            printw("^");
        }

        //draw thread info
        for(int y = 0 ; y < threads ; y++){
            move(SIZE + 2 + y + MAX_SPEED*2, 0);
            printw("Thread: %d Position %d %d", y, cursor_y[y], cursor_x[y]);
            pthread_mutex_lock(&mutexLock[y]);
            if(cursor_x[y] != -1 && cursor_y[y] != -1){
                move(cursor_y[y] ,cursor_x[y] );
                printw("o");
            }
            pthread_mutex_unlock(&mutexLock[y]);
        }
        refresh();
    }
    //finish threads
    for(int i=0; i < threads + 1 ; i++){
        pthread_join( thread, NULL);
    }
    //ending ncurses
    endwin();
    //ending mutexes and condition locks
    for(int i = 0;i < threads; i++){
        pthread_mutex_destroy(&mutexLock[i]);
        pthread_cond_destroy(&condLock[i]);
    }
    //freeing space
    free(cursor_x);
    free(cursor_y);
    free(mutexLock);
    free(message);
    exit(EXIT_SUCCESS);
    return 0;
} 