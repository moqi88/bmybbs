#include "bbslib.h"
#include "identify.h"

int
bbsfindpass_main()
{
	html_header(1);

	printf("<body>");
	printf("<nobr><center>%s -- �һ��ʺŻ�����<hr>\n", BBSNAME);
	printf
	    ("<font color=green>���ý�������ʵ����֤���û������ڴ��������롣������������֤����.</font>");
	printf("<form method=post action=bbsresetpass>\n");
	printf("<table width=100%%>\n");
	printf
	    ("<tr><td align=right>*������ID  :<td align=left><input name=userid size=12 maxlength=12> (2-12�ַ�, ����ȫΪӢ����ĸ)\n");
	printf
	    ("<tr><td align=right>*������������:<td align=left><input type=password name=pass1 size=12 maxlength=12> (4-12�ַ�)\n");
	printf
	    ("<tr><td align=right>*��ȷ��������:<td align=left><input type=password name=pass2 size=12 maxlength=12>\n");

#ifdef POP_CHECK
	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // ע�⣺�������10��pop��������Ҫ��������ˣ�
	char ippop[10][256];
	printf("<tr><td align=right>*�������ε��ʼ��������б�:<td align=left><SELECT NAME=popserver>\n");
	int n = 1;
	while(n <= DOMAIN_COUNT)
	{
//		namepop[n - 1][strlen(namepop[n - 1]) - 1] = 0;
//		ippop[n - 1][strlen(ippop[n - 1]) - 1] = 0;
		
		char tempbuf[512];
		strncpy(tempbuf, MAIL_DOMAINS[n], 256);
		strcat(tempbuf, "+");
		strcat(tempbuf, IP_POP[n]);
		if (n == 1)
			printf("<OPTION VALUE=%s SELECTED>", tempbuf);
		else
			printf("<OPTION VALUE=%s>", tempbuf);

		printf("%s", MAIL_DOMAINS[n]);
		n++;

	}
	printf("</select>\n");
	printf
	    ("<tr><td align=right>*�����������û���:<td align=left><input name=user size=20 maxlength=20> \n");
	printf
	    ("<tr><td align=right>*��������������:<td align=left><input type=password name=pass size=20 maxlength=20> \n");
	
#endif	

	printf
	    ("<input type=submit value=ȷ����������>\n");
	printf("</form></center>");
	printf("</table><br><hr>\n");
	


	printf
	    ("<font color=green>���ý�������ʵ����֤���û�������������id�����ڴ˲�ѯ������������֤��id.</font>");
	printf("<form method=post action=bbsfindacc>\n");
	printf("<table width=100%%>\n");
	
#ifdef POP_CHECK
//	char bufpop[256];
//	int numpop = 0;
//	char namepop[10][256]; // ע�⣺�������10��pop��������Ҫ��������ˣ�
//	char ippop[10][256];

	printf("<tr><td align=right>*�������ε��ʼ��������б�:<td align=left><SELECT NAME=popserver1>\n");
	n = 1;
	while(n <= DOMAIN_COUNT)
	{
//		namepop[n - 1][strlen(namepop[n - 1]) - 1] = 0;
//		ippop[n - 1][strlen(ippop[n - 1]) - 1] = 0;
		
		char tempbuf[512];
		strncpy(tempbuf, MAIL_DOMAINS[n], 256);
		strcat(tempbuf, "+");
		strcat(tempbuf, IP_POP[n]);
		if (n == 1)
			printf("<OPTION VALUE=%s SELECTED>", tempbuf);
		else
			printf("<OPTION VALUE=%s>", tempbuf);

		printf("%s", MAIL_DOMAINS[n]);
		n++;

	}
	printf("</select>\n");
	printf
	    ("<tr><td align=right>*�����������û���:<td align=left><input name=user1 size=20 maxlength=20> \n");
	printf
	    ("<tr><td align=right>*��������������:<td align=left><input type=password name=pass1 size=20 maxlength=20> \n");

	
#endif	

	printf
	    ("<input type=submit value=ȷ�ϲ�ѯ>\n");
	printf("</form></center>");
	printf("</table><br><hr>\n");

	
	http_quit();
	return 0;
}

