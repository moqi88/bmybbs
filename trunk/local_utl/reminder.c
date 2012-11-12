//by ecnegrevid 2001.9.29

#include "../include/bbs.h"
#define PASSWDFILE MY_BBS_HOME"/.PASSWDS"

int
compute_user_value(struct userec *urec)
{
	int value;

	/* if (urec) has XEMPT permission, don't kick it */
	if ((urec->userlevel & PERM_XEMPT)
	    || strcmp(urec->userid, "guest") == 0)
		return 999;
	value = (time(0) - urec->lastlogin) / 60;	/* min */
	/* new user should register in 30 mins */
	if (strcmp(urec->userid, "new") == 0)
		return (30 - value) * 60;
	if (urec->numlogins <= 3)
		return (15 * 1440 - value) / 1440;
	if (!(urec->userlevel & PERM_LOGINOK))
		return (30 * 1440 - value) / 1440;
	return (120 * 1440 - value) / 1440;
}

int
sendreminder(struct userec *urec)
{
	char cmd[1024], *ptr;
	time_t t;
	FILE *fp;
	if (strchr(urec->email, '@') == NULL)
		return;
	ptr = urec->email;
	while (*ptr) {
		if (!isalnum(*ptr) && !strchr(".@", *ptr))
			return;
		ptr++;
	}
	if (strcasestr(urec->email, ".bbs@ytht.") != NULL)
		return;
	sprintf(cmd, "mail -s 'ϵͳ���ѣ�" MY_BBS_NAME "��' %s", urec->email);
	fp = popen(cmd, "w");
	if (fp == NULL)
		return;
	fprintf(fp,
		"    " MY_BBS_NAME "(" MY_BBS_DOMAIN
		")���û����ã����ڱ�վע���\n"
		"�ʺ� %s �����������Ѿ����͵� 10����Ҫ����\n"
		"���ʺŵ�½һ�β���ʹ�������ָ���������ʻ�������\n"
		"��ע��ģ���������žͿ����ˡ�\n\n" "������������˵��:\n"
		"    ��BBSϵͳ�ϣ�ÿ���ʺŶ���һ�������������û���\n"
		"��¼������£�������ÿ�����1�������������ٵ�0��ʱ\n"
		"���ʺžͻ��Զ���ʧ���ʺ�ÿ�ε�¼���������ͻָ���\n"
		"һ���̶�ֵ������ͨ��ע������Ѿ���¼4�ε��û������\n"
		"�̶�ֵ������120������ͨ��ע�ᵫ��¼����4�ε��û���\n"
		"����̶�ֵ��30������δͨ��ע����û�������̶�ֵ��15��\n",
		urec->userid);
	pclose(fp);
	if ((fp = fopen(MY_BBS_HOME "/reminder.log", "a")) != NULL) {
		t = time(NULL);
		ptr = ctime(&t);
		ptr[strlen(ptr) - 1] = 0;
		fprintf(fp, "%s %s %s\n", ptr, urec->userid, urec->email);
		fclose(fp);
	}
	sleep(5);
}

main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	char buf[100];
	int size1 = sizeof (rec);

	if ((fd1 = open(PASSWDFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		if (!rec.userid[0])
			continue;
		if (compute_user_value(&rec) == 10)
			//printf("%s  \t%s\n", rec.userid, rec.email);
			sendreminder(&rec);
	}
	close(fd1);
}
