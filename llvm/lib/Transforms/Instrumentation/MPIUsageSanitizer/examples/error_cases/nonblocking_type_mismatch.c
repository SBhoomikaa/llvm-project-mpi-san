/**
 * @file nonblocking_type_mismatch.c
 * @brief Seeded Error Test: Type Mismatch in Non-blocking communication
 * 
 * This test simulates a type mismatch error involving non-blocking operations.
 * Rank 0 initiates an MPI_Isend with type MPI_DOUBLE, but Rank 1 initiates
 * an MPI_Irecv with type MPI_INT.
 * The MPI Usage Sanitizer is expected to track the request types and flag the mismatch.
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

    MPI_Request req;

    if (rank == 0) {
        double val = 99.9;
        // Rank 0 non-blocking sends 1 MPI_DOUBLE to Rank 1
        MPI_Isend(&val, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        printf("[Rank 0] Completed Wait for Isend\n");
    } else if (rank == 1) {
        int val = 0;
        // Rank 1 non-blocking receives 1 MPI_INT from Rank 0 (Type Mismatch!)
        MPI_Irecv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        printf("[Rank 1] Completed Wait for Irecv, value = %d\n", val);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
