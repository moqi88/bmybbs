#include "bbslib.h"

static int show_form(char *board);

int
bbsbfind_main()
{
	FILE *fp;
	int num = 0, total = 0, type, dt, mg = 0, og = 0, at = 0;
	char dir[80], title[80], title2[80], title3[80], board[80], userid[80];
	struct boardmem *brd;
	struct fileheader x;
	html_header(1);
	changemode(READING);
	check_msg();
	printf("<center>%s -- ������������<hr>\n", BBSNAME);
	type = atoi(getparm("type"));
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 30);
	if (type == 0)
		return show_form(board);
	// һ������
	else if (type == 1)
	{ 
	
	strsncpy(title, getparm("title"), 60);
	strsncpy(title2, getparm("title2"), 60);
	strsncpy(title3, getparm("title3"), 60);
	strsncpy(userid, getparm("userid"), 60);
	if (!strcasecmp(userid, "Anonymous"))
		userid[0] = 0;
	dt = atoi(getparm("dt"));
	if (!strcasecmp(getparm("mg"), "on"))
		mg = 1;
	if (!strcasecmp(getparm("at"), "on"))
		at = 1;
	if (!strcasecmp(getparm("og"), "on"))
		og = 1;
	if (dt < 0)
		dt = 0;
	if (dt > 9999)
		dt = 9999;
	brd = getboard(board);
	if (brd == 0)
		http_fatal("�����������");
	sprintf(dir, "boards/%s/.DIR", board);
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("�����������û��Ŀǰ����");
	printf("����������'%s'��, ���⺬: '%s' ", board, nohtml(title));
	if (title2[0])

		printf("�� '%s' ", nohtml(title2));
	if (title3[0])
		printf("���� '%s' ", nohtml(title3));
	printf("����Ϊ: '%s', '%d'�����ڵ�%s%s����.<br>\n",
	       userid[0] ? userid_str(userid) : "��������", dt,
	       mg ? "����" : "����", at ? "�и���" : "");
	printf("<table>\n");
	printf("<tr><td>���<td>���<td>����<td>����<td>����\n");
	if (search_filter(title, title2, title3))
		goto E;

	while (1) {
		if (fread(&x, sizeof (x), 1, fp) == 0)
			break;
		num++;
		if (title[0] && !strcasestr(x.title, title))
			continue;
		if (title2[0] && !strcasestr(x.title, title2))
			continue;
		if (userid[0] && strcasecmp(x.owner, userid))
			continue;
		if (title3[0] && strcasestr(x.title, title3))
			continue;
		if (abs(now_t - x.filetime) > dt * 86400)
			continue;
		if (mg && !(x.accessed & FH_MARKED)
		    && !(x.accessed & FH_DIGEST))
			continue;
		if (at && !(x.accessed & FH_ATTACHED))
			continue;
		if (og && !strncmp(x.title, "Re: ", 4))
			continue;
		total++;
		printf("<tr><td>%d", num);
		
printf("<td>%s", flag_str(x.accessed));
		printf("<td>%s", userid_str(x.owner));
		printf("<td>%12.12s", 4 + Ctime(x.filetime));
		printf("<td><a href=con?B=%s&F=%s&N=%d&T=%d>%40.40s </a>\n", board,
		       fh2fname(&x), num, feditmark(x), x.title);
		if (total >= 999)
			break;
	}
	
	}
	//ȫ������
	else if (type == 2)

	{
		char content[200];
		char cmd[256];
		strsncpy(content, getparm("content"), 200);
		sprintf(cmd, MY_BBS_HOME "/bin/searcher.py %s '%s'", board, content);
		
		brd = getboard(board);
		if (brd == 0)
			http_fatal("�����������");

		fp = popen(cmd, "r");
		if (fp == 0)
			http_fatal("�����������");

		printf("����������'%s'��, ���ĺ�: '%s' ����������", board, nohtml(content));
		printf("<table>\n");
		printf("<tr><td>����<td>����<td>����\n");
		char line[256];
		while (fgets(line, 256, fp) != NULL)
		{
			if (line[0] != 'M')
				break;

 			char f_buf[16];
    		int filetime;
    		char t_buf[81];
    		char o_buf[16];

    		int len = strlen(line);
    		strsncpy(f_buf, line, 15); 
    		sscanf(f_buf, "M.%d.A", &filetime);

    		char *p2s = strchr(line+15, ' ');
    		int owner_len = p2s-line-15;
    		strsncpy(o_buf, line+15, owner_len+1);

    		strsncpy(t_buf, p2s+1, len-2-owner_len-14+1);

			printf("<tr><td><a href=qry?U=%s>%s", o_buf, o_buf);
			printf("<td>%s",Ctime(filetime));

			printf("<td><a href=con?B=%s&F=%s>%40.40s </a>\n", board, f_buf, t_buf);

			total++;
			if (total >= 999)
				break;
		}
	    goto E;
	}
	//����������
	else if(type==3)
	{
		 if (*system_load() >= 5.0 || count_online() > 4000)
			http_fatal
			    ("ϵͳ����(%f)����������(%d)����, ������վ�������ٵ�ʱ���ѯ.",
			     *system_load(), count_online());
				brd = getboard(board);

		brd = getboard(board);		
		if (brd == 0)
			http_fatal("�����������");
		 
		 char essential_path[80]="\0";
		 strncpy(title,getparm("title"),60);
		 fp=fopen("0Announce/.Search","r");
		 char linebuf[512];
		 char *tempbuf;
		 int flag=0;
		 int brdlen=strlen(board);
		 while(fgets(linebuf,512,fp)!=NULL)
		 {
			if(strncmp(linebuf,board,brdlen)==0)
			{
				tempbuf=strstr(linebuf,": ")+2;
				flag=1;
				break;
			}
		 }
		 fclose(fp);
		 if(flag==0)
			http_fatal("�����������");

		 tempbuf[strlen(tempbuf)-1]='\0';
		 strcpy(essential_path,tempbuf);
		 printf("����������'%s'�ľ�������, ���⺬: '%s' ����������", board, nohtml(title));
		 printf("<table>\n");
		 printf("<tr><td width=80px>���<td width=350px>����<td width=200px>���·��\n");
		 char searchcmd[256];
		 sprintf(searchcmd,MY_BBS_HOME "/bin/esearch %s %s",essential_path,title);
		 fp=popen(searchcmd,"r"); 
		 while(fgets(linebuf,512,fp)!=NULL)
		 {
		    char postindex[64];
			char posttitle[64];
			char postpath[256];
			char postnum[32];
			char *ptemp;
			ptemp=strtok(linebuf,",");
			strcpy(postindex,ptemp);
			ptemp=strtok(NULL,",");
			strcpy(postpath,ptemp);
			ptemp=strtok(NULL,",");
			strcpy(postnum,ptemp);
			ptemp=strtok(NULL,"\n");
			strcpy(posttitle,ptemp);
			printf("<tr><td>%d",total+1);
			printf("<td><a href=bbsanc?path=%s&item=%s>%40.40s </a>",postpath,postnum,posttitle);
			printf("<td>%s\n",postindex);
			total++;
			if(total>999)
				break;
		 }
	}
E:
	if (type == 1)
		fclose(fp);
	else if (type == 2 || type == 3 )
		pclose(fp);
	printf("</table>\n");
	printf("<br>���ҵ� %d ƪ���·�������", total);
	if (total > 999)
		printf("(ƥ��������, ʡ�Ե�1000�Ժ�Ĳ�ѯ���)");
	printf("<br>\n");
	printf
	    ("[<a href=bbsdoc?board=%s>���ر�������</a>] [<a href='javascript:history.go(-1)'>������һҳ</a>]",
	     board);
	http_quit();
	return 0;
}

static int
show_form(char *board)
{
	printf("<table><form action=bbsbfind?type=1 method=post>\n");
	printf("<tr><td><h4>һ���ѯ</h4>\n");
	printf
	    ("<tr><td>��������: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n",
	     board);
	printf
	    ("<tr><td>���⺬��: <input type=text maxlength=50 size=20 name=title> AND ");
	printf("<input type=text maxlength=50 size=20 name=title2>\n");
	printf
	    ("<tr><td>���ⲻ��: <input type=text maxlength=50 size=20 name=title3>\n");
	printf
	    ("<tr><td>�����ʺ�: <input type=text maxlength=12 size=12 name=userid><br>\n");
	printf
	    ("<tr><td>ʱ�䷶Χ: <input type=text maxlength=4  size=4  name=dt value=7> ������<br>\n");
	printf("<tr><td>��M����: <input type=checkbox name=mg> ���и���: <input type=checkbox name=at> ��������: <input type=checkbox name=og><br><br>\n");

	/* modified by freely@BMY@20070601 */	
	printf("<tr><td><input type=submit value=�ݽ���ѯ���>\n");
	printf("</form></table>");
	
	printf("<hr />\n");
	
	printf("<table><form action=bbsbfind?type=2 method=post>\n");
	printf("<tr><td><h4>ȫ�Ĳ�ѯ</h4>\n");
	printf
	    ("<tr><td>��������: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n",
	     board);
	printf("<tr><td>���ĺ���: <input type=text maxlength=100 size=50 name=content>\n");
	printf("<tr><td>(֧��ν��: AND, OR, NOT)<br><br>\n");
	printf("<tr><td><input type=submit value=�ݽ���ѯ���>\n");
	printf("</form></table>");

	printf("<hr />\n");
	printf("<table><form action=bbsbfind?type=3 method=post>\n");
	printf("<tr><td><h4>��������ѯ</h4>\n");
	printf
		("<tr><td>��������: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n",		board);
	printf("<tr><td>���⺬��: <input type=text maxlength=50 size=50 name=title>\n");
	printf("<tr><td><input type=submit value=�ݽ���ѯ���>\n");
	printf("</form></table>");
	printf("<hr />\n");

	printf
	    ("[<a href='bbsdoc?board=%s'>������һҳ</a>] [<a href=bbsfind>ȫվ���²�ѯ</a>]",
	     board);
	http_quit();
	return 0;
}
