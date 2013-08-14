#include "bbslib.h"

char* userid_str_class(char *s, char* class)
{
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	strsncpy(tmp, s, 255);
	buf[0] = 0;
	ptr = strtok(tmp, " ,();\r\n\t");
	while (ptr && strlen(buf) < 400) {
		if ((ptr2 = strchr(ptr, '.'))) {
			ptr2[1] = 0;
			strcat(buf, ptr);
		} else {
			ptr = nohtml(ptr);
			sprintf(buf2, "<a href=qry?U=%s class=%s>%s</a>", ptr, class, ptr);
			strcat(buf, buf2);
		}
		ptr = strtok(0, " ,();\r\n\t");
		if (ptr)
			strcat(buf, " ");
	}
	return buf;
}

void tshare(char * board, int start, int thread, int article_count, char * owner, char * thread_title)
{
	char thread_title_utf8[480];
	char title[256];
	strcpy(title, thread_title);
	g2u(title,sizeof(title),thread_title_utf8,sizeof(thread_title_utf8));
	printf("���� ");
	// function thread_share(a,title,owner,board,thread,start,article_count) prototype in function.js
	printf("<a href=\"#\" onclick=\"javascript:thread_share('sina','%s','%s','%s','%d','%d','%d');\"><img src=\"/images/share-sina.png\"/></a>",
			url_encode(thread_title_utf8), owner, board, thread, start, article_count);
	printf("<a href=\"#\" onclick=\"javascript:thread_share('tencent','%s','%s','%s','%d','%d','%d');\"><img src=\"/images/share-tencent.png\"/></a>",
			url_encode(thread_title_utf8), owner, board, thread, start, article_count);
}

int
bbstcon_main()
{	//modify by mintbaggio 040529 for new www, 041228 for www v2.0
	char title[256], board[80], dir[80];
	char thread_title[256]; // ͬ������� by IronBlood@bmy 20120426
	char bmbuf[IDLEN * 4 + 4], odd_even_class[16], class[10];
	struct fileheader *x;
	struct boardmem *x1;
	int num = 0, firstnum = 0, found = 0, start, total;
	int count = 0, nextstart = 0, flpage = 0;
	int article_count = 0;
	int thread;
	int ismozilla;
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	printf("<script src=/function.js></script>\n");
	strsncpy(board, getparm("board"), 32);
	thread = atoi(getparm("th"));
	printf("<body leftmargin=0 topmargin=0>\n<img src=\"/images/bmy.gif\" style=\"position: absolute;top:-160px;\"/>\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n");
	if ((x1 = getboard(board)) == NULL)
		http_fatal("�����������");
	start = atoi(getparm("start"));
	brc_initial(currentuser.userid, board);
	sprintf(dir, "boards/%s/.DIR", board);
	count = 0;
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("Ŀ¼����");
		}
		total = mf.size / sizeof (struct fileheader);
		if (start < 0 || start >= total) {
			start = Search_Bin(mf.ptr, thread, 0, total - 1);
			if (start < 0)
				start = -(start + 1);
		}
		printf("<tr><td height=30 colspan=2>\n"
			"<table width=100%%  border=0 cellspacing=0 cellpadding=0>\n"
			"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
			"<td><table width=100%% border=0 align=right cellpadding=0 cellspacing=0>\n");
		printf("<tr><td><a href=\"boa?secstr=%s\">%s</a> / <a href=\"%s%s\">%s</a> / �Ķ����� "
			"</td></tr></table></td>\n", x1->header.sec1, nohtml(getsectree(x1->header.sec1)->title), showByDefMode(), board, board);
		printf("<td><table border=0 align=right cellpadding=0 cellspacing=0>\n"
			"<tr><td> ���� [%s]</tr></table></td></tr></table></td></tr>\n", 
			userid_str(bm2str(bmbuf, &(x1->header))));
		printf("<tr><td height=70 colspan=2>\n"
			"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=level2>\n"
			"<tr><td width=40>&nbsp; </td>\n"
			"<td height=70>\n"
			"<table width=95%% height=100%%  border=0 cellpadding=0 cellspacing=0>\n"
			"<tr><td colspan=2 valign=bottom>\n"
			"<table width=100%% border=0 cellpadding=0 cellspacing=0>\n");
		printf("<tr><td><DIV class=menu><span class=btncurrent>&lt;%s&gt;</span>\n", 
			void1(titlestr(x1->header.title)));
#if 0
		printf("<div><a class=N0040 href='doc?B=%s&amp;S=%d'>&lt;����������&gt;</a>\n",
				    board, (num > 4) ? (num - 4) : 1);
#endif
		printf("<span><A class=\"btnfunc\" href=\"%s%s&amp;S=%d\" title=\"���������� accesskey: b\" accesskey=\"b\">/ ���������� </a></span>", showByDefMode(), board, (num > 4) ? (num - 4) : 1 );
		printf("</div></td></tr></table></td></tr>\n");
		printf("<tr><td width=59%%> ͬ�����Ķ���\n");

		for (num = start; num < total; num++) {
			x = (struct fileheader *) (mf.ptr +
						   num *
						   sizeof (struct fileheader));
			if (thread != 0) {
				if (x->thread != thread) {
					continue;
				}
			} else {
				if (strncmp(x->title, title, 39)) {
					continue;
				}
			}
			flpage = num;
			break;
		}

		x = (struct fileheader *) (mf.ptr + flpage * sizeof (struct fileheader));
		memset(thread_title, 0, sizeof(thread_title));
		strcpy(thread_title, x->title);
		printf(" %s </td>\n", x->title);
		printf("</tr></table></td></tr>\n");
		printf("<tr><td width=40 class=level1>&nbsp;</td><td class=level1>\n");
/*		printf
		    ("%s -- ���������Ķ� [������: <a href=doc?B=%s&S=%d>%s</a>]<hr>",
		     BBSNAME, board, start, board);
*/		ismozilla = testmozilla();
		for (num = start; num < total; num++) {
			x = (struct fileheader *) (mf.ptr +
						   num *
						   sizeof (struct fileheader));
			if (thread != 0) {
				if (x->thread != thread) {
					continue;
				}
			} else {
				if (strncmp(x->title, title, 39)) {
					continue;
				}
			}
			//x->viewtime++;
			//if ((sizeof(struct fileheader) * (num+1)) % pagesize ==0 )msync(mem, dirstat.st_size, MS_SYNC); 
//			printf("<table width=100%%><tr><td>");
			if(article_count % 2)
				strcpy(odd_even_class, "tdtitlegrey");
			else	strcpy(odd_even_class, "tdtitletheme");
			printf("<TABLE width=95%% cellpadding=5 cellspacing=0>\n"
				"<TBODY><TR><TD class=%s><a id=%d></a>No. %d\n",  odd_even_class, article_count, article_count);
			//printf("<TD align=right class=%s>\n", odd_even_class);

//			article_count++;

			printf("<a href=con?B=%s&F=%s&N=%d&T=%d class=linkwhite>��ƪȫ��</a>&nbsp;",
			       board, fh2fname(x), num + 1, feditmark(*x));
			printf("<a href='pst?B=%s&F=%s&num=%d' class=linkwhite>�ظ�����</a>&nbsp;",
			       board, fh2fname(x), num);

			printf
			    ("<a href='pstmail?B=%s&F=%s&num=%d' class=linkwhite>���Ÿ�����</a>&nbsp;",
			     board, fh2fname(x), num);
			strcpy(class, "1105");
			printf("��ƪ����: %s ", userid_str_class(fh2owner(x), "linkwhite"));
			printf("��ƪ�Ǽ�: %d ", x->staravg50 / 50);
			printf("��������: %d ", x->hasvoted);
			if(article_count % 2)
				strcpy(odd_even_class, "bordergrey");
			else	strcpy(odd_even_class, "bordertheme");
			printf("</td><td width=10%% align=right class=%s>", odd_even_class);
			tshare(board, start, thread, article_count, x->owner, thread_title);
			printf("</td></tr>\n<tr>");
			//if(article_count % 2)
			//	strcpy(odd_even_class, "bordergrey");
			//else	strcpy(odd_even_class, "bordertheme");
			if (ismozilla && wwwstylenum % 2)
				printf("<td>");
			else{
				printf("<td colspan=2 class=%s>\n", odd_even_class);
			}
			printf("<div id='filecontent' style='width:800px;'>\n");
			show_file(board, x, num);
			printf("</div>");
#ifdef ENABLE_MYSQL
			if (loginok && now_t - x->filetime <= 3 * 86400) {
				printf("</td></tr>\n<tr><td>");
				printf("<script>eva('%s','%s');</script>",
				       board, fh2fname(x));
			}
#endif
//			printf("</table><hr>");
			if(article_count % 2)
				strcpy(odd_even_class, "topgrey");
			else	strcpy(odd_even_class, "toptheme");
			printf("</TD></TR><TR >\n");
			printf("<td>��������&nbsp;&nbsp;"
				   "<a href='http://bbs.xjtu.edu.cn/BMY/con?B=%s&F=%s' target='_blank'>http://bbs.xjtu.edu.cn/BMY/con?B=%s&F=%s<a></td>\n",
				   board,fh2fname(x),board,fh2fname(x));
			printf("<TD width=10%% align=right><a href=\"#\" class=%s>top</a></TD>\n"
				"</TR></TBODY></TABLE><br />\n", odd_even_class);
			if (!found) {
				found = 1;
				firstnum = num - 1;
			}
			article_count++;
			count++;
			if (count >= w_info->t_lines) {
				nextstart = num;
				break;
			}
		}
	}
	MMAP_CATCH {
		found = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (found == 0)
		http_fatal("������ļ���");
	printf("[<a href='javascript:history.go(-1)'>������һҳ</a>]");
	printf("[<a href=%s%s&start=%d>��������</a>]", showByDefMode(), board,
	       firstnum - 4);
//add by landefeng@BMY for ��βҳ
	printf("[<a href=tfind?B=%s&th=%d>ͬ�����б�</a>]", board, thread);
	flpage = 0;
		printf("[<a href=bbstcon?board=%s&start=%d&th=%d>��ҳ</a>]",
			board, flpage, thread);
	if (nextstart)
		printf("[<a href=bbstcon?board=%s&start=%d&th=%d>��ҳ</a>]",
		       board, nextstart, thread);
	flpage = total - total%20;
	if (nextstart)
		printf("[<a href=bbstcon?board=%s&start=%d&th=%d>βҳ</a>]",
		       board, flpage, thread);
//

//	printf("</center></body>\n");
	processMath();
	printf("</table></td></tr></table></body>\n");
	brc_update(currentuser.userid);
	http_quit();
	return 0;
}

int
fshow_file(FILE * output, char *board, struct fileheader *x, int n)
{
	FILE *fp;
	char path[80], buf[512], *ptr,*bufptr;
	int ano = 0, nquote = 0, lastq = 0;
	char interurl[256];
	//add by macintosh 050619 for Tex Math Equ
	if ((x->accessed & FH_MATH)) {
		usedMath = 1;
		usingMath = 1;
		withinMath = 0;
	} else {
		usingMath = 0;
	}   
	sprintf(path, "boards/%s/%s", board, fh2fname(x));
	if (!w_info->doc_mode && !hideboard(board)
	    && (via_proxy || (wwwcache->accel_ip && wwwcache->accel_port))) {
		if (via_proxy)
			snprintf(interurl, sizeof (interurl),
				 "/" SMAGIC "/%s+%d", path,
				 x->edittime ? (x->edittime - x->filetime) : 0);
		else
			snprintf(interurl, sizeof (interurl),
				 "http://%s:%d/" SMAGIC "/%s+%d",
				 inet_ntoa(wwwcache->accel_addr),
				 wwwcache->accel_port,
				 path,
				 x->edittime ? (x->edittime - x->filetime) : 0);

		fprintf(output, "<script src=\"%s\"></script>", interurl);
		return 0;
	}
	fp = fopen(path, "r");
	if (fp == 0)
		return -1;
	fdisplay_attach(NULL, NULL, NULL, NULL);
	while (1) {
		if (fgets(buf, 500, fp) == 0)
			break;
		if (!strncmp(buf, "begin 644 ", 10)) {
			errlog("old attach %s", path);
			ano++;
			fdisplay_attach(output, fp, buf, fh2fname(x));
			fprintf(output, "\n<br>");
			continue;
		}
		if (!strncmp(buf, "beginbinaryattach ", 18)) {
			unsigned int len;
			char ch;
			char buf2[256], buf3[256];
			fread(&ch, 1, 1, fp);
			if (ch != 0) {
				ungetc(ch, fp);
				fhhprintf(output, "%s", buf);
				continue;
			}
			ptr = strchr(buf, '\r');
			if (ptr)
				*ptr = 0;
			ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			ano++;
			ptr = buf + 18;
			fread(&len, 4, 1, fp);
			len = ntohl(len);
			sprintf(buf2, "attach/bbscon/%s?B=%s&F=%s", ptr,
				board, fh2fname(x));
			sprintf(buf3, "%s/%s", board, fh2fname(x));
			fprintbinaryattachlink(output, ano, ptr,
					       -4 + (int) ftell(fp), len, buf2,
					       buf3);
			fseek(fp, len, SEEK_CUR);
			continue;
		}
		ptr = buf;
		if (!strncmp(buf, ": : ", 4) || !strncmp(buf, ": ����վ", 8) ||
		    !strncmp(buf, ": ��  ��", 8))
			continue;
		if (!strncmp(buf, ": ", 2)) {
			if (nquote > 3 || strlen(buf) < 4)
				continue;
			nquote++;
			if (!lastq)
				fprintf(output, "<font color=808080>");
			lastq = 1;
		} else {
			if (lastq)
				fprintf(output, "</font>");
			lastq = 0;
		}
		if (0 && !strncmp(buf, "�� �� ", 4))
			continue;
		if (!strncmp(buf, "--\n", 3)){
			fprintf(output, "<br>---<br>");
			bufptr=buf;
			while(1){
				if (fgets(buf, 500, fp) == 0){
					fhhprintf(output, "%s", bufptr);
					break;
					}
				else
					bufptr=buf;
			}
			break;
		}
		fhhprintf(output, "%s", buf);
		
	}
	if (lastq)
		fprintf(output, "</font>");
	fclose(fp);
	return 0;
}

int
show_file(char *board, struct fileheader *x, int n)
{
	char showfile[NAME_MAX + 1];
	char filename[NAME_MAX + 1];
	static char showpath[PATH_MAX + 1];
	struct stat st;
	char *ptr;
	FILE *fp;
	int fd;
	int level;
	brc_add_read(x);

	fshow_file(stdout, board, x, n);
	return 0;

	sprintf(filename, "boards/%s/%s", board, fh2fname(x));
	if (x->edittime == 0) {
		snprintf(showfile, NAME_MAX, "short:%s", filename);
	} else {
		snprintf(showfile, NAME_MAX, "short:%s_%d", filename,
			 x->edittime);
	}

	level = cachelevel(x->filetime, x->accessed & FH_ATTACHED);
	normalize(showfile);
	snprintf(showpath, PATH_MAX, "%s/%s", ATTACHCACHE, showfile);
//      f_append("stat4cache2", "show\n");
	if (access(showpath, R_OK)) {
		if (level < 2)
			return fshow_file(stdout, board, x, n);
		fp = fopen(showpath, "w");
		if (!fp)
			return fshow_file(stdout, board, x, n);
		fshow_file(fp, board, x, n);
		fclose(fp);
//              printf("<!-- make cache -->");
	}

	fd = open(showpath, O_RDONLY);
	if (fd < 0) {
		fshow_file(stdout, board, x, n);
		return -1;
	}
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)
	    || st.st_size <= 0) {
		close(fd);
		fshow_file(stdout, board, x, n);
		return -1;
	}
	MMAP_TRY {
		ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		if (ptr == (void *) -1) {
			fshow_file(stdout, board, x, n);
			MMAP_RETURN(-1);
		}
		fwrite(ptr, st.st_size, 1, stdout);
//      printf("<!-- show cache -->");
	} MMAP_CATCH {
		close(fd);
	}
	MMAP_END munmap(ptr, st.st_size);
	return 0;
}
