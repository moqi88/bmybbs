#define MY_CSS "/bbs.css"
#include "bbslib.h"

int
bbsnewleft_main()
{
	int i;
	printf("Content-type: text/html; charset=%s\n\n\n", CHARSET);
	printf("<HTML><HEAD>\n");
	printf
	    ("<meta http-equiv='Content-Type' content='text/html; charset=%s'>\n",
	     CHARSET);
	changemode(MMENU);
	if (1) {
		char buf[200], *ptr;
		ptr = getsenv("HTTP_USER_AGENT");
		sprintf(buf, "%-14.14s %.100s", currentuser.userid, ptr);
		addtofile(MY_BBS_HOME "/browser.log", buf);
	}
	if (!loginok || isguest)
		printf
		    ("<script>function dtree(){\nparent.UserID='';\nparent.UserLevel='';\nparent.HasLogin=0;\n"
		     "parent.MyBRDList=new Array();\nparent.ShowReg=0;\nparent.ShowCloak=0;\n");
	else {
		char buf[STRLEN];
		if (HAS_PERM(PERM_LOGINOK) && !HAS_PERM(PERM_POST))
			printf("<script>alert('���������ȫվ�������µ�Ȩ��, "
			       "��ο�sysop�湫��, ��������sysop��������. "
			       "��������, ����Appeal���������.')</script>\n");
		if (count_new_mails() > 0)
			printf("<script>alert('�������ż�!')</script>\n");
		strcpy(buf, "δע���û�");
		if (currentuser.userlevel & PERM_LOGINOK)
			strcpy(buf, cexp(countexp(&currentuser)));
		if (currentuser.userlevel & PERM_BOARDS)
			strcpy(buf, "����");
		if (currentuser.userlevel & PERM_XEMPT)
			strcpy(buf, "�����ʺ�");
		if (currentuser.userlevel & PERM_SYSOP)
			strcpy(buf, "��վվ��");
		printf("<script>function dtree(){\n");
		printf
		    ("parent.UserID=\"%s\";parent.UserLevel=\"%s\";parent.HasLogin=1;",
		     currentuser.userid, buf);
		printmybrdlist();
		if (loginok && !isguest
		    && !(currentuser.userlevel & PERM_LOGINOK)
		    && !has_fill_form())printf("parent.ShowReg=1;");
		printf("parent.ShowCloak=%d;",
		       (currentuser.userlevel & PERM_CLOAK) ? 1 : 0);

	}
	printf("parent.GroupList=new Array(");
	for (i = 0; i < SECNUM; i++) {
		printf("'%s'", secname[i][0]);
		if (i < SECNUM - 1)
			printf(",");
	}
	printf(");");
	printf
	    ("parent.DoRefresh=1;parent.redrawTree(0);\n}\nsetTimeout('dtree()',20);</script></HEAD><BODY></BODY>");
	http_quit();
}

int
printmybrdlist()
{
	FILE *fp;
	char buf[256];
	char mybrd[32][80];
	int i, mybrdnum = 0;
	sprintf(buf, "home/%c/%s/.goodbrd", mytoupper(currentuser.userid[0]),
		currentuser.userid);
	fp = fopen(buf, "r");
	if (fp) {
		while (fgets(mybrd[mybrdnum], 80, fp) != NULL && mybrdnum < 20) {
			if (mybrd[mybrdnum][strlen(mybrd[mybrdnum]) - 1] ==
			    '\n') mybrd[mybrdnum][strlen(mybrd[mybrdnum]) - 1] =
				    0;
			mybrdnum++;
		}
		fclose(fp);
	}
	printf("parent.MyBRDList=new Array(");
	for (i = 0; i < mybrdnum; i++) {
		printf("'%s'", mybrd[i]);
		if (i < mybrdnum - 1)
			printf(",");
	}
	printf(");");
}
