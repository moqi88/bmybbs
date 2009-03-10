#include "bbslib.h"

int
bbsfwd_main()
{
	struct fileheader *x = NULL;
	char board[80], file[80], target[80], dir[80];
	int num;
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("F"), 30);
	if (!file[0])
		strsncpy(file, getparm("file"), 30);
	strsncpy(target, getparm("target"), 30);
	if (!loginok || isguest)
		http_fatal("�Ҵҹ��Ͳ��ܽ��б������");
	changemode(SMAIL);
	if (!getboard(board))
		http_fatal("�����������");
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("�����������");
		}
		num = -1;
		x = findbarticle(&mf, file, &num, 1);
	}
	MMAP_CATCH {
		x = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (x == 0)
		http_fatal("������ļ���");
	printf("<center>%s -- ת��/�Ƽ������� [ʹ����: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	if (target[0]) {
		if (!strstr(target, "@")) {
			if (!getuser(target))
				http_fatal("�����ʹ�����ʺ�");
			strcpy(target, getuser(target)->userid);
		}
		return do_fwd(x, board, target);
	}
	printf("<table><tr><td>\n");
	printf("���±���: %s<br>\n", nohtml(x->title));
	printf("��������: %s<br>\n", fh2owner(x));
	printf("ԭ������: %s<br>\n", board);
	printf("<form action=bbsfwd method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf
	    ("������ת�ĸ� <input name=target size=30 maxlength=30 value=%s> (������Է���id��email��ַ). <br>\n",
	     currentuser.email);
	printf("<input type=submit value=ȷ��ת��></form>");
	return 0;
}

int
do_fwd(struct fileheader *x, char *board, char *target)
{
	char title[512], path[200];
	sprintf(path, "boards/%s/%s", board, fh2fname(x));
	if (!file_exist(path))
		http_fatal("�ļ������Ѷ�ʧ, �޷�ת��");
	sprintf(title, "[ת��] %s", x->title);
	title[60] = 0;
	post_mail(target, title, path, currentuser.userid, currentuser.username,
		  fromhost, -1, 0);
	printf("������ת�ĸ�'%s'<br>\n", nohtml(target));
	printf("[<a href='javascript:history.go(-2)'>����</a>]");
	return 0;
}
