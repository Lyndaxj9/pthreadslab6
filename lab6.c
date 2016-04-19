/* ------------------------------------------------------------------
 * CS 350 Lab6 pthreads
 * lab6.c
 * Omowumi L. Ademola
 * ----------------------------------------------------------------*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct Stock{
  int threadId;
  int stockPrice;
};

struct Market{
  float marketValue;
  int running;
  float mD;
  float mU;
};

struct Market theMarket;
int marketValue;

pthread_mutex_t marketVsum;
pthread_cond_t marketWatch;
pthread_cond_t markDown;

void* initStock(void* threadArg);
void* initWatchDown(void* threadArg);
void* initWatchUp(void* threadArg);

int main(int argc, char *argv[]){

  int numThreads = 0;
  int marketPerc = 0; //maybe make a float

//-- Taking in and parsing arguments --------------------------------
  if(argc==3){
    numThreads = atoi(argv[1]);
    marketPerc = atoi(argv[2]);

    if(numThreads < 1 || numThreads > 1000){
      fprintf(stderr, "Error: The number of threads are not in the \
range 1 - 1000, exiting.\n");
      exit(1);
    }else if(marketPerc < 1 || marketPerc > 20){
      fprintf(stderr, "Error: The number for the market percentage \
is not in the range 1 - 20, exiting.\n");
      exit(1);
    }
  }else{
    printf("Invalid arguments\n");
    exit(1);
  }   

  //-- Init market value to 100 * numThreads
  //get rid of magic numbers below
  //int marketValue = 100 * numThreads;
  //marketValue = 100 * numThreads;
  theMarket.marketValue = 100 * numThreads;
  float change = theMarket.marketValue * (0.01*marketPerc);
  float marketDown = theMarket.marketValue - change;
  printf("MarketDown value %f\n", marketDown);
  float marketUp = theMarket.marketValue + change;
  printf("MarketUp value %f\n", marketUp);
  theMarket.running = 1;
  theMarket.mD = marketDown;
  theMarket.mU = marketUp;

//-- Create array of stocks -----------------------------------------
  int* stocks = malloc(sizeof(*stocks) * numThreads);
  struct Stock threadStock[numThreads];
//-- Create pthreads ------------------------------------------------
  pthread_t threads[numThreads+2];
  //pthread_t aThread;
  //pthread_t wThread;
  int rc;
  int cr;
//-- Create mutex ---------------------------------------------------
  pthread_mutex_t marketVsum;
  
  int rd;
  rd = pthread_create(&threads[numThreads], NULL, initWatchDown, &marketDown);
  int ru;
  ru = pthread_create(&threads[numThreads+1], NULL, initWatchUp, &marketUp);
  
  int t;
  for(t=0; t<numThreads; t++){
    threadStock[t].threadId = t+1;
    rc = pthread_create(&threads[t], NULL, initStock, (void*) &threadStock[t]);
    if(rc){
      fprintf(stderr, "ERROR: return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for(t=0; t<numThreads+2; t++){
    pthread_join(threads[t], NULL);
  }
  printf("Total Market Price of %d Stocks: %.2f\n", numThreads, theMarket.marketValue);
  return 0;
 // rc = pthread_create(&aThread, NULL, initStock, (void*) &threadStock[0]);
 // printf("A thread was created\n");
  //pthread_exit(NULL);
  //have a while look that continuously increases stocks
}

void* initStock(void* threadArg){
  struct Stock *theStock;
  theStock = (struct Stock *)threadArg;
  theStock->stockPrice = 100;
  //printf("The stock for thread %d is %d\n", theStock->threadId, theStock->stockPrice);
  int i;
  srand(time(NULL));
  float stockChange;
  //for(i = 1; i<=5; i++){
  while(theMarket.running == 1){
    pthread_mutex_lock (&marketVsum);
    //printf("%f\n", theMarket.mU);
    if(theMarket.marketValue>theMarket.mU){
      pthread_cond_signal(&marketWatch);
      //pthread_cond_broadcast(&marketWatch);     
    }else if(theMarket.marketValue<theMarket.mD){
      pthread_cond_signal(&markDown);
      //pthread_cond_broadcast(&marketWatch);     
    }else{
      //pthread_cond_broadcast(&marketWatch);     
    //Get rid of magic numbers below
    //stockChange = (float)rand() % 31 + (-15);
    int choose = rand() % 5 + (-2);
    //printf("choose, %d\n", choose);
    stockChange = choose * ((float)rand()/(float)(RAND_MAX))*20;
    //printf("Stock change, %d\n", stockChange);
    theStock->stockPrice+=stockChange;

    //pthread_mutex_lock (&marketVsum);
    //marketValue+=theStock->stockPrice;
    theMarket.marketValue+=stockChange;
    //theMarket.marketValue+=theStock->stockPrice;
    //if(theMarket.marketValue>theMarket.mU){
      //pthread_cond_signal(&marketWatch);
    //}
    //pthread_mutex_unlock (&marketVsum);
    //printf("The stock price for thread %d has gone up to %d\n", theStock->threadId, theStock->stockPrice);
    printf("Market at %.2f\n", theMarket.marketValue);
    }
    /*
    //pthread_mutex_lock(&getIn);
    //Get rid of magic numbers below
    stockChange = rand() % 11 + (-5);
    //printf("Stock change, %d\n", stockChange);
    theStock->stockPrice+=stockChange;

    //pthread_mutex_lock (&marketVsum);
    //marketValue+=theStock->stockPrice;
    theMarket.marketValue+=theStock->stockPrice;
    //if(theMarket.marketValue>theMarket.mU){
      //pthread_cond_signal(&marketWatch);
    //}
    //pthread_mutex_unlock (&marketVsum);
    //printf("The stock price for thread %d has gone up to %d\n", theStock->threadId, theStock->stockPrice);
    printf("Market Value is: %f\n", theMarket.marketValue);
    //pthread_mutex_unlock(&getIn);*/
    pthread_mutex_unlock (&marketVsum);
    sleep(1);
  }

  pthread_exit(NULL);
}

void* initWatchDown(void* threadArg){
  pthread_mutex_lock(&marketVsum);
  float marketDown = *(float*)threadArg;

  //pthread_mutex_lock(&marketVsum);
  while(theMarket.marketValue>marketDown && theMarket.running==1){
    //wait for signal
    pthread_cond_wait(&markDown, &marketVsum);
    pthread_cond_signal(&marketWatch);
  }
  theMarket.running = 0;
  pthread_mutex_unlock(&marketVsum);
  
  pthread_exit(NULL);
}

void* initWatchUp(void* threadArg){
  pthread_mutex_lock(&marketVsum);
  //pthread_mutex_lock(&marketVsum);
  float marketUp = *(float*)threadArg;
  //printf("In the marketup\n");
  //pthread_mutex_lock(&marketVsum);
  while(theMarket.marketValue<marketUp && theMarket.running==1){
    //wait for signal
    //printf("Waiting for signal\n");
    pthread_cond_wait(&marketWatch, &marketVsum);
    pthread_cond_signal(&markDown);
  }
  theMarket.running = 0;
  //printf("Change run status\n");
  pthread_mutex_unlock(&marketVsum);
  //theMarket.running = 0;
  pthread_exit(NULL);
}
