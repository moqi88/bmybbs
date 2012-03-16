#include "bbslib.h"
int
bbsgcon_main()
{
	FILE *fp;
	char board[80], dir[80], file[80], filename[80], *ptr;
	struct fileheader x;
	int num, total = 0;
	struct fileheader *dirinfo = NULL;
	struct mmapfile mf = { ptr:NULL };
	changemode(READING);
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("F"), 32);
	if (!file[0])
		strsncpy(file, getparm("file"), 32);
	num = atoi(getparm("num"));
	if (getboard(board) == NULL)
		http_fatal("�����������");
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		http_fatal("����Ĳ���1");
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("����Ĳ���2");
	sprintf(dir, "boards/%s/.DIGEST", board);
	sprintf(filename, "boards/%s/%s", board, file);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("�������������ڻ���Ϊ��");
		}
		dirinfo = findbarticle(&mf, file, &num, 0);
		total = mf.size / sizeof (struct fileheader);
	}
	MMAP_CATCH {
		dirinfo = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (NULL == dirinfo)
		http_fatal("���²����ڻ��ѱ�ɾ��");
	if (*getparm("attachname") == '/') {
		showbinaryattach(filename);
		return 0;
	}
	html_header(1);
	check_msg();
	changemode(READING);
	printf("<body><center>\n");
	printf("%s -- �����Ķ� [������: %s]<hr>", BBSNAME, board);
	showcon(filename);
	printf("[<a href=bbsboa>����������</a>]");
	printf("[<a href=bbsall>ȫ��������</a>]");
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("dir error2");
	if (num > 0) {
		fseek(fp, sizeof (x) * (num - 1), SEEK_SET);
		fread(&x, sizeof (x), 1, fp);
		printf("[<a href=bbsgcon?board=%s&file=%s&num=%d>��һƪ</a>]",
		       board, fh2fname(&x), num - 1);
	}
	printf("[<a href=%s%s>��������</a>]", showByDefMode(), board);
	if (num < total - 1) {
		fseek(fp, sizeof (x) * (num + 1), SEEK_SET);
		fread(&x, sizeof (x), 1, fp);
		printf("[<a href=bbsgcon?board=%s&file=%s&num=%d>��һƪ</a>]",
		       board, fh2fname(&x), num + 1);
	}
	fclose(fp);
	ptr = dirinfo->title;
	if (!strncmp(ptr, "Re: ", 4))
		ptr += 4;
	printf("[<a href='bbstfind?board=%s&th=%d'>ͬ�����Ķ�</a>]\n",
	       board, dirinfo->thread);
	printf("</center></body>\n");
	http_quit();
	return 0;
}
