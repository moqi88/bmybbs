#include "bbslib.h"
FILE *fp;

int
bbsplan_main()
{	//modify by mintbaggio 20040829 for new www
	FILE *fp;
	char *ptr, plan[256], buf[10000];
	int size;
	html_header(1);
	check_msg();
	printf("<body><center>\n");
	if (!loginok || isguest)
		http_fatal("�Ҵҹ��Ͳ�������˵���������ȵ�¼");
	changemode(EDITUFILE);
	sethomefile(plan, currentuser.userid, "plans");
	if (!strcasecmp(getparm("type"), "update"))
		save_plan(plan);
	printf("<div class=rhead>%s -- ���ø���˵���� [<span class=h11>%s</span>]</div><hr>\n", BBSNAME, currentuser.userid);
	printf("<form name=form1 method=post action=bbsplan?type=update>\n");
	fp = fopen(plan, "r");
	if (fp) {
		size = fread(buf, 1, 9999, fp);
		buf[size] = 0;
		ptr = strcasestr(buf, "<textarea>");
		if (ptr)
			*ptr = 0;
		fclose(fp);
	}
	printf("<table border=1><tr><td>");
	printf("<textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=80 wrap=virtual>\n");
	printf("%s", void1(buf));
	printf("</textarea></table>\n");
	printf("<input type=submit value=����> ");
	printf("<input type=reset value=��ԭ>\n");
	printf("<hr></body></html>\n");
	http_quit();
	return 0;
}

int
save_plan(char *plan)
{
	char buf[10000];
	fp = fopen(plan, "w");
	strsncpy(buf, getparm("text"), 9999);
	buf[9999] = 0;
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("����˵�����޸ĳɹ���");
	http_quit();
	return 0;
}
