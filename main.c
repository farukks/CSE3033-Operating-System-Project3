#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "Vector.h"
#include "reads.h"
 

int main(int argc, char **argv)
{
 
    if(argc < 8){
        printf("params error!");
        return 1;
    }

    strcpy(filePath,argv[2]);
    mainThread(atoi(argv[4]),atoi(argv[5]),atoi(argv[6]),atoi(argv[7]));


    // printf("\n\n");
    // printf("| /%s\\  /\\ |\n\n","cc");
    // printf("| \\/  \\/ |");
    

}

