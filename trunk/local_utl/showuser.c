#include "bbs.h"
#define PASSWDFILE "PASS123"
main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	int size1 = sizeof (rec);

	if ((fd1 = open(PASSWDFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		if (strcmp(rec.userid, "forpass") == 0) {
			printf("����         : %s\n", rec.userid);
			printf("�ǳ�         : %s\n", rec.username);
			printf("��վ����     : %d ��\n", rec.numlogins);
			printf("������Ŀ     : %d\n", rec.numposts);
			printf("pass	   : %s\n", rec.passwd);
			break;
		}
	}
	close(fd1);
}
