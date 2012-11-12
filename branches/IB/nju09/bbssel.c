#include "bbslib.h"

int
bbssel_main()
{
	char *board, buf[80], *board1, *title;
	int i, total = 0;
	html_header(1);
	check_msg();
	changemode(SELECT);
	board = nohtml(getparm("board"));
	if (board[0] == 0) {
		printf("%s -- ѡ��������<hr>\n", BBSNAME);
		printf("<form action=bbssel>\n");
		printf("����������: <input type=text name=board>");
		printf(" <input type=submit value=ȷ��>");
		printf("</form>\n");
		http_quit();
	} else {
		struct boardmem *x;
		x = getboard(board);
		if (x) {
			sprintf(buf, "%s%s", showByDefMode(), x->header.filename);
			redirect(buf);
			http_quit();
		}
		printf("%s -- ѡ��������<hr>\n", BBSNAME);
		printf("�Ҳ������������, ", board);
		printf("�����к���'%s'����������: <br><br>\n", board);
		printf("<table>");
		for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
			board1 = shm_bcache->bcache[i].header.filename;
			title = shm_bcache->bcache[i].header.title;
			if (!has_read_perm(&currentuser, board1))
				continue;
			if (strcasestr(board1, board)
			    || strcasestr(title, board)) {
				total++;
				printf("<tr><td>%d", total);
				printf
				    ("<td><a href=%s%s>%s</a><td>%s<br>\n",
				     showByDefMode(), board1, board1, title + 7);
			}
		}
		printf("</table><br>\n");
		printf("���ҵ�%d������������������.\n", total);
	}
}
