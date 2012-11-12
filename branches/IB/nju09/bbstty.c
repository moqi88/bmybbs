#include "bbslib.h"

int
bbstty_main()
{
	char *cmd;
	char buf[256];
	char s1[256] = "", s2[256] = "", s3[356] = "";
	int r;
	html_header(1);
	cmd = getparm("cmd");
	r = sscanf(cmd, "%80s %80s %80s", s1, s2, s3);
	if (!strcasecmp(s1, "/q")) {
		sprintf(buf, "bbsqry?userid=%s", s2);
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/s")) {
		sprintf(buf, "bbssendmsg?destid=%s&msg=%s", s2, s3);
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/m")) {
		sprintf(buf, "bbspstmail?userid=%s&title=%s", s2, s3);
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/logout")) {
		printf("<script>top.location='bbslogout'</script>");
		http_quit();
	}
	if (!strcasecmp(s1, "/top10")) {
		sprintf(buf, "bbstop10");
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/g")) {
		sprintf(buf, "bbssel?board=%s", s2);
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/l")) {
		sprintf(buf, "bbsusr");
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/f")) {
		sprintf(buf, "bbsfind");
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/z")) {
		if (s2[0] == 0) {
			sprintf(buf, "bbsdict");
		} else {
			sprintf(buf, "bbsdict?type=1&word=%s", s2);
		}
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/i")) {
		if (s2[0] == 0) {
			sprintf(buf, "bbsfreeip");
		} else {
			sprintf(buf, "bbsfreeip?ip=%s&type=1", s2);
		}
		redirect(buf);
		http_quit();
	}
	if (!strcasecmp(s1, "/h")) {
		printf("<pre>\n");
		printf("��������̨����<hr>\n");
		printf("/q [userid]           ��ѯ����\n");
		printf("/s [userid] [msg]     ����ѶϢ\n");
		printf("/m [userid] [title]   �����ż�\n");
		printf("/logout               ע����¼\n");
		printf("/top10                �鿴10��\n");
		printf("/g [board]            ��ת������\n");
		printf("/l                    �����ķ�\n");
		printf("/f                    ��ѯ����\n");
		printf("/z [word]             ����Ӣ���ֵ�\n");
		printf("/i [ip]               IP��ַ��ѯ\n");
		printf("/h                    ��ʾ����������\n");
		printf("http://xx.xx.xx       ��ת��url��ַ\n\n");
		printf("</pre>\n");
		http_quit();
	}
	if (!strncasecmp(s1, "http://", 7)) {
		sprintf(buf,
			"<script>window.open('%s','','');history.go(-1)</script>",
			nohtml(s1));
		printf("%s", buf);
		http_quit();
	}
	http_fatal("δ֪����");
}
