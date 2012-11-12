// logger.c
// ����: interma@BMY
// ����ʱ�䣺2006-04-06
// ���ܣ������е�log��¼����

//�޸�: clearboy@BMY
//��Ϊ���ܴ���һЩ�ڴ�����⣬���˲����޸�

#include "bbs.h"
#include <math.h>
#define MAIL_ID "program" //������ʼ���������id�������id������ӳ�䵽����

int alter = 0;
extern int mail_buf(char *buf, char *userid, char *title); //���������mail.c�У���userid����

static const char *header[] = {
	"����",
	"����ﶨ",
	"�������"
};

// ��buf�е����ݽ���log
void logbuf(char *buf, char *title, int headerindex)
{
	// headerindexע�ⲻҪԽ��
	// buf��Ҫ̫����Ϊinnerbufֻ��2048
	// title��Ҫ̫����Ϊinnertitleֻ��64
	char innerbuf[2048];
	char innertitle[64];
	
	if (headerindex < 0 || headerindex > sizeof(header)/sizeof(char *) - 1 )
		return;

	if (alter)
	{
		sprintf(innertitle, "{%s} %s", header[headerindex], title);
	}
	else
	{
		sprintf(innertitle, "<%s> %s", header[headerindex], title);
	}
	
	alter = abs(alter -1); // Ϊ�˼���Ķ�����

	strncpy(innerbuf, "<�˷��ż�����logger�������>\n\n", 34);
	strncat(innerbuf, buf, 990);
	mail_buf(innerbuf, MAIL_ID, innertitle);
}
