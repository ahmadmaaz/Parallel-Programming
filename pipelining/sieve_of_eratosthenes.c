#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/*
    This function is used to pad the primes integer array with -1 in order to split the prime integers equally on the processes
    MPI Scatter function requires the array to be divisible by the size --> Pad the array with -1 and ignore them in the logic
*/
int *padd_primes_arr(int *arr, int *primes_count, int processes_count)
{
    if (*primes_count % processes_count == 0)
        return arr;
    int new_arr_primes_size = ((int)ceil((double)(*primes_count) / processes_count)) * processes_count;
    arr = realloc(arr, new_arr_primes_size * sizeof(int));
    printf("%d", new_arr_primes_size - *primes_count);
    for (int i = *primes_count; i < new_arr_primes_size; i++)
    {
        arr[i] = -1;
    }
    *primes_count = new_arr_primes_size;
    return arr;
}

/*
    This function will generate array with prime numbers with range [2,n]
*/

int *generate_primes(int n, int *prime_count)
{
    bool *is_prime = malloc((n + 1) * sizeof(bool));
    memset(is_prime, true, (n + 1) * sizeof(bool));

    for (int i = 2; i <= n; i++)
    {
        if (is_prime[i])
        {
            for (int j = i * 2; j <= n; j += i)
                is_prime[j] = false;
        }
    }

    int *prime_numbers = NULL;
    *prime_count = 0;
    for (int i = 2; i <= n; i++)
    {
        if (is_prime[i])
        {
            int *temp = realloc(prime_numbers, (*prime_count + 1) * sizeof(int)); // Dynamically add element to array through re-allocation
            prime_numbers = temp;
            prime_numbers[*prime_count] = i;
            (*prime_count)++;
        }
    }
    free(is_prime);
    return prime_numbers;
}

int main(int argc, char **argv)
{
    int rank, size;
    double sequential_start, sequential_end, sequential_time;
    double parallel_start, parallel_end, parallel_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = atoi(argv[1]); // n is set through args

    int *prime_numbers = NULL;
    int total_prime_numbers = 0;
    int prime_numbers_per_processor = 0;

    // Start of seq approach
    if (rank == 0)
    {
        sequential_start = MPI_Wtime();
        int dummy_prime_count;
        int *dummy_primes = generate_primes(n, &dummy_prime_count);
        free(dummy_primes);
        sequential_end = MPI_Wtime();
        sequential_time = sequential_end - sequential_start;
    }
    // End of seq approach
    //
    //
    //
    // Start of parallel approach

    MPI_Barrier(MPI_COMM_WORLD);
    parallel_start = MPI_Wtime();

    if (rank == 0)
    {

        prime_numbers = generate_primes((int)sqrt(n), &total_prime_numbers);
        // Error handling
        if (prime_numbers == NULL)
        {
            fprintf(stderr, "Error generating primes.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        prime_numbers = padd_primes_arr(prime_numbers, &total_prime_numbers, size);
        prime_numbers_per_processor = total_prime_numbers / size;
    }

    MPI_Bcast(&prime_numbers_per_processor, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&total_prime_numbers, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int *local_prime_numbers = (int *)malloc(prime_numbers_per_processor * sizeof(int));

    MPI_Scatter(prime_numbers, prime_numbers_per_processor, MPI_INT, local_prime_numbers, prime_numbers_per_processor, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        for (int i = (int)sqrt(n); i <= n; i++)
        {
            bool is_prime = true;
            for (int j = 0; j < prime_numbers_per_processor; j++)
            {
                if (local_prime_numbers[j] == -1) // Ignoring padded elements
                    continue;
                if (i % local_prime_numbers[j] == 0)
                {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime)
                MPI_Send(&i, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        }
        int terminate = -1;
        MPI_Send(&terminate, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        while (1)
        {
            int nmb;
            MPI_Recv(&nmb, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (nmb == -1)
            {
                if (rank < size - 1)
                    MPI_Send(&nmb, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                break;
            }

            bool is_prime = true;
            for (int j = 0; j < prime_numbers_per_processor; j++)
            {
                if (local_prime_numbers[j] == -1) // Ignoring padded elements
                    continue;
                if (nmb % local_prime_numbers[j] == 0)
                {
                    is_prime = false;
                    break;
                }
            }

            if (is_prime && rank < size - 1)
            {
                MPI_Send(&nmb, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            }
            else if (is_prime)
            {
                // Element here is a prime element , you can store them in an array and then return them back to master when done
                // printf("%d is prime\n", nmb);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    free(local_prime_numbers);
    if (rank == 0)
    {
        free(prime_numbers);
    }

    parallel_end = MPI_Wtime();
    parallel_time = parallel_end - parallel_start;
    // Printing results
    if (rank == 0)
    {
        printf("----------------- Parallel Results ----------------\n");
        printf("The time taken  is %f\n", parallel_time);
        printf("----------------- Seq Results ----------------\n");
        printf("The time taken  is %f\n", sequential_time);
        float speed_up = sequential_time / parallel_time;
        printf("-----------------------------------------------\n");
        printf("The speedup factor is %f\n", speed_up);
        printf("The efficiency is %f%%\n", (speed_up / size) * 100);
    }

    MPI_Finalize();
    return 0;
}
