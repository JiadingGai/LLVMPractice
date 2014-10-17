struct RT {
  char A;
  int B[10][20];
  char c;
};

struct ST {
  int X;
  double Y;
  struct RT Z;
};

extern char bar();
int test() {
  struct ST s;
  struct RT r[16];

  int x[11];
  x[6] = bar();

  int *P = &x[7];
  P = P + 1;

  return /*s.Z.B[5][13] + r[10].c + x[6] +*/ *P;
}

