/**
 * @file collective_param_mismatch_root.c
 * @brief Seeded Error Test: Collective Parameter Mismatch (Root Rank Mismatch)
 * 
 * This test simulates a parameter mismatch error where different processes
 * specify different values for the root parameter of MPI_Bcast.
 * The MPI Usage Sanitizer is expected to detect this mismatched root parameter.
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

    int val = 10;
    int root = (rank == 0) ? 0 : 1; // Rank 0 thinks root is 0, Rank 1 thinks root is 1 (Mismatch!)

    MPI_Bcast(&val, 1, MPI_INT, root, MPI_COMM_WORLD);
    printf("[Rank %d] Completed Bcast with claimed root = %d\n", rank, root);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
