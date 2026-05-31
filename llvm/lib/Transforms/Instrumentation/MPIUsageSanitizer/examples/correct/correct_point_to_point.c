/**
 * @file correct_point_to_point.c
 * @brief Correct point-to-point communication example (Zero False Positives Test)
 * 
 * This program demonstrates correct and standards-compliant point-to-point MPI communication
 * using MPI_Send and MPI_Recv with matching tags, data types, and counts.
 * The MPI Usage Sanitizer is expected to run this program without flagging any errors/warnings.
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

    const int count = 100;
    int *buffer = (int *)malloc(count * sizeof(int));

    if (rank == 0) {
        // Initialize buffer
        for (int i = 0; i < count; i++) {
            buffer[i] = i;
        }
        // Send exactly 100 MPI_INT to Rank 1 with tag 99
        printf("[Rank 0] Sending %d integers to Rank 1...\n", count);
        MPI_Send(buffer, count, MPI_INT, 1, 99, MPI_COMM_WORLD);
        printf("[Rank 0] Send completed.\n");
    } else if (rank == 1) {
        // Initialize receive buffer to zero
        for (int i = 0; i < count; i++) {
            buffer[i] = 0;
        }
        // Receive exactly 100 MPI_INT from Rank 0 with tag 99
        printf("[Rank 1] Receiving %d integers from Rank 0...\n", count);
        MPI_Recv(buffer, count, MPI_INT, 0, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Recv completed. First element = %d, Last element = %d\n", buffer[0], buffer[count - 1]);
    }

    free(buffer);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
