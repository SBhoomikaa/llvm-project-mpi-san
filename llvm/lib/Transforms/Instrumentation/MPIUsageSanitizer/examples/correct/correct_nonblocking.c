/**
 * @file correct_nonblocking.c
 * @brief Correct non-blocking communication example (Zero False Positives Test)
 * 
 * This program demonstrates correct and standards-compliant non-blocking communication
 * using MPI_Isend and MPI_Irecv followed by MPI_Wait to ensure the operation completes
 * before accessing or deallocating the communication buffers.
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

    const int count = 50;
    int *send_buffer = (int *)malloc(count * sizeof(int));
    int *recv_buffer = (int *)malloc(count * sizeof(int));

    MPI_Request request;
    MPI_Status status;

    if (rank == 0) {
        // Initialize send buffer
        for (int i = 0; i < count; i++) {
            send_buffer[i] = i * 2;
        }
        // Initiate non-blocking send
        printf("[Rank 0] Initiating non-blocking send of %d integers to Rank 1...\n", count);
        MPI_Isend(send_buffer, count, MPI_INT, 1, 101, MPI_COMM_WORLD, &request);
        
        // Safe: Do not touch send_buffer until MPI_Wait completes
        printf("[Rank 0] Waiting for Isend completion...\n");
        MPI_Wait(&request, &status);
        printf("[Rank 0] Isend completed successfully.\n");
    } else if (rank == 1) {
        // Initialize receive buffer to zero
        for (int i = 0; i < count; i++) {
            recv_buffer[i] = 0;
        }
        // Initiate non-blocking receive
        printf("[Rank 1] Initiating non-blocking receive of %d integers from Rank 0...\n", count);
        MPI_Irecv(recv_buffer, count, MPI_INT, 0, 101, MPI_COMM_WORLD, &request);
        
        // Safe: Do not access recv_buffer until MPI_Wait completes
        printf("[Rank 1] Waiting for Irecv completion...\n");
        MPI_Wait(&request, &status);
        printf("[Rank 1] Irecv completed successfully. Element 5 = %d\n", recv_buffer[5]);
    }

    free(send_buffer);
    free(recv_buffer);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
