/**
 * @file deadlock_circular_send.c
 * @brief Seeded Error Test: Circular Deadlock from Blocking Sends
 * 
 * This test simulates a classic deadlock scenario where both Rank 0 and Rank 1
 * initiate a blocking MPI_Send to each other before issuing their corresponding
 * MPI_Recv. With large messages or unbuffered sends, this is guaranteed to deadlock.
 * The MPI Usage Sanitizer is expected to flag this blocking circular pattern.
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

    // To ensure deadlock across all MPI implementations, we use a larger payload
    // that exceeds typical eager-protocol buffering limits.
    const int count = 200000; 
    int *send_buf = (int *)malloc(count * sizeof(int));
    int *recv_buf = (int *)malloc(count * sizeof(int));

    if (rank == 0) {
        // Rank 0 sends to Rank 1 first, blocking
        printf("[Rank 0] Initiating blocking Send to Rank 1...\n");
        MPI_Send(send_buf, count, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(recv_buf, count, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 0] Completed Send and Recv\n");
    } else if (rank == 1) {
        // Rank 1 sends to Rank 0 first, blocking (Circular dependency / Deadlock!)
        printf("[Rank 1] Initiating blocking Send to Rank 0...\n");
        MPI_Send(send_buf, count, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(recv_buf, count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Completed Send and Recv\n");
    }

    free(send_buf);
    free(recv_buf);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
