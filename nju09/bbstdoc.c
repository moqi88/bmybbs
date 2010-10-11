#include "bbslib.h"
char *stat1();

int
bbstdoc_main()
{
	char board[80], buf[128],only_for_b[80];
	struct boardmem *x1;
	struct fileheader *data = NULL;
	int i, start = 0, total2 = 0, total = 0, sum = 0, fd, size;
	changemode(READING);
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	x1 = getboard(board);
	if (x1 == 0) {
		html_header(1);
		nosuchboard(board, "bbstdoc");
	}
	updateinboard(x1);
	strcpy(board, x1->header.filename);
	sprintf(buf, "boards/%s/.DIR", board);
	if(cache_header(file_time(buf),10))
			return 0;
	html_header(1);
	check_msg();
	printf("<script src=/function.js></script>\n");
	size = file_size(buf);
	if (!size)
		http_fatal("本讨论区目前没有文章");
	fd = open(buf, O_RDONLY);
	if (fd < 0)
		http_fatal("本讨论区目前没有文章");
	MMAP_TRY {
		data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		if (data == (void *) -1) {
			MMAP_UNTRY;
			http_fatal("无法读取文章列表");
		}
		total = size / sizeof (struct fileheader);
		for (i = 0; i < total; i++)
			if (data[i].thread == data[i].filetime)
				total2++;
		start = getdocstart(total2, w_info->t_lines);
		printf("<nobr><center>\n");
// 	lanboy add next line
		printf("<body topmargin=0 leftmargin=0>\n");
//		printf("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n");
// 	lanboy
		printboardtop(x1, 3);
		printf("<table align=\"center\">主题模式 文章数[%d] 主题数[%d] ", total, total2);
		printf("<a href=bbsdoc?board=%s>一般模式</a> ", board);
		printf("<a href=bbspst?board=%s>发表文章</a> ", board);
		sprintf(buf, "bbstdoc?board=%s", board);
		bbsdoc_helper(buf, start, total2, w_info->t_lines);
		if (total <= 0) {
			munmap(data, size);
			MMAP_UNTRY;
			http_fatal("本讨论区目前没有文章");
		}
		printhr();
		printf("</table></table><table>\n");
		printf
		    ("<tr><td>序号<td>状态<td>作者<td>日期<td>标题<td>回帖/推荐度\n");
		for (i = 0; i < total; i++) {

			//判断是否有b标记
			if(data[i].accessed & FH_ALLREPLY)
 				strcpy(only_for_b,"style='color:red;' ");
			else
				strcpy(only_for_b,"");
			//
			
			if (data[i].thread != data[i].filetime)
				continue;
			sum++;
			if (sum < start)
				continue;
			printf("<tr><td>%d<td>%s<td>%s",
			       sum, flag_str(data[i].accessed),
			       userid_str(fh2owner(&data[i])));
			printf("<td>%6.6s", Ctime(data[i].filetime) + 4);
			printf
			    ("<td><a href=bbstcon?board=%s&start=%d&th=%d %s>○ %s </a><td>%s\n",
			     board, i, data[i].thread, only_for_b,void1(titlestr(data[i].title)),
			     stat1(data, i, total));
			if (sum > start + w_info->t_lines - 2)
				break;
		}
	}
	MMAP_CATCH {
		close(fd);
	}
	MMAP_END munmap(data, size);
	printf("</table>");
	printhr();
	printf("主题模式 文章数[%d] 主题数[%d] ", total, total2);
	printf("<a href=bbsdoc?board=%s>一般模式</a> ", board);
	printf("<a href=bbspst?board=%s>发表文章</a> ", board);
	sprintf(buf, "bbstdoc?board=%s", board);
	bbsdoc_helper(buf, start, total2, w_info->t_lines);
	printdocform("bbstdoc", board);
	http_quit();
	return 0;
}

char *
stat1(struct fileheader *data, int from, int total)
{
	static char buf[256];
	char *ptr = data[from].title;
	int i, re = 0, click = data[from].staravg50 * data[from].hasvoted / 50;
	for (i = from; i < total; i++) {
		if (!strncmp(ptr, data[i].title + 4, 40)) {
			re++;
			click += data[i].staravg50 * data[i].hasvoted / 50;
		}
	}
	sprintf(buf, "<font class=%s>%d</font>/<font class=%s>%d</font>",
		re > 9 ? "c31" : "c37", re, click ? "c31" : "c37", click);
	return buf;
}
