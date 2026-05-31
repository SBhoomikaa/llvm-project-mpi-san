/**
 * @file deadlock_circular_recv.c
 * @brief Seeded Error Test: Circular Deadlock from Blocking Receives
 * 
 * This test simulates a deadlock scenario where both Rank 0 and Rank 1
 * initiate a blocking MPI_Recv from each other before calling their corresponding
 * MPI_Send. This causes both ranks to wait indefinitely, resulting in an immediate deadlock.
 * The MPI Usage Sanitizer is expected to flag this circular dependency.
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
    int recv_val = -1;

    if (rank == 0) {
        // Rank 0 receives from Rank 1, blocking
        printf("[Rank 0] Initiating blocking Recv from Rank 1...\n");
        MPI_Recv(&recv_val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&send_val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Completed Recv and Send\n");
    } else if (rank == 1) {
        // Rank 1 receives from Rank 0, blocking (Circular deadlock!)
        printf("[Rank 1] Initiating blocking Recv from Rank 0...\n");
        MPI_Recv(&recv_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&send_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("[Rank 1] Completed Recv and Send\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
