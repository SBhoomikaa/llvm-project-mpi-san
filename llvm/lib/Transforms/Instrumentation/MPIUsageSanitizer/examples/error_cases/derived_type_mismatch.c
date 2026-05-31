/**
 * @file derived_type_mismatch.c
 * @brief Seeded Error Test: Type Mismatch with Custom Derived Datatype
 * 
 * This test simulates a type mismatch involving a custom committed MPI_Datatype.
 * Rank 0 defines and sends a structure consisting of an integer and a double
 * (custom derived datatype), but Rank 1 expects to receive it as a simple double.
 * The MPI Usage Sanitizer is expected to detect this signature mismatch.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    int id;
    double value;
} my_struct_t;

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

    // Define MPI Derived Datatype for my_struct_t
    MPI_Datatype struct_type;
    int blocklengths[2] = {1, 1};
    MPI_Aint displacements[2];
    MPI_Datatype types[2] = {MPI_INT, MPI_DOUBLE};

    displacements[0] = offsetof(my_struct_t, id);
    displacements[1] = offsetof(my_struct_t, value);

    MPI_Type_create_struct(2, blocklengths, displacements, types, &struct_type);
    MPI_Type_commit(&struct_type);

    if (rank == 0) {
        my_struct_t data = {101, 99.88};
        // Rank 0 sends 1 custom struct_type to Rank 1
        MPI_Send(&data, 1, struct_type, 1, 0, MPI_COMM_WORLD);
        printf("[Rank 0] Sent derived struct type\n");
    } else if (rank == 1) {
        double val = 0.0;
        // Rank 1 receives it as a standard MPI_DOUBLE (Derived Type Mismatch!)
        MPI_Recv(&val, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[Rank 1] Received double val: %f\n", val);
    }

    MPI_Type_free(&struct_type);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
