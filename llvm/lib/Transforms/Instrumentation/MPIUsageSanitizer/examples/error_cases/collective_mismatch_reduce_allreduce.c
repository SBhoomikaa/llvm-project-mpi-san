/**
 * @file collective_mismatch_reduce_allreduce.c
 * @brief Seeded Error Test: Collective Operation Mismatch (Reduce vs Allreduce)
 * 
 * This test simulates a collective mismatch error where Rank 0 calls MPI_Reduce
 * but Rank 1 calls MPI_Allreduce on the same communicator.
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

    int send_val = rank;
    int recv_val = 0;

    if (rank == 0) {
        // Rank 0 calls MPI_Reduce
        MPI_Reduce(&send_val, &recv_val, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Completed Reduce: sum = %d\n", recv_val);
    } else {
        // Rank 1+ calls MPI_Allreduce (Mismatched Collective!)
        MPI_Allreduce(&send_val, &recv_val, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        printf("[Rank %d] Completed Allreduce: sum = %d\n", rank, recv_val);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
