#include <stdio.h>

int main(int argc, char const *argv[])
{
	int nb;
	while ( read(0, &nb, sizeof(int))){
		printf("%d", nb);
	}
	printf("\n");
	return 0;
}