/**
 * @file deadlock_send_recv_cycle.c
 * @brief Seeded Error Test: 3-Process Circular Deadlock Cycle
 * 
 * This test simulates a multi-process deadlock cycle involving 3 ranks:
 * - Rank 0 sends to Rank 1, then receives from Rank 2.
 * - Rank 1 sends to Rank 2, then receives from Rank 0.
 * - Rank 2 sends to Rank 0, then receives from Rank 1.
 * Since all calls are blocking and payload size is large, a circular dependency cycle forms.
 * The MPI Usage Sanitizer is expected to detect this multi-process deadlock.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 3) {
        if (rank == 0) {
            fprintf(stderr, "This test requires at least 3 processes.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int count = 200000;
    int *send_buf = (int *)malloc(count * sizeof(int));
    int *recv_buf = (int *)malloc(count * sizeof(int));

    if (rank == 0) {
        printf("[Rank 0] Sending to 1, waiting for 2...\n");
        MPI_Send(send_buf, count, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(recv_buf, count, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (rank == 1) {
        printf("[Rank 1] Sending to 2, waiting for 0...\n");
        MPI_Send(send_buf, count, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Recv(recv_buf, count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (rank == 2) {
        printf("[Rank 2] Sending to 0, waiting for 1...\n");
        MPI_Send(send_buf, count, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(recv_buf, count, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    free(send_buf);
    free(recv_buf);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
