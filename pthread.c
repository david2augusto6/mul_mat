#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
    Programa para multiplicação de matrizes usando pthreads
*/

// Estrutura para passar argumentos para as threads
typedef struct {
    int **m1;
    int **m2;
    int **result;
    int SIZE;
    int start_row;
    int end_row;
} thread_args;

// Função que cada thread irá executar
void *mul_mat_thread(void *arg) {
    thread_args *args = (thread_args *)arg;
    
    for(int i = args->start_row; i < args->end_row; i++) {
        for(int j = 0; j < args->SIZE; j++) {
            int sum = 0;
            for(int k = 0; k < args->SIZE; k++) {
                sum += args->m1[i][k] * args->m2[k][j];
            }
            args->result[i][j] = sum;
        }
    }
    
    pthread_exit(NULL);
}

// Função para obter o número de processadores disponíveis
int get_num_processors() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

// Função para multiplicar matrizes usando threads
void mul_mat_parallel(int **m1, int **m2, int **result, int SIZE, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_args *args = malloc(num_threads * sizeof(thread_args));
    
    int rows_per_thread = SIZE / num_threads;
    int remaining_rows = SIZE % num_threads;
    int current_row = 0;
    
    // Criar e iniciar as threads
    for(int i = 0; i < num_threads; i++) {
        args[i].m1 = m1;
        args[i].m2 = m2;
        args[i].result = result;
        args[i].SIZE = SIZE;
        args[i].start_row = current_row;
        
        // Distribuir as linhas restantes entre as primeiras threads
        int extra_row = (i < remaining_rows) ? 1 : 0;
        args[i].end_row = current_row + rows_per_thread + extra_row;
        current_row = args[i].end_row;
        
        pthread_create(&threads[i], NULL, mul_mat_thread, &args[i]);
    }
    
    // Aguardar todas as threads terminarem
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(args);
}

int main() {
    int **m1;
    int **m2;
    int **result;
    struct timeval start, end;
    long mtime, seconds, useconds;
    FILE *fp;

    int SIZE;
    int size[] = {100, 200, 500, 1000, 2000};
    int num_sizes = sizeof(size) / sizeof(size[0]);
    
    // Get number of available processors
    int max_threads = get_num_processors();
    printf("Number of processors available: %d\n", max_threads);

    // Open CSV file for writing
    fp = fopen("pthread_results.csv", "w");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    // Write CSV header
    fprintf(fp, "Matrix Size,Num Threads,Execution Time (ms)\n");

    // Test each matrix size
    for(int s = 0; s < num_sizes; s++) {
        SIZE = size[s];
        printf("\nTesting matrix size: %d x %d\n", SIZE, SIZE);

        // Allocate memory for matrices
        m1 = malloc(SIZE * sizeof(int *));
        m2 = malloc(SIZE * sizeof(int *));
        result = malloc(SIZE * sizeof(int *));
        
        for (int i = 0; i < SIZE; i++) {
            m1[i] = malloc(SIZE * sizeof(int));
            m2[i] = malloc(SIZE * sizeof(int));
            result[i] = malloc(SIZE * sizeof(int));
            for (int j = 0; j < SIZE; j++) {
                m1[i][j] = rand() % 10;
                m2[i][j] = rand() % 10;
            }
        }

        // Test different numbers of threads
        for(int num_threads = 1; num_threads <= max_threads; num_threads++) {
            if (num_threads == max_threads / 2 || num_threads == max_threads) {
                printf("  Using %d thread(s)...\n", num_threads);

                // Reset result matrix
                for (int i = 0; i < SIZE; i++) {
                    for (int j = 0; j < SIZE; j++) {
                        result[i][j] = 0;
                    }
                }

                // Start timing
                gettimeofday(&start, NULL);
                
                // Perform parallel matrix multiplication
                mul_mat_parallel(m1, m2, result, SIZE, num_threads);
                
                // End timing
                gettimeofday(&end, NULL);

                // Calculate time difference
                seconds = end.tv_sec - start.tv_sec;
                useconds = end.tv_usec - start.tv_usec;
                mtime = ((seconds) * 1000 + useconds/1000.0);

                // Write results to CSV
                fprintf(fp, "%d,%d,%ld\n", SIZE, num_threads, mtime);
                printf("    Time taken: %ld milliseconds\n", mtime);
            }
        }

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
    printf("\nResults have been saved to pthread_results.csv\n");

    return 0;
}