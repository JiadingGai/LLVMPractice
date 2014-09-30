__kernel void test(__global int *A, __global int *B, __global int *C, int n) {
  int foo1;
  int foo2;
  int foo3;
  int foo4;
  C[0] = A[0] + B[0];
  int k;
  k = 0;

  do {
    int i = get_global_id(0);
    if(n > foo2) {
      C[i] = 2 * C[i];

      if(i > foo3)
        C[i] = A[i] + B[i];
      else
        C[i] = A[i] - B[i];
    } else {
        do {
            C[k] = A[k] + B[k];
            if (i < k)
                k = 2 * i;
            else
                k = 2 * k;
        } while (k < foo4); 
    }
    k = k + 2;
  } while(n < foo1);
}

