#include "bbslib.h"
#include "tmpl.h"

static int
seek_in_file(filename, seekstr)
char filename[STRLEN], seekstr[STRLEN];
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int
bbspst_main()
{
	FILE *fp;
	int local_article, i, num, fullquote = 0, guestre = 0, thread = -1;
	char userid[80], buf[512], path[512], file[512], board[512], title[80] =
	    "";
	struct fileheader *dirinfo = NULL;
	struct boardmem *x;
	char bmbuf[IDLEN * 4 + 4];		//add by mintbaggio 040807 for new www
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if(strcmp(board, "welcome")  &&
	   strcmp(board, "KaoYan")){	///modify by mintbaggio 040614 for guest post at board "welcome" + "KaoYan"(by wsf)
		if (!loginok) {
			printf("<script src=/function.js></script>\n");
			printf("�Ҵҹ��Ͳ��ܷ������£����ȵ�¼!<br><br>");
			printf("<script>openlog();</script>");
			http_quit();
		}
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost) && !loginok)
		http_fatal("����ip����ֹʹ��guest�ڱ��淢������!��");
	local_article = 1; // modified by linux @ 2006.6.6 for the default post status to no outgo
//	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 20);
	strsncpy(file, getparm("F"), 20);
	if (!file[0])
		strsncpy(file, getparm("file"), 20);
	fullquote = atoi(getparm("fullquote"));
	if (file[0] != 'M' && file[0])
		http_fatal("������ļ���");
	if (!(x = getboard(board)))
		http_fatal("�������������������Ȩ�ڴ���������������");
	if (file[0]) {
		num = atoi(getparm("num"));
		sprintf(path, "boards/%s/.DIR", board);
		MMAP_TRY {
			if (mmapfile(path, &mf) == -1) {
				MMAP_UNTRY;
				http_fatal("�����������");
			}
			dirinfo = findbarticle(&mf, file, &num, 1);
		}
		MMAP_CATCH {
			dirinfo = NULL;
		}
		MMAP_END mmapfile(NULL, &mf);
		if (dirinfo) {
			thread = dirinfo->thread;
			//if (dirinfo->accessed & FH_ALLREPLY)
			//	guestre = 1;
			strsncpy(userid, fh2owner(dirinfo), 20);
			if (strncmp(dirinfo->title, "Re: ", 4))
			  {
				snprintf(title, 60, "Re: %s", dirinfo->title);
				local_article = atoi(getparm("la")); // added by linux @2006.6.6 for the post status to the status of the article before when doing a reply post
			  }
			else
				strsncpy(title, dirinfo->title, 60);
		} else
			http_fatal("������ļ���");
		if (dirinfo->accessed & FH_NOREPLY)
			http_fatal("���ı���Ϊ����Reģʽ");

	}
	if (!has_post_perm(&currentuser, x) && !isguest) 
	{
		  if (x->header.secnumber2=='C')
			http_fatal("���ֲ����棬����ϵ���������������ֲ����ܷ���.");
		  else
		  	http_fatal("�������������������Ȩ�ڴ���������������");
	}
	if (noadm4political(board))
		http_fatal("�Բ���,��Ϊû�а��������Ա����,������ʱ���.");
	if(strcmp(board, "welcome") &&
	   strcmp(board, "KaoYan")){	//add by mintbaggio 040614 for guest post at "welcome" + "KaoYan"(by wsf)
		if (isguest && !guestre) {
			printf("<script src=/function.js></script>\n");
			printf("�Ҵҹ��Ͳ��ܷ������£����ȵ�¼!<br><br>");
			printf("<script>openlog();</script>");
			http_quit();
		}
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost) && !guestre)
		http_fatal("����ip����ֹʹ��guest�ڱ��淢������!��");
	changemode(POSTING);
//	printf("<body><center>\n");
	printf("<body leftmargin=0 topmargin=0>\n");
	printf("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n");
	printf("%s", "<tr>\n<td height=30 colspan=2>\n" 
		"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
        	"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
		"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n"
		"<tr><td>\n");
	printf("<a href=\"boa?secstr=%s\">%s</a> / <a href=\"%s%s\">%s��</a> / �������� </td>\n"
		"</tr></table></td>\n<td><table border=0 align=right cellpadding=0 cellspacing=0>\n"
		"<tr><td> ���� %s \n"
		"</td></tr></table></td></tr></table></td></tr>\n", 
		x->header.sec1,nohtml(getsectree(x->header.sec1)->title), showByDefMode(), board, board, userid_str(bm2str(bmbuf, &(x->header))));
//	printf("%s -- �������� [ʹ����: %s]<hr>\n", BBSNAME, currentuser.userid);
	if (x->header.flag & IS1984_FLAG)
		printf("<tr><td height=30 colspan=2><font color=red>��ע�⣬���ķ������ͨ�����</font></td></tr>");
 	printf("%s", "<tr><td height=70 colspan=2>\n"
 		"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 bgcolor=\"#efefef\">\n"
		"<tr><td width=40>&nbsp; </td>\n"
		"<td height=70><table width=\"95%\" height=\"100%\"  border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n");
 	printf("<td valign=bottom>\n"
 		"<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td class=F0002><div class=\"menu\">\n"
		"<DIV class=btncurrent>&lt;%s&gt;</DIV>\n"
		"<DIV><A class=btnfunc href=\"%s%s\" title=\"���������� accesskey: b\" accesskey=\"b\">/ ����������</A></DIV>\n"
		"<DIV style=\"width:10px\" class=N1001></DIV>\n"
		"</div></td></tr></table></td></tr>\n", void1(titlestr(x->header.title)), showByDefMode(), board);
	printf("<tr><td width=\"100%\"><table  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr><td> ����ע������: <br>\n"
		"����ʱӦ���ؿ������������Ƿ��ʺϹ������Ϸ������������ˮ��лл���ĺ����� <br>"
		"��վ�涨ͬ�����ݵ������Ͻ��� 4 ���� 4 ���������������ظ�����Υ�߽�������ڱ�վ���ĵ�Ȩ����<br>"
		"����һ�Ķ෢�����Ʋ�<a target=f3 href='home?B=sysop'> SYSOP </a>������һ�¡�"
		"</td>\n"
		"</tr></table></td></table></td></tr>\n");
/*	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<font color=green>����ע������: <br>\n");
	printf
	    ("����ʱӦ���ؿ������������Ƿ��ʺϹ������Ϸ������������ˮ��лл���ĺ�����<br>\n</font>");
*/	if (file[0])
		snprintf(buf, sizeof (buf), "&ref=%s&rid=%d", file, num);
/*	printf("<tr><td><form name=form1 method=post action=bbssnd?board=%s&th=%d%s>\n",
	       board, thread, file[0] ? buf : "");
	printf
	    ("ʹ�ñ���: <input type=text name=title size=40 maxlength=100 value='%s'>",
	     (void1(noquote_html(title))));
	printf(" ������: [%s]\n", board);
*/
	printf("<tr><td width=40 class=\"level1\"></td>\n"
		"<td class=\"level1\"><br>\n"
		"<TABLE width=\"95%\" cellpadding=5 cellspacing=0>\n"
		"<TBODY><TR><TD class=tdtitletheme>&nbsp;</TD>\n"
		"</TR>\n");
	int hastmpl;
	char tmplfile[STRLEN];
	sprintf(tmplfile, "boards/%s/%s", board, ".tmpl");
	if (fopen(tmplfile, "r") == 0)
		hastmpl = 0;
	else
		hastmpl = file_size(tmplfile) / sizeof (struct a_template);
	if (!file[0] && hastmpl > 0) {
		printf("<tr><td class=bordertheme>");
		printf("<a target=_self href=bbstmpl?action=show&board=%s class=btnsubmittheme>", board);
		printf("ģ�巢��</a>");
		printf("</td></tr>\n");		
	}
	printf("<TR><TD class=bordertheme>\n"
		"<form name=form1 method=post action=bbssnd?board=%s&th=%d%s>\n",
		board, thread, file[0] ? buf : "");
	printf("<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr>\n<td><table border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td> ʹ�ñ���: </td>\n");
	
		//ArthurF�޸Ĳ��ֿ�ʼ 
		//Ԥ��ʵ�ֹ��� www�µı��ⳤ������ ���⽫ͨ��js������45��Ӣ�ĺ�22������֮��
		//ʧȥ�����ʱ�����ͳ�� �����򵯳���ʾ�� Ҫ���޸�
		if (file[0]){
			printf("<td><input name=title type=text class=inputtitle maxlength=45 size=50 value='%s'></td>\n", (void1(noquote_html(title))));
		}
		else{	
			printf("<script language=\"JavaScript\">\n"
					"function realLen(v){\n"
					"  l=0;\n"
					"  for (var i=0;i<v.length;i++){\n"
					"    if (Math.abs(v.charCodeAt(i))>255)\n"
					"      l+=2;\n"
					"    else\n"
					"      l++;\n"
					"  }\n"
					"  return l;}\n"
					"</script>\n"
					"\n"
					"<td><input id=edittitle name=title type=text class=inputtitle maxlength=45 size=50 value='%s' ></td>\n"
					, (void1(noquote_html(title))));
			printf("<script language=\"JavaScript\">\n"
			        " document.getElementById(\"edittitle\").focus(); \n"
			        "  </script>");
		}
			//�޸Ĳ��ֽ��� 

	printf("<td height=20>\n"
		" ������: [%s]</td>\n"
		"</tr></table></td></tr>\n", board);
	printf("%s", "<tr><td><table border=0 cellpadding=0 cellspacing=0><tr>\n");
	printf("<td> ���ߣ�%s &nbsp</td>\n", currentuser.userid);
/*	if (innd_board(board))
		printf("ת��<input type=checkbox name=outgoing %s>\n",
		       local_article ? "" : "checked");
	if (anony_board(board))
		printf("����<input type=checkbox name=anony>\n");
	printf("<br>���ߣ�%s &nbsp;", currentuser.userid);
*/
	printselsignature();
	printuploadattach();
	if (innd_board(board))
                printf("ת��<input type=checkbox name=outgoing %s>\n",
                       local_article ? "" : "checked");
        if (anony_board(board))
                printf("����<input type=checkbox name=anony>\n");
	printf("</td></tr></table></td></tr>\n");
	//printusemath(0);
	printf("<tr><td>\n");
	printf("ʹ��Tex������ѧ��ʽ<input type=checkbox name=usemath>\n");
	printf("��Ϊ���ɻظ�<input type=checkbox name=nore>\n");
	printf("�ظ�����������<input type=checkbox name=mailback>\n");
	if (file[0])
		if (dirinfo->accessed & FH_MAILREPLY)
			printf("<input type=hidden name=replyto value=%s>\n", userid);
	printf("</td></tr>\n");
	if (file[0]) {
		printf("<tr><td>����ģʽ: %s ", fullquote ? "��ȫ" : "����");
		printf
		    ("[<a target=_self href=bbspst?inframe=1&board=%s&file=%s&num=%d&la=%d",
		     board, file, num, local_article);
		printf("&fullquote=%d>�л�Ϊ%sģʽ</a> (����������������)]",
		       !fullquote, (!fullquote) ? "��ȫ" : "����");
		printf("</td></tr>\n");
	}
	printf
	    ("<tr><td><textarea id=textedit onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=76 wrap=virtual class=f2 >\n\n");
	if (file[0]) {
		int lines = 0;
		printf("�� �� %s �Ĵ������ᵽ: ��\n", userid);
		sprintf(path, "boards/%s/%s", board, file);
		fp = fopen(path, "r");
		if (fp) {
			for (i = 0; i < 3; i++)
				if (fgets(buf, 500, fp) == 0)
					break;
			while (1) {
				if (fgets(buf, 500, fp) == 0)
					break;
				if (!strncmp(buf, ": ��", 4))
					continue;
				if (!strncmp(buf, ": : ", 4))
					continue;
				if (!strncmp(buf, "--\n", 3)
				    || !strncmp(buf, "begin 644 ", 10)
				    || !strncmp(buf, "beginbinaryattach ", 18))
					break;
				if (buf[0] == '\n')
					continue;
				if (!fullquote && lines++ >= 10) {
					printf(": (��������ʡ��...)\n");
					break;
				}
				printf(": %s", nohtml(buf));
			}
			fclose(fp);
		}
	}
	printf("</textarea></td></tr>\n");
	if(file[0])
		printf("<script language=\"JavaScript\">\n"
			   " document.getElementById(\"textedit\").focus(); \n"
			   "  </script>");
	
		
/*	printf
	    ("<tr><td class=post align=center><input type=submit value=����> ");
	printf("<input type=reset value=���></form>\n");
	printf("</table>");
*/
	printf("%s", "<tr><td><input name=Submit2 type=submit class=resetlong value=\"����\" "
			"onclick=\"if (realLen(value)>45){alert('���±��ⳤ�Ȳ��ܳ���22�����ֻ�45��Ӣ�ĳ���,����ᶪʧ��Ϣ,���޸����±���.');this.focus();return false}else{this.value='�����ύ�У����Ժ�...';this.disabled=true;form1.submit()}\">\n"
			"<input name=Submit3 type=reset class=sumbitlong value=\"���\" onclick='return confirm(\"ȷ��Ҫȫ�������?\")'></td>\n"
			"</tr>\n");
	printf("%s", "</table></TD></TR></TBODY></TABLE></form></td></tr>\n"
		"<tr>\n<td height=40 bgcolor=\"#FFFFFF\">��</td>\n"
		"<td height=40 bgcolor=\"#FFFFFF\">��</td>\n"
		"</tr></table></td></tr></table>\n");
	http_quit();
	return 0;
}

void
printselsignature()
{	//modify by mintbaggio 040809 for new www
	int i, sigln, numofsig;
	char path[200];
	sprintf(path, "home/%c/%s/signatures",
		mytoupper(currentuser.userid[0]), currentuser.userid);
	sigln = countln(path);
	numofsig = (sigln + MAXSIGLINES - 1) / MAXSIGLINES;
	printf("%s", "<td>ʹ��ǩ����:</td><td height=20> <select name=\"signature\" class=2015>\n");
	if (currentuser.signature == 0)
		printf("<option value=\"0\" selected>��ʹ��ǩ����</option>\n");
	else
		printf("<option value=\"0\">��ʹ��ǩ����</option>\n");
	if (numofsig>0) {
		if (currentuser.signature == -1)
			printf("<option value=\"-1\" selected>ʹ�����ǩ����</option>\n");
		else
			printf("<option value=\"-1\">ʹ�����ǩ����</option>\n");
	}
	for (i = 1; i <= numofsig; i++) {
		if (currentuser.signature == i)
			printf
			    ("<option value=\"%d\" selected>�� %d ��</option>\n",
			     i, i);
		else
			printf("<option value=\"%d\">�� %d ��</option>\n", i,
			       i);
	}
	printf("</select></td>\n");
	printf("%s", "<td> [<a target=_blank href=bbssig>�鿴ǩ����</a>]");
}

void
printuploadattach()
{	//modify by mintbaggio 040809 for new www
	int i = u_info - &(shm_utmp->uinfo[0]);
	printf
	    (" [<a href=/cgi-bin/bbs/upload/%c%c%c%s target=uploadytht>���/ɾ������</a>]\n",
	     i / (26 * 26) + 'A', i / 26 % 26 + 'A', i % 26 + 'A',
	     u_info->sessionid);
}

void
printusemath(int checked)
{
	printf
	    ("<tr><td>ʹ��Tex������ѧ��ʽ<input type=checkbox name=usemath%s>\n",
	     checked ? " checked" : "");
	//printf
	//    (" <a href=home/boards/BBSHelp/html/itex/itexintro.html target=_blank>����ʲô��</a></td></tr>\n");
	printf
	    ("��Ϊ���ɻظ�<input type=checkbox name=nore%s></td></tr>\n",
	     checked ? " checked" : "");
}
