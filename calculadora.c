#include <stdio.h>
#include "calc.h"

int main(){
	int a, b;
	a = 9;
	b = 3;

	printf("soma: %d\n", soma(a,b));
	printf("subt: %d\n", subt(a,b));
	printf("mult: %d\n", mult(a,b));
	printf("divi: %d\n", divi(a,b));

	return 0;
}

