#include "bbs.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	//postfile file boardname title
	
	int ret=postfile(argv[1], argv[2], argv[3], 1);
	return 0;
}
