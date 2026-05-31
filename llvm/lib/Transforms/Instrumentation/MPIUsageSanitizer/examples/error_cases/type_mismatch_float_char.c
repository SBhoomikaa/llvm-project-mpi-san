/**
 * @file type_mismatch_float_char.c
 * @brief Seeded Error Test: Type Mismatch between MPI_FLOAT and MPI_CHAR
 * 
 * This test simulates a type mismatch error where the sender sends data
 * of type MPI_FLOAT, but the receiver expects data of type MPI_CHAR.
 * The MPI Usage Sanitizer is expected to detect this mismatch.
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
        float send_val = 3.14f;
        // Sending 1 MPI_FLOAT to Rank 1
        MPI_Send(&send_val, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Sent 1 MPI_FLOAT\n");
    } else if (rank == 1) {
        char recv_val = '\0';
        // Receiving 1 MPI_CHAR from Rank 0 (Type Mismatch!)
        MPI_Recv(&recv_val, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Received char value: %c\n", recv_val);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
