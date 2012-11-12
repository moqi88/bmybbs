#include "bbslib.h"

static int show_form(char *board);
static int inform(char *board, char *user, char *exp, int dt);

int
bbsdenyadd_main()
{
	int i;
	char buf[256], exp[80], board[80], *userid;
	int dt;
	struct userec *x;
	struct boardmem *x1;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("����δ��¼, ���ȵ�¼");
	changemode(READING);
	strsncpy(board, getparm("board"), 30);
	strsncpy(exp, getparm("exp"), 30);
	dt = atoi(getparm("dt"));
	if (!(x1 = getboard(board)))
		http_fatal("�����������");
	if (!has_BM_perm(&currentuser, x1))
		http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
	userid = getparm("userid");
	if (userid[0] == 0)
		return show_form(board);
	if ((x = getuser(userid)) == 0)
		http_fatal("�����ʹ�����ʺ�");
	if (!has_post_perm(x, x1))
		http_fatal("����˱�����û��postȨ");
	strcpy(userid, x->userid);
	if (!(currentuser.userlevel & PERM_SYSOP) && (dt > 20))
		http_fatal("���ʱ�����20��,������Ȩ��,����Ҫ,����ϵվ��");
	if (dt < 1 || dt > 99)
		http_fatal("�����뱻������(1-99)");
	if (exp[0] == 0)
		http_fatal("���������ԭ��");
	for (i = 0; i < denynum; i++)
		if (!strcasecmp(denyuser[i].id, userid))
			http_fatal("���û��Ѿ�����");
	if (denynum > 40)
		http_fatal("̫���˱�����");
	strsncpy(denyuser[denynum].id, userid, 13);
	strsncpy(denyuser[denynum].exp, exp, 30);
	denyuser[denynum].free_time = now_t + dt * 86400;
	denynum++;
	savedenyuser(board);
	printf("��� %s �ɹ�<br>\n", userid);
	snprintf(buf, 256, "%s deny %s %s", currentuser.userid, board, userid);
	newtrace(buf);
	inform(board, userid, exp, dt);
	printf("[<a href=bbsdenyall?board=%s>���ر����ʺ�����</a>]", board);
	http_quit();
	return 0;
}

static int
show_form(char *board)
{
	printf("<center>%s -- ������� [������: %s]<hr>\n", BBSNAME, board);
	printf
	    ("<form action=bbsdenyadd><input type=hidden name=board value='%s'>",
	     board);
	printf
	    ("���ʹ����<input name=userid size=12> ����POSTȨ <input name=dt size=2> ��, ԭ��<input name=exp size=20>\n");
	printf("<input type=submit value=ȷ��></form>");
	return 0;
}

static int
inform(char *board, char *user, char *exp, int dt)
{
	FILE *fp;
	char path[80], title[80], buf[256];
	struct tm *tmtime;
	time_t daytime = now_t + dt * 24 * 60 * 60;//by bjgyt
	tmtime = gmtime(&daytime);
	sprintf(title, "%s��%sȡ����%s��POSTȨ��", user,
		currentuser.userid, board);
	sprintf(path, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(path, "w");
	fprintf(fp, "����ƪ���������Զ�����ϵͳ��������\n\n");
	snprintf(buf, sizeof (buf),
		 "����ԭ��: %s\n"
		 "��������: %d\n"
		 "�������: %d��%d��\n"
		 "�������飬��������������Appeal��Ͷ��\n",
		 exp, dt, tmtime->tm_mon + 1, tmtime->tm_mday);
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
