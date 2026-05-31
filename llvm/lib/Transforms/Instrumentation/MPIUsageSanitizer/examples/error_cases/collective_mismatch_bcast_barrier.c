/**
 * @file collective_mismatch_bcast_barrier.c
 * @brief Seeded Error Test: Collective Operation Mismatch (Bcast vs Barrier)
 * 
 * This test simulates a collective mismatch error where Rank 0 calls MPI_Bcast
 * but Rank 1 calls MPI_Barrier on the same communicator.
 * The MPI Usage Sanitizer is expected to detect this mismatched collective.
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

    if (rank == 0) {
        int val = 100;
        // Rank 0 calls MPI_Bcast
        MPI_Bcast(&val, 1, MPI_INT, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Completed Bcast\n");
    } else {
        // Rank 1+ calls MPI_Barrier (Mismatched Collective!)
        MPI_Barrier(MPI_COMM_WORLD);
        printf("[Rank %d] Completed Barrier\n", rank);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
