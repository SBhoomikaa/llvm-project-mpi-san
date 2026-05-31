/**
 * @file correct_collectives.c
 * @brief Correct collective operations example (Zero False Positives Test)
 * 
 * This program demonstrates correct and standards-compliant collective MPI operations
 * (MPI_Bcast, MPI_Reduce, and MPI_Barrier) where all processes within the communicator
 * participate uniformly with matching roots, data types, and count parameters.
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

    int root = 0;
    int data[5] = {0};

    // 1. Correct Bcast: All processes call Bcast with matching count (5), type (MPI_INT), and root (0)
    if (rank == root) {
        for (int i = 0; i < 5; i++) {
            data[i] = (i + 1) * 10;
        }
    }
    printf("[Rank %d] Entering Bcast...\n", rank);
    MPI_Bcast(data, 5, MPI_INT, root, MPI_COMM_WORLD);
    printf("[Rank %d] Bcast finished: data[4] = %d\n", rank, data[4]);

    // 2. Correct Barrier: All processes participate uniformly
    printf("[Rank %d] Entering Barrier...\n", rank);
    MPI_Barrier(MPI_COMM_WORLD);
    printf("[Rank %d] Barrier finished.\n", rank);

    // 3. Correct Reduce: All processes call Reduce with matching parameters
    int send_val = rank + 1;
    int sum_recv = 0;
    printf("[Rank %d] Entering Reduce...\n", rank);
    MPI_Reduce(&send_val, &sum_recv, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);
    
    if (rank == root) {
        printf("[Rank 0] Reduce finished: Sum = %d\n", sum_recv);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
