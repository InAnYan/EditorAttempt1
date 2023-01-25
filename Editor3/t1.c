#include <stdio.h>

int main()
{
	unsigned a = 'a';
	printf("int a = %c\n", a);

	unsigned b = 'a' & 0x1F;
	printf("int b = %c\n", b);

	unsigned c = a - b;
	printf("int c = %d\n", c);

	unsigned d = b + 96;
	printf("int d = %c\n", d);

	return 0;
}
