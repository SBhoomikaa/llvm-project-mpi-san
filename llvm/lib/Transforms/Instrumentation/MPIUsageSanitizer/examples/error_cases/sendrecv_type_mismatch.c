/**
 * @file sendrecv_type_mismatch.c
 * @brief Seeded Error Test: Type Mismatch in MPI_Sendrecv
 * 
 * This test simulates a type mismatch error inside MPI_Sendrecv.
 * - Rank 0 calls Sendrecv, sending MPI_FLOAT to Rank 1 and receiving MPI_INT from Rank 1.
 * - Rank 1 calls Sendrecv, sending MPI_INT to Rank 0 but expecting to receive MPI_DOUBLE from Rank 0.
 * This triggers type mismatches on the communications.
 * The MPI Usage Sanitizer is expected to flag these mismatched transfer types.
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
        float send_val = 1.23f;
        int recv_val = 0;
        // Send MPI_FLOAT to 1, receive MPI_INT from 1
        MPI_Sendrecv(&send_val, 1, MPI_FLOAT, 1, 0,
                     &recv_val, 1, MPI_INT, 1, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 0] Sendrecv complete: received %d\n", recv_val);
    } else if (rank == 1) {
        int send_val = 456;
        double recv_val = 0.0;
        // Send MPI_INT to 0, receive MPI_DOUBLE from 0 (Type Mismatch: Rank 0 sent float!)
        MPI_Sendrecv(&send_val, 1, MPI_INT, 0, 0,
                     &recv_val, 1, MPI_DOUBLE, 0, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Sendrecv complete: received %f\n", recv_val);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
