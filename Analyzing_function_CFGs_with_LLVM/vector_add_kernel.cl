__kernel void vector_add(__global int *A, __global int *B, __global int *C, int n) {

    int i = get_global_id(0);
    if (i < n)
        C[i] = A[i] + B[i];
}

