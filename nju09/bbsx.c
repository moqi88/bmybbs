#include "bbslib.h"

static void show_xbanner(void);
static void show_xsec(struct sectree *sec);
static void show_xboards(char *secstr);
static void show_xsec_boards(struct boardmem *(data[]), int total);
int chm=0;

static void 
show_xbanner()
{	
	printf("&nbsp;</td></tr></table></td></tr></table>\n");
	printf("%s", "<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td height=30></td></tr>\n"
	"<tr><td height=70>\n"
	"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n"
	"<tr><td><td> <div id=\"bmy\"><span class=\"hidden\">����ٸ</span></div> </td></td>\n"
	"<td width=290><table border=0 cellpadding=0 cellspacing=0>\n"
	"<tr>");
	printf("<form action=bbsx target=f3><td colspan=2><input name=chm type=hidden value=%d>\n"
	"<input name=Submit type=submit class=sumbitgrey value=\"����%s��ʽ\" title=\"����л������ļ���ʽ\">\n"
	"</td>\n</form>", chm?0:1, chm?"tgz":"chm");
	printf("</tr></table></td>\n"
	"<td align=right>&nbsp;</td></tr></table></td></tr></table>\n");
	return;
}

static void 
show_xsec(struct sectree *sec)
{	
	int i;
	for (i = 0; i < sec->nsubsec; i++) {
		printf("<tr>");
		printf("<td><div class=\"linediv\"><a href=boa?secstr=%s class=linkboardtheme>"
		       "%s</a></div></td>\n", sec->subsec[i]->basestr,
		       nohtml(sec->subsec[i]->title));
		printf("<td rowspan=2 align=right valign=bottom><a href=boa?secstr=%s class=linkbigtheme>%s</a></td></tr>\n",
			sec->subsec[i]->basestr, sec->subsec[i]->basestr);
		show_xboards(sec->subsec[i]->basestr);
	}
	return;
}

static void 
show_xboards(char *secstr)
{	
	struct boardmem *(data[MAXBOARD]), *x;
	int len, hasintro = 0;
	int i, total = 0;
	const struct sectree *sec;
	
	sec = getsectree(secstr);
	len = strlen(secstr);
	if (sec->introstr[0])
		hasintro = 1;
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->header.filename[0] <= 32 || x->header.filename[0] > 'z')
			continue;
		if (hasintro) {
			if (strcmp(secstr, x->header.sec1) &&
			    strcmp(secstr, x->header.sec2)) continue;
		} else {
			if (strncmp(secstr, x->header.sec1, len) &&
			    strncmp(secstr, x->header.sec2, len))
				continue;
		}
		if (!has_read_perm_x(&currentuser, x))
			continue;
		data[total] = x;
		total++;
	}
	show_xsec_boards(data, total);
}

static void 
show_xsec_boards(struct boardmem *(data[]), int total)
{	
	int i;
	printf("<tr><td>\n");
	for(i=0; i<total; i++){
		printf("<a href=bbsx/%s.%s>%s</a> ", 
			data[i]->header.filename, 
			chm ? "chm":"tgz",
			data[i]->header.title);
	}
	printf("</td></tr>\n");
}

int
bbsx_main()
{

	char path[256];
	struct boardmem *x1;
	char *ptr, *board;
	
	board = getsenv("SCRIPT_URL");
	board = strrchr(board , '/')+1;

	if (strstr(board, ".tgz") && !strstr(board, "bbsx"))
	{
		if (!loginok || isguest) {
			http_fatal("���ȵ�½������");
			return 0;
		}
		ptr=strstr(board, ".tgz");
		*ptr=0;
		if (!board[0]){
			http_fatal("����Ĳ���");
			return -1;
		}
		x1 = getboard(board);
		if (!x1){
			http_fatal("������ļ���");
			return -1;
		}
		sprintf(path, MY_BBS_HOME "/0Announce%s/%s.tgz", anno_path_of(board), board);	
		if (!file_exist(path)){
			http_fatal("�ļ�������");
			return -1;
		}
		printf("Content-type: application/tgz\n\n");
		showfile(path);
		return 0;
	}else if (!strstr(board, ".tgz") && !strstr(board, "bbsx")){ 
		http_fatal("������ļ���");
		return -1;
	}

	chm = atoi(getparm("chm"));

	/*
	char BoardsFile[256];
	int index, fd, SectNumber, total, start;
	struct boardheader *buffer;
	struct stat st;
	char   filename[255] ;

	printf("Content-type: text/html; charset=%s\n\n", CHARSET);
	printf("<html>\n");
	check_msg();
	printf("<link rel=stylesheet href=/x.css>\n");
	sprintf(BoardsFile, "%s/.BOARDS", BBSHOME);
	fd = open(BoardsFile, O_RDONLY);
	if (fd == -1)
		http_fatal("error in opening .BOARDS file!");
	fstat(fd, &st);
	total = st.st_size / sizeof (struct boardheader);
	buffer =
	    (struct boardheader *) calloc((size_t) total,
					  sizeof (struct boardheader));
	if (buffer == NULL) {
		close(fd);
		http_fatal("Out of memory!");
	}
	if (read(fd, buffer, (size_t) st.st_size) < (ssize_t) st.st_size) {
		close(fd);
		free(buffer);
		http_fatal("Error readinf .BOARDS file!");
	}
	close(fd);
	printf("<body background=>\n");
	printf
	    ("<center><strong>%s ��ʽ ����������,�˸�ʽ�����ٶȴ�Լÿ%sһ�� %s</center></strong>\n",
	     chm ? "chm" : "tgz", chm ? "��" : "��1/3/5",
	     chm ? "<a href=bbsx?chm=0>tgz��ʽ</a>" :
	     "<a href=bbsx?chm=1>chm��ʽ</a>");
	printf
	    ("<hr><b>����ע������:</b> ���ھ������û����������ϵȹ��߿���������"
	     "���ؾ�����, �Ӷ����ϵͳ���ز��ȶ�, �ʽ�����������ȫ����Ϊ�� ftp "
	     "��ʽ����, ��ɵĲ��������½�. " MY_BBS_NAME " ftp ���ӷ�ʽΪ,<br>"
	     " &nbsp; &nbsp; ��ַ: " MY_BBS_DOMAIN "<br>"
	     " &nbsp; &nbsp; �˿�: 2121<br>" " &nbsp; &nbsp; �û�: �����ʺ�<br>"
	     " &nbsp; &nbsp; ����: ��������<br>"
	     "ͬʱ��վ ftp Ҳ֧����������. Ҫע�����, ÿ���û����ֻ��ͬʱ���� 2 "
	     "�� ftp ����. �����ʱ���ڽ����ܶ�����, �ᵼ�� ftp ��ʱ������� ip 5 ����.<br>"
	     "���� ftp ����ĸ���ϸ˵��, ��μ� <a href=%sBBSHelp>BBSHelp ��</a><hr>", showByDefMode());
	printf("<table width=100%% border=0 cellspacing=0 cellpadding=3>\n");
	for (SectNumber = 0; SectNumber < sectree.nsubsec; SectNumber++) {
		const struct sectree *sec = sectree.subsec[SectNumber];
		printf("<tr>\n");
		printf("<TD width=15%% valign=top align=center><B>%s</B></TD>",
		       sec->title);
		printf("<TD width=85%%>");
		start = 1;
		for (index = 0; index < total; index++) {
			if (sec->basestr[0] != buffer[index].sec1[0]
			    && sec->basestr[0] != buffer[index].sec2[0])
				continue;
			if ((buffer[index].level > 0
			     && (buffer[index].level & PERM_NOZAP) == 0
			     && (buffer[index].level & PERM_POSTMASK) == 0)
			    || (buffer[index].clubnum
				&& !(buffer[index].flag & CLUBTYPE_FLAG)))
				continue;
			sprintf(buf, MY_BBS_HOME "/ftphome/root/pub/X/%s%s.%s",
				chm ? "chm/" : "", buffer[index].filename,
				chm ? "chm" : "tgz");
			if (!file_exist(buf)) {
				//errlog("nofile:%s", buf);
				continue;
			}
			if (!start)
				printf(" - ");
			start = 0;
			printf
			    ("<A HREF=\"ftp://" MY_BBS_DOMAIN ":2121/pub/X/%s%s.%s\">%s</A>",
			     chm ? "chm/" : "", buffer[index].filename,
			     chm ? "chm" : "tgz", buffer[index].title);
		}
		printf("\n</TD></tr>");
	}
	printf("</table>\n");
	free(buffer);
	http_quit();
	*/

	const struct sectree *sec;
	sec = getsectree("?");
	html_header(1);
	check_msg();
	changemode(SELECT);
	printf("<script src=\"/inc/tog.js\"></script></head><body leftmargin=0 topmargin=0>\n");
	show_xbanner();		
	printf("%s", "<table width=75% border=0 cellpadding=0 cellspacing=0>\n");
	show_xsec(sec);
	printf("</table>\n<br></td>\n");
	printf("</body></html>");
	return 0;
}

