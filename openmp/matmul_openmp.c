#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>

void transpose(double *A, double *B, int n)
{
    int i, j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            B[j * n + i] = A[i * n + j];
        }
    }
}

void mm(double *A, double *B, double *C, int n)
{
    int i, j, k;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            double dot = 0;
            for (k = 0; k < n; k++)
            {
                dot += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = dot;
        }
    }
}

void mm_omp(double *A, double *B, double *C, int n)
{
    #pragma omp parallel
    {
        int i, j, k;
        #pragma omp for
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                double dot = 0;
                for (k = 0; k < n; k++)
                {
                    dot += A[i * n + k] * B[k * n + j];
                }
                C[i * n + j] = dot;
            }
        }
    }
}

void mmT(double *A, double *B, double *C, int n)
{
    int i, j, k;
    double *B2;
    B2 = (double *)malloc(sizeof(double) * n * n);
    transpose(B, B2, n);
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            double dot = 0;
            for (k = 0; k < n; k++)
            {
                dot += A[i * n + k] * B2[j * n + k];
            }
            C[i * n + j] = dot;
        }
    }
    free(B2);
}

void mmT_omp(double *A, double *B, double *C, int n)
{
    double *B2;
    B2 = (double *)malloc(sizeof(double) * n * n);
    transpose(B, B2, n);
    #pragma omp parallel
    {
        int i, j, k;
        #pragma omp for
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                double dot = 0;
                for (k = 0; k < n; k++)
                {
                    dot += A[i * n + k] * B2[j * n + k];
                }
                C[i * n + j] = dot;
            }
        }
    }
    free(B2);
}

int main(int argc, char **argv)
{
    int NUM_THREADS;
    if (argc < 2)
    {
        NUM_THREADS = 2;
    }
    else
    {
        NUM_THREADS = atoi(argv[1]);
    }
    omp_set_num_threads(NUM_THREADS); // Set num of threads

    int i, n;
    double *A, *B, *C, dtime;

    n = 2048;
    A = (double *)malloc(sizeof(double) * n * n);
    B = (double *)malloc(sizeof(double) * n * n);
    C = (double *)malloc(sizeof(double) * n * n);

    for (i = 0; i < n * n; i++)
    {
        A[i] = rand() / RAND_MAX;
        B[i] = rand() / RAND_MAX;
    }
    printf("Number of threads %d\n", NUM_THREADS);
    dtime = omp_get_wtime();
    mm(A, B, C, n);
    dtime = omp_get_wtime() - dtime;
    printf("mm: %f\n", dtime);

    dtime = omp_get_wtime();
    mm_omp(A, B, C, n);
    dtime = omp_get_wtime() - dtime;
    printf("mm_omp: %f\n", dtime);

    dtime = omp_get_wtime();
    mmT(A, B, C, n);
    dtime = omp_get_wtime() - dtime;
    printf("mmT: %f\n", dtime);

    dtime = omp_get_wtime();
    mmT_omp(A, B, C, n);
    dtime = omp_get_wtime() - dtime;
    printf("mmT_omp: %f\n", dtime);

    return 0;
}