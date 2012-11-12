#include "bbslib.h"

int
bbsdel_main()
{
	//��ԭ��
	struct fileheader f;
	struct userec *u;
	char dir[80], board[80], file[80], *id;
	int num = 0, filetime, total;
	struct boardmem *x;
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("���ȵ�¼");
	changemode(EDIT);
	id = currentuser.userid;
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("F"), 30);
	if (!file[0])
		strsncpy(file, getparm("file"), 20);
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		http_fatal("����Ĳ���");
	if (strstr(file, ".."))
		http_fatal("����Ĳ���");
	filetime = atoi(file + 2);
	x = getboard(board);
	if (x == 0)
		http_fatal("�������");
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("�������");
		}
		total = mf.size / sizeof (struct fileheader);
		num = Search_Bin(mf.ptr, filetime, 0, total - 1);
		if (num >= 0)
			memcpy(&f,mf.ptr+num*sizeof(struct fileheader) ,sizeof(struct fileheader));
	}
	MMAP_CATCH {
		num = -1;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (num >= 0) {
		if (strcmp(id, f.owner))
//                  && !has_BM_perm(&currentuser, x))
			http_fatal("����Ȩɾ������");
		if (!strcmp(board, "syssecurity"))
			http_fatal("��Ȩɾ��ϵͳ��¼");
		del_record(dir, sizeof (struct fileheader), num);
		cancelpost(board, currentuser.userid, &f,
			   !strcmp(currentuser.userid, f.owner));
		updatelastpost(board);
		printf
		    ("ɾ���ɹ�.<br><a href='%s%s&S=%d'>���ر�������</a>",
		     showByDefMode(), board, num - 10);
		if (!strcmp(id, f.owner)) {
			u = getuser(f.owner);
			if (x->header.clubnum == 0 && !junkboard(board) && u) {
				if (u->numposts > 0) {
					u->numposts--;
					save_user_data(u);
				}
			}
		}
		http_quit();
	}
	printf("�ļ�������, ɾ��ʧ��.<br>\n");
	printf("<a href='bbsdoc?board=%s'>���ر�������</a>", board);
	http_quit();
	return 0;
}
