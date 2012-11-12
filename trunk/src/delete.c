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
    
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

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

int
d_board()
{
	struct boardheader binfo;
	int bid, ans;
	char bname[STRLEN];
	extern char lookgrp[];

	if (!HAS_PERM(PERM_BLEVELS)) {
		return -1;
	}
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("ɾ��������");
	make_blist_full();
	move(1, 0);
	namecomplete("������������: ", bname);
	if (bname[0] == '\0')
		return -1;
	bid = getbnum(bname);
	if (get_record(BOARDS, &binfo, sizeof (binfo), bid) == -1) {
		move(2, 0);
		prints("����ȷ��������\n");
		pressreturn();
		clear();
		return -1;
	}
	ans = askyn("��ȷ��Ҫɾ�����������", NA, NA);
	if (ans != 1) {
		move(2, 0);
		prints("ȡ��ɾ���ж�\n");
		pressreturn();
		clear();
		return -1;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "ɾ����������%s", binfo.filename);
		securityreport(secu, secu);
	}
	if (seek_in_file("0Announce/.Search", bname)) {
		move(4, 0);
		if (askyn("�Ƴ�������", NA, NA) == YEA) {
			get_grp(binfo.filename);
			del_grp(lookgrp, binfo.filename, binfo.title);
		}
	}
	if (seek_in_file("etc/junkboards", bname))
		del_from_file("etc/junkboards", bname);
	if (seek_in_file("0Announce/.Search", bname))
		del_from_file("0Announce/.Search", bname);

	if (binfo.filename[0] == '\0')
		return -1;	/* rrr - precaution */
	sprintf(genbuf, "boards/%s", binfo.filename);
	deltree(genbuf);
	sprintf(genbuf, "vote/%s", binfo.filename);
	deltree(genbuf);
	sprintf(genbuf, " << '%s'�� %s ɾ�� >>",
		binfo.filename, currentuser.userid);
	memset(&binfo, 0, sizeof (binfo));
	strsncpy(binfo.title, genbuf, sizeof (binfo.title));
	binfo.level = PERM_SYSOP;
	substitute_record(BOARDS, &binfo, sizeof (binfo), bid);

	reload_boards();
	update_postboards();

	move(4, 0);
	prints("\n���������Ѿ�ɾ��...\n");
	pressreturn();
	clear();
	return 0;
}

void
offline()
{
	char buf[STRLEN];

	modify_user_mode(OFFLINE);
	clear();
	if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_BOARDS)
	    || HAS_PERM(PERM_ADMINMENU) || HAS_PERM(PERM_SEEULEVELS)) {
		move(1, 0);
		prints("\n\n������������, ���������ɱ��!!\n");
		pressreturn();
		clear();
		return;
	}
	if (currentuser.stay < 86400) {
		move(1, 0);

		prints("\n\n�Բ���, ����δ���ʸ�ִ�д�����!!\n");
		prints("ֻ����վʱ�䳬��24Сʱ���û�������ɱ.\n");
		pressreturn();
		clear();
		return;
	}

	getdata(1, 0, "�������������: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		prints("\n\n�ܱ�Ǹ, ����������벻��ȷ��\n");
		pressreturn();
		clear();
		return;
	}
	getdata(3, 0, "�������ʲô����? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		prints("\n\n�ܱ�Ǹ, �Ҳ�����ʶ�㡣\n");
		pressreturn();
		clear();
		return;
	}
	clear();
	move(1, 0);
	prints("[1;5;31m����[0;1;31m ��ɱ��id�����ͷ�����û���κι�ϵ[m \n");
	prints("��ɱ������������id�޷�����ʹ�ã���id������ֻ���������й�ϵ\n");
	prints("������˸�������ɱ���԰������������е�id������ע���µ�id�����뿪����\n");
	prints("Ȼ��Ѹ�������˴�һ��\n\n");
	prints
	    ("[1;5;31m����[0;1;31m�� ��ɱ��, ������꽫���������������, Ը����Ϣ");
	prints("\n\n\n[1;32m���ѹ��.....[m\n\n\n");
	if (askyn("��ȷ��Ҫ�뿪������ͥ", NA, NA) == 1) {
		clear();
		set_safe_record();
		currentuser.dietime = currentuser.stay + 6*18 * 24 * 60 * 60;//����ɱ�ָ�Ϊ6*18��   �����ֻ�+18�����
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser),
				  usernum);
		Q_Goodbye();
		return;
		/*if(d_user(currentuser.userid)==1)
		   {
		   mail_info();
		   modify_user_mode( OFFLINE );
		   kick_user(&uinfo);
		   exit(0);
		   }
		 */
	}
}

int
online()
{
	char buf[STRLEN];
	struct tm *nowtime;
	time_t nowtimeins;
	modify_user_mode(OFFLINE);
	clear();
	nowtimeins = time(NULL);
	nowtime = localtime(&nowtimeins);
	if ((currentuser.stay <= currentuser.dietime)) {
		move(1, 0);
		prints("\n\n����δ����!!\n");
		prints("��Ҫ�����˰�!\n");
		prints("������ڻ��� %d ����",
		       1 + (currentuser.dietime - currentuser.stay) / 60);
		pressreturn();
		clear();
		return -1;
	}
	getdata(1, 0, "�������������: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		prints("\n\n�ܱ�Ǹ, ����������벻��ȷ��\n");
		pressreturn();
		clear();
		return -2;
	}
	getdata(3, 0, "�������ʲô����? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		prints("\n\n�ܱ�Ǹ, �Ҳ�����ʶ�㡣\n");
		pressreturn();
		clear();
		return -3;
	}
	clear();
	move(1, 0);
	prints
	    ("\033[1;5;31m����\033[0;1;31m�� ����,���ǻ���,�Ǹ�ֵ�ÿ��ǵ�����");
	prints("\n\n\n\033[1;32m����Ҫ�����.....\033[m\n\n\n");
	if (askyn("��ȷ��Ҫ����Ϊ������?", NA, NA) == 1) {
		clear();
		currentuser.dietime = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser),
				  usernum);
		return Q_Goodbye();
	}
	return 0;
}

void
getuinfo(fn)
FILE *fn;
{
	int num;
	char buf[40];

	fprintf(fn, "\n\n���Ĵ���     : %s\n", currentuser.userid);
	fprintf(fn, "�����ǳ�     : %s\n", currentuser.username);
	fprintf(fn, "��ʵ����     : %s\n", currentuser.realname);
	fprintf(fn, "��ססַ     : %s\n", currentuser.address);
	fprintf(fn, "�����ʼ����� : %s\n", currentuser.email);
	fprintf(fn, "��ʵ E-mail  : %s\n", currentuser.realmail);
	//fprintf(fn,"Ident ����   : %s\n", currentuser.ident);
	fprintf(fn, "����ָ��     : %s\n", currentuser.ip);
	fprintf(fn, "�ʺŽ������� : %s", ctime(&currentuser.firstlogin));
	fprintf(fn, "����������� : %s", ctime(&currentuser.lastlogin));
	fprintf(fn, "������ٻ��� : %s\n", currentuser.lasthost);
	fprintf(fn, "�����վʱ�� : %s", ctime(&currentuser.lastlogout));
	fprintf(fn, "��վ����     : %d ��\n", currentuser.numlogins);
	fprintf(fn, "������Ŀ     : %d\n", currentuser.numposts);
	fprintf(fn, "��վ��ʱ��   : %ld Сʱ %ld ����\n",
		(long int) (currentuser.stay / 3600),
		(long int) ((currentuser.stay / 60) % 60));
	strcpy(buf, "bTCPRp#@XWBA#VS-DOM-F012345678");
	for (num = 0; num < 30; num++)
		if (!(currentuser.userlevel & (1 << num)))
			buf[num] = '-';
	buf[num] = '\0';
	fprintf(fn, "ʹ����Ȩ��   : %s\n\n", buf);
}

#if 0
static void
mail_info()
{
	FILE *fn;
	time_t now;
	char filename[STRLEN];

	now = time(0);
	sprintf(filename, "tmp/suicide.%s", currentuser.userid);
	if ((fn = fopen(filename, "w")) != NULL) {
		fprintf(fn,
			"[1m%s[m �Ѿ��� [1m%24.24s[m ��ɱ�ˣ��������������ϣ��뱣��...",
			currentuser.userid, ctime(&now));
		getuinfo(fn);
		fprintf(fn,
			"\n                      [1m ϵͳ�Զ�����ϵͳ��[m\n\n");
		fclose(fn);
		postfile(filename, "syssecurity", "��ɱ֪ͨ....", 2);
		unlink(filename);
	}
	if ((fn = fopen(filename, "w")) != NULL) {
		fprintf(fn, "��Һ�,\n\n");
		fprintf(fn, "���� %s (%s)�� ���Ѿ��뿪�����ˡ�\n\n",
			currentuser.userid, currentuser.username);
		fprintf(fn,
			"�Ҳ���������������� %s�������ڱ�վ %d �� login ���ܹ� %d ���Ӷ����ڼ�ĵ��εΡ�\n",
			ctime(&currentuser.firstlogin), currentuser.numlogins,
			(int) currentuser.stay / 60);
		fprintf(fn, "���ҵĺ��Ѱ� %s �����ǵĺ����������õ��ɡ�\n\n",
			currentuser.userid);
		fprintf(fn, "�����г�һ���һ�����ġ� ����!! �ټ�!!\n\n\n");
		fprintf(fn, "%s �� %24.24s ��.\n\n", currentuser.userid,
			ctime(&now));
		fclose(fn);
		postfile(filename, "notepad", "��ɱ����....", 2);
		unlink(filename);
	}
}
#endif
int
d_user(cid)
char *cid;
{
	int id;

	if (uinfo.mode != OFFLINE) {
		modify_user_mode(ADMIN);
		if (!check_systempasswd()) {
			return -1;
		}
		clear();
		stand_title("ɾ��ʹ�����ʺ�");
		move(1, 0);
		usercomplete("��������ɾ����ʹ���ߴ���: ", genbuf);
		if (*genbuf == '\0') {
			clear();
			return 0;
		}
	} else
		strcpy(genbuf, cid);
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("�����ʹ���ߴ���...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
/*    if (!isalpha(lookupuser.userid[0])) return 0;*/
	/* rrr - don't know how... */
	move(1, 0);
	if (uinfo.mode != OFFLINE)
		prints("ɾ��ʹ���� '%s'.", genbuf);
	else
		prints(" %s ���뿪����", cid);
	clrtoeol();
	getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, YEA);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		if (uinfo.mode != OFFLINE)
			prints("ȡ��ɾ��ʹ����...\n");
		else
			prints("�����ڻ���ת���ˣ��ø����...");
		pressreturn();
		clear();
		return 0;
	}
	if (lookupuser.userid[0] == '\0' || !strcmp(lookupuser.userid, "SYSOP")) {
		prints("�޷�ɾ��!!\n");
		pressreturn();
		clear();
		return 0;
	}
	if (uinfo.mode != OFFLINE) {
		char secu[STRLEN];
		sprintf(secu, "ɾ��ʹ���ߣ�%s", lookupuser.userid);
		securityreport(secu, secu);
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
	move(2, 0);
	prints("%s �Ѿ��������...\n", lookupuser.userid);
	pressreturn();

	clear();
	return 1;
}

int
kick_user(userinfo,mode)
struct user_info *userinfo;
int mode;
{
	int id, ind;
	struct user_info uin;
	struct userec kuinfo;
	char kickuser[40];
       char kickreason[STRLEN];
       char titlebuf[STRLEN];
       char contentbuf[STRLEN];
       char repbuf[STRLEN];
       char msgbuf[STRLEN];
	if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE
	    && uinfo.mode != FRIEND) {
		modify_user_mode(ADMIN);
		stand_title("Kick User");
		move(1, 0);
		usercomplete("Enter userid to be kicked: ", kickuser);
		if (*kickuser == '\0') {
			clear();
			return 0;
		}
		if (!(id = getuser(kickuser))) {
			move(3, 0);
			prints("Invalid User Id");
			clrtoeol();
			pressreturn();
			clear();
			return 0;
		}
		move(1, 0);
		prints("Kick User '%s'.", kickuser);
		clrtoeol();
		getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, YEA);
		if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
			move(2, 0);
			prints("Aborting Kick User\n");
			pressreturn();
			clear();
			return 0;
		}
		search_record(PASSFILE, &kuinfo, sizeof (kuinfo),
			      (void *) cmpuids, kickuser);
		ind = search_ulist(&uin, t_cmpuids, id);
	} else {
		uin = *userinfo;
		strcpy(kickuser, uin.userid);
/*        id = getuser(kickuser);
        search_record(PASSFILE, &kuinfo, sizeof(kuinfo), cmpuids, kickuser);
        ind = search_ulist( &uin, t_cmpuids, id ); */
		ind = YEA;
	}


	//���Լ�ʱ mode=1
       if(mode!=1)
       {
           	clear();
             	move(1,0);
              getdata(2,0,"����ԭ��: ", kickreason,STRLEN,DOECHO,YEA);
        }
	if (uin.pid != 1
	    && (!ind || !uin.active || uin.pid <= 0
		|| (kill(uin.pid, 0) == -1))) {
		if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE
		    && uinfo.mode != FRIEND) {
			move(3, 0);
			prints("User Has Logged Out");
			clrtoeol();
			pressreturn();
			clear();
		}
		return 0;
	} else if (kill(uin.pid, SIGHUP) < 0) {
		prints("User can't be kicked");
		pressreturn();
		clear();
		return 1;
	}
       sprintf(contentbuf, "%s",kickreason);
       sprintf(titlebuf,"%s��%s�߳�վ��", currentuser.userid, kickuser);
       securityreport(titlebuf,contentbuf);

       sprintf(repbuf,"����%sǿ���뿪��վ",currentuser.userid);
       sprintf(msgbuf,"����:%s\n",kickreason);
       mail_buf(msgbuf,kickuser, repbuf);
	sprintf(genbuf, "%s kick %s", currentuser.userid, kickuser);
	newtrace(genbuf);
	move(2, 0);
	if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE
	    && uinfo.mode != FRIEND) {
		prints("User has been Kicked\n");
		pressreturn();
		clear();
	}
	return 1;
}
