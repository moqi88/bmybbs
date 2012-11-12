#include "bbslib.h"
#include "mulan.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char *girls[] = {
	"��ӯӨ", "����", "���", "��ӱ", "��ٻٻ", "������",
	"��ԪԪ", "����", "����", "�⽡", "����ٻ", "�콣"
};
int voted[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
int
bbsdmulan_main()
{
	struct mulan t;
	char fromhost[33];
	unsigned int ip, i;
	html_header(1);
	http_quit();
	t.ip = inet_addr(getsenv("REMOTE_ADDR"));
	strsncpy(fromhost, getsenv("REMOTE_ADDR"), 32);
	t.select = atoi(getparm("select")) - 1;
	t.time = now_t;
	printf("<center>Ů���Ļ���ͶƱ [������: %s]<hr>\n", fromhost);
	printf("<table><tr><td>");
	do_mulan(&t);
	printf("</td></table>");
	printf("<TABLE cellSpacing=0 cellPadding=0 bgColor=#c2bed6><tr><td>");
	printf("ĿǰͶƱ���:");
	printf("</td></tr>");
	for (i = 0; i < 12; i++) {
		printf("<tr><td>");
		printf(" %s : %d", girls[i], voted[i]);
		printf("</tr></td>");
	}
	printf("</td></table>");
	printf
	    ("<br><a href=bbssec target=f3>���YTHT</a>  <a href=javascript:history.go(-1)>���ҿ�������Ů</a>");
	http_quit();
}

int
do_mulan(struct mulan *t)
{
	struct mulan tmp;
	time_t temptime = 0;
	FILE *fp;
	fp = fopen("mulan", "r+");
	if (fp == 0)
		http_fatal("����Ĳ���");
	if (t->select < 0 || t->select > 12)
		http_fatal("����Ĳ���");
	flock(fileno(fp), LOCK_EX);
	while (fread(&tmp, 1, sizeof (struct mulan), fp) ==
	       sizeof (struct mulan)) {
		if (tmp.ip == t->ip) {
			if (tmp.time > temptime)
				temptime = tmp.time;
		}
		voted[tmp.select]++;
	}
	if (t->time - temptime < 24 * 60 * 60) {
		printf
		    ("<font color=FF0000>�Բ���ͬһ̨����ÿ24Сʱֻ��Ͷ1Ʊ</font>");
		fclose(fp);
		return 0;
	}
	printf("����ѡ����:%s", girls[t->select]);
	fwrite(t, sizeof (struct mulan), 1, fp);
	voted[t->select]++;
	fclose(fp);
	return 1;
}
