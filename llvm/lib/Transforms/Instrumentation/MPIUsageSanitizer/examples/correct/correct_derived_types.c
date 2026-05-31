/**
 * @file correct_derived_types.c
 * @brief Correct custom derived types example (Zero False Positives Test)
 * 
 * This program demonstrates correct and standards-compliant usage of MPI custom derived datatypes
 * created using MPI_Type_create_struct, committed using MPI_Type_commit, sent/received cleanly
 * with identical matching derived type signatures, and then properly freed using MPI_Type_free.
 * The MPI Usage Sanitizer is expected to run this program without flagging any errors/warnings.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    int rank_id;
    double timestamp;
    float value[3];
} packet_t;

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

    // 1. Define custom derived type structure matching packet_t
    MPI_Datatype struct_type;
    int blocklengths[3] = {1, 1, 3};
    MPI_Aint displacements[3];
    MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_FLOAT};

    displacements[0] = offsetof(packet_t, rank_id);
    displacements[1] = offsetof(packet_t, timestamp);
    displacements[2] = offsetof(packet_t, value);

    // Create and commit type
    MPI_Type_create_struct(3, blocklengths, displacements, types, &struct_type);
    MPI_Type_commit(&struct_type);

    packet_t packet;

    if (rank == 0) {
        // Initialize packet
        packet.rank_id = rank;
        packet.timestamp = 123456.789;
        packet.value[0] = 1.1f;
        packet.value[1] = 2.2f;
        packet.value[2] = 3.3f;

        // Send 1 element of struct_type to Rank 1
        printf("[Rank 0] Sending custom packet...\n");
        MPI_Send(&packet, 1, struct_type, 1, 55, MPI_COMM_WORLD);
        printf("[Rank 0] Custom packet sent.\n");
    } else if (rank == 1) {
        // Initialize recv packet to zero
        packet.rank_id = -1;
        packet.timestamp = 0.0;
        packet.value[0] = 0.0f;
        packet.value[1] = 0.0f;
        packet.value[2] = 0.0f;

        // Receive 1 element of struct_type from Rank 0 (Matching derived type signature!)
        printf("[Rank 1] Receiving custom packet...\n");
        MPI_Recv(&packet, 1, struct_type, 0, 55, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Received packet: rank_id = %d, timestamp = %f, values = {%f, %f, %f}\n",
               packet.rank_id, packet.timestamp, packet.value[0], packet.value[1], packet.value[2]);
    }

    // Free the committed derived type
    MPI_Type_free(&struct_type);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
