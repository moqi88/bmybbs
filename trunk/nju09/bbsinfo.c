#include "bbslib.h"

int
bbsinfo_main()
{	//modify by mintbaggio 20040829 for new www
	int type;
	html_header(1);
	check_msg();
	printf("<body>");
	if (!loginok || isguest)
		http_fatal("����δ��¼");
	changemode(EDITUFILE);
	type = atoi(getparm("type"));
	printf("<div class=rhead>%s -- �û���������</div><hr>\n", BBSNAME);
	if (type != 0) {
		check_info();
		http_quit();
	}
	printf("<form action=bbsinfo?type=1 method=post>");
	printf("�����ʺ�: %s<br>\n", currentuser.userid);
	printf
	    ("�����ǳ�: <input type=text name=nick value='%s' size=24 maxlength=30><br>\n",
	     currentuser.username);
	printf("�������: %d ƪ<br>\n", currentuser.numposts);
//      printf("�ż�����: %d ��<br>\n", currentuser.nummails);
	printf("��վ����: %d ��<br>\n", currentuser.numlogins);
	printf("��վʱ��: %ld ����<br>\n", currentuser.stay / 60);
	printf
	    ("��ʵ����: <input type=text name=realname value='%s' size=16 maxlength=16><br>\n",
	     currentuser.realname);
	printf
	    ("��ס��ַ: <input type=text name=address value='%s' size=40 maxlength=40><br>\n",
	     currentuser.address);
	printf("�ʺŽ���: %s<br>", Ctime(currentuser.firstlogin));
	printf("�������: %s<br>", Ctime(currentuser.lastlogin));
	printf("��Դ��ַ: %s<br>", currentuser.lasthost);
#ifndef POP_CHECK	
	printf
	    ("�����ʼ�: <input type=text name=email value='%s' size=32 maxlength=32><br>\n",
	     currentuser.email);
#endif

#if 0
	printf
	    ("��������: <input type=text name=year value=%d size=4 maxlength=4>��",
	     currentuser.birthyear + 1900);
	printf("<input type=text name=month value=%d size=2 maxlength=2>��",
	       currentuser.birthmonth);
	printf("<input type=text name=day value=%d size=2 maxlength=2>��<br>\n",
	       currentuser.birthday);
	printf("�û��Ա�: ");
	printf("��<input type=radio value=M name=gender %s>",
	       currentuser.gender == 'M' ? "checked" : "");
	printf("Ů<input type=radio value=F name=gender %s><br>",
	       currentuser.gender == 'F' ? "checked" : "");
#endif
	printf
	    ("<input type=submit value=ȷ��> <input type=reset value=��ԭ>\n");
	printf("</form>");
	printf("<hr>");
	printf("</body></html>");
	return 0;
}

int
check_info()
{
	int m;
	char buf[256];
	strsncpy(buf, getparm("nick"), 30);
	for (m = 0; m < strlen(buf); m++)
		if ((buf[m] < 32 && buf[m] > 0) || buf[m] == -1)
			buf[m] = ' ';
	if (strlen(buf) > 1) {
		strcpy(currentuser.username, buf);
	} else {
		printf("����: �ǳ�̫��!<br>\n");
	}
	strsncpy(buf, getparm("realname"), 9);
	if (strlen(buf) > 1) {
		strcpy(currentuser.realname, buf);
	} else {
		printf("����: ��ʵ����̫��!<br>\n");
	}
	strsncpy(buf, getparm("address"), 40);
	if (strlen(buf) > 8) {
		strcpy(currentuser.address, buf);
	} else {
		printf("����: ��ס��ַ̫��!<br>\n");
	}
	/*
	strsncpy(buf, getparm("email"), 32);
	if (strlen(buf) > 8 && strchr(buf, '@')) {
		strcpy(currentuser.email, buf);
	} else {
		printf("����: email��ַ���Ϸ�!<br>\n");
	}*/
#if 0
	strsncpy(buf, getparm("year"), 5);
	if (atoi(buf) > 1910 && atoi(buf) < 1998) {
		currentuser.birthyear = atoi(buf) - 1900;
	} else {
		printf("����: ����ĳ������!<br>\n");
	}
	strsncpy(buf, getparm("month"), 3);
	if (atoi(buf) > 0 && atoi(buf) <= 12) {
		currentuser.birthmonth = atoi(buf);
	} else {
		printf("����: ����ĳ����·�!<br>\n");
	}
	strsncpy(buf, getparm("day"), 3);
	if (atoi(buf) > 0 && atoi(buf) <= 31) {
		currentuser.birthday = atoi(buf);
	} else {
		printf("����: ����ĳ�������!<br>\n");
	}
	strsncpy(buf, getparm("gender"), 2);
	if (!strcasecmp(buf, "F"))
		currentuser.gender = 'F';
	if (!strcasecmp(buf, "M"))
		currentuser.gender = 'M';
#endif
	save_user_data(&currentuser);
	printf("[%s] ���������޸ĳɹ�.", currentuser.userid);
	return 0;
}
