/**
 * @file count_mismatch.c
 * @brief Seeded Error Test: Count Mismatch in point-to-point communication
 * 
 * This test simulates a count mismatch error where the sender sends a larger
 * number of elements (10) than the receiver is expecting to receive (5).
 * The MPI Usage Sanitizer is expected to detect this count mismatch.
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
        int send_buffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        // Sending 10 MPI_INT to Rank 1
        MPI_Send(send_buffer, 10, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Sent 10 MPI_INTs\n");
    } else if (rank == 1) {
        int recv_buffer[5];
        // Receiving 5 MPI_INT from Rank 0 (Count Mismatch!)
        MPI_Recv(recv_buffer, 5, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Received 5 MPI_INTs\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
