#include <sys/ipc.h>
#include <sys/msg.h>
#include "ythtbbs.h"
#include <time.h>
#include <stdlib.h>
#include <iconv.h>
int pu = 0;

void
getrandomint(unsigned int *s)
{
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, 4);
	close(fd);
#else
	srandom(getpid() - 19751016);
	*s=random();
#endif
}

void
getrandomstr(unsigned char *s)
{
	int i;
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, 30);
	close(fd);
	for (i = 0; i < 30; i++)
		s[i] = 65 + s[i] % 26;
#else
	time_t now_t;
	now_t = time(NULL);
	srandom(now_t - 19751016);
	for (i = 0; i < 30; i++)
		s[i] = 65 + random() % 26;
#endif
	s[30] = 0;
}

int
init_newtracelogmsq()
{
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(BBSLOG_MSQKEY, IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &buf);
	buf.msg_qbytes = 50 * 1024;
	msgctl(msqid, IPC_SET, &buf);
	return msqid;
}

void
newtrace(s)
char *s;
{
	static int disable = 0;
	static int msqid = -1;
	time_t dtime;
	char buf[512];
	char timestr[16];
	char *ptr;
	struct tm *n;
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;
	if (disable)
		return;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(timestr, "%02d:%02d:%02d", n->tm_hour, n->tm_min, n->tm_sec);
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype),
		 "%s %s\n", timestr, s);
	ptr = msg->mtext;
	while ((ptr = strchr(ptr, '\n'))) {
		if (!ptr[1])
			break;
		*ptr = '*';
	}
	msg->mtype = 1;
	if (msqid < 0) {
		msqid = init_newtracelogmsq();
		if (msqid < 0) {
			disable = 1;
			return;
		}
	}
	msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR);
	return;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
	iconv_close(cd);
	return 0;
}

//UNICODE码转为GB2312码
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312码转为UNICODE码
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}
