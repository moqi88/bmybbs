#include "bbslib.h"

int
bbsdenyall_main()
{
	int i;
	char board[80];
	struct boardmem *x;
	html_header(1);
	if (!loginok)
		http_fatal("����δ��¼, ���ȵ�¼");
	changemode(READING);
	strsncpy(board, getparm("board"), 30);
	if (!(x = getboard(board)))
		http_fatal("�����������");
	if (!has_BM_perm(&currentuser, x))
		http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
	printf("<center>\n");
	printf("%s -- �����û����� [������: %s]<hr><br>\n", BBSNAME, board);
	printf("���湲�� %d �˱���<br>", denynum);
	printf
	    ("<table border=1><tr><td>���<td>�û��ʺ�<td>����ԭ��<td>ԭ���������<td>����\n");
	for (i = 0; i < denynum; i++) {
		printf("<tr><td>%d", i + 1);
		printf("<td><a href=bbsqry?userid=%s>%s</a>", denyuser[i].id,
		       denyuser[i].id);
		printf("<td>%s\n", nohtml(denyuser[i].exp));
		printf("<td>%s\n", Ctime(denyuser[i].free_time) + 4);
		printf
		    ("<td>[<a onclick='return confirm(\"ȷʵ�����?\")' href=bbsdenydel?board=%s&userid=%s>���</a>]",
		     board, denyuser[i].id);
	}
	printf("</table><hr>\n");
	printf
	    ("[<a href=bbsdenyadd?board=%s>�趨�µĲ���POST�û�</a>]</center>\n",
	     board);
	http_quit();
	return 0;
}
