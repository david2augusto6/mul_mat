#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/*
    Programa para multiplicação sequencial de matrizes
*/

void mul_mat(int **m1, int **m2, int **result, int SIZE){
    for(int i=0; i < SIZE; i++){
            for(int j=0; j < SIZE; j++){
                int sum = 0;
                for(int k=0; k < SIZE; k++){
                    sum += m1[i][k] * m2[k][j];
                }
                result[i][j] = sum;
            }
        }
}

int main(){
    int **m1;
    int **m2;
    int **result;
    struct timeval start, end;
    long mtime, seconds, useconds;
    FILE *fp;

    int SIZE;
    int size[] = {100, 200, 500, 1000, 2000};
    int num_sizes = sizeof(size) / sizeof(size[0]);

    // Open CSV file for writing
    fp = fopen("sequencial_results.csv", "w");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    // Write CSV header
    fprintf(fp, "Matrix Size,Execution Time (ms)\n");

    for(int s = 0; s < num_sizes; s++){
        SIZE = size[s];
        printf("Processing matrix size: %d x %d\n", SIZE, SIZE);

        // Allocate memory for matrices
        m1 = malloc(SIZE * sizeof(int *));
        m2 = malloc(SIZE * sizeof(int *));
        result = malloc(SIZE * sizeof(int *));
        
        for (int i = 0; i < SIZE; i++){
            m1[i] = malloc(SIZE * sizeof(int));
            m2[i] = malloc(SIZE * sizeof(int));
            result[i] = malloc(SIZE * sizeof(int));
            for (int j = 0; j < SIZE; j++){
                m1[i][j] = rand() % 10;
                m2[i][j] = rand() % 10;
                result[i][j] = 0;
            }
        }

        // Start timing
        gettimeofday(&start, NULL);
        
        // Perform matrix multiplication
        mul_mat(m1, m2, result, SIZE);
        
        // End timing
        gettimeofday(&end, NULL);

        // Calculate time difference
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0);

        // Write results to CSV
        fprintf(fp, "%d,%ld\n", SIZE, mtime);
        printf("Time taken: %ld milliseconds\n", mtime);

        // Free memory
        for (int i = 0; i < SIZE; i++) {
            free(m1[i]);
            free(m2[i]);
            free(result[i]);
        }
        free(m1);
        free(m2);
        free(result);
    }

    // Close the CSV file
    fclose(fp);
    printf("Results have been saved to sequencial_results.csv\n");

    return 0;
}