#include <stdio.h>
int process_ytml(char *filename);
int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("����������!\n");
		return -1;
	}
	process_ytml(argv[1]);
	return 0;
}
