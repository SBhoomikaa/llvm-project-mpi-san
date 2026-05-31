/**
 * @file collective_param_mismatch_count.c
 * @brief Seeded Error Test: Collective Parameter Mismatch (Count Mismatch)
 * 
 * This test simulates a parameter mismatch error where processes call a collective
 * operation (MPI_Bcast) with differing count values.
 * The MPI Usage Sanitizer is expected to detect this mismatched count parameter.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "This test requires at least 2 processes.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int buf[10] = {0};
    int count = (rank == 0) ? 10 : 5; // Rank 0 broadcasts 10 elements, others receive 5 (Mismatch!)

    MPI_Bcast(buf, count, MPI_INT, 0, MPI_COMM_WORLD);
    printf("[Rank %d] Completed Bcast with count = %d\n", rank, count);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
