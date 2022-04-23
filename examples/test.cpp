#include <bits/stdc++.h>
using namespace std;

void bar() {
  int x = 1+1;
}

void foo(int){
  int x = 3+3;
  bar();
}

int main(int argc, char** argv) {
  int i = 1;
  int max = 0;
  while (i < argc) {
    int aux = atoi(argv[i]);
    i++;
    if (aux > max) {
      max = aux;
    }
  }
  foo();
  bar();
  printf("Max = %d\n", max);
}