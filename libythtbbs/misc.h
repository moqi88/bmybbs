/* misc.c */
#ifndef __MISC_H
#define __MISC_H
extern int pu;
void getrandomint(unsigned int *s);
void getrandomstr(unsigned char *s);
struct mymsgbuf {
	long int mtype;
	char mtext[1];
};
void newtrace(char *s);
int init_newtracelogmsq();
#endif
