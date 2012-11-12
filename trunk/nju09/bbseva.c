#include "bbslib.h"
#ifdef ENABLE_MYSQL
int
bbseva_main()
{
	char board[80], file[80];
	int star;
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("F"), 20);
	if (!file[0])
		strsncpy(file, getparm("file"), 32);
	star = atoi(getparm("star"));
	if (!loginok)
		http_fatal("�Ҵҹ��Ͳ��ܽ��б������");
	changemode(READING);
	if(! getboard(board))
		http_fatal("�����������");
	if (hideboard(board))
		http_fatal("���ذ���Ͳ�Ҫ����������!");
	if (star < 1 || star > 5)
		http_fatal("����Ĳ���");
	if (star == 1)
		star++;
	printf("<center>%s -- �������� [ʹ����: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	printf("<table><td>");
	do_eva(board, file, star);
	printf("</td></table>");
	printf("[<a href='javascript:history.go(-1)'>����</a>]");
	http_quit();
	return 0;
}

char *des[6] = { "û����", "����", "����", "�ܺ�", "�ܺ�", "ǿ���Ƽ�!" };

int
set_eva(char *board, char *file, int star, int result[2], char *buf)
{
	int count, starchanged, oldstar;
	float avg;
	starchanged = 0;
	if (now_t - atoi(file + 2) > 3 * 86400) {
		sprintf(buf, "��ô�ϵ����¾ͱ������˰�,ȥ���������°�!");
		return 0;
	}
	//oldstar = 0/1����Ϊ��δ����״̬
	if (bbseva_qset
	    (utmpent, board, file, currentuser.userid, star, &oldstar, &count,
	     &avg) < 0) {
		sprintf(buf, "�����µ�����δ�ܸı�");
		return 0;
	}
	if (star == oldstar) {
		sprintf(buf, "��û�иı�������ƪ���µ�����");
	} else {
		if (oldstar != 1 && oldstar != 0)
			sprintf(buf, "����������ƪ���µ����۴� %s �ĵ� %s",
				des[oldstar], des[star]);
		else
			sprintf(buf, "��ƪ���±�������Ϊ %s", des[star]);
	}
	if (oldstar != star) {
		starchanged = 1;
		result[0] = (int) (50 * avg);
		result[1] = count > 255 ? 255 : count;
	}
	return starchanged;
}

int
do_eva(char *board, char *file, int star)
{
	FILE *fp;
	char dir[256];
	struct fileheader f;
	int result[2], filetime;
	filetime = atoi(file + 2);
	sprintf(dir, "boards/%s/.DIR", board);
	fp = fopen(dir, "r+");
	if (fp == 0)
		http_fatal("����Ĳ���");
	flock(fileno(fp), LOCK_EX);
	while (1) {
		if (fread(&f, sizeof (struct fileheader), 1, fp) <= 0)
			break;
		if (f.filetime == filetime) {
			if (set_eva(board, file, star, result, dir)) {
				f.staravg50 = result[0];
				f.hasvoted = result[1];
				fseek(fp, -1 * sizeof (struct fileheader),
				      SEEK_CUR);
				fwrite(&f, sizeof (struct fileheader), 1, fp);
			}
			fclose(fp);
			printf("%s", dir);
			return 0;
		}
	}
	fclose(fp);
	printf("<td><td><td>%s<td>�ļ�������.\n", file);
	return -1;
}
#else
int
bbseva_main()
{
	html_header(1);
	http_fatal("�밲װMySQL֧�ֻ���!");
	return 0;
}
#endif
