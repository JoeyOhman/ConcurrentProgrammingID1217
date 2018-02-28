#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void findWoman(int, int, int);
void findMan(int, int);
void genRanking (int[], int, bool, int);
void swap(int*, int*);
void printArr(int[], int);
void countEngagedWomen(int);
int findIndex(int[], int, int);

int main(int argc, char* argv[]) {
  int rank, size;
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  srand(time(NULL) + rank);
  size--;
  if(size % 2) {
    if(rank == size) // Counter process
      printf("Process count must be odd! (including counter process)\n");
    MPI_Finalize();
    return 0;
  }
  // Men: 1, 3, 5
  // Women: 0, 2, 4
  bool man = rank % 2;

  // Counter process
  if(rank == size) {
    printf("Men: ");
    int i;
    for(i = 1; i < size; i+=2)
      printf("%d ", i);
    printf("\nWomen: ");
    for(i = 0; i < size; i+=2)
      printf("%d ", i);
    printf("\n");
  }
  MPI_Barrier(MPI_COMM_WORLD);

  if(rank == size) {
    countEngagedWomen(size/2);
  } else {
    if(man)
      findWoman(size/2, size, rank);
    else
      findMan(size/2, size);
  }

  MPI_Finalize();
  return 0;
}

void countEngagedWomen(int numWomenRemaining) {
  int i;
  int numPersons = numWomenRemaining*2;
  int ranking[numWomenRemaining];
  MPI_Status status;
  printf("\n");

  for(i = 0; i < numPersons; i++) {
    MPI_Recv(&ranking, numWomenRemaining, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(i % 2) // if received from man
      printf("Man %d ranking: ", i);
    else
      printf("Woman %d ranking: ", i);
    printArr(ranking, numWomenRemaining);
  }

  bool womenTaken[numWomenRemaining];
  for(i = 0; i < numWomenRemaining; i++)
    womenTaken[i] = false;

  int info[3]; // 0: manrank, 1: womanrank, 2: accepted (bool)
  while(numWomenRemaining > 0) {
    MPI_Recv(&info, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(info[2])
      printf("Man %d got engaged to woman %d\n", info[0], info[1]);
    else
      printf("Man %d got denied by woman %d\n", info[0], info[1]);

    if(!womenTaken[info[1]/2]) {
      numWomenRemaining--; // She received her first proposal
      womenTaken[info[1]/2] = 1;
      //printf("Woman %d got her first proposal!\n", info[1]);
    }
  }
  printf("\n");
  for(i = 0; i < numPersons; i++) { // Tell all processes to exit
    printf("Sending exit to %d\n", i);
    MPI_Send(&i, 0, MPI_INT, i, 0, MPI_COMM_WORLD);
  }

}

void findWoman(int numWomen, int counterProc, int rank) {
  int accepted = 0;
  MPI_Status status;
  int womanIndex = 0;
  int ranking[numWomen];
  genRanking(ranking, numWomen, false, counterProc);
  while (true) {
    while(!accepted) {
      MPI_Send(&accepted, 0, MPI_INT, ranking[womanIndex], 0, MPI_COMM_WORLD);
      MPI_Recv(&accepted, 1, MPI_INT, ranking[womanIndex], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      int infoArr[3] = {rank, ranking[womanIndex], accepted};
      MPI_Send(infoArr, 3, MPI_INT, counterProc, 0, MPI_COMM_WORLD); // Send info to counterproc
      if(accepted)
        womanIndex = ranking[womanIndex];
      else
        womanIndex = (womanIndex+1) % numWomen;
    }
    // Check if counterprocess told us to quit or woman left
    MPI_Recv(&accepted, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_SOURCE == counterProc) {
      break;
    } else { // Find a new woman
      accepted = 0;
      womanIndex = (womanIndex+1) % numWomen;
    }
  }
}

void findMan(int numMen, int counterProc) {
  int manIndex = -1;
  int intBuff;
  int ranking[numMen];
  MPI_Status status;
  genRanking(ranking, numMen, true, counterProc);
  while(true) {
    MPI_Recv(&intBuff, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_SOURCE == counterProc) {
      break;
    } else { // Proposal from man
      if(manIndex == -1 || findIndex(ranking, numMen, status.MPI_SOURCE) < manIndex) {
        intBuff = 1;
        MPI_Send(&intBuff, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD); // Accept
        if(manIndex != -1)
          MPI_Send(&intBuff, 0, MPI_INT, ranking[manIndex], 0, MPI_COMM_WORLD); // Cancel prev man

        manIndex = findIndex(ranking, numMen, status.MPI_SOURCE);
      } else {
        intBuff = 0;
        MPI_Send(&intBuff, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD); // Deny
      }
    }
  }
}

int findIndex(int arr[], int n, int val) {
  int i;
  for(i = 0; i < n; i++) {
    if(arr[i] == val)
      return i;
  }
  return -1;
}

void genRanking (int arr[], int n, bool findMan, int counterProc) {
  int i;
  for (i = findMan; i < (n*2); i+=2) // Generate numbers
    arr[i/2] = i;
  for (i = n-1; i > 0; i--) { // Random permutation
      int j = rand() % (i+1);
      swap(&arr[i], &arr[j]);
  }
  //printArr(arr, n);
  MPI_Send(arr, n, MPI_INT, counterProc, 0, MPI_COMM_WORLD);
}

void swap(int* a, int* b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void printArr(int list[], int n) {
  int i;
  printf("[");
  for(i = 0; i < n; i++)
    printf(" %d ", list[i]);
  printf("]\n");
}
