/**
 * @file type_mismatch_int_double.c
 * @brief Seeded Error Test: Type Mismatch between MPI_INT and MPI_DOUBLE
 * 
 * This test simulates a type mismatch error where the sender sends data
 * of type MPI_INT, but the receiver expects data of type MPI_DOUBLE.
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
        int send_val = 42;
        // Sending 1 MPI_INT to Rank 1
        MPI_Send(&send_val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Sent 1 MPI_INT\n");
    } else if (rank == 1) {
        double recv_val = 0.0;
        // Receiving 1 MPI_DOUBLE from Rank 0 (Type Mismatch!)
        MPI_Recv(&recv_val, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Received double value: %f\n", recv_val);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
