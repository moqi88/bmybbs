#include "bbslib.h"
#include "identify.h"

int
bbsreg_main()
{
	html_header(1);

#ifdef POP_CHECK
	struct stat temp;
	/*
	if (stat(MY_BBS_HOME "/etc/pop_register/pop_list", &temp) == -1)
	{
		http_fatal("Ŀǰû�п������ε��ʼ��������б�, ����޷���֤�û�\n");
	}
	
	FILE *fp;
	fp = fopen(MY_BBS_HOME "/etc/pop_register/pop_list", "r");
	if (fp == NULL)
	{
		http_fatal("�򿪿������ε��ʼ��������б����, ����޷���֤�û�\n");
	}	
	*/
#endif

	printf("<body>");
	printf("<nobr><center>%s -- ���û�ע��<hr>\n", BBSNAME);
	printf
	    ("<font color=green>��ӭ���뱾վ. ������������ʵ��д. ��*�ŵ���ĿΪ��������.</font>");
	printf
	    ("<br><font color=green>���� �� ID ��Ϊ���ڱ�վ��ʵ����ʾ���û����ƣ������޸ģ���������д.</font>");
	printf("<form method=post action=bbsdoreg>\n");
	printf("<table width=100%%>\n");
	printf
	    ("<tr><td align=right>*������ID  :<td align=left><input name=userid size=12 maxlength=12> (2-12�ַ�, ����ȫΪӢ����ĸ)\n");
	printf
	    ("<tr><td align=right>*����������:<td align=left><input type=password name=pass1 size=12 maxlength=12> (4-12�ַ�)\n");
	printf
	    ("<tr><td align=right>*��ȷ������:<td align=left><input type=password name=pass2 size=12 maxlength=12>\n");
	printf
	    ("<tr><td align=right>*�������ǳ�:<td align=left><input name=username size=20 maxlength=32> (2-30�ַ�, ��Ӣ�Ĳ���)\n");
	printf
	    ("<tr><td align=right>*������������ʵ����:<td align=left><input name=realname size=20> (��������, ����2������)\n");
	printf
	    ("<tr><td align=right>*��ϸͨѶ��ַ/Ŀǰסַ:<td align=left><input name=address size=40>  (����6���ַ�)\n");
#if 0
	printf
	    ("<tr><td align=right>*��ϸͨѶ��ַ:<td align=left><input name=address size=40> (����6���ַ�)\n");
	printf("<tr><td align=right>*���ĳ�������:\n");
	printf("<td align=left><input name=year size=4 maxlength=4>��");
	printf("<input name=month size=2 maxlength=2>��");
	printf("<input name=day size=2 maxlength=2>��<br>");
	printf
	    ("<tr><td align=right>�����������Ա�: <td align=left><input type=radio name=gender value=0 checked>�� ");
	printf("<input type=radio name=gender value=1>Ů<br>\n");
	printf("<tr><td align=right>������ʾ��ʽ: ");
	printf
	    ("<td align=left><input type=radio name=xz value=0 checked>��ɫ�����Ա� ");
	printf("<input type=radio name=xz value=1>��������ɫ ");
	printf("<input type=radio name=xz value=2>����ʾ����\n");
#endif
	
	printf
	    ("<tr><td align=right>*ѧУϵ�����߹�˾��λ:<td align=left><input name=dept size=40>\n");

#ifndef POP_CHECK
	printf
	    ("<tr><td align=right>email��ַ(��ѡ):<td align=left><input name=email size=40>\n");
#endif	

	printf
	    ("<tr><td align=right>����绰(��ѡ):<td align=left><input name=phone size=40>\n");
	printf
	    ("<tr><td align=right>У�ѻ���߱�ҵѧУ(��ѡ):<td align=left><input name=assoc size=40>\n");
	printf("<tr><td align=right>��վ����(��ѡ):<td align=left>");
	printf("<textarea name=words rows=3 cols=40 wrap=virutal></textarea>");

#ifdef POP_CHECK
	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // ע�⣺�������10��pop��������Ҫ��������ˣ�
	char ippop[10][256];
/*
	while(fgets(bufpop, 256, fp) != NULL)
	{
		if (strcmp(bufpop, "") == 0 || strcmp(bufpop, " ") == 0 || strcmp(bufpop, "\n") == 0)
			break;
		strcpy(namepop[numpop], bufpop);
		fgets(bufpop, 256, fp);
		strcpy(ippop[numpop], bufpop);
		numpop ++;
	}
	fclose(fp);
*/
	printf("<tr><td align=right>*�������ε��ʼ��������б�������ѡ��stu.xjtu.edu.cn��:<td align=left><SELECT NAME=popserver>\n");
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
	    ("<tr><td align=right>*�����������û���������������test��:<td align=left><input name=user size=20 maxlength=20> \n");
	printf
	    ("<tr><td align=right>*�������������루����������test��:<td align=left><input type=password name=pass size=20 maxlength=20> \n");
	printf("<tr>ÿ��������������֤ %d ��bbs�ʺ�\n", MAX_USER_PER_RECORD);
	
#endif	

	printf("</table><br><hr>\n");
	printf
	    ("<input type=submit value=�ύ���> <input type=reset value=������д> <input type=button value=�鿴���� onclick=\"javascript:{open('/reghelp.html','winreghelp','width=600,height=460,resizeable=yes,scrollbars=yes');return false;}\"\n");
	printf("</form></center>");
	http_quit();
	return 0;
}
