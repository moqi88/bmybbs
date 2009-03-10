#include "bbslib.h"

int
bbsbrdadd_main()
{
	FILE *fp;
	char file[200], board[200];
	int i;
	html_header(1);
	changemode(ZAP);
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	if (!loginok)
		http_fatal("��ʱ��δ��¼��������login");
	readmybrd(currentuser.userid);
	if (mybrdnum >= GOOD_BRC_NUM)
		http_fatal("��Ԥ����������Ŀ�Ѵ����ޣ���������Ԥ��");
	if (ismybrd(board))
		http_fatal("���Ѿ�Ԥ�������������");
	if (!getboard(board))
		http_fatal("��������������");
	strcpy(mybrd[mybrdnum], board);
	mybrdnum++;
	sethomefile(file, currentuser.userid, ".goodbrd");
	fp = fopen(file, "w");
	if (fp) {
		for (i = 0; i < mybrdnum; i++)
			fprintf(fp, "%s\n", mybrd[i]);
		fclose(fp);
	} else
		http_fatal("Can't save");
	//printf("<script>top.f2.location='bbsleft?t=%d'</script>\n", now_t);
	printf("<script>top.f2.location.reload();</script>\n");
	printf
	    ("Ԥ���������ɹ�<br><a href='javascript:history.go(-1)'>���ٷ���</a>");
	http_quit();
	return 0;
}
