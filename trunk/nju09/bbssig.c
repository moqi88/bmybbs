#include "bbslib.h"
FILE *fp;

int
bbssig_main()
{	//modify by mintbaggio 20040829 for new www
	FILE *fp;
	char path[256], buf[512];
	html_header(1);
	if (!loginok || isguest)
		http_fatal("�Ҵҹ��Ͳ�������ǩ���������ȵ�¼");
	changemode(EDITUFILE);
	printf("<body><center><div class=rhead>%s -- ����ǩ���� [ʹ����: <span class=h11>%s</span>]</div><hr>\n",
	       BBSNAME, currentuser.userid);
	sprintf(path, "home/%c/%s/signatures",
		mytoupper(currentuser.userid[0]), currentuser.userid);
	if (!strcasecmp(getparm("type"), "1"))
		save_sig(path);
	printf("<form name=form1 method=post action=bbssig?type=1>\n");
	printf("ǩ����ÿ6��Ϊһ����λ, �����ö��ǩ����.<br>"
	       "(<a href=bbscon?B=Announce&F=M.1047666649.A>"
	       "[��ʱ����]����ͼƬǩ�����Ĵ�С����</a>)<br>");
	printf("<textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=80>\n");
	fp = fopen(path, "r");
	if (fp) {
		while (fgets(buf, sizeof (buf), fp))
			printf("%s", nohtml(void1(buf)));
		fclose(fp);
	}
	printf("</textarea><br>\n");
	printf("<input type=submit value=����> ");
	printf("<input type=reset value=��ԭ>\n");
	printf("</form><hr></body>\n");
	http_quit();
	return 0;
}

void
save_sig(char *path)
{
	char *buf;
	fp = fopen(path, "w");
	buf = getparm("text");
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("ǩ�����޸ĳɹ���");
	http_quit();
}
