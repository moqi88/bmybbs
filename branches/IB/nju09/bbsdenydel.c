#include "bbslib.h"

static int inform(char *board, char *user);

int
bbsdenydel_main()
{
	int i;
	char board[80], *userid;
	struct boardmem *x;
	html_header(1);
	check_msg();
	if (!loginok)
		http_fatal("����δ��¼, ���ȵ�¼");
	changemode(READING);
	strsncpy(board, getparm("board"), 30);
	if (!(x = getboard(board)))
		http_fatal("�����������");
	if (!has_BM_perm(&currentuser, x))
		http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
	userid = getparm("userid");
	for (i = 0; i < denynum; i++) {
		if (!strcasecmp(denyuser[i].id, userid)) {
			denyuser[i].id[0] = 0;
			savedenyuser(board);
			printf("�Ѿ��� %s ���. <br>\n", userid);
			inform(board, userid);
			printf("[<a href=bbsdenyall?board=%s>���ر�������</a>]",
			       board);
			http_quit();
		}
	}
	http_fatal("����û����ڱ���������");
	http_quit();
	return 0;
}

static int
inform(char *board, char *user)
{
	FILE *fp;
	char path[80], title[80], buf[256];
	sprintf(title, "�ָ� %s �� %s ��POSTȨ", user, board);
	sprintf(path, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(path, "w");
	fprintf(fp, "����ƪ���������Զ�����ϵͳ��������\n\n");
	snprintf(buf, sizeof (buf), "%s �ָ��� %s %s��POSTȨ.\n"
		 "�������������,лл!\n", currentuser.userid, user, board);
	fputs(buf, fp);
	fclose(fp);
	securityreport(title, buf);
	post_article(board, title, path, "XJTU-XANET", "�Զ�����ϵͳ",
		     "�Զ�����ϵͳ", -1, 4, 0, "XJTU-XANET", -1);//8 bit 0->4 bjgyt
	post_mail(user, title, path, currentuser.userid, currentuser.username,
		  fromhost, -1, 0);
	unlink(path);
	printf("ϵͳ�Ѿ�����֪ͨ��%s.<br>\n", user);
	return 0;
}
