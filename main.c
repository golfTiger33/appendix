/*
Dan George 
C program that demonstrates consumer producer problem
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h> 
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <time.h> 

#define UPPER 9000
#define LOWER 1000
#define TIMESTEP 250000


//thread functions
void * consumer(void *);
void * producer(void *);

typedef struct {
   int * buffer;
   int front; 
   int back;
   //-1 = empty, 0 = available, 1 = full
   int state;
}qeue;




//create mutex
sem_t mtx; 
//semaphore for full
sem_t full;
//semaphore for empty 
sem_t empty;


//random number 
int num;
//flag to quit
int quit = 0;
//input option for char
char option[10];
//buffer size
int bufSize = 0; 
//qeue
qeue Q;
//sleep time consumer 
int cSleep = 0;
//sleep time producer 
int pSleep = 0;






int main(int argc, char* argv[]) {
  
    //Initialize semaphore
    sem_init(&mtx, 0, 1);  
    
    //random number seed
    srand(time(NULL));

    // get args
    if(argc != 4)
   {
     printf("please enter correct number of arguments!\n");
     return 1;
   }
    
    //get buffer size
    bufSize = atol(argv[1]);
    //get consumer sleep time 
    cSleep = atol(argv[2]);
    //get producer sleep 
    pSleep = atol(argv[3]); 
    
    //set variables for sleep time
    pSleep = TIMESTEP;
    cSleep = TIMESTEP;
    quit = 0; 
    
    //allocate space for buffer, set qeue as ready and empty
    Q.buffer = (int *)calloc(bufSize, sizeof(int));
    Q.front = 0;
    Q.back = -1;
    Q.state = -1;

    //start threads 
    pthread_t consumerThread;
    pthread_t producerThread; 
    pthread_create(&consumerThread, NULL, consumer, NULL);
    pthread_create(&producerThread, NULL, producer, NULL);
    
    
    
    
    
    //main loop
    while(1)
    {
       //get options
       fgets(option, 10, stdin);
       
       /*
         >>>set thread times<<< 
       */
       //increase producer sleep by 250
       if(strncmp(option, "a", 1) == 0)
       {
         //grab semaphore 
         sem_wait(&mtx);
         //increment sleep timer
         pSleep += TIMESTEP;
         //release semephore
         sem_post(&mtx);
         printf("%d pSleep\n", pSleep);
       }
       //decrease producer sleep by 250
       else if(strncmp(option, "z", 1) == 0)
       {
          if(pSleep > 0){
             //grab semaphore 
             sem_wait(&mtx);
             //increment sleep timer
             pSleep -= TIMESTEP;
             //release semephore
             sem_post(&mtx);
             printf("%d pSleep\n", pSleep);
          }
       }
       //increase consumer sleep by 250 
       else if(strncmp(option, "s", 1) == 0)
       {
         //grab semaphore 
         sem_wait(&mtx);
         //increment sleep timer
         cSleep += TIMESTEP;
         //release semephore
         sem_post(&mtx);
         printf("%d cSleep\n", cSleep);
       }
       //decrease consumer sleep by 250
       else if(strncmp(option, "x", 1) == 0)
       { 
          if(cSleep > 0){
             //grab semaphore 
             sem_wait(&mtx);
             //increment sleep timer
             cSleep -= TIMESTEP;
             //release semephore
             sem_post(&mtx);
             printf("%d cSleep\n", cSleep);
          }
       }
       //quit
       else if(strncmp(option, "q", 1) == 0)
       {
          puts("Quitting!");
          //grab semaphore 
          sem_wait(&mtx);
          
          //signal to quit
          quit = 1;
         
          //release semephore
          sem_post(&mtx);
          
          //int for receiving thread 
          int rc;
          //receive producer thread first
          pthread_join(producerThread, (void**) &rc);
          //receive consumer thread
          pthread_join(consumerThread, (void**) &rc);
         
          //destroy semaphore 
          sem_destroy(&mtx);
          //exit loop
          break;
       }
       //invalid data 
       else {
         puts("enter valid command!");
       }


    }
}

void * consumer(void * arg) { 
while(!quit || Q.state != -1){
  //sleep
  usleep((useconds_t)cSleep);
  
  if(Q.state != -1){
    //read data from buffer 
    printf("Read %d from bin %d \n", Q.buffer[Q.back], Q.back);
    //decrement buffer pointer 
    Q.back--;
    
    //if first element read, then signal empty
    if(Q.back <= -1){
       //signal not empty 
       Q.state = -1;
       //move pointer back to start of buffer
       Q.back = -1;
    }
  }
  
  //wait for control of semaphore
  sem_wait(&mtx); 

  //release semaphore 
  sem_post(&mtx);
  }
  //exit 
  return 0;
}



void * producer(void * arg) {
while(!quit){
  //sleep
  usleep((useconds_t)pSleep); 
  
  //wait for control of semaphore
  sem_wait(&mtx);
  


  //add to buffer 
  if(Q.back < bufSize && Q.state != 1){
    //generate random number
    int r = (rand() % (UPPER - LOWER +1))+ LOWER;

    //move qeue pointer
    Q.back++;
    Q.buffer[Q.back] = r;
    //print data to user 
    printf("Added %d to bin %d \n", Q.buffer[Q.back], Q.back);

    //signal available
    Q.state = 0;
    
    
  //signal full if so 
  if(Q.back >= bufSize){
    //flag full
    Q.state = 1;
    //set pointer to last element for reader
    Q.back = bufSize -1;
  }
  }

  
  //release semaphore 
  sem_post(&mtx);
 } 
  //exit 
  return 0;
}












