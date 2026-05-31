/**
 * @file buffer_aliasing_overlap.c
 * @brief Seeded Error Test: Buffer Aliasing / Overlapping Buffers
 * 
 * This test simulates a buffer aliasing error where MPI_Sendrecv is called
 * with overlapping send and receive buffers in the same memory array.
 * According to the MPI standard, this results in undefined behavior.
 * The MPI Usage Sanitizer is expected to detect this buffer overlap.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    // MPI_Sendrecv where send buffer (&arr[0]) and receive buffer (&arr[2]) overlap.
    // Overlap for 5 elements:
    // Send region: arr[0..4] (bytes 0 to 19)
    // Recv region: arr[2..6] (bytes 8 to 27)
    // Overlapping bytes: 8 to 19 (elements arr[2..4])
    MPI_Sendrecv(&arr[0], 5, MPI_INT, rank, 0,
                 &arr[2], 5, MPI_INT, rank, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("[Rank %d] Completed overlapping Sendrecv\n", rank);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
