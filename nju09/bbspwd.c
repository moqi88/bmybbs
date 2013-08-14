#include "bbslib.h"


// ����һ����Ϊlen������ַ���
char *random_str(char *buf, int len)
{
	srand((unsigned)time(NULL));
	int i;
	for (i = 0; i < len; i++)
		buf[i] = rand()%10 + '0';
	buf[len] = '\0';
	return buf;
}

// ����ʼ���ʽ����Ч��
// ����1Ϊ��Ч
int vaild_mail(const char *email)
{
	if (!strchr(email, '@'))
		return 0;
	if (strstr(email, ".bbs"))
		return 0;
	return 1;
}

int
bbspwd_main()
{	//modify by mintbaggio 20040829 for new www
	int type;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("����δ��¼, ���ȵ�¼");
	changemode(GMENU);

	int mode = atoi(getparm("mode"));

	//�޸ı�������
	if (mode == 1)
	{
		char pw1[20], pw2[20], pw3[20], salt[3];
		printf("<body>");
		type = atoi(getparm("type"));
		if (type == 0) {
			printf("<div class=rhead>%s -- �޸����� [�û�: <span class=h11>%s</span>]</div><hr>\n",
				   BBSNAME, currentuser.userid);
			printf("<form action=bbspwd?mode=1&type=1 method=post>\n");
			printf
				("��ľ�����: <input maxlength=12 size=12 type=password name=pw1><br>\n");
			printf
				("���������: <input maxlength=12 size=12 type=password name=pw2><br>\n");
			printf
				("������һ��: <input maxlength=12 size=12 type=password name=pw3><br><br>\n");
			printf("<input type=submit value=ȷ���޸�>\n");
			printf("</body>");
			http_quit();
		}
		strsncpy(pw1, getparm("pw1"), 13);
		strsncpy(pw2, getparm("pw2"), 13);
		strsncpy(pw3, getparm("pw3"), 13);
		if (strcmp(pw2, pw3))
			http_fatal("�������벻��ͬ");
		if (strlen(pw2) < 2)
			http_fatal("������̫��");
		if (!checkpasswd(currentuser.passwd, pw1))
			http_fatal("���벻��ȷ");
		getsalt(salt);
		strcpy(currentuser.passwd, crypt1(pw2, salt));
		save_user_data(&currentuser);
		printf("[%s] �����޸ĳɹ�.", currentuser.userid);
	}
	//�һ���������
	else if (mode == 2)
	{
		struct userec *uptr;
		char id[32];
		char code[11];
		char buf[512];
		printf("<body>");
		type = atoi(getparm("type"));
		if (type == 0) {
			printf("<div class=rhead>%s -- �������һ�����</div><hr>\n", BBSNAME);
			
			printf("<form action=bbspwd?mode=2&type=1 method=post>\n");
			printf("[1] ��һ�����������һ������ID: <input maxlength=12 size=12 name=id><br>\n");
			printf("<input type=submit value=������֤�뵽��������>\n");
			printf("</form>");

			printf("<form action=bbspwd?mode=2&type=2 method=post>\n");
			printf("[2] �ڶ�����������֤��: <input maxlength=12 size=12 name=code><br>\n");
			printf("<input type=submit value=��������>\n");
			printf("</form>");

			printf("</body>");
			http_quit();
		}
		else if (type == 1) 
		{
			strsncpy(id, getparm("id"), 32);
			if (!(uptr=getuser(id))) 
				http_fatal("�����ʹ���ߴ���");
			if (!vaild_mail(uptr->email))
				http_fatal("�����ʽ��Ч������ϵվ���ֶ���������");
			
			random_str(code, 10);

			sprintf(buf, MY_BBS_HOME "/etc/findpasswd");
			int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
			FILE *fp = fopen(buf, "a");
			if (fp == NULL) 
			{
				close(lockfd);
				http_fatal("�޷����һ������ļ�������ϵվ��");
			}
			sprintf(buf, "%s %s\n", code, id);
			fputs(buf, fp);
			fclose(fp);
			close(lockfd);

			sprintf(buf, MY_BBS_HOME "/bin/sendmail.py '%s' 'Authentication Code, From %s@bmy' '%s'", uptr->email, currentuser.userid, code);
			int ret = system(buf);
			if (ret != 0)
				http_fatal("�����ʼ�ʧ�ܣ������Ի���ϵվ��");

			char titbuf[64];
			sprintf(titbuf, "�һ������һ��: %s", id);
			sprintf(buf, "UserId: %s\n������Id: %s\nE-Mail: %s\n", id, currentuser.userid, uptr->email);
			securityreport(titbuf, buf);

			printf("��֤���Ѿ����͵� [%s] �У������ţ����ڵڶ�������д��֤�롣\n", uptr->email);
			//printf("%s\n", random_str(code, 10));
		}
		else if (type == 2)
		{
			strsncpy(code, getparm("code"), 11);

			int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
			sprintf(buf, MY_BBS_HOME "/etc/findpasswd");
			FILE *fp1 = fopen(buf, "r");
			sprintf(buf, MY_BBS_HOME "/etc/findpasswd_tmp");
			FILE *fp2 = fopen(buf, "w");
			if (fp1 == NULL || fp2 == NULL) 
			{
				close(lockfd);
				http_fatal("�޷����һ������ļ�������ϵվ��");
			}

			int isfind = 0;
			while(fgets(buf, 512, fp1) != NULL)
			{
				char idtmp[32];
				char codetmp[11];
				strncpy(codetmp, buf, 11);
				codetmp[10] = '\0';
				strncpy(idtmp, buf+11, 32);
				idtmp[strlen(idtmp) - 1] = '\0';

				if (strcmp(code, codetmp) == 0)
				{
					isfind = 1;
					strncpy(id, idtmp, 32);
				}
				else
				{
					fputs(buf, fp2);
				}
			}

			fclose(fp1);
			fclose(fp2);
			rename(MY_BBS_HOME "/etc/findpasswd_tmp", MY_BBS_HOME "/etc/findpasswd");
			close(lockfd);

			char titbuf[64];
			if (isfind)
			{
				if (!(uptr=getuser(id))) 
					http_fatal("�����ʹ���ߴ���");
				
				char newpw[20];

				random_str(newpw, 4);
				//getsalt(salt);
				strcpy(uptr->passwd, genpasswd(newpw));
				save_user_data(uptr);
				
				sprintf(titbuf, "�һ�����ڶ���[�ɹ�]: %s", id);
				sprintf(buf, "UserId: %s\n������Id: %s\n", id, currentuser.userid);
				securityreport(titbuf, buf);

				printf("[%s] �����޸ĳɹ���\n������Ϊ: %s����������¼�޸����룡", uptr->userid, newpw);
			}
			else
			{
				sprintf(titbuf, "�һ�����ڶ���[ʧ��]");
				sprintf(buf, "������Id: %s\n", currentuser.userid);
				securityreport(titbuf, buf);
				http_fatal("��֤����Ч");
			}
		}
	}

	return 0;
}
