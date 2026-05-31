/**
 * @file buffer_aliasing_same.c
 * @brief Seeded Error Test: Buffer Aliasing / Same Buffer in Collective
 * 
 * This test simulates a buffer aliasing error where MPI_Reduce is called
 * with the same buffer pointer for both the sendbuf and recvbuf parameters
 * without utilizing MPI_IN_PLACE. This is prohibited by the MPI standard.
 * The MPI Usage Sanitizer is expected to detect this buffer aliasing.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int val = rank + 1;

    // MPI_Reduce with identical sendbuf and recvbuf pointers without MPI_IN_PLACE.
    // This is illegal and undefined in standard MPI.
    MPI_Reduce(&val, &val, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("[Rank 0] Reductions finished: val = %d\n", val);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
