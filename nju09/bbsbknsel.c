#include "bbslib.h"

int
bbsbknsel_main()
{
	FILE *fp;
	char board[80], dir[160], genbuf[STRLEN];
	struct boardmem *x1;
	struct bknheader x;
	int i, start, total;
	html_header(1);
	printf("<script src=/function.js></script>\n");
	check_msg();
	changemode(SELBACKNUMBER);
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	x1 = getboard(board);
	if (x1 == 0)
		nosuchboard(board, "bbsbknsel");
	else {
		strcpy(board, x1->header.filename);
		sprintf(dir, "boards/.backnumbers/%s/.DIR", board);
		fp = fopen(dir, "r");
		if (fp == NULL)
			http_fatal("�����������");
		total = file_size(dir) / sizeof (struct bknheader);
		start = getdocstart(total, w_info->t_lines);
		printf("<body topmargin=0><nobr><center>\n");
		printboardtop(x1, 5);
		printf("ѡ����� ������[%d] ", total);
		sprintf(genbuf, "bbsbknsel?board=%s", board);
		bbsdoc_helper(genbuf, start, total, w_info->t_lines);
		printhr();
		if (total <= 0) {
			fclose(fp);
			http_fatal("��������Ŀǰû�й���");
		}
		printf("<table>\n");
		printf
		    ("<tr><td>���</td><td>������</td><td>��������</td><td>���⡡��������������������������</td></tr>\n");
		if (fp) {
			fseek(fp, (start - 1) * sizeof (struct bknheader),
			      SEEK_SET);
			for (i = 0; i < w_info->t_lines; i++) {
				if (fread(&x, sizeof (x), 1, fp) <= 0)
					break;
				printf("<tr><td>%d</td><td>%s</td>",
				       start + i, x.boardname);
				if (!i)
					printf("<td><nobr>%12.12s</td>",
					       Ctime(x.filetime) + 4);
				else
					printf("<td>%12.12s</td>",
					       Ctime(x.filetime) + 4);
				printf
				    ("<td><a href=bbsbkndoc?board=%s&bkn=%s&num=%d>�� %s </a></td></tr>",
				     board, bknh2bknname(&x), start + i - 1,
				     void1(titlestr(x.title)));
			}
			printf("</table>");
			printhr();
			fclose(fp);
		}
		printf("ѡ����� ������[%d] ", total);
		sprintf(genbuf, "bbsbknsel?board=%s", board);
		bbsdoc_helper(genbuf, start, total, w_info->t_lines);
		printdocform("bbsbknsel", board);
		http_quit();
	}
	return 0;
}
