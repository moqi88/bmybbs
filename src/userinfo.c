/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    Copyright (C) 1999, Zhou Lin, kcn@cic.tsinghua.edu.cn
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"


#ifdef POP_CHECK
// ��½�ʼ��������õ�ͷ�ļ� added by interma@BMY 2005.5.12
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include "identify.h"
// �ʼ����������û���������ĳ��ȣ� added by interma@BMY 2005.5.12
#define USER_LEN 20
#define PASS_LEN 20
#endif


extern time_t login_start_time;
extern char fromhost[60];

static void getfield(int line, char *info, char *desc, char *buf, int len);

#ifdef POP_CHECK
// ��½�ʼ������������������֤�� added by interma@BMY 2005.5.12
/* ����ֵΪ1��ʾ��Ч��0��ʾ��Ч, -1��ʾ��pop���������ӳ��� */
static int test_mail_valid(char *user, char *pass, char *popip)
{
    char buffer[512]; 
    int sockfd;
    struct sockaddr_in server_addr; 
    struct hostent *host; 
	
	if (user[0] == ' ' || pass[0] == ' ')
		return 0;

    /* �ͻ�����ʼ���� sockfd������ */ 
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) 
    {  
        return -1;
    } 
    int i;
    for ( i = 0; i < 8; i++)
        server_addr.sin_zero[i] = 0;
    server_addr.sin_family=AF_INET; 
    server_addr.sin_port=htons(110);
    // 202.117.1.22 == stu.xjtu.edu.cn 
    if(inet_aton(popip, &server_addr.sin_addr) == 0) 
    {  
        return -1;
    }

    /* �ͻ��������������� */ 
    if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
    {  
        return -1; 
    } 

    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
        return -1;
    
    sprintf(buffer, "USER %s\r\n\0", user);
    if (write(sockfd, buffer, strlen(buffer)) == -1)
    { 
        return -1; 
    }   
     
    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
    {   
        return 0;
    }   
     
    sprintf(buffer, "PASS %s\r\n\0", pass);
    if (write(sockfd, buffer, strlen(buffer)) == -1)
    { 
        return -1; 
    }
     
    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
    {
        return 0;
    }
            
    write(sockfd, "QUIT\r\n", strlen("QUIT\r\n"));          
    return 1;
}    

void securityreport(char *str, char *content);

// ��username�û�ͨ����֤�� added by interma@BMY 2005.5.12
static void register_success(int usernum, char *userid, char *realname, char *dept, 
char *addr, char *phone, char *assoc, char *email)
{
	struct userec uinfo;
	FILE *fout, *fn;
	char buf[STRLEN];
	int n;

	//int id = getuser(userid);
	usernum = getuser(userid);

	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);

	memcpy(&uinfo, &lookupuser, sizeof (uinfo));

			strsncpy(uinfo.userid, userid,
				 sizeof (uinfo.userid));
			strsncpy(uinfo.realname, realname,
				 sizeof (uinfo.realname));
			strsncpy(uinfo.address, addr,
				 sizeof (uinfo.address));
			sprintf(genbuf, "%s$%s@%s", dept, phone, userid);
			strsncpy(uinfo.realmail, genbuf,
				 sizeof (uinfo.realmail));

			strsncpy(uinfo.email, email, sizeof (uinfo.email));

			uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
			substitute_record(PASSFILE, &uinfo, sizeof (struct userec), usernum);

			sethomefile(buf, uinfo.userid, "sucessreg");
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "\n");
				fclose(fout);
			}

			sethomefile(buf, uinfo.userid, "register");
	
			if ((fout = fopen(buf, "w")) != NULL) 
			{
				
				fprintf(fout, "%s: %d\n", "usernum", usernum);
				fprintf(fout, "%s: %s\n", "userid", userid);
				fprintf(fout, "%s: %s\n", "realname", realname);
				fprintf(fout, "%s: %s\n", "dept", dept);
				fprintf(fout, "%s: %s\n", "addr", addr);
				fprintf(fout, "%s: %s\n", "phone", phone);
				fprintf(fout, "%s: %s\n", "assoc", assoc);

				n = time(NULL);
				fprintf(fout, "Date: %s",
					ctime((time_t *) & n));
				fprintf(fout, "Approved: %s\n", userid);
				fclose(fout);
			}

			mail_file("etc/s_fill", uinfo.userid,
				  "������ͨ�������֤"); // ����ط��и�覴ã����Ƿ�����Ϊ����

			mail_file("etc/s_fill2", uinfo.userid,
				  "��ӭ����" MY_BBS_NAME "���ͥ");
			sethomefile(buf, uinfo.userid, "mailcheck");
			unlink(buf);
			sprintf(genbuf, "�� %s ͨ�����ȷ��.", uinfo.userid);
			securityreport(genbuf, genbuf);

	return ;
}

// username�û���֤ʧ�ܵĴ�����������û�������Ŀǰ��δʹ������������� added by interma@BMY 2005.5.16
void register_fail(char *userid)
{
	int id;
	strcpy(genbuf, userid);
	id = getuser(genbuf);

	if (lookupuser.userid[0] == '\0' || !strcmp(lookupuser.userid, "SYSOP")) {
		return;
	}

	sprintf(genbuf, "mail/%c/%s", mytoupper(lookupuser.userid[0]),
		lookupuser.userid);
	deltree(genbuf);
	sprintf(genbuf, "home/%c/%s", mytoupper(lookupuser.userid[0]),
		lookupuser.userid);
	deltree(genbuf);
	lookupuser.userlevel = 0;
	strcpy(lookupuser.address, "");
	strcpy(lookupuser.username, "");
	strcpy(lookupuser.realname, "");
	strcpy(lookupuser.ip, "");
	strcpy(lookupuser.realmail, "");
	lookupuser.userid[0] = '\0';
	substitute_record(PASSFILE, &lookupuser, sizeof (lookupuser), id);
	setuserid(id, lookupuser.userid);
}

char * str_to_upper(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0)
	{
		*str = toupper(*str);
		str++;
	}
	return h;
}

extern char fromhost[60];
// ��¼pop�������ϵ��û�������ֹ�ظ�ע����id�� added by interma@BMY 2005.5.16
/* ����ֵΪ0��ʾ�Ѽ�¼��δ���ڣ���1��ʾ�Ѵ��� */
int write_pop_user(char *user, char *userid, char *pop_name)
{	
	FILE *fp;
	char buf[256];
	char path[256];
	int isprivilege = 0; 

	char username[USER_LEN + 2];
	sprintf(username, "%s\n", user);

	// ���Ƚ�����Ȩ�û���privilege������
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s_privilege" , pop_name);

	fp = fopen(path, "r");
	if (fp != NULL)
	{
		while(fgets(buf, 256, fp) != NULL)
		{
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0)
			{
				isprivilege = 1;
				break;
			}
		}
			
		fclose(fp);
	}


	// ���½�����ͨ�û�����
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s", pop_name);

	int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX); // ��������֤�������
	
	fp = fopen(path, "a+");

	if (fp == NULL) 
	{	
		close(lockfd);
		return 0;
	}

	if (isprivilege == 0)
	{
		fseek(fp, 0, SEEK_SET);
		while(fgets(buf, 256, fp) != NULL)
		{
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0)
			{
				fclose(fp);
				close(lockfd);
				return 1;
			}
			fgets(buf, 256, fp);
		}
	}

	fseek(fp, 0, SEEK_END);
	fputs(user, fp);

	time_t t;
	time(&t);

	sprintf(buf, "\n%s : %s : %s", userid, fromhost, ctime(&t));
	fputs(buf, fp);

	fclose(fp);
	close(lockfd);
	return 0;
}
#endif
// -------------------------------------------------------------------------------

void
permtostr(perm, str)
int perm;
char *str;
{
	int num;
	strcpy(str, "bTCPRp#@XWBA#VS-DOM-F012345678");
	for (num = 0; num < 30; num++)
		if (!(perm & (1 << num)))
			str[num] = '-';
	str[num] = '\0';
}

void
disply_userinfo(u, real)
struct userec *u;
int real;
{
	struct stat st;
	#ifdef __LP64
	long diff, num;
	#else
	int diff, num;
	#endif
	
	int exp;

	move(real == 1 ? 2 : 3, 0);
	clrtobot();
	prints("���Ĵ���     : %s\n", u->userid);
	prints("�����ǳ�     : %s\n", u->username);
	prints("��ʵ����     : %s\n", u->realname);
	prints("��ססַ     : %s\n", u->address);
	prints("�����ʼ����� : %s\n", u->email);
	if (real) {
		prints("��ʵ E-mail  : %s\n", u->realmail);
//        if HAS_PERM(PERM_ADMINMENU)
//           prints("Ident ����   : %s\n", u->ident );
	}
	prints("����ָ��     : %s\n", u->ip);
	prints("�ʺŽ������� : %s", ctime(&u->firstlogin));
	prints("����������� : %s", ctime(&u->lastlogin));
	if (real) {
		prints("������ٻ��� : %s\n", u->lasthost);
	}
	prints("�ϴ���վʱ�� : %s",
	       u->lastlogout ? ctime(&u->lastlogout) : "����\n");
	prints("��վ����     : %d ��\n", u->numlogins);
	if (real) {
		prints("������Ŀ     : %d\n", u->numposts);
	}
	exp = countexp(u);
	prints("����ֵ       : %d(%s)\n", exp, cexp(exp));
	exp = countperf(u);
	prints("����ֵ       : %d(%s)\n", exp, cperf(exp));
	prints("��վ��ʱ��   : %d Сʱ %d ����\n", u->stay / 3600,
	       (u->stay / 60) % 60);
	sprintf(genbuf, "mail/%c/%s/%s", mytoupper(u->userid[0]), u->userid,
		DOT_DIR);
	if (stat(genbuf, &st) >= 0)
		num = st.st_size / (sizeof (struct fileheader));
	else
		num = 0;
	#ifdef __LP64
	prints("˽������     : %ld ��\n", num);
	#else
	prints("˽������     : %d ��\n", num);
	#endif

	if (real) {
		//modified by pzhg 071005
		strcpy(genbuf, "bTCPRp#@XWBA#VS-DOM-F0s23456789H");
		for (num = 0; num < strlen(genbuf); num++)
			if (!(u->userlevel & (1 << num)))
				genbuf[num] = '-';
		//genbuf[num] = '\0';
		//permtostr(u->userlevel, genbuf);
		prints("ʹ����Ȩ��   : %s\n", genbuf);
	} else {
		diff = (time(0) - login_start_time) / 60;
		#ifdef __LP64
		prints("ͣ���ڼ�     : %ld Сʱ %02ld ��\n", diff / 60,
		       diff % 60);
		#else
		prints("ͣ���ڼ�     : %d Сʱ %02d ��\n", diff / 60,
		       diff % 60);
		#endif
		prints("өĻ��С     : %dx%d\n", t_lines, t_columns);
	}
	prints("\n");
	if (u->userlevel & PERM_LOGINOK) {
		prints("  ����ע������Ѿ����, ��ӭ���뱾վ.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		prints("  ������·, ���Ķ� Announce ������.\n");
	} else {
		prints("  ע����δ�ɹ�, ��ο���վ��վ����˵��.\n");
	}
}

int
uinfo_query(u, real, unum)
struct userec *u;
int real, unum;
{
	struct userec newinfo;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	int i, fail = 0, netty_check = 0;
	FILE *fin, *fout, *dp;
	time_t code;

	memcpy(&newinfo, u, sizeof (currentuser));
	getdata(t_lines - 1, 0, real ?
		"��ѡ�� (0)���� (1)�޸����� (2)�趨���� (3) �� ID ==> [0]" :
		"��ѡ�� (0)���� (1)�޸����� (2)�趨���� (3) ѡǩ���� ==> [0]",
		ans, 2, DOECHO, YEA);
	clear();

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		prints("ʹ���ߴ���: %s\n", u->userid);

	switch (ans[0]) {
	case '1':
		move(1, 0);
		prints("�������޸�,ֱ�Ӱ� <ENTER> ����ʹ�� [] �ڵ����ϡ�\n");

		sprintf(genbuf, "�ǳ� [%s]: ", u->username);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.username, buf, NAMELEN);
		if (!real && buf[0])
			strncpy(uinfo.username, buf, 40);

		sprintf(genbuf, "��ʵ���� [%s]: ", u->realname);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.realname, buf, NAMELEN);

		sprintf(genbuf, "��ס��ַ [%s]: ", u->address);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.address, buf, NAMELEN);

		#ifndef POP_CHECK  // ���ø������ַ�ˣ���ΪҪ��
		sprintf(genbuf, "�������� [%s]: ", u->email);
		getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
		if (buf[0]) {
			strncpy(newinfo.email, buf, STRLEN);
		}
		#endif

		sprintf(genbuf, "����ָ�� [%s]: ", u->ip);
		getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.ip, buf, 16);

		if (real) {
			sprintf(genbuf, "��ʵEmail[%s]: ", u->realmail);
			getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
			if (buf[0])
				strncpy(newinfo.realmail, buf, STRLEN - 16);

			sprintf(genbuf, "���ߴ��� [%d]: ", u->numlogins);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.numlogins = atoi(buf);

			sprintf(genbuf, "������Ŀ [%d]: ", u->numposts);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.numposts = atoi(buf);

			sprintf(genbuf, "��վСʱ�� [%ld Сʱ %ld ����]: ",
				(long int) (u->stay / 3600),
				(long int) ((u->stay / 60) % 60));
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.stay = atoi(buf) * 3600;

		}

		break;
	case '2':
		if (!real) {
			getdata(i++, 0, "������ԭ����: ", buf, PASSLEN, NOECHO,
				YEA);
			if (*buf == '\0' || !checkpasswd(u->passwd, buf)) {
				prints("\n\n�ܱ�Ǹ, ����������벻��ȷ��\n");
				fail++;
				break;
			}
		}
		getdata(i++, 0, "���趨������: ", buf, PASSLEN, NOECHO, YEA);
		if (buf[0] == '\0') {
			prints("\n\n�����趨ȡ��, ����ʹ�þ�����\n");
			fail++;
			break;
		}
		strncpy(genbuf, buf, PASSLEN);

		getdata(i++, 0, "����������������: ", buf, PASSLEN, NOECHO,
			YEA);
		if (strncmp(buf, genbuf, PASSLEN)) {
			prints("\n\n������ȷ��ʧ��, �޷��趨�����롣\n");
			fail++;
			break;
		}
		buf[8] = '\0';
		strncpy(newinfo.passwd, genpasswd(buf), PASSLEN);
		break;
	case '3':
		if (!real) {
			sprintf(genbuf, "Ŀǰʹ��ǩ���� [%d]: ", u->signature);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.signature = atoi(buf);
		} else {
			getdata(i++, 0, "�µ�ʹ���ߴ���: ", genbuf, IDLEN + 1,
				DOECHO, YEA);
			if (*genbuf != '\0') {
				if (getuser(genbuf)) {
					prints
					    ("\n����! �Ѿ���ͬ�� ID ��ʹ����\n");
					fail++;
				} else if (!goodgbid(genbuf)) {
					prints("\n����! ���Ϸ��� ID\n");
					fail++;
				} else {
					strncpy(newinfo.userid, genbuf,
						IDLEN + 2);
				}
			}
		}
		break;
	default:
		clear();
		return 0;
	}
	if (fail != 0) {
		pressreturn();
		clear();
		return 0;
	}
	if (askyn("ȷ��Ҫ�ı���", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			sprintf(secu, "�޸� %s �Ļ������ϻ����롣", u->userid);
			securityreport(secu, secu);
		}
		if (strcmp(u->userid, newinfo.userid)) {

			sprintf(src, "mail/%c/%s", mytoupper(u->userid[0]),
				u->userid);
			sprintf(dst, "mail/%c/%s", mytoupper(newinfo.userid[0]),
				newinfo.userid);
			rename(src, dst);
			sethomepath(src, u->userid);
			sethomepath(dst, newinfo.userid);
			rename(src, dst);
			sethomefile(src, u->userid, "register");
			unlink(src);
			sethomefile(src, u->userid, "register.old");
			unlink(src);
			setuserid(unum, newinfo.userid);
		}
		if (!strcmp(u->userid, currentuser.userid)) {
			extern int WishNum;
			strncpy(uinfo.username, newinfo.username, NAMELEN);
			WishNum = 9999;
		}
/* added by netty to automatically send a mail to new user. */

		if ((netty_check == 1)) {
			sprintf(genbuf, "%s", email_domain());
			if ((sysconf_str("EMAILFILE") != NULL) &&
			    (!strstr(newinfo.email, genbuf)) &&
			    (!invalidaddr(newinfo.email)) &&
			    (!invalid_email(newinfo.email))) {
				randomize();
				code = (time(0) / 2) + (rand() / 10);
				sethomefile(genbuf, u->userid, "mailcheck");
				if ((dp = fopen(genbuf, "w")) == NULL) {
					fclose(dp);
					return -2;
				}
				fprintf(dp, "%9.9ld\n", (long int) code);
				fclose(dp);
				sprintf(genbuf,
					"/usr/lib/sendmail -f %s.bbs@%s %s ",
					u->userid, email_domain(),
					newinfo.email);
				fout = popen(genbuf, "w");
				fin = fopen(sysconf_str("EMAILFILE"), "r");
				if (fin == NULL || fout == NULL)
					return -1;
				fprintf(fout, "Reply-To: SYSOP.bbs@%s\n",
					email_domain());
				fprintf(fout, "From: SYSOP.bbs@%s\n",
					email_domain());
				fprintf(fout, "To: %s\n", newinfo.email);
				fprintf(fout,
					"Subject: @%s@[-%9.9ld-]%s mail check.\n",
					u->userid, (long int) code, MY_BBS_ID);
				fprintf(fout, "X-Forwarded-By: SYSOP \n");
				fprintf(fout,
					"X-Disclaimer: %s registration mail.\n",
					MY_BBS_ID);
				fprintf(fout, "\n");
				fprintf(fout, "BBS LOCATION     : %s (%s)\n",
					email_domain(), MY_BBS_IP);
				fprintf(fout, "YOUR BBS USER ID : %s\n",
					u->userid);
				fprintf(fout, "APPLICATION DATE : %s",
					ctime(&u->firstlogin));
				fprintf(fout, "LOGIN HOST       : %s\n",
					fromhost);
				fprintf(fout, "YOUR NICK NAME   : %s\n",
					u->username);
				fprintf(fout, "YOUR NAME        : %s\n",
					u->realname);
				while (fgets(genbuf, 255, fin) != NULL) {
					if (genbuf[0] == '.'
					    && genbuf[1] == '\n')
						fputs(". \n", fout);
					else
						fputs(genbuf, fout);
				}
				fprintf(fout, ".\n");
				fclose(fin);
				pclose(fout);
			} else {
				if (sysconf_str("EMAILFILE") != NULL) {
					move(t_lines - 5, 0);
					prints
					    ("\n������ĵ����ʼ���ַ ��[1;33m%s[m��\n",
					     newinfo.email);
					prints
					    ("���ǺϷ�֮ UNIX �ʺţ�ϵͳ����Ͷ��ע���ţ������������...\n");
					pressanykey();
				}
			}
		}
		memcpy(u, &newinfo, sizeof (newinfo));
		set_safe_record();
		if (netty_check == 1) {
			newinfo.userlevel &= ~(PERM_LOGINOK | PERM_PAGE);
			sethomefile(src, newinfo.userid, "register");
			sethomefile(dst, newinfo.userid, "register.old");
			rename(src, dst);
		}
		substitute_record(PASSFILE, &newinfo, sizeof (newinfo), unum);
	}
	clear();
	return 0;
}

void
x_info()
{
	modify_user_mode(GMENU);
	if (!strcmp("guest", currentuser.userid)) {
		disply_userinfo(&currentuser, 0);
		pressreturn();
		return;
	}
	disply_userinfo(&currentuser, 1);
	uinfo_query(&currentuser, 0, usernum);
}

static void
getfield(line, info, desc, buf, len)
int line, len;
char *info, *desc, *buf;
{
	char prompt[STRLEN];

	sprintf(genbuf, "  ԭ���趨: %-46.46s [1;32m(%s)[m",
		(buf[0] == '\0') ? "(δ�趨)" : buf, info);
	move(line, 0);
	prints("%s", genbuf);
	sprintf(prompt, "  %s: ", desc);
	getdata(line + 1, 0, prompt, genbuf, len, DOECHO, YEA);
	if (genbuf[0] != '\0') {
		strncpy(buf, genbuf, len);
	}
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

#ifdef POP_CHECK
void
x_fillform()
{
	char rname[NAMELEN], addr[STRLEN];
	char phone[STRLEN], dept[STRLEN], assoc[STRLEN];
	char ans[5], *mesg, *ptr;
	FILE *fn;
	time_t now;
	int lockfd;
	struct active_data act_data;
	int index;

	modify_user_mode(NEW);
	move(3, 0);
	clrtobot();
	if (!strcmp("guest", currentuser.userid)) {
		prints("��Ǹ, ���� new ����һ�����ʺź����������.");
		pressreturn();
		return;
	}
	if (currentuser.userlevel & PERM_LOGINOK) {
		prints("���Ѿ���ɱ�վ��ʹ����ע������, ��ӭ���뱾վ������.");
		pressreturn();
		return;
	}
	if ((fn = fopen("new_register", "r")) != NULL) {
		while (fgets(genbuf, STRLEN, fn) != NULL) {
			if ((ptr = strchr(genbuf, '\n')) != NULL)
				*ptr = '\0';
			if (strncmp(genbuf, "userid: ", 8) == 0 &&
			    strcmp(genbuf + 8, currentuser.userid) == 0) {
				fclose(fn);
				prints
				    ("վ����δ��������ע�����뵥, �����ĵȺ�.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}
	move(3, 0);
	sprintf(genbuf, "��Ҫ��дע�ᵥ������%s���ͥ��", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
		return;
	strncpy(rname, currentuser.realname, NAMELEN);
	strncpy(addr, currentuser.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s ����, ���ʵ��д���µ�����:\n", currentuser.userid);
		getfield(6, "��������", "��ʵ����", rname, NAMELEN);
		getfield(8, "ѧУϵ����˾ְ��", "ѧУϵ��������λ", dept,
			 STRLEN);
		getfield(10, "�������һ����ƺ���", "Ŀǰסַ��ͨѶ��ַ", addr,
			 STRLEN);
		getfield(12, "����������ʱ��", "����绰", phone, STRLEN);
		getfield(14, "У�ѻ���ҵѧУ", "У �� ��", assoc, STRLEN);
/* only for 9#        getfield( 14, "������ID����ʵ����",    "������(������ID����:ID/����)",   assoc, STRLEN );
*/
		mesg = "���������Ƿ���ȷ, �� Q ����ע�� (Y/N/Quit)? [N]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return;
		if (ans[0] == 'Y' || ans[0] == 'y')
			break;
	}
	strncpy(currentuser.realname, rname, NAMELEN);
	strncpy(currentuser.address, addr, STRLEN);
	memset(&act_data, 0, sizeof(act_data));
	strcpy(act_data.name, rname);
	strcpy(act_data.dept, dept);
	strcpy(act_data.userid, currentuser.userid);
	strcpy(act_data.phone, phone);
	strcpy(act_data.operator, currentuser.userid);
	strcpy(act_data.ip, currentuser.lasthost);
	act_data.status=0;
	write_active(&act_data);
	
	/*
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	if ((fn = fopen("new_register", "a")) != NULL) {
		now = time(NULL);
		fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
		fprintf(fn, "userid: %s\n", currentuser.userid);
		fprintf(fn, "realname: %s\n", rname);
		fprintf(fn, "dept: %s\n", dept);
		fprintf(fn, "addr: %s\n", addr);
		fprintf(fn, "phone: %s\n", phone);
		fprintf(fn, "assoc: %s\n", assoc);
		fprintf(fn, "----\n");
		fclose(fn);
	}
	close(lockfd);
	*/
	/*
	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);
	*/

	// ����Ҫ�û�ѡ���Ƿ�Ҫͨ���ʼ�������������ˣ� added by interma@BMY 2005.5.12
	clear();
	move(3, 0);
	prints("���潫����ʵ����֤����վĿǰ֧�����������ĵ������������֤. \n");
	prints("ÿ�����������֤ %d ��id.\n\n", MAX_USER_PER_RECORD);
	for (index=1; index<=DOMAIN_COUNT; ++index) {
		prints("[%d] %s \n", index, MAIL_DOMAINS[index]);
	}
	char tempn[3];
	int n=-1;
	while (!(n > 0 && n <= DOMAIN_COUNT))
	{
		getdata(10, 0, "��ѡ���������������ţ�����ע����ѡ��1�� >>  ", tempn, 3, DOECHO, YEA);
		sscanf(tempn, "%d", &n);
	}

	char user[USER_LEN + 1];
    	char pass[PASS_LEN + 1];
	int result;
	getdata(13, 0, "�����û���(����x������֤������ע���������û���test������test) >>  ", user, USER_LEN, DOECHO, YEA);
	getdata(14, 0, "�������� >>  ", pass, PASSLEN, NOECHO, YEA);

	while (test_mail_valid(user, pass, IP_POP[n])!=1) {
       	if (strcmp(user, "x")==0) {
        		return;
        	}
		if (strcmp(user, "test")==0) {
			clear();			
			move(5, 0);
			prints("��ӭ�����뽻����������ٸBBS��\n��������������������ע�ᣬĿǰ���������û���ݡ�");
			prints("Ŀǰ��û�з��ġ��ż�����Ϣ��Ȩ�ޡ�\n\n");
			prints("���ڿ�ѧȡ��stu.xjtu.edu.cn�����\n������վ��¼�����ʾ����������֤��������Ϊ��վ��ʽ�û���");	
			pressanykey();
			return;	
		}
		move(11, 0);
		clrtobot();
		move(12, 0);
		prints("��֤ʧ�ܣ��������������.");
		getdata(13, 0, "�����û���(����x������֤������ע���������û���test������test) >>  ", user, USER_LEN, DOECHO, YEA);
		getdata(14, 0, "�������� >>  ", pass, PASSLEN, NOECHO, YEA);
    	}
	
	char email[STRLEN];
	strcpy(email, str_to_lowercase(user));
	strcat(email, "@");
	strcat(email, MAIL_DOMAINS[n]);

	//FILE* fp;
	char path[128];
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s_privilege" , MAIL_DOMAINS[n]);
	int isprivilege=0;

	if (seek_in_file(path, user)) {
		isprivilege = 1;
	}

	
   	if (query_record_num(email, MAIL_ACTIVE)>=MAX_USER_PER_RECORD && isprivilege==0) {
        	clear();
        	move(3, 0);
        	prints("���������Ѿ���֤�� %d ��id���޷���������֤��!\n", MAX_USER_PER_RECORD);
		pressreturn();
        	return;
    	}
	
	int response;

	strcpy(act_data.email, email);
	act_data.status=1;
	response=write_active(&act_data);

    	if (response==WRITE_SUCCESS || response==UPDATE_SUCCESS)  {
		clear();
		move(5, 0);
		prints("�����˳ɹ������Ѿ�����ʹ�����ù����ˣ�\n"); 
		strncpy(currentuser.email, email, STRLEN);
		register_success(usernum, currentuser.userid, rname, dept, addr, phone, assoc, email);
		  
	 	//scroll();
		pressreturn();
		return;
    	}
	clear();
    	move(3, 0);
    	prints("  ��֤ʧ��!");
    	pressreturn();
    	return;
     
	/*
	struct stat temp;
	if (stat(MY_BBS_HOME "/etc/pop_register/pop_list", &temp) == -1)
	{
		prints("Ŀǰû�п������ε��ʼ��������б�, ����޷���֤�û���\n");
		//register_fail(currentuser.userid);
		scroll();
		pressreturn();
		exit(1);
	}
	
	FILE *fp;
	fp = fopen(MY_BBS_HOME "/etc/pop_register/pop_list", "r");
	if (fp == NULL)
	{
		prints("�򿪿������ε��ʼ��������б����, ����޷���֤�û���\n");
		//register_fail(currentuser.userid);
		scroll();
		pressreturn();
		exit(1);
	}

	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // ע�⣺�������10��pop��������Ҫ��������ˣ�
	char ippop[10][256];

	prints("Ŀǰ�������ε��ʼ��������б�: \n");

	while(fgets(bufpop, 256, fp) != NULL)
	{
		if (strcmp(bufpop, "") == 0 || strcmp(bufpop, " ") == 0 || strcmp(bufpop, "\n") == 0)
			break;
		strcpy(namepop[numpop], bufpop);
		fgets(bufpop, 256, fp);
		strcpy(ippop[numpop], bufpop);

		//scroll();
		prints("[%d] %s\n", numpop + 1, namepop[numpop]);
		numpop ++;
	}
	fclose(fp);
	
	char tempn[3];
	int n = -1;
	
	while (!(n > 0 && n <= numpop))
	{
		getdata(t_lines - 1, 0, "��ѡ������ʼ�������: ��������ţ�", tempn, 3, DOECHO, YEA);
		scroll();
		sscanf(tempn, "%d", &n);
	}

	// ���¿�ʼͨ���ʼ��������������
	
	scroll();
	char user[USER_LEN + 1];
    char pass[PASS_LEN + 1];
    
	int i = 0;
	int result;
	
	clear();

	while (i < 3) // 3Ϊ�������ԵĴ���
	{
	getdata(1, 0, "�������ʼ��������ϵ��û���:  ", user, USER_LEN, DOECHO, YEA);
	scroll();
	getdata(1, 0, "�������ʼ��������ϵ�����:    ", pass, PASS_LEN, NOECHO, YEA);
	scroll();
	scroll();

	result = test_mail_valid(user, pass, ippop[n - 1]);
	switch (result)
    {
		  case 0:
          prints("������ʧ�ܣ�������                       \n"); 
		  scroll(); break;
          case -1:
          prints("�ʼ����������ӳ���������                  \n"); scroll(); break; 
          case 1:     
          // prints("�����˳ɹ������Ѿ�����ʹ�����ù����ˣ�\n"); 
		  i = 3;
		  break;
     
    }
	i++;
	} // end of while

	switch (result)
    {
		  case -1:
		  case 0:
          prints("3�������˾�ʧ�ܣ�����ֻ��ʹ�ñ�bbs����������ܣ�ʮ�ֱ�Ǹ\n"); 
		  
		  //register_fail(currentuser.userid);

		  scroll();
	      pressreturn();
		  return;
		  break;

		  case 1:
		  namepop[n - 1][strlen(namepop[n - 1]) - 1] = 0;	  
		  if (write_pop_user(user, currentuser.userid, namepop[n - 1]) == 1)
		  {
				prints("���Ѿ�ʹ�ø�����ע���ID��,������޷�ע�����ID,ʮ�ֱ�Ǹ\n"); 
				//register_fail(currentuser.userid);

				scroll();
				pressreturn();
				return;
		  }

          prints("�����˳ɹ������Ѿ�����ʹ�����ù����ˣ�\n"); 

		  char email[256];
		  sprintf(email, "%s@%s", user, namepop[n - 1]);
		  strncpy(currentuser.email, email, STRLEN);
		  
		  register_success(usernum, currentuser.userid, rname, dept, addr, phone, assoc, email);
		  
	 	  scroll();
		  pressreturn();
		  break;
     
    }
    */

}
#else
void
x_fillform()
{
	char rname[NAMELEN], addr[STRLEN];
	char phone[STRLEN], dept[STRLEN], assoc[STRLEN];
	char ans[5], *mesg, *ptr;
	FILE *fn;
	time_t now;
	int lockfd;

	modify_user_mode(NEW);
	move(3, 0);
	clrtobot();
	if (!strcmp("guest", currentuser.userid)) {
		prints("��Ǹ, ���� new ����һ�����ʺź����������.");
		pressreturn();
		return;
	}
	if (currentuser.userlevel & PERM_LOGINOK) {
		prints("���Ѿ���ɱ�վ��ʹ����ע������, ��ӭ���뱾վ������.");
		pressreturn();
		return;
	}
	if ((fn = fopen("new_register", "r")) != NULL) {
		while (fgets(genbuf, STRLEN, fn) != NULL) {
			if ((ptr = strchr(genbuf, '\n')) != NULL)
				*ptr = '\0';
			if (strncmp(genbuf, "userid: ", 8) == 0 &&
			    strcmp(genbuf + 8, currentuser.userid) == 0) {
				fclose(fn);
				prints
				    ("վ����δ��������ע�����뵥, �����ĵȺ�.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}
	move(3, 0);
	sprintf(genbuf, "��Ҫ��дע�ᵥ������%s���ͥ��", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
		return;
	strncpy(rname, currentuser.realname, NAMELEN);
	strncpy(addr, currentuser.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s ����, ���ʵ��д���µ�����:\n", currentuser.userid);
		getfield(6, "��������", "��ʵ����", rname, NAMELEN);
		getfield(8, "ѧУϵ����˾ְ��", "ѧУϵ��������λ", dept,
			 STRLEN);
		getfield(10, "�������һ����ƺ���", "Ŀǰסַ��ͨѶ��ַ", addr,
			 STRLEN);
		getfield(12, "����������ʱ��", "����绰", phone, STRLEN);
		getfield(14, "У�ѻ���ҵѧУ", "У �� ��", assoc, STRLEN);
/* only for 9#        getfield( 14, "������ID����ʵ����",    "������(������ID����:ID/����)",   assoc, STRLEN );
*/
		mesg = "���������Ƿ���ȷ, �� Q ����ע�� (Y/N/Quit)? [N]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return;
		if (ans[0] == 'Y' || ans[0] == 'y')
			break;
	}
	strncpy(currentuser.realname, rname, NAMELEN);
	strncpy(currentuser.address, addr, STRLEN);
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	if ((fn = fopen("new_register", "a")) != NULL) {
		now = time(NULL);
		fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
		fprintf(fn, "userid: %s\n", currentuser.userid);
		fprintf(fn, "realname: %s\n", rname);
		fprintf(fn, "dept: %s\n", dept);
		fprintf(fn, "addr: %s\n", addr);
		fprintf(fn, "phone: %s\n", phone);
		fprintf(fn, "assoc: %s\n", assoc);
		fprintf(fn, "----\n");
		fclose(fn);
	}
	close(lockfd);
	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);
}

#endif
