#include "bbslib.h"


int
bbsdefcss_main()
{
	char *ptr, buf[256];
	int type;
	html_header(1);
	check_msg();
	printf("<body>");
	if (!loginok || isguest)
		http_fatal("�Ҵҹ��Ͳ��ܶ��ƽ���");
	changemode(GMENU);
	type = atoi(getparm("type"));
	if (type > 0){
		sethomefile(buf, currentuser.userid, "ubbs.css");
		ptr=getparm("ucss");
		f_write(buf,ptr);
/*		sethomefile(buf, currentuser.userid, "uleft.css");
		ptr=getparm("uleftcss");
		f_write(buf,ptr);*/
		printf("WWW�û�������ʽ�趨�ɹ�.<br>\n");
		printf("[<a href='javascript:history.go(-2)'>����</a>]");
		return 0;
	}
	printf("<table align=center><form action=bbsdefcss method=post>\n");
	printf("<tr><td>\n");
	printf("<input type=hidden name=type value=1>");
	printf("<span class=c31> ����CSSģʽ�����û��Զ���WWW�ĸ�����ʾЧ����<br>ע�⣬���������ڵ�ǰ�Ľ���[%s]�������CSS��<br>ȷ���󽫸���ԭ�е������Զ���CSS��</span><br><br>\n",currstyle->name);
	printf
	     ("��ǰ��CSS�������£�<br><textarea name=ucss rows=20 cols=76>");
	if(strstr(currstyle->cssfile,"ubbs.css"))
		sethomefile(buf, currentuser.userid, "ubbs.css");
	else
		sprintf(buf,HTMPATH "%s",currstyle->cssfile);
	showfile(buf);
	printf("</textarea><br>");
/*	printf
	     ("</textarea><br><br>��ǰ�����ѡ����CSS��������(body.foot�ǵ��е�css)��<br><textarea name=uleftcss rows=5 cols=76>");
	if(strstr(currstyle->leftcssfile,"uleft.css"))
		sethomefile(buf, currentuser.userid, "uleft.css");
	else
		sprintf(buf,HTMPATH "%s", currstyle->leftcssfile);
	showfile(buf);
	printf("</textarea><br><br>");*/
	printf
	    ("</textarea></td></tr><tr><td><input type=submit class=resetlong value=ȷ��>"
	    " <input type=reset class=sumbitlong value=��ԭ>\n");
	printf("</td></tr></form></table></body></html>\n");
	return 0;
}

