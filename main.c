#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

//max definitions
#define MAX_SPEED 3
#define MAX_BOX_BALLS 1
#define SIZE 20

//data * definitions
#define X 0
#define Y 1
#define OPTION 2
#define SPEED 3
#define INSIDE 4

//box definitions
#define X1 12
#define X2 18
#define Y1 2
#define Y2 19

int *cursor_x, *cursor_y, *cursor_inside, balls_inside;
pthread_mutex_t *mutexLockThreads, mutexLockShared;
pthread_cond_t condLock;
bool finish;

int border_check(int * data){
    if(data[X] == X2 && data[Y] > Y1 && data[Y] < Y2){
        return 0;
    }else if(data[X] == X2 && data[Y] == Y1){
        return 1;
    }else if(data[X] > X1 && data[X] < X2 && data[Y] == Y1){
        return 2;
    }else if(data[X] == X1 && data[Y] == Y1){
        return 3;
    }else if(data[X] == X1 && data[Y] > Y1 && data[Y] < Y2){
        return 4;
    }else if(data[X] == X1 && data[Y] == Y2){
        return 5;
    }else if(data[X] > X1 && data[X] < X2 && data[Y] == Y2){
        return 6;
    }else if(data[X] == X2 && data[Y] == Y2){
        return 7;
    }

    return -1;
}

bool inside_check(int * data){
    if((data[X] >= X1 && data[X] <= X2) && (data[Y] >= Y1 && data[Y] <= Y2)){
        return true;
    }
    return false;
}

void move_position(int * data){


    if(data[INSIDE] == 1){
        switch(border_check(data)){
            case 0:
                if(data[OPTION]%2==0){
                    data[OPTION] = (data[OPTION] + 4) % 8;
                }else{
                    data[OPTION] = data[OPTION] - 2;
                }
            break;
            case 1:
                data[OPTION] = 5;
            break;
            case 2:
            case 3:
            case 5:
            case 6:
                if(data[OPTION]%2==0){
                    data[OPTION] = (data[OPTION] + 4) % 8;
                }else{
                    data[OPTION] = 8 - data[OPTION];
                }
            break;
            case 4:
                //do nothing
            break;
            case 7:
                data[OPTION] = 3;
            break;

        }
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

    int pos[] = {SIZE/2,
    SIZE/2,
    (int)(rand() % 8),
    (int)(rand() % MAX_SPEED) + 1,
    0}; //X, Y, Option, Speed, Inside
    int thread_id = atoi((char*) ptr);
    
    //pthread_mutex_lock(&mutexLockThreads[thread_id]);
    //pthread_cond_wait(&condLock, &mutexLockThreads[thread_id]);
    //pthread_mutex_unlock(&mutexLockThreads[thread_id]);
    
    while(!finish){
        sleep(MAX_SPEED / pos[SPEED]);
        //movement function
        move_position(pos);
        //enter check
        if(inside_check(pos) && pos[INSIDE]==0){
            pos[INSIDE]=1;
            pthread_mutex_lock(&mutexLockShared);
            balls_inside++;
            pthread_mutex_unlock(&mutexLockShared);
            if(balls_inside > MAX_BOX_BALLS){
                pthread_mutex_lock(&mutexLockThreads[thread_id]);
                pthread_cond_wait(&condLock, &mutexLockThreads[thread_id]);
                pthread_mutex_unlock(&mutexLockThreads[thread_id]);
            }
        }
        //leave check
        else if (!inside_check(pos) && pos[INSIDE]==1){
            pos[INSIDE]=0;
            pthread_mutex_lock(&mutexLockShared);
            balls_inside--;
            pthread_mutex_unlock(&mutexLockShared);
            pthread_cond_signal(&condLock);
        }

        //write to critical section
        pthread_mutex_lock(&mutexLockThreads[thread_id]);
        cursor_x[thread_id] = pos[X];
        cursor_y[thread_id] = pos[Y];
        cursor_inside[thread_id] = pos[INSIDE];
        pthread_mutex_unlock(&mutexLockThreads[thread_id]);
    }
}

void *thread_getch_function( void * ptr ){
    /*
    for(int i =0; i < 4; i++){
        getch();
        pthread_cond_signal(&condLock);
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
    int threads = 8;
    if(argc > 1){
        threads = atoi(argv[1]); 
    }

    //allocate
    cursor_x = malloc(threads * sizeof(int));
    cursor_y = malloc(threads * sizeof(int));
    cursor_inside = malloc(threads * sizeof(int));
    mutexLockThreads = malloc(threads * sizeof(pthread_mutex_t));
    //condLock = malloc(threads * sizeof(pthread_cond_t)); 

    //initialize
    for(int i = 0;i < threads; i++){
        cursor_x[i] = -1;
        cursor_y[i] = -1;
        cursor_inside[i] = 0;
        if (pthread_mutex_init(&mutexLockThreads[i], NULL) != 0)
        {
            printf("\n mutex init failed\n");
            return 1;
        }
        //if (pthread_cond_init(&condLock[i], NULL) != 0)
        //{
        //    printf("\n condi init failed\n");
        //    return 1;
        //}
    }
    balls_inside = 0;
    if (pthread_mutex_init(&mutexLockShared, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_cond_init(&condLock, NULL) != 0)
        {
            printf("\n condi init failed\n");
            return 1;
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
        printw("Threads: %d Size: %d BoxBalls: %d", threads, SIZE, balls_inside);
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
        for(int i = X1; i <= X2; i++){
            move(Y1, i);
            printw("#");
        }
        for(int i = X1; i <= X2; i++){
            move(Y2, i);
            printw("#");
        }
        for(int i = Y1; i <= Y2; i++){
            move(i, X2);
            printw("#");
        }
        for(int i = Y1; i <= Y2; i++){
            move(i, X1);
            printw("^");
        }

        //draw thread info
        for(int y = 0 ; y < threads ; y++){
            move(SIZE + 2 + y + MAX_SPEED*2, 0);
            printw("Thread: %d Inside: %d Position %d %d", y,cursor_inside[y], cursor_x[y], cursor_y[y]);
            pthread_mutex_lock(&mutexLockThreads[y]);
            if(cursor_x[y] != -1 && cursor_y[y] != -1){
                move(cursor_y[y] ,cursor_x[y] );
                printw("o");
            }
            pthread_mutex_unlock(&mutexLockThreads[y]);
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
        pthread_mutex_destroy(&mutexLockThreads[i]);
        //pthread_cond_destroy(&condLock[i]);
    }
    pthread_cond_destroy(&condLock);
    //freeing space
    free(cursor_x);
    free(cursor_y);
    free(cursor_inside);
    free(mutexLockThreads);
    free(message);
    exit(EXIT_SUCCESS);
    return 0;
} 