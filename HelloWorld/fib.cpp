int fib(int x)
{
  if (x < 3)
    return x;
  else
    return fib(x-1) + fib(x-2);
}

int main()
{
  return fib(10);
}
