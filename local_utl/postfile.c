#include "bbs.h"
#include "ythtbbs.h"
#include <stdio.h>
 
 


int main(int argc, char **argv)
{
	//postfile file user boardname title
	if (argc != 4 && argc!=5) {
	  printf("usage: ./postfile file author board title\n");
	  printf("which means post \"file\" to \"board\", with the \"title\" and \"author\"\n");
	  return -1;
	  return 1;
     }
	
	int ret=postfile(argv[1], argv[2], argv[3], argv[4]);
	return 0;
}
