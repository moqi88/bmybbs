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
#include "bbstelnet.h"
#define DENY 1
#define UNDENY 2
#define CHANGEDENY 3

static int addtodeny(char *uident, char *msg, int ischange, int isglobal,
		     int isanony);
static int deldeny(char *uident, int isglobal, int isanony);
static int delclubmember(char *uident);
static int deny_notice(int action, char *user, int isglobal, int isanony,
		       char *msgbuf);

static int
addtodeny(uident, msg, ischange, isglobal, isanony)
char *uident;
char *msg;
int ischange, isglobal, isanony;
{
	char buf[50], strtosave[256];
	char buf2[50];
	int day;
	time_t nowtime;
	char ans[8];
	int seek;

	if (isglobal)
		strcpy(genbuf, "deny_users");
	else if (isanony)
		setbfile(genbuf, currboard, "deny_anony");
	else
		setbfile(genbuf, currboard, "deny_users");
	seek = seek_in_file(genbuf, uident);
	if ((ischange && !seek) || (!ischange && seek)) {
		move(2, 0);
		prints("�����ID����!");
		pressreturn();
		return -1;
	}
	buf[0] = 0;
	move(2, 0);
	prints("�������%s", (isanony) ? "Anonymous" : uident);
	while (strlen(buf) < 4)
		getdata(3, 0, "����˵��(��������): ", buf, 40, DOECHO, YEA);

	do {
		getdata(4, 0, "��������(0-�ֶ����): ", buf2, 4, DOECHO, YEA);
		day = atoi(buf2);
		if (day < 0)
			continue;
		if (!(currentuser.userlevel & PERM_SYSOP) && (!day || day > 20)) {
			move(4, 0);
			prints("����Ȩ��,����Ҫ,����ϵվ��!");
			pressreturn();
		} else
			break;
	} while (1);
	//add by lepton for deny bm's right

	nowtime = time(NULL);
	if (day) {
		struct tm *tmtime;
		//time_t daytime = nowtime + (day + 1) * 24 * 60 * 60; 
		time_t undenytime = nowtime + day * 24 * 60 * 60;
		//tmtime = gmtime(&daytime); by bjgyt
		tmtime = gmtime(&undenytime);
		sprintf(strtosave, "%-12s %-40s %2d��%2d�ս� \x1b[%ldm", uident,
			buf, tmtime->tm_mon + 1, tmtime->tm_mday,
			(long int) undenytime);
		//modified by pzhg
		if (currentuser.userlevel & PERM_SPECIAL5) {
			if (isglobal)
				sprintf(msg,
	                        "����ԭ��: %s\n��������: %d%s\n�������: %d��%d��\n"
	                        "��վ�����ܶԴ˷��������Ͷ��\n", buf, day,
	                        "(ȫվ)" , tmtime->tm_mon + 1,
	                        tmtime->tm_mday);
			else
				sprintf(msg,
				"����ԭ��: %s\n��������: %d%s\n�������: %d��%d��\n"
				"��վ�����ܶԴ˷��������Ͷ��\n", buf, day,
				isglobal ? "(ȫվ)" : "", tmtime->tm_mon + 1,
				tmtime->tm_mday);
		}
		else if(isglobal || day>20)
		sprintf(msg,
                        "����ԭ��: %s\n��������: %d%s\n�������: %d��%d��\n"
                        "�������飬����%s�������committee��Ͷ��\n", buf, day,
                        "(ȫվ)" , tmtime->tm_mon + 1,
                        tmtime->tm_mday, "վ��");
		else {
			if (seek_in_file(MY_BBS_HOME"/etc/sysboards",currboard)) {
				sprintf(msg,
				"����ԭ��: %s\n��������: %d%s\n�������: %d��%d��\n"
				"�������飬����%s�������committee��Ͷ��\n", buf, day,
				isglobal ? "(ȫվ)" : "", tmtime->tm_mon + 1,
				tmtime->tm_mday, "վ��" );
			}
			else {
				sprintf(msg,
					"����ԭ��: %s\n��������: %d%s\n�������: %d��%d��\n"
					"�������飬����%s�������Appeal��Ͷ��\n", buf, day,
					isglobal ? "(ȫվ)" : "", tmtime->tm_mon + 1,
					tmtime->tm_mday, isglobal ? "վ��" : "����");
			}
		}
	} else {
		if (currentuser.userlevel & PERM_SPECIAL5) {
			sprintf(strtosave, "%-12s %-35s �ֶ����", uident, buf);
			sprintf(msg, "����ԭ��: %s\n��������: �ֶ����%s\n"
				"��վ�����ܶԴ˷��������Ͷ��\n",
				buf, isglobal ? "(ȫվ)" : "");
		}
		else {
			sprintf(strtosave, "%-12s %-35s �ֶ����", uident, buf);
			sprintf(msg, "����ԭ��: %s\n��������: �ֶ����%s\n"
				"�������飬����%s�������committee��Ͷ��\n",
				buf, isglobal ? "(ȫվ)" : "",
				isglobal ? "վ��" : "����");
		}
	}
	if (ischange)
		getdata(5, 0, "���Ҫ�ı�ô?[Y/N]: ", ans, 7, DOECHO, YEA);
	else
		getdata(5, 0, "���Ҫ��ô?[Y/N]: ", ans, 7, DOECHO, YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
	if (ischange)
		deldeny(uident, isglobal, 0);
	if (isglobal)
		strcpy(genbuf, "deny_users");
	else if (isanony)
		setbfile(genbuf, currboard, "deny_anony");
	else
		setbfile(genbuf, currboard, "deny_users");
	return addtofile(genbuf, strtosave);
}

static int
deldeny(uident, isglobal, isanony)
char *uident;
int isglobal;
int isanony;
{
	char fn[STRLEN];

	if (isglobal)
		strcpy(fn, "deny_users");
	else if (isanony)
		setbfile(fn, currboard, "deny_anony");
	else
		setbfile(fn, currboard, "deny_users");
	return del_from_file(fn, uident);
}

int
deny_user()
{
	char uident[STRLEN];
	char ans[8];
	char msgbuf[256];
	int count, isglobal = 0;

	if (!IScurrBM) {
		return DONOTHING;
	}

	if (!strcmp(currboard, "sysop")) {
		getdata(0, 0,
			"�趨�ĸ��޷� Post ��������(A) sysop�� (B) ϵͳ (E)�뿪 [E]:",
			ans, 7, DOECHO, YEA);
		if (!strchr("AaBb", *ans))
			return FULLUPDATE;
		if (*ans == 'B' || *ans == 'b')
			isglobal = 1;
	}
	if (isglobal)
		strcpy(genbuf, "deny_users");
	else
		setbfile(genbuf, currboard, "deny_users");
//      ansimore(genbuf, YEA);
	while (1) {
		clear();
		prints("�趨�޷� Post ������\n");
		if (isglobal)
			strcpy(genbuf, "deny_users");
		else
			setbfile(genbuf, currboard, "deny_users");
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				"(A)���� (D)ɾ�� (C)�ı� or (E)�뿪 [E]: ", ans,
				7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)���� or (E)�뿪 [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			if (isglobal)
				usercomplete("�����޷� POST ��ʹ����: ",
					     uident);
			else {
				int canpost = 0;
				while (!canpost) {
					move(1, 0);
					clrtoeol();
					usercomplete("�����޷� POST ��ʹ���ߣ�",
						     uident);
					if (*uident == '\0')
						break;
					canpost = posttest(uident, currboard);
				}
			}
			if (*uident != '\0') {
				if (addtodeny(uident, msgbuf, 0, isglobal, 0) ==
				    1) {
					deny_notice(DENY, uident, isglobal, 0,
						    msgbuf);
					sprintf(genbuf, "%s deny %s %s",
						currentuser.userid, currboard,
						uident);
					newtrace(genbuf);

				}
			}
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("�ı�˭�ķ��ʱ���˵��: ", uident);
			if (*uident != '\0') {
				if (addtodeny(uident, msgbuf, 1, isglobal, 0) ==
				    1) {
					deny_notice(CHANGEDENY, uident,
						    isglobal, 0, msgbuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("ɾ���޷� POST ��ʹ����: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				if (deldeny(uident, isglobal, 0)) {
					deny_notice(UNDENY, uident, isglobal, 0,
						    msgbuf);
				}
			}
		} else
			break;
	}

	clear();
	return FULLUPDATE;
}

int
addclubmember(uident, clubnum)
char *uident;
int clubnum;
{
	char genbuf1[80], genbuf2[80];
	int id;
	int i;
	char ans[8];
	int seek;

	if (clubnum == 0) {
		if (!(id = getuser(uident))) {
			move(3, 0);
			prints("Invalid User Id");
			clrtoeol();
			pressreturn();
			clear();
			return 0;
		}
		setbfile(genbuf, currboard, "club_users");
		seek = seek_in_file(genbuf, uident);
		if (seek) {
			move(2, 0);
			prints("�����ID �Ѿ�����!");
			pressreturn();
			return -1;
		}

		getdata(4, 0, "���Ҫ���ô?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y'))
			return -1;
		setbfile(genbuf, currboard, "club_users");
		sethomefile(genbuf1, uident, "clubrights");
		if ((i = getbnum(currboard)) == 0)
			return DONOTHING;
		sprintf(genbuf2, "%d", bcache[i - 1].header.clubnum);
		addtofile(genbuf1, genbuf2);
		return addtofile(genbuf, uident);
	} else {
		setbfile(genbuf, currboard, "club_users");
		seek = seek_in_file(genbuf, uident);
		if (seek)
			return DONOTHING;
		sethomefile(genbuf1, uident, "clubrights");
		sprintf(genbuf2, "%d", clubnum);
		addtofile(genbuf1, genbuf2);
		return addtofile(genbuf, uident);
	}
}

static int
delclubmember(uident)
char *uident;
{
	char genbuf1[80], genbuf2[80];
	char fn[STRLEN];
	int id;
	int i;
	if (!(id = getuser(uident))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	setbfile(fn, currboard, "club_users");
	sethomefile(genbuf1, uident, "clubrights");
	sprintf(genbuf2, "%d", bcache[i - 1].header.clubnum);
	del_from_file(genbuf1, genbuf2);
	return del_from_file(fn, uident);
}

int
clubmember()
{
	char uident[STRLEN];
	char ans[8], repbuf[STRLEN], buf[STRLEN], titlebuf[STRLEN];
	int count, i;

	if (!(IScurrBM)) {
		return DONOTHING;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	if (bcache[i - 1].header.clubnum == 0)
		return DONOTHING;
	setbfile(genbuf, currboard, "club_users");
	ansimore(genbuf, YEA);
	while (1) {
		clear();
		prints("�趨���ֲ�����\n");
		setbfile(genbuf, currboard, "club_users");
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				"(A)���� (D)ɾ��or (E)�뿪or (M)д�Ÿ����г�Ա [E]: ",
				ans, 7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)���� or (E)�뿪 [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("���Ӿ��ֲ���Ա: ", uident);
			if (*uident != '\0') {
				if (addclubmember(uident, 0) == 1) {
					getdata(5, 0, "����ԭ��", buf, 50, DOECHO, YEA);
					sprintf(titlebuf,
						"%s��%s����%s���ֲ�Ȩ��",
						uident, currentuser.userid,
						currboard);
					sprintf(repbuf,	
						"%s%s%s", titlebuf, buf[0] ? "\n\nԭ��":"", buf);
					securityreport(titlebuf, buf);
					deliverreport(titlebuf, repbuf);
					mail_buf(repbuf, uident, titlebuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("ɾ�����ֲ�ʹ����: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				sprintf(genbuf, "���Ҫȡ��%s�ľ��ֲ�Ȩ��ô��",
					uident);
				if (askyn(genbuf, YEA, NA))
					if (delclubmember(uident)) {
						getdata(5, 0, "ɾ��ԭ��", buf, 50, DOECHO, YEA);
						sprintf(titlebuf,
							"%s��%sȡ��%s���ֲ�Ȩ��",
							uident, currentuser.userid, currboard);
						sprintf(repbuf,	
							"%s%s%s", titlebuf, buf[0] ? "\n\nԭ��":"", buf);
						securityreport(titlebuf, buf);
						deliverreport(titlebuf, repbuf);
						mail_buf(repbuf, uident, titlebuf);
					}
			}
		} else if ((*ans == 'M' || *ans == 'm') && count) {
			club_send();
		} else
			break;
	}
	clear();
	return FULLUPDATE;
}

int
deny_from_article(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char msgbuf[512];
	char user[STRLEN];
	char ans[8];
	int seek, canpost, isanony;
	if (!IScurrBM) {
		return DONOTHING;
	}
	if (!strcmp(fh2owner(fileinfo), "Anonymous")) {	/* ���������� */
		isanony = 1;
		setbfile(genbuf, currboard, "deny_anony");
		strcpy(user, fh2realauthor(fileinfo));
	} else {
		isanony = 0;
		setbfile(genbuf, currboard, "deny_users");
		strcpy(user, fileinfo->owner);
	}
	seek = seek_in_file(genbuf, user);
	if (seek) {		/* ��� */
		move(2, 0);
		getdata(4, 0, "���Ҫ���ô?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y'))
			return -1;
		if (deldeny(user, 0, isanony) == 1)
			deny_notice(UNDENY, user, 0, isanony, msgbuf);

	} else {		/* ������� */
		canpost = posttest(user, currboard);
		if ((canpost) && (addtodeny(user, msgbuf, 0, 0, isanony) == 1))
			deny_notice(DENY, user, 0, isanony, msgbuf);
	}
	return 0;
}

static int
deny_notice(action, user, isglobal, isanony, msgbuf)
char *user, *msgbuf;
int action, isglobal, isanony;
{
	char repbuf[STRLEN];
	char tmpbuf[STRLEN], tmpbuf2[STRLEN];
	int i;
	char repuser[IDLEN + 1];
	if (isanony)
		strcpy(repuser, "Anonymous");
	else
		strcpy(repuser, user);
	switch (action) {
	case DENY:
		if (isglobal) {
			sprintf(repbuf,
				"%s��%sȡ����ȫվ��POSTȨ��",
				user, currentuser.userid);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			mail_buf(msgbuf, user, repbuf);
		} else if (((currentuser.userlevel & PERM_SYSOP)
			    || (currentuser.userlevel & PERM_OBOARDS))
			   && (msgbuf[10] == 'W' || msgbuf[10] == 'w')) {
			for (i = 10; msgbuf[i]; i++)
				if (msgbuf[i + 1] == '\n')
					msgbuf[i + 1] = 0;
			strcpy(tmpbuf, msgbuf + 11);
			strcpy(tmpbuf2, msgbuf + i + 1);
			sprintf(repbuf,
				"%s��վ��%s��ʱ������%s��POSTȨ��",
				repuser, currentuser.userid, currboard);
			sprintf(msgbuf,
				"վ��%s��Ϊ%s��%s����.�뱾�����/��\n"
				"��ʱ��%s����Ϊ����������׼����ȷ��.\n"
				"�ָ�POSTȨ���߸�����ȷ�������.\n\n%s",
				currentuser.userid, repuser, tmpbuf, repuser,
				tmpbuf2);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf,
				"%s��վ��%s��ʱ������%s��POSTȨ��",
				user, currentuser.userid, currboard);
			sprintf(msgbuf,
				"վ��%s��Ϊ%s��%s����.�뱾�����/��\n"
				"��ʱ��%s����Ϊ����������׼����ȷ��.\n"
				"�ָ�POSTȨ���߸�����ȷ�������.\n\n%s",
				currentuser.userid, user, tmpbuf, user,
				tmpbuf2);
			mail_buf(msgbuf, user, repbuf);
		}
		/*old action */
		else {
			sprintf(repbuf,
				"%s��%sȡ����%s��POSTȨ��",
				repuser, currentuser.userid, currboard);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf,
				"%s��%sȡ����%s��POSTȨ��",
				user, currentuser.userid, currboard);
			mail_buf(msgbuf, user, repbuf);
		}
		break;
	case CHANGEDENY:
		sprintf(repbuf,
			"%s�ı��%s��ʱ���˵��", currentuser.userid, user);
		securityreport(repbuf, msgbuf);
		deliverreport(repbuf, msgbuf);
		mail_buf(msgbuf, user, repbuf);
		break;
	case UNDENY:
		sprintf(repbuf,
			"�ָ� %s �� %s ��POSTȨ",
			repuser, isglobal ? "ȫվ" : currboard);
		snprintf(msgbuf, 256, "%s %s\n"
			 "�������������,лл!\n", currentuser.userid,
			 repbuf);
		securityreport(repbuf, msgbuf);
		deliverreport(repbuf, msgbuf);
		sprintf(repbuf,
			"�ָ� %s �� %s ��POSTȨ",
			user, isglobal ? "ȫվ" : currboard);
		snprintf(msgbuf, 256, "%s %s\n"
			 "�������������,лл!\n", currentuser.userid,
			 repbuf);
		mail_buf(msgbuf, user, repbuf);
		break;
	}
	return 0;
}

int                                                                                          
mail_buf_slow(char *userid, char *title, char *content, char *sender)                        
{                                                                                            
        FILE *fp;                                                                            
        char buf[256], dir[256];                                                             
        struct fileheader header;                                                            
        int t;                                                                               
        int now;                                                                             
        bzero(&header, sizeof (header));                                                     
        fh_setowner(&header, sender, 0);                                                     
        sprintf(buf, "mail/%c/%s/", mytoupper(userid[0]), userid);                           
        if (!file_isdir(buf))                                                                
                return -1;                                                                   
        now = time(NULL);                                                                    
        t = trycreatefile(buf, "M.%d.A", now, 100);                                          
        if (t < 0)                                                                           
                return -1;                                                                   
        header.filetime = t;                                                                 
        strsncpy(header.title, title, sizeof (header.title));                                
        fp = fopen(buf, "w");                                                                
        if (fp == 0)                                                                         
                return -2;                                                                   
        fprintf(fp, "%s", content);                                                          
        fclose(fp);                                                                          
        setmailfile(dir, userid, ".DIR");                                                    
        append_record(dir, &header, sizeof (header));                                        
        return 0;                                                                            
}                                                                                            

