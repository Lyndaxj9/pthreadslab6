//lab6 pthreads
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct Stock{
  int stockPrice = 100;
};

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
//-- Create array of stocks -----------------------------------------
  int stocks[numThreads] = {100};
//-- Create pthreads ------------------------------------------------
  pthread_t threads[numThreads];
  int rc;
  rc = pthread_create(&threads[t], NULL, NULL, NULL);
  printf("A thread was created\n");
  pthread_exit(NULL);
  //have a while look that continuously increases stocks
}

void initStock(int* array, int stockNumber){
  array[stockNumber] = 100;
}
