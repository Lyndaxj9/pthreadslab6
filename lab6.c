/* ------------------------------------------------------------------
 * CS 350 Lab6 pthreads
 * lab6.c
 * Omowumi L. Ademola
 * ----------------------------------------------------------------*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//-- Struct for stock thread arguments ------------------------------
struct Stock{
  int threadId;
  int stockPrice;
};

//-- Global struct shared between all threads for the market --------
struct Market{
  float marketValue;
  int running;
  float mD;
  float mU;
};
struct Market theMarket;

//-- mutex and condiion values --------------------------------------
pthread_mutex_t marketVsum;
pthread_cond_t marketWatch;
pthread_cond_t markDown;

//-- function declerations ------------------------------------------
void* initStock(void* threadArg);
void* initWatchDown(void* threadArg);
void* initWatchUp(void* threadArg);

int main(int argc, char *argv[]){

  int numThreads = 0;
  int marketPerc = 0;

//-- Taking in and parsing arguments --------------------------------
  if(argc==3){
    numThreads = atoi(argv[1]);
    marketPerc = atoi(argv[2]);

    // number of threads btw 1 and 1000
    if(numThreads < 1 || numThreads > 1000){
      fprintf(stderr, "Error: The number of threads are not in the \
range 1 - 1000, exiting.\n");
      exit(1);
    }
    // market % change btw 1 and 20
    else if(marketPerc < 1 || marketPerc > 20){
      fprintf(stderr, "Error: The number for the market percentage \
is not in the range 1 - 20, exiting.\n");
      exit(1);
    }
  }else{
    printf("Arguments should take form ./lab6 <numberThreads> <percent>\n");
    exit(1);
  }   

//-- Init market struct values --------------------------------------
  int stockStartValue = 100;
  theMarket.marketValue = stockStartValue * numThreads;
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

//-- Initialize mutex -----------------------------------------------
  pthread_mutex_init(&marketVsum, NULL);

//-- Create pthreads ------------------------------------------------
  pthread_t threads[numThreads+2];
  int rc;
  int rd;
  int ru;
  
  rd = pthread_create(&threads[numThreads], NULL, initWatchDown, &marketDown);
  if(rd){
    fprintf(stderr, "ERROR: return code from pthread_create() is %d\n", rd);
    exit(-1);
  }
  ru = pthread_create(&threads[numThreads+1], NULL, initWatchUp, &marketUp);
  if(ru){
    fprintf(stderr, "ERROR: return code from pthread_create() is %d\n", ru);
    exit(-1);
  }
  
  int t;
  for(t=0; t<numThreads; t++){
    threadStock[t].threadId = t+1;
    rc = pthread_create(&threads[t], NULL, initStock, (void*) &threadStock[t]);
    if(rc){
      fprintf(stderr, "ERROR: return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

//-- Wait for all the threads to exit -------------------------------
  for(t=0; t<numThreads+2; t++){
    pthread_join(threads[t], NULL);
  }

  printf("Total Market Price of %d Stocks: %.2f\n", numThreads, theMarket.marketValue);

  free(stocks);

  return 0;
  //pthread_exit(NULL);
}
/* This thread is used to initialize the stock variables in the
 * struct for each thread and then increase or decrease the values
 * ----------------------------------------------------------------*/
void* initStock(void* threadArg){
  struct Stock *theStock;
  theStock = (struct Stock *)threadArg;
  theStock->stockPrice = 100;
  //printf("The stock for thread %d is %d\n", theStock->threadId, theStock->stockPrice);

  int i;
  srand(time(NULL));
  float stockChange;
  
  while(theMarket.running == 1){
    pthread_mutex_lock (&marketVsum);
    //printf("%f\n", theMarket.mU);
    
    //-- First check market then signal -----------------------------
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
    //-- Generate a random pos or neg number ------------------------
      int choose = rand() % 5 + (-2);
    //printf("choose, %d\n", choose);
    //-- Generate a random float and multiple by the pos or neg number
      stockChange = choose * ((float)rand()/(float)(RAND_MAX))*20;
    //printf("Stock change, %d\n", stockChange);
    //-- Change stock price -----------------------------------------
      theStock->stockPrice+=stockChange;
    //-- Change market value ----------------------------------------
      theMarket.marketValue+=stockChange;
    
    //printf("The stock price for thread %d has gone up to %d\n", theStock->threadId, theStock->stockPrice);
      printf("Market at %.2f\n", theMarket.marketValue);
    }
    pthread_mutex_unlock (&marketVsum);
    sleep(1);
  }

//-- After the marketWatcher sees specified change in market running
//running gets set to 0 and the stock thread jumps out of the while
//loop and comes here to exit
  pthread_exit(NULL);
}

/* Thread used to initialize the marketDown watcher thread
 * ----------------------------------------------------------------*/
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

/* Thread used to initalize the marketUp watcher thread
 * ----------------------------------------------------------------*/
void* initWatchUp(void* threadArg){
  pthread_mutex_lock(&marketVsum);
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
  
  pthread_exit(NULL);
}
