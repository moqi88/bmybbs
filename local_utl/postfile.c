#include "bbs.h"
#include "ythtbbs.h"
#include <stdio.h>
 
 


int main(int argc, char **argv)
{
	//postfile file user boardname title
	
	int ret=postfile(argv[1], argv[2], argv[3], argv[4]);
	return 0;
}
