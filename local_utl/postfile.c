#include "bbs.h"
#include "ythtbbs.h"
#include <stdio.h>
 
 static int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
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

int main(int argc, char **argv)
{
	//postfile file user boardname title
	char title[128];
	if (argc != 4 && argc!=5) {
	  printf("usage: ./postfile file author board title [codechange]\n");
	  printf("which means post \"file\" to \"board\", with the \"title\" and \"author\"\n");
	  return -1;
	  return 1;
     }
	if (!strcmp(argv[5], "1")) {
		char cmd[256];
		sprintf(cmd, "iconv -f utf8 it gbk %s -o %s", argv[1], argv[1]);
		system(cmd);
		u2g(argv[4], strlen(argv[4]), cmd, 256);
	}
	int ret=postfile(argv[1], argv[2], argv[3], argv[4]);
	return 0;
}
