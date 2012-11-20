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
#include "bbsgopher.h"

#define PATHLEN         1024
//modified by ylsdd #define A_PAGESIZE      (t_lines - 5)
#define A_PAGESIZE      (t_lines - 4)
#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2
#define ADDGOPHER       3

void a_menu();

extern void a_prompt();		/* added by netty */
extern int can_R_endline;
typedef struct {
	char title[72];
	char fname[80];
	char *host;
	int port;
} ITEM;

int a_fmode = 1;
int copymode;
int inlink = 0;

typedef struct {
	ITEM *item[MAXITEMS];
	char mtitle[STRLEN];
	char *path;
	int num, page, now;
	int level;
} MENU;

static void
freeitem(MENU * me)
{
	int i;
	for (i = 0; i < me->num; i++)
		free(me->item[i]);
	return;
}

int
valid_fname(str)
char *str;
{
	char ch;

	while ((ch = *str++) != '\0') {
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		    strchr("0123456789-_", ch) != NULL) {
			;
		} else {
			return 0;
		}
	}
	return 1;
}

static void
a_showmenu(pm)
MENU *pm;
{
	struct stat st;
	struct tm *pt;
	char title[STRLEN * 2], kind[20];
	char fname[STRLEN];
	char ch;
	char buf[PATHLEN], pathbuf[PATHLEN];
	time_t mtime;
	int n;
	int visit[2];
	int overlen;

	clear();
	if (chkmail()) {
		prints("[5m");
		sprintf(pathbuf, "[�����ż�,�밴w�鿴�ż�]");
	} else
		strcpy(pathbuf, pm->mtitle);
	sprintf(buf, "%*s", (80 - strlen(pathbuf)) / 2, "");
	prints("[1;44m%s%s%s[m\n", buf, pathbuf, buf);

	getvisit(visit, pm->path);

	prints("\033[1;31m[%s]\033[m \033[1;32mF\033[37m �Ļ�����"
	       "\033[32m����\033[37m�ƶ� [32m�� r \033[37m"
	       "��ȡ \033[32m�� q\033[37m �뿪 \033[1;32mh\033[37m ���� \033[33m���ͳ�ƣ�%4d��%4d%s\n",
	       (pm->level & PERM_BOARDS) ? "��  ��" : "���ܼ�",
	       visit[0],
	       (visit[1] > 100000) ? (visit[1] / 3600) : (visit[1] / 60),
	       (visit[1] > 100000) ? "ʱ" : "��");

	prints("[1;44;37m ��� %-45s ��  ��           %8s [m",
	       "[���] ��    ��", (a_fmode == 2
				   && (pm->level & PERM_BOARDS) !=
				   0) ? "��������" : "�༭����");
	prints("\n");
	if (pm->num == 0)
		prints("      << Ŀǰû������ >>\n");
	for (n = pm->page; n < pm->page + A_PAGESIZE && n < pm->num; n++) {
		overlen = 0;
		strsncpy(title, pm->item[n]->title, sizeof (title));
		if (a_fmode) {
			snprintf(fname, STRLEN, "%s", pm->item[n]->fname);
			if (snprintf(pathbuf, PATHLEN, "%s/%s", pm->path, fname)
			    > PATHLEN - 1)
				overlen = 1;
			/*���϶�level�ļ��, ʹ�ó������Լ��Ĺ�Ͻ��ʱ�������ļ��� ylsdd */
			if (a_fmode == 2 && (pm->level & PERM_BOARDS) != 0) {
				ch = (dashf(pathbuf) ? ' '
				      : (dashd(pathbuf) ? '/' : ' '));
				fname[10] = '\0';
			} else {
				if (dashf(pathbuf) || dashd(pathbuf)) {
					stat(pathbuf, &st);
					mtime = st.st_mtime;
				} else
					mtime = time(0);

				pt = localtime(&mtime);
				sprintf(fname,
					"[1m%04d[m.[1m%02d[m.[1m%02d[m",
					1900 + pt->tm_year, pt->tm_mon + 1,
					pt->tm_mday);
				ch = ' ';
			}
			if (overlen) {
				strcpy(kind, "[\033[1;32m̫��\033[m]");
			} else if (pm->item[n]->host != NULL) {
				if (pm->item[n]->fname[0] == '0')
					strcpy(kind, "[[1;32m����[m]");
				else
					strcpy(kind, "[[1;33m��Ŀ[m]");
			} else if (dashf(pathbuf)) {
				if (dashl(pathbuf))
					strcpy(kind, "[[1;36m����[m]");
				else
					strcpy(kind, "[\033[1;36m�ļ�\033[m]");
			} else if (dashd(pathbuf)) {
				if (dashl(pathbuf))
					strcpy(kind, "[[1m��Ŀ[m]");
				else
					strcpy(kind, "[\033[1mĿ¼\033[m]");
			} else {
				strcpy(kind, "[[1;32m����[m]");
			}
			if (!strncmp(title, "[Ŀ¼] ", 7)
			    || !strncmp(title, "[�ļ�] ", 7)
			    || !strncmp(title, "[��Ŀ] ", 7)
			    || !strncmp(title, "[����] ", 7))
				sprintf(pathbuf, "%-s %-55.55s%-s%c", kind,
					title + 7, fname, ch);
			else
				sprintf(pathbuf, "%-s %-55.55s%-s%c", kind,
					title, fname, ch);
			strncpy(title, pathbuf, STRLEN * 2);
			title[STRLEN * 2 - 1] = '\0';
		}
		prints("  %3d %s\n", n + 1, title);
	}
	clrtobot();
	move(t_lines - 1, 0);
	update_endline();
}

static void
a_additem(pm, title, fname, host, port)
MENU *pm;
char *title, *fname, *host;
int port;
{
	ITEM *newitem;
	char *ptr;

	if (countstr(pm->path, "/") < countstr(fname, ".."))
		return;
	ptr = fname;
	while (*ptr) {
		if (strchr(";|,$%^&*(!@=+\\/[]", *ptr) != NULL)
			return;
		ptr++;
	}

	if (pm->num < MAXITEMS) {
		newitem = (ITEM *) malloc(sizeof (ITEM));
		strsncpy(newitem->title, title, sizeof (newitem->title));
		if (host != NULL) {
			newitem->host =
			    (char *) malloc(sizeof (char) * (strlen(host) + 1));
			strcpy(newitem->host, host);
		} else
			newitem->host = host;
		newitem->port = port;
		strsncpy(newitem->fname, fname, sizeof (newitem->fname));
		pm->item[(pm->num)++] = newitem;
	}
}

int
countstr(char *s0, char *s1)
{
	int i = 0, j;
	char *ptr = s0;
	j = strlen(s1);
	while ((ptr = strstr(ptr, s1)) != NULL) {
		i++;
		ptr += j;
	}
	return i;
}

static int
a_loadnames(pm)
MENU *pm;
{
	FILE *fn;
	ITEM litem;
	char buf[PATHLEN], *ptr;
	char hostname[STRLEN];

	pm->num = 0;
	if (snprintf(buf, PATHLEN, "%s/.Names", pm->path) > PATHLEN - 1)
		return 0;
	if ((fn = fopen(buf, "r")) == NULL)
		return 0;
	hostname[0] = '\0';
	while (fgets(buf, sizeof (buf), fn) != NULL) {
		if ((ptr = strchr(buf, '\n')) != NULL)
			*ptr = '\0';
		if (strncmp(buf, "Name=", 5) == 0) {
			bzero(litem.title, sizeof (litem.title));	/*add by ylsdd */
			strncpy(litem.title, buf + 5, 72);
			litem.title[71] = '\0';
		} else if (strncmp(buf, "Path=", 5) == 0) {
			if (strncmp(buf, "Path=~/", 7) == 0)
				strncpy(litem.fname, buf + 7, 80);
			else
				strncpy(litem.fname, buf + 5, 80);
			litem.fname[79] = '\0';
			if (((!strstr(litem.title + 38, "(BM: BMS)")
			      || HAS_PERM(PERM_BOARDS))
			     && (!strstr(litem.title + 38, "(BM: SYSOPS)")
				 || HAS_PERM(PERM_SYSOP))
			     && (strstr(litem.title, "<HIDE>") != litem.title))
			    || (pm->level & PERM_BOARDS)) {	/*modified by ylsdd */
				if (strstr(litem.fname, "!@#$%")) {
					char *ptr1, *ptr2, gtmp[STRLEN];

					strcpy(gtmp, litem.fname);
					ptr1 = strtok(gtmp, "!#$%@");
					strcpy(hostname, ptr1);
					ptr2 = strtok(NULL, "@");
					strcpy(litem.fname, ptr2);
					litem.port = atoi(strtok(NULL, "@"));
				}
				a_additem(pm, litem.title, litem.fname,
					  (strlen(hostname) ==
					   0) ? NULL : hostname, litem.port);
			}
			hostname[0] = '\0';
		} else if (strncmp(buf, "# Title=", 8) == 0) {
			if (pm->mtitle[0] == '\0') {
				strsncpy(pm->mtitle, buf + 8,
					 sizeof (pm->mtitle));
			}
		} else if (strncmp(buf, "Host=", 5) == 0) {
			strncpy(hostname, buf + 5, STRLEN);
			hostname[STRLEN - 1] = 0;
		} else if (strncmp(buf, "Port=", 5) == 0) {
			litem.port = atoi(buf + 5);
		}
	}
	fclose(fn);
	return 1;
}

static void
a_savenames(pm)
MENU *pm;
{
	FILE *fn;
	ITEM *item;
	char fpath[PATHLEN];
	int n;

	if (snprintf(fpath, PATHLEN, "%s/.Names", pm->path) > PATHLEN - 1)
		return;
	if ((fn = fopen(fpath, "w")) == NULL)
		return;
	fprintf(fn, "#\n");
	if (!strncmp(pm->mtitle, "[Ŀ¼] ", 7)
	    || !strncmp(pm->mtitle, "[�ļ�] ", 7)
	    || !strncmp(pm->mtitle, "[��Ŀ] ", 7)
	    || !strncmp(pm->mtitle, "[����] ", 7)) {
		fprintf(fn, "# Title=%s\n", pm->mtitle + 7);
	} else {
		fprintf(fn, "# Title=%s\n", pm->mtitle);
	}
	fprintf(fn, "#\n");
	for (n = 0; n < pm->num; n++) {
		item = pm->item[n];
		if (!strncmp(item->title, "[Ŀ¼] ", 7)
		    || !strncmp(item->title, "[�ļ�] ", 7)
		    || !strncmp(item->title, "[��Ŀ] ", 7)
		    || !strncmp(item->title, "[����] ", 7)) {
			fprintf(fn, "Name=%s\n", item->title + 7);
		} else
			fprintf(fn, "Name=%s\n", item->title);
		if (item->host != NULL) {
			fprintf(fn, "Host=%s\n", item->host);
			fprintf(fn, "Port=%d\n", item->port);
			fprintf(fn, "Type=1\n");
			fprintf(fn, "Path=%s\n", item->fname);
		} else
			fprintf(fn, "Path=~/%s\n", item->fname);
		fprintf(fn, "Numb=%d\n", n + 1);
		fprintf(fn, "#\n");
	}
	fclose(fn);
	chmod(fpath, 0664);
}

void
a_prompt(bot, pmt, buf, len)
int bot;
char *pmt, *buf;
int len;
{
	move(t_lines + bot, 0);
	clrtoeol();
	getdata(t_lines + bot, 0, pmt, buf, len, DOECHO, YEA);
}

/* added by netty to handle post saving into (0)Announce */
int
a_Save(path, key, fileinfo, nomsg)
char *path, *key;
struct fileheader *fileinfo;
int nomsg;
{

	char board[40];
	int ans = NA;
	FILE *fps, *fpd;
	char line[200];
	if (!nomsg) {
		sprintf(genbuf, "ȷ���� [%-.40s] �����ݴ浵��",
			fileinfo->title);
		if (askyn(genbuf, YEA, YEA) == NA)
			return FULLUPDATE;
	}
	sprintf(board, "bm/%s", currentuser.userid);
	if (dashf(board)) {
		if (nomsg)
			ans = YEA;
		else
			ans = askyn("Ҫ�����ھ��ݴ浵֮����", YEA, YEA);
	}
	if (in_mail) {
		sprintf(genbuf,
			"/bin/%s mail/%c/%s/%s %s %s &>/dev/null",
			(ans) ? "cat" : "cp -r",
			mytoupper(currentuser.userid[0]), currentuser.userid,
			fh2fname(fileinfo), (ans) ? ">>" : "", board);
		system(genbuf);
	} else {
		sprintf(genbuf, "boards/%s/%s", key, fh2fname(fileinfo));
		fps = fopen(genbuf, "r");
		if (fps == NULL)
			return FULLUPDATE;
		fpd = fopen(board, (ans) ? "a+" : "w");
		if (fpd == NULL) {
			fclose(fps);
			return FULLUPDATE;
		}
		keepoldheader(fps, SKIPHEADER);
		fputs
		    ("\033[1;36m������������������������������������������������������������������������������\033[0m\n",
		     fpd);
		fprintf(fpd, "\033[0;44;37m ���� %-14s ʱ�� %-52s\033[0m\n",
			fh2owner(fileinfo), Ctime(fileinfo->filetime));
		fputs
		    ("\033[1;36m������������������������������������������������������������������������������\033[0m\n",
		     fpd);
		while (fgets(line, sizeof (line), fps) != NULL) {
			if (transferattach(line, sizeof (line), fps, fpd))
				continue;
			if (!strcmp(line, "--\n") || !strcmp(line, "--\r\n"))
				break;
			if (!strncmp(line, ": : ", 4))
				continue;
			fputs(line, fpd);
		}
		fclose(fps);
		fclose(fpd);
	}
	//  sprintf( genbuf, "/bin/%s boards/%s/%s %s %s",(ans)?"cat":"cp -r",
	//     key , fileinfo->filename, (ans)?">>":"",board);
	// system( genbuf );
	if (!nomsg)
		a_prompt(-1, "�ѽ������´����ݴ浵, �밴<Enter>����...", genbuf,
			 2);
	return FULLUPDATE;
}

/* added by netty to handle post saving into (0)Announce */

char Importname[PATHLEN];

int
check_import(char *anboard)
{
	struct boardmem *bp;
	char *ptr;
	if (!strncmp(Importname, "0Announce/groups/GROUP_", 23)) {
		strncpy(anboard, &(Importname[25]), STRLEN);
		anboard[STRLEN - 1] = 0;
		ptr = index(anboard, '/');
		if (ptr != NULL)
			*ptr = 0;
		//by bjgyt if (!strcmp(anboard, "Personal_Corpus"))
		if (!strcmp(anboard, "PersonalCorpus"))
			return 0;
		bp = getbcache(anboard);
		if (bp == NULL)
			return -1;
		if (!chk_currBM(&(bp->header), 1))
			return -2;
		return 0;
	} else if (HAS_PERM(PERM_BLEVELS)) {
		strcpy(anboard, "noboard");
		return 0;
	} else
		return -2;
}

int
an_log(char *action, char *path)
{
	char anboard[STRLEN], *ptr, buf[256];
	if (!strncmp(path, "0Announce/groups/GROUP_", 23)) {
		strncpy(anboard, &(path[25]), STRLEN);
		anboard[STRLEN - 1] = 0;
		ptr = index(anboard, '/');
		if (ptr != NULL)
			*ptr = 0;
	} else
		strcpy(anboard, "noboard");
	snprintf(buf, 256, "%s %s %s %s", currentuser.userid, action, anboard,
		 path);
	newtrace(buf);
	return 0;
}

int
a_Import(direct, fileinfo, nomsg)
char *direct;
struct fileheader *fileinfo;
int nomsg;
{

	char fname[STRLEN], *ip, bname[PATHLEN];
	char ans[5], filepath[PATHLEN];
	MENU pm;
	char anboard[STRLEN], tmpboard[STRLEN];

	if (!nomsg) {
		if (select_anpath() < 0)
			return 0;
		switch (check_import(anboard)) {
		case 0:
			break;
		case -1:
			a_prompt(-1,
				 "���趨�ĸ�˿·�����ڲ�����,��֪ͨϵͳά��",
				 ans, 2);
			return 0;
		default:
			a_prompt(-1, "���趨�ĸ�˿·��ʧЧ,����������", ans, 2);
			return 0;
		}
	}

	if (!dashd(Importname)) {
		if (!nomsg)
			a_prompt(-1, "���趨�ĸ�˿·�Ѷ�ʧ,����������", ans, 2);
		return 0;
	}
	if (!fileinfo->filetime) {
		if (!nomsg)
			a_prompt(-1, "��Ч�ļ�,�޷����뾫����", ans, 2);
		return 0;
	}
	if ((!strcmp(currboard, anboard)) || nomsg == 2) {
		fileinfo->accessed |= FH_ANNOUNCE;
	}

	modify_user_mode(DIGEST);
	pm.path = Importname;
	strcpy(pm.mtitle, "");
	pm.level |= PERM_BOARDS;	/* add by ylsdd */
	strcpy(fname, fh2fname(fileinfo));
	if (snprintf(bname, PATHLEN, "%s/%s", pm.path, fname) > PATHLEN - 1)
		return -1;

	a_loadnames(&pm);
	ip = &fname[strlen(fname) - 1];
	while (dashf(bname)) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(bname, "%s/%s", pm.path, fname);
	}
	sprintf(genbuf, "%-38.38s %s ", fileinfo->title, currentuser.userid);
	a_additem(&pm, genbuf, fname, NULL, 0);
	a_savenames(&pm);
	/*if(in_mail)
	   sprintf( genbuf, "/bin/cp -r mail/%c/%s/%s %s 1>/dev/null 2>/dev/null", 
	   mytoupper(currentuser.userid[0]), currentuser.userid, fileinfo->filename , bname );
	   else
	   sprintf( genbuf, "/bin/cp -r boards/%s/%s %s 1>/dev/null 2>/dev/null", key , fileinfo->filename , bname );
	 */
	directfile(filepath, direct, fh2fname(fileinfo));
	copyfile(filepath, bname);
	if (0)
		if (!nomsg) {
			a_prompt(-1, "�ѽ������·Ž�������, �밴<Enter>����...",
				 ans, 2);
		}
	if (!nomsg) {
		strcpy(tmpboard, currboard);
		strcpy(currboard, anboard);
	}
	snprintf(genbuf, 256, "%s import %s %s %s", currentuser.userid,
		 currboard, fileinfo->owner, fileinfo->title);
	newtrace(genbuf);
	if (!nomsg)
		strcpy(currboard, tmpboard);
	freeitem(&pm);
	return 1;
}

static void
a_search(pm, offset)
MENU *pm;
int offset;
{
	int i;
	static char title[STRLEN];
	char ans[STRLEN], pmt[STRLEN];
	strcpy(ans, title);
	sprintf(pmt, "%s��Ѱ���� [%.16s]: ", offset > 0 ? "����" : "��ǰ", ans);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, 46, DOECHO, YEA);
	if (*ans != '\0')
		strcpy(title, ans);
	for (i = pm->now + offset; (offset == 1) ? (i < pm->num) : (i >= 0);
	     i += offset) {
		strcpy(genbuf, pm->item[i]->title);
		genbuf[38] = 0;
		if (strstr(genbuf, title)) {
			pm->now = i;
			break;
		}
	}
	update_endline();
}

int
a_menusearch(path, key, level)
char *path, *key;
int level;
{
	FILE *fn;
	char bname[20];
	char buf[PATHLEN], *ptr;
	char found[PATHLEN];
	int searchmode = 0;

	if (key == NULL) {
		key = bname;
		a_prompt(-1, "��������Ѱ֮����������: ", key, 18);
		searchmode = 1;
	}
	found[0] = '\0';
	sprintf(buf, "0Announce/.Search");
	if (key[0] != '\0' && (fn = fopen(buf, "r")) != NULL) {
		while (fgets(buf, sizeof (buf), fn) != NULL) {
			if (searchmode && !strstr(buf, "groups/"))
				continue;
			ptr = strchr(buf, ':');
			if (!ptr)
				return 0;
			else {
				*ptr = '\0';
				ptr = strtok(ptr + 1, " \t\n");
			}
			if (!strcasecmp(buf, key)) {
				sprintf(found, "0Announce/%s", ptr);
				break;
			}
		}
		fclose(fn);
		if (found[0]) {
			a_menu("", found, level, 0);
			return 1;
		} else {
			a_prompt(-1, "�Ҳ������������������, ��<Enter>����...",
				 buf, 2);
			return 1;
		}
	}
	return 0;
}

static void
a_forward(path, pitem, mode)
char *path;
ITEM *pitem;
int mode;
{
	char fname[PATHLEN], *mesg;

	if (snprintf(fname, PATHLEN, "%s/%s", path, pitem->fname) < PATHLEN
	    && dashf(fname)) {
		switch (doforward(fname, pitem->title, mode)) {
		case 0:
			mesg = "����ת�����!\n";
			break;
		case -1:
			mesg = "System error!!.\n";
			break;
		case -2:
			mesg = "Invalid address.\n";
			break;
		default:
			mesg = "ȡ��ת�Ķ���.\n";
		}
		prints(mesg);
	} else {
		move(t_lines - 1, 0);
		prints("�޷�ת�Ĵ���Ŀ.\n");
	}
	sleep(2);
	pressanykey();
}

static void
a_newitem(pm, mode)
MENU *pm;
int mode;
{
	char uident[STRLEN];
	char board[STRLEN], title[STRLEN];
	char fname[STRLEN], fpath[PATHLEN], fpath2[PATHLEN];
	FILE *pn;
	char ans[10];
	time_t now;
	int count, oldlevel;

	pm->page = 9999;
	switch (mode) {
	case ADDITEM:
	case ADDGROUP:
		break;
	case ADDMAIL:
		sprintf(board, "bm/%s", currentuser.userid);
		if (!dashf(board)) {
			a_prompt(-1,
				 "�������������������´����ݴ浵, ��<Enter>����...",
				 ans, 2);
			return;
		}
		break;
	case ADDGOPHER:
		{
			int gport;
			char ghost[STRLEN], gtitle[STRLEN], gfname[STRLEN];

			a_prompt(-2, "���ߵ�λ�ã�", ghost, STRLEN - 14);
			if (ghost[0] == '\0')
				return;
			a_prompt(-2, "���ߵ�Ŀ¼��", gfname, STRLEN - 14);
			if (gfname[0] == '\0')
				return;
			a_prompt(-2, "���ߵ�Port��", ans, 6);
			if (ans[0] == '\0')
				return;
			a_prompt(-2, "���⣺", gtitle, 70);
			if (gtitle[0] == '\0')
				return;
			gport = atoi(ans);
			a_additem(pm, gtitle, gfname, ghost, gport);
			a_savenames(pm);
			return;
		}
	}

	a_prompt(-1, "�������ļ���Ŀ¼֮�������ƣ� ", title, 35);
	if (*title == '\0')
		return;

	time(&now);
	count = 0;

	do {
		if (count++ > MAX_POSTRETRY)
			return;
		sprintf(fname, "M%d", (int) now++);
		if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, fname) >
		    PATHLEN - 1)
			return;
	} while (dashf(fpath) || dashd(fpath));

	switch (mode) {
	case ADDITEM:
		clear();
		do_delay(1);
		if (vedit(fpath, 0, YEA) == -1)
			return;
		chmod(fpath, 0660);
		break;
	case ADDGROUP:
		mkdir(fpath, 0770);
		chmod(fpath, 0770);
		break;
	case ADDMAIL:
		crossfs_rename(board, fpath);
		break;
	}
	if (mode != ADDGROUP)
		sprintf(genbuf, "%-38.38s %s ", title, currentuser.userid);
	else {
/*Add by SmallPig*/
		if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_ANNOUNCE)) {
			move(1, 0);
			clrtoeol();
/*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$*/
			getdata(1, 0, "����: ", uident, 24, DOECHO, YEA);
			if (uident[0] != '\0')
				sprintf(genbuf, "%-38.38s(BM: %s)", title,
					uident);
			else
				sprintf(genbuf, "%-38.38s", title);
		} else
			sprintf(genbuf, "%-38.38s", title);
	}

	oldlevel = pm->level;
	pm->level |= PERM_BOARDS;
	a_loadnames(pm);
	a_additem(pm, genbuf, fname, NULL, 0);
	a_savenames(pm);
	freeitem(pm);
	pm->level = oldlevel;
	a_loadnames(pm);
	an_log("additem", pm->path);
	if (mode == ADDGROUP) {
		if (snprintf(fpath2, PATHLEN, "%s/%s/.Names", pm->path, fname) <
		    PATHLEN && (pn = fopen(fpath2, "w")) != NULL) {
			fprintf(pn, "#\n");
			fprintf(pn, "# Title=%s\n", genbuf);
			fprintf(pn, "#\n");
			fclose(pn);
		}
	}
}

static void
a_moveitem(pm)
MENU *pm;
{
	ITEM *tmp;
	char newnum[STRLEN];
	int num, n;

	sprintf(genbuf, "������� %d ����´���: ", pm->now + 1);
	a_prompt(-2, genbuf, newnum, 6);
	num = (newnum[0] == '$') ? 9999 : atoi(newnum) - 1;
	if (num >= pm->num)
		num = pm->num - 1;
	else if (num < 0)
		return;
	tmp = pm->item[pm->now];
	if (num > pm->now) {
		for (n = pm->now; n < num; n++)
			pm->item[n] = pm->item[n + 1];
	} else {
		for (n = pm->now; n > num; n--)
			pm->item[n] = pm->item[n - 1];
	}
	pm->item[num] = tmp;
	pm->now = num;
	a_savenames(pm);
	an_log("moveitem", pm->path);

}

static void a_delete(MENU *, int);
static int a_repair(MENU *);
static int a_rjunk(MENU *);

static void
a_copypaste(pm, paste)
MENU *pm;
int paste;			// -1:cut 0:copy have perm 1:paste 2:copy have no perm
{
	char title[STRLEN], filename[STRLEN], fpath[PATHLEN];
	ITEM *item;
	char newpath[PATHLEN], ans[3], realfpath[PATH_MAX + 1],
	    realnewpath[PATH_MAX + 1];
	FILE *fn;		//add by gluon for copypate in two window
	int x, y, hasc;
	MENU pmforcut;

	move(t_lines - 1, 0);
	if (paste != 1) {
		copymode = paste;
		paste = 0;
	}
	if (!paste) {
#define AN_PATH "0Announce/groups/GROUP_0/"
#define AN_PES_PATH AN_PATH "PersonalCorpus"
		item = pm->item[pm->now];
		strsncpy(title, item->title, sizeof (title));
		strsncpy(filename, item->fname, sizeof (filename));
		if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, filename) >
		    PATHLEN - 1) {
			prints("����·������, �޷�����");
			egetch();
			return;
		}
		if ((strlen(fpath) < sizeof (AN_PATH)
		     || !strncmp(fpath, AN_PES_PATH, sizeof (AN_PES_PATH) - 1))
		    && (HAS_PERM(PERM_ANNOUNCE) || HAS_PERM(PERM_SYSOP)
			|| HAS_PERM(PERM_OBOARDS))) {
			prints("վ�����ܿ��������ļ�, ��������, ��ϵϵͳά��");
			egetch();
			return;
		}
		//add by gluon
		sprintf(genbuf, MY_BBS_HOME "/tmp/%s.copypaste",
			currentuser.userid);
		if ((fn = fopen(genbuf, "w+")) == NULL) {
			prints("���õ�����ʶ����, ����վ������. Thanks");
			egetch();
			return;
		}
		fwrite(title, sizeof (item->title), 1, fn);
		fwrite(filename, sizeof (item->fname), 1, fn);
		fwrite(fpath, sizeof (fpath), 1, fn);
		fwrite(&copymode, sizeof (int), 1, fn);
		fclose(fn);
		//end
		if (copymode >= 0)
			prints
			    ("������ʶ���. (ע��! ճ�����º���ܽ����� delete!)");
		else
			prints("������ʶ���. ճ���󽫱�ɾ��.");
		egetch();
	} else {
		//add by gluon
		sprintf(genbuf, MY_BBS_HOME "/tmp/%s.copypaste",
			currentuser.userid);
		if ((fn = fopen(genbuf, "r")) == NULL) {
			prints("����ʹ�� copy �� cut ������ʹ�� paste ����.");
			egetch();
			return;
		}
		fread(title, sizeof (item->title), 1, fn);
		fread(filename, sizeof (item->fname), 1, fn);
		fread(fpath, sizeof (fpath), 1, fn);
		fread(&copymode, sizeof (copymode), 1, fn);
		fclose(fn);
		//end
		if (snprintf(newpath, PATHLEN, "%s/%s", pm->path, filename) >
		    PATHLEN - 1) {
			prints("Ŀ¼·��̫��,�޷�ճ��");
			egetch();
		} else if (realpath(pm->path, realnewpath) == NULL
			   || realpath(fpath, realfpath) == NULL) {
			prints("ϵͳ�ڲ�����,����ϵϵͳά��!");
			egetch();
		} else if (*title == '\0' || *filename == '\0') {
			prints("����ʹ�� copy �� cut ������ʹ�� paste ����. ");
			egetch();
			//add by gluon
		} else if (!(dashf(fpath) || dashd(fpath))) {
			prints("�㿽���ĵ���/Ŀ¼������,���ܱ�ɾ��,ȡ��ճ��.");
			egetch();
			//end
		} else if (dashf(newpath) || dashd(newpath)) {
			prints("%s ����/Ŀ¼�Ѿ�����. ", filename);
			egetch();
		} else if (strstr(newpath, fpath) != NULL
			   || strstr(realnewpath, realfpath) != NULL) {
			prints("�޷���Ŀ¼����Լ�����Ŀ¼��, ���������Ȧ.");
			egetch();
		} else {
			char defaultans[3];
			switch (copymode) {
			case -1:	//cut
				defaultans[0] = 'C';
				sprintf(genbuf,
					"�Ƿ�ճ��: (C)ճ�� (N)ȡ�� (C):");
				break;
			case 0:	//copy with perm
				defaultans[0] = 'C';
				sprintf(genbuf,
					"����ѡ��ճ����ʽ: (C)ճ�� (L)���� (N)ȡ�� (C):");
				break;
			case 2:	//copy without perm
				defaultans[0] = 'L';
				sprintf(genbuf,
					"�Ƿ񴴽�����: (L)���� (N)ȡ�� (L):");
				break;
			}
			getyx(&x, &y);
			getdata(x, y, genbuf, ans, 2, DOECHO, YEA);

			if (ans[0] != 'c' && ans[0] != 'C' && ans[0] != 'l'
			    && ans[0] != 'L' && ans[0] != 'n' && ans[0] != 'N')
				ans[0] = defaultans[0];
			hasc = 1;
			switch (ans[0]) {
			case 'N':
			case 'n':
				hasc = 0;
				break;
			case 'L':
			case 'l':
				if (copymode >= 0) {
					symlink(realfpath, newpath);
					break;
				}
			case 'C':
			case 'c':
				if (copymode == -1) {
					char oldpath[PATHLEN];
					char *ptr;
					int i;
					strncpy(oldpath, fpath, PATHLEN - 1);
					oldpath[PATHLEN - 1] = 0;
					ptr = strrchr(oldpath, '/');
					*ptr = 0;
					pmforcut.path = oldpath;
					pmforcut.mtitle[0] = 0;
					pmforcut.level |= PERM_BOARDS;
					a_loadnames(&pmforcut);
					for (i = 0; i < pmforcut.num; i++)
						if (!strcmp
						    (pmforcut.item[i]->fname,
						     filename)) {
							pmforcut.now = i;
							break;
						}
					if (i < pmforcut.num) {
						a_delete(&pmforcut, 1);
						rename(fpath, newpath);
						sprintf(genbuf,
							MY_BBS_HOME
							"/tmp/%s.copypaste",
							currentuser.userid);
						unlink(genbuf);
					}
					freeitem(&pmforcut);
				} else if (copymode == 2) {
					symlink(realfpath, newpath);
				} else {
					sprintf(genbuf,
						"/bin/cp -a %s %s 1>/dev/null 2>/dev/null",
						fpath, newpath);
					system(genbuf);
				}
			}
			if (hasc) {
				a_additem(pm, title, filename, NULL, 0);
				a_savenames(pm);
				an_log("paste", newpath);
			}
		}
	}
	pm->page = 9999;
}

static void
a_delete(pm, mode)
MENU *pm;
int mode;
{
	ITEM *item;
	char fpath[PATHLEN];
	int n;

	item = pm->item[pm->now];
	move(t_lines - 2, 0);
	prints("%5d  %-50s\n", pm->now + 1, item->title);
	if (mode == 0) {
		if (item->host == NULL) {
			if (snprintf
			    (fpath, PATHLEN, "%s/%s", pm->path,
			     item->fname) > PATHLEN - 1) {
				prints("Ŀ¼����,����ϵϵͳά��!");
				egetch();
				return;
			} else if (dashl(fpath)) {
				if (askyn("ɾ������Ŀ, ȷ����", NA, YEA) == NA)
					return;
				unlink(fpath);
			} else if (dashf(fpath)) {
				if (askyn("ɾ���˵���, ȷ����", NA, YEA) == NA)
					return;
				deltree(fpath);
			} else if (dashd(fpath)) {
				if (askyn
				    ("ɾ��������Ŀ¼, ����ЦŶ, ȷ����", NA,
				     YEA) == NA)
					return;
				deltree(fpath);
			}
		} else {
			if (askyn("ɾ������ѡ��, ȷ����", NA, YEA) == NA)
				return;
		}
	}
	free(item);
	(pm->num)--;
	for (n = pm->now; n < pm->num; n++)
		pm->item[n] = pm->item[n + 1];
	a_savenames(pm);
}

static int
a_changemtitle(char *fpath, char *newmtitle)
{
	MENU pm;
	pm.path = fpath;
	pm.level |= PERM_BOARDS;
	a_loadnames(&pm);
	strsncpy(pm.mtitle, newmtitle, sizeof (pm.mtitle));
	a_savenames(&pm);
	freeitem(&pm);
	return 0;
}

static void
a_newname(pm)
MENU *pm;
{
	ITEM *item;
	char fname[STRLEN];
	char fpath[PATHLEN];
	char *mesg;

	item = pm->item[pm->now];
	a_prompt(-2, "�µ���: ", fname, 13);
	if (*fname == '\0')
		return;
	if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, fname) > PATHLEN - 1)
		mesg = "·��̫��";
	//����������IDһ����ַ�����ΪĿ¼��, Ϊ�����ļ�.
	else if (!valid_fname(fname) && !goodgbid(fname)) {
		mesg = "���Ϸ���������.";
	} else if (dashf(fpath) || dashd(fpath)) {
		mesg = "ϵͳ�����д˵���������.";
	} else {
		sprintf(genbuf, "%s/%s", pm->path, item->fname);
		if (rename(genbuf, fpath) == 0) {
			strcpy(item->fname, fname);
			a_savenames(pm);
			return;
		}
		mesg = "��������ʧ��!!";
	}
	prints(mesg);
	egetch();
}

static void
a_manager(pm, ch)
MENU *pm;
int ch;
{
	char uident[STRLEN];
	ITEM *item = NULL;
	char fpath[PATHLEN], changed_T[STRLEN], ans[5], fname[STRLEN],
	    newfpath[PATHLEN];
	time_t now;
	int count, ret;

	if (pm->num > 0) {
		item = pm->item[pm->now];
		if (snprintf(fpath, PATHLEN, "%s/%s", pm->path, item->fname) >
		    PATHLEN - 1)
			return;
	}
	if (!strchr("agiGpfsMDVvTEnx", ch) && ch != Ctrl('r')
	    && ch != Ctrl('e'))
		return;
	modify_user_mode(EDITANN);
	switch (ch) {
	case 'a':
		a_newitem(pm, ADDITEM);
		break;
	case 'g':
		a_newitem(pm, ADDGROUP);
		break;
	case 'i':
		a_newitem(pm, ADDMAIL);
		break;
	case 'G':
		if (HAS_PERM(PERM_SYSOP))
			a_newitem(pm, ADDGOPHER);
		break;
	case 'p':
		a_copypaste(pm, 1);
		break;
		/*�ռ���ʧ��Ŀ, add by ylsdd */
	case Ctrl('r'):
		sprintf(genbuf,
			"���� %d ����ʧ��Ŀ, �밴<Enter>����...", a_repair(pm));
		a_prompt(-1, genbuf, ans, 2);
		pm->page = 9999;
		break;
	/*--- end add by ylsdd ---*/

	case Ctrl('e'):
		sprintf(genbuf,
			"��� %d ������,�밴<Enter>����...", a_rjunk(pm));
		a_prompt(-1, genbuf, ans, 2);
		pm->page = 9999;
		break;

	case 'f':
		pm->page = 9999;
		add_anpath(pm->mtitle, pm->path);
		break;
	}
	if (pm->num > 0)
		switch (ch) {
		case 's':
			if (++a_fmode >= 3)
				a_fmode = 1;
			pm->page = 9999;
			break;
		case 'M':
			a_moveitem(pm);
			pm->page = 9999;
			break;
		case 'D':
			a_delete(pm, 0);
			pm->page = 9999;
			break;
		case 'V':
		case 'v':
			if (HAS_PERM(PERM_SYSOP)) {
				if (ch == 'v')
					sprintf(fpath, "%s/.Names", pm->path);
				else
					sprintf(fpath, "0Announce/.Search");

				if (dashf(fpath)) {
					vedit(fpath, 0, YEA);
				}
				pm->page = 9999;
			}
			break;
		case 'T':
			ret = 0;
			if (dashl(fpath))
				break;
			strsncpy(changed_T, item->title, 39);
			{
				int i = strlen(changed_T) - 1;
				while (i > 0 && isspace(changed_T[i]))
					changed_T[i--] = 0;
			}
			move(t_lines - 2, 0);
			clrtoeol();
			getdata(t_lines - 2, 0, "�±��⣺", changed_T, 39,
				DOECHO, NA);
			pm->page = 9999;
			if (!*changed_T)
				break;
			/* modified by netty to properly handle title change,add bm by SmallPig */
			if (dashf(fpath)) {
				sprintf(genbuf, "%-38.38s %s ",
					changed_T, currentuser.userid);
				strsncpy(item->title, genbuf,
					 sizeof (item->title));
			} else if (dashd(fpath)) {
				if (HAS_PERM(PERM_SYSOP)
				    || HAS_PERM(PERM_ANNOUNCE)) {
					char *dir = fpath + 25;
					char *rcon;
					move(1, 0);
					clrtoeol();
					/*usercomplete("����: ",uident) ; */
					/*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$ */
					getdata(1, 0, "����: ", uident,
						35, DOECHO, YEA);
					rcon =
					    malloc(strlen(dir) +
						   strlen(changed_T) + 100);
					sprintf(genbuf,
						"�޸�%.34s�ı���Ϊ:%.34s,BM:%.34s",
						dir, changed_T, uident);
					if (NULL != rcon)
						sprintf(rcon,
							"�޸�%s�ı���Ϊ:%s,BM:%s",
							dir, changed_T, uident);
					else
						rcon = "";
					securityreport(genbuf, rcon);
					if (strcmp(rcon, ""))
						free(rcon);
					if (uident[0] != '\0')
						sprintf(genbuf,
							"%-38.38s(BM: %s)",
							changed_T, uident);
					else
						sprintf(genbuf,
							"%-38.38s", changed_T);
				} else
					sprintf(genbuf, "%-38.38s", changed_T);
				if ((!strstr(changed_T, "<GUESTBOOK>")
				     && strstr(item->title, "<GUESTBOOK>"))
				    || (strstr(changed_T, "<HIDE>")
					&& !strstr(item->title, "<HIDE>"))) {
					//���������жϣ���ҪrenameĿ¼
					time(&now);
					count = 0;
					do {
						if (count++ > MAX_POSTRETRY) {
							ret = -1;
							break;
						}
						sprintf(fname, "M%d",
							(int) now++);
						if (snprintf
						    (newfpath, PATHLEN, "%s/%s",
						     pm->path,
						     fname) > PATHLEN - 1) {
							ret = -2;
							break;
						}
					} while (dashf(newfpath)
						 || dashd(newfpath));
					if (ret == 0) {
						if (rename(fpath, newfpath) ==
						    0) strcpy(fpath, newfpath);
						strcpy(item->fname, fname);
					}
				}
				strsncpy(item->title, genbuf,
					 sizeof (item->title));
			} else if (pm->item[pm->now]->host != NULL)
				strsncpy(item->title, changed_T,
					 sizeof (item->title));
			if (ret == 0) {
				a_savenames(pm);
				a_changemtitle(fpath, genbuf);
			}
			pm->page = 9999;
			break;
		case 'E':
			if (!dashl(fpath) && dashf(fpath)) {
				if (vedit(fpath, 0, YEA) >= 0) {
					sprintf(genbuf, "%-38.38s %s",
						item->title,
						currentuser.userid);
					strsncpy(item->title, genbuf,
						 sizeof (item->title));
					a_savenames(pm);
				}
				pm->page = 9999;
			}
			break;
		case 'n':
			a_newname(pm);
			pm->page = 9999;
			break;
		case 'x':
			a_copypaste(pm, -1);
			break;
		}
	modify_user_mode(DIGEST);
}

void
a_menu(maintitle, path, lastlevel, lastbmonly)
char *maintitle, *path;
int lastlevel, lastbmonly;
{
	MENU me;
	char fname[PATHLEN], tmp[STRLEN];
	int ch;
	char *bmstr;
	int bmonly;
	int number = 0;
	int retvBM = 0;
	time_t timein = 0;
	int firstlink = 0;

	modify_user_mode(DIGEST);
	me.path = path;
	strncpy(me.mtitle, maintitle, STRLEN - 1);
	me.mtitle[STRLEN - 1] = 0;
	me.level = lastlevel;
	bmonly = lastbmonly;

	a_loadnames(&me);
	bmstr = strstr(me.mtitle + 38, "(BM:");	/*buf+38��Ϊ�˱�����������ݻ����� */
	if (bmstr != NULL) {
		retvBM = chk_currBM_Personal(bmstr + 4);
		switch (retvBM) {
		case 3:	/*PERM_BLEVELS�Ҹ�Ŀ¼�ǲ��������Լ���
				   ����Ŀ¼, ���ܽ��������Լ���Ŀ¼ */
			freeitem(&me);
			return;
		case 2:	/*�ǰ��� */
		case 1:	/*�Ǹ��˰�, ���ҵ�ǰID��ӵ���� */
			if ((me.level & PERM_BOARDS) == 0) {
				me.level |= PERM_BOARDS;
				freeitem(&me);
				a_loadnames(&me);
			}
			break;
		case -1:	/*�Ǹ��˰�, ���ҵ�ǰID����ӵ���� */
			if ((me.level & PERM_BOARDS) != 0) {
				me.level &= ~PERM_BOARDS;
				freeitem(&me);
				a_loadnames(&me);
			}
			break;
		default:
			break;
		}
		if (strstr(bmstr, "(BM: BMS)")
		    || strstr(bmstr, "(BM: SECRET)")
		    || strstr(bmstr, "(BM: SYSOPS)"))
			bmonly = 1;
	}
	if (bmonly == 1 && !(me.level & PERM_BOARDS)) {
		freeitem(&me);
		return;
	}

	if (!(me.level & PERM_BOARDS))	/*���ǰ��񣬽��з���ͳ�� */
		time(&timein);

	if (dashl(path)) {
		if (!inlink)
			firstlink = 1;
		inlink = 1;
	}
	if (inlink) {
		me.level &= ~PERM_BOARDS;
		freeitem(&me);
		a_loadnames(&me);
	}
	me.page = 9999;
	me.now = 0;
	while (1) {
		if (me.now >= me.num && me.num > 0) {
			me.now = me.num - 1;
		} else if (me.now < 0) {
			me.now = 0;
		}
		if (me.now < me.page || me.now >= me.page + A_PAGESIZE) {
			me.page = me.now - (me.now % A_PAGESIZE);
			a_showmenu(&me);
		}
		move(3 + me.now - me.page, 0);
		prints("->");
		can_R_endline = 1;
		ch = egetch();
		can_R_endline = 0;
		move(3 + me.now - me.page, 0);
		prints("  ");
		if (ch == 'Q' || ch == 'q' || ch == KEY_LEFT || ch == EOF)
			break;
	      EXPRESS:		/* add by djq,990725 */
		switch (ch) {
		case KEY_UP:
		case 'K':
		case 'k':
			if (--me.now < 0)
				me.now = me.num - 1;
			break;
		case KEY_DOWN:
		case 'J':
		case 'j':
			if (++me.now >= me.num)
				me.now = 0;
			break;
		case KEY_PGUP:
		case Ctrl('B'):
			if (me.now >= A_PAGESIZE)
				me.now -= A_PAGESIZE;
			else if (me.now > 0)
				me.now = 0;
			else
				me.now = me.num - 1;
			break;
		case KEY_PGDN:
		case Ctrl('F'):
		case ' ':
			if (me.now < me.num - A_PAGESIZE)
				me.now += A_PAGESIZE;
			else if (me.now < me.num - 1)
				me.now = me.num - 1;
			else
				me.now = 0;
			break;
		case Ctrl('C'):
			if (me.num == 0)
				break;
			if (!HAS_PERM(PERM_POST))
				break;
			if (snprintf
			    (fname, PATHLEN, "%s/%s", path,
			     me.item[me.now]->fname) > PATHLEN - 1)
				break;	//add by ylsdd
			if (!dashf(fname))
				break;
			if (me.now >= me.num)
				break;
			{
				char bname[30];
				clear();
				prints
				    ("[1m��ע�⣺��վվ��涨��������ͬ�����Ƶ������Ͻ���[31m3(����)[37m�������������ظ�������\n");
				prints
				    ("[1mת������3���������߳��������»ᱻȫ��ɾ��֮�⣬����������ȫվ�������µ�Ȩ����\n");
				prints
				    ("[1m             ���ҹ�ͬά�� BBS �Ļ�������ʡϵͳ��Դ��лл������\n[0m");
				move(4, 0);
				if (!get_a_boardname
				    (bname, "������Ҫת��������������: ")) {
					me.page = 999;
					break;
				}
				//add by macintosh 050516
				if (seek_in_file("deny_users", currentuser.userid)
					&& strcmp(bname, "sysop") 
					&& strcmp(bname,"committee")
					&& strcmp(bname, "welcome")
					&& strcmp(bname, "KaoYan")
					&& strcmp(bname, "Appeal") 
				       && !HAS_PERM(PERM_SYSOP)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n                 �ܱ�Ǹ���㱻վ��ֹͣȫվ POST ��Ȩ����");
					pressreturn();
					me.page = 999;
					break;
				}
				if (deny_me(bname)
				    && !HAS_PERM(PERM_SYSOP)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n                 �ܱ�Ǹ���㱻����ֹͣ POST ��Ȩ����");
					pressreturn();
					me.page = 999;
					break;
				}
				if (!haspostperm(bname)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n               ������Ȩ���� %s ��������",
					     bname);
					pressreturn();
					me.page = 999;
					break;
				}
				if (noadm4political(bname)) {
					move(5, 0);
					clrtobot();
					prints
					    ("\n\n               �Բ���,��Ϊû�а��������Ա����,������ʱ���.");
					pressreturn();
					me.page = 999;
					break;
				}
				move(5, 0);
				sprintf(tmp, "��ȷ��Ҫת���� %s ����", bname);
				if (askyn(tmp, NA, NA) != 1) {
					me.page = 999;
					break;
				}
				if (postfile
				    (fname, bname,
				     me.item[me.now]->title, 2) != -1) {
					move(7, 0);
					sprintf(tmp,
						"[1m�Ѿ�����ת���� %s ����[m",
						bname);
					prints(tmp);
				}
				refresh();
				sleep(1);
			}
			me.page = 9999;
			break;
		case 'w':
			if ((in_mail != YEA) && HAS_PERM(PERM_READMAIL))
				m_read();
			me.page = 9999;
			break;
		case 'h':
			show_help("help/announcereadhelp");
			me.page = 9999;
			break;
		case '\n':
		case '\r':
			if (number > 0) {
				me.now = number - 1;
				number = 0;
				continue;
			}
		case 'R':
		case 'r':
		case KEY_RIGHT:
			if (me.now < me.num) {
				if (me.item[me.now]->host != NULL) {
					if (me.item[me.now]->fname[0] == '0') {
						if (get_con
						    (me.item[me.now]->host,
						     me.item[me.now]->port)
						    != -1) {
							char
							 tmpfile[30];

							GOPHER tmp;
							extern GOPHER *tmpitem;

							tmpitem = &tmp;
							strcpy
							    (tmp.server,
							     me.item
							     [me.now]->host);
							strcpy(tmp.file,
							       me.item
							       [me.now]->fname);
							sprintf
							    (tmp.title,
							     "0%s",
							     me.item
							     [me.now]->title);
							tmp.port =
							    me.item[me.now]->
							    port;
							enterdir(me.
								 item[me.now]->
								 fname);
							setuserfile(tmpfile,
								    "gopher.tmp");
							savetmpfile(tmpfile);
							ansimore(tmpfile, YEA);
							unlink(tmpfile);
						}
					} else {
						gopher(me.item[me.now]->host,
						       me.item[me.now]->fname,
						       me.item[me.now]->port,
						       me.item[me.now]->title);
					}
					me.page = 9999;
					break;
				}
				if (snprintf
				    (fname, PATHLEN, "%s/%s", path,
				     me.item[me.now]->fname) > PATHLEN - 1)
					break;
				if (dashf(fname)) {
					ansimore_withzmodem(fname, NA,
							    me.item[me.now]->
							    title);
					prints
					    ("[1m[44m[31m[�Ķ�����������]  [33m���� Q, �� �� ��һ������ U,���� ��һ������ <Enter>,<Space>,�� [m");
					switch (ch = egetch()) {
					case KEY_DOWN:
					case ' ':
					case '\n':
						if (++me.now >= me.num)
							me.now = 0;
						ch = KEY_RIGHT;
						goto EXPRESS;
					case KEY_UP:
					case 'u':
					case 'U':
						if (--me.now < 0)
							me.now = me.num - 1;
						ch = KEY_RIGHT;
						goto EXPRESS;
					case 'h':
						goto EXPRESS;
					case Ctrl('Y'):
						zsend_file(fname,
							   me.item[me.now]->
							   title);
						break;
					default:
						break;
					}

				} else if (dashd(fname)) {
					a_menu(me.item[me.now]->title,
					       fname, me.level, bmonly);
				}
				me.page = 9999;
			}
			freeitem(&me);
			a_loadnames(&me);
			break;
		case '/':
			a_search(&me, 1);
			break;
		case '?':
			a_search(&me, -1);
			break;
		case 'F':
		case 'U':
			if (me.now < me.num && HAS_PERM(PERM_BASIC)) {
				a_forward(path, me.item[me.now], ch == 'U');
				me.page = 9999;
			}
			break;
		case '!':
			if (!Q_Goodbye())
				break;	/* youzi leave */
		case 'c':
			if (me.now < me.num) {
				if (snprintf
				    (fname, PATHLEN, "%s/%s", path,
				     me.item[me.now]->fname) > PATHLEN - 1)
					break;	//add by ylsdd
				if (dashf(fname) && (me.level & PERM_BOARDS))
					a_copypaste(&me, 0);
				else
					a_copypaste(&me, 2);
			}
			break;
		case Ctrl('Y'):
			if (me.now < me.num) {
				if (me.item[me.now]->host != NULL) {
					me.page = 9999;
					break;
				} else
					sprintf(fname, "%s/%s", path,
						me.item[me.now]->fname);
				if (dashf(fname)) {
					zsend_file(fname,
						   me.item[me.now]->title);
					me.page = 9999;
				}
			}
			break;
		}
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
		if (me.level & PERM_BOARDS)
			a_manager(&me, ch);
		else if (ch == 'a' && HAS_PERM(PERM_POST)
			 && strstr(me.mtitle, "<GUESTBOOK>") == me.mtitle) {
			a_newitem(&me, ADDITEM);
		}
	}
	freeitem(&me);
	if (timein)
		logvisit(timein, path);
	if (firstlink)
		inlink = 0;
}

void
linkto(path, fname, title)
char *path, *title, *fname;
{
	MENU pm;

	pm.path = path;
	pm.level |= PERM_BOARDS;	/*add by ylsdd */
	a_loadnames(&pm);
	a_additem(&pm, title, fname, NULL, 0);
	a_savenames(&pm);
	freeitem(&pm);
}

int
add_grp(group, gname, bname, title)
char group[STRLEN], bname[STRLEN], title[STRLEN], gname[STRLEN];
{
	FILE *fn;
	char buf[PATHLEN];
	char searchname[STRLEN];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];

	sprintf(searchname, "%s: groups/%s/%s", bname, group, bname);
	sprintf(gpath, "0Announce/groups/%s", group);
	sprintf(bpath, "%s/%s", gpath, bname);
	if (!dashd("0Announce")) {
		mkdir("0Announce", 0770);
		chmod("0Announce", 0770);
		if ((fn = fopen("0Announce/.Names", "w")) == NULL)
			return -1;
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s ������������\n", MY_BBS_NAME);
		fprintf(fn, "#\n");
		fclose(fn);
	}
	if (!seek_in_file("0Announce/.Search", bname))
		addtofile("0Announce/.Search", searchname);
	if (!dashd("0Announce/groups")) {
		mkdir("0Announce/groups", 0777);
		chmod("0Announce/groups", 0777);

		linkto("0Announce", "groups", "����������");
	}
	if (!dashd(gpath)) {
		mkdir(gpath, 0777);
		chmod(gpath, 0777);
		linkto("0Announce/groups", group, gname);
	}
	if (!dashd(bpath)) {
		mkdir(bpath, 0770);
		chmod(bpath, 0770);
		linkto(gpath, bname, title);
		sprintf(buf, "%s/.Names", bpath);
		if ((fn = fopen(buf, "w")) == NULL) {
			return -1;
		}
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s\n", title);
		fprintf(fn, "#\n");
		fclose(fn);
	}
	return 1;

}

int
del_grp(grp, bname, title)
char grp[STRLEN], bname[STRLEN], title[STRLEN];
{
	char buf[STRLEN], buf2[STRLEN], buf3[30];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];
	char check[30];
	int i, n;
	MENU pm;

	strncpy(buf3, grp, 29);
	buf3[29] = '\0';
	sprintf(buf, "0Announce/.Search");
	sprintf(gpath, "0Announce/groups/%s", buf3);
	sprintf(bpath, "%s/%s", gpath, bname);
	deltree(bpath);

	pm.path = gpath;
	pm.level |= PERM_BOARDS;	/*add by ylsdd */
	a_loadnames(&pm);
	for (i = 0; i < pm.num; i++) {
		strsncpy(buf2, pm.item[i]->fname, sizeof (buf2));
		strcpy(check, strtok(buf2, "/~\n\b"));
		if (strstr(pm.item[i]->title, title)
		    && !strcmp(check, bname)) {
			free(pm.item[i]);
			(pm.num)--;
			for (n = i; n < pm.num; n++)
				pm.item[n] = pm.item[n + 1];
			a_savenames(&pm);
			break;
		}
	}
	freeitem(&pm);
	return 0;
}

int
edit_grp(bname, grp, title, newtitle)
char bname[STRLEN], grp[STRLEN], title[STRLEN], newtitle[100];
{
	char buf[STRLEN], buf2[STRLEN], buf3[30];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];
	char check[30];
	int i;
	MENU pm;

	strncpy(buf3, grp, 29);
	buf3[29] = '\0';
	sprintf(buf, "0Announce/.Search");
	sprintf(gpath, "0Announce/groups/%s", buf3);
	sprintf(bpath, "%s/%s", gpath, bname);
	if (!seek_in_file(buf, bname))
		return 0;

	pm.path = gpath;
	pm.level |= PERM_BOARDS;	/*add by ylsdd */
	a_loadnames(&pm);
	for (i = 0; i < pm.num; i++) {
		strsncpy(buf2, pm.item[i]->fname, sizeof (buf2));
		strcpy(check, strtok(buf2, "/~\n\b"));
		if (strstr(pm.item[i]->title, title)
		    && !strcmp(check, bname)) {
			strsncpy(pm.item[i]->title, newtitle,
				 sizeof (pm.item[i]->title));
			break;
		}
	}
	a_savenames(&pm);
	freeitem(&pm);
	pm.path = bpath;
	a_loadnames(&pm);
	strsncpy(pm.mtitle, newtitle, sizeof (pm.mtitle));
	a_savenames(&pm);
	freeitem(&pm);
	return 0;
}

void
Announce()
{
	sprintf(genbuf, "%s ������������", MY_BBS_NAME);
	a_menu(genbuf, "0Announce", (HAS_PERM(PERM_ANNOUNCE)
				     || HAS_PERM(PERM_SYSOP)) ?
	       PERM_BOARDS : 0, 0);
	clear();
}

//add by gluon, modified by ylsdd*/ 
void
Personal(cmd)
char *cmd;
{
	char buf[100], ch;
	/*strcpy( genbuf, "�����ļ�"); */
	strcpy(genbuf, "");
	switch (cmd[0]) {
	case '*':
		ch = currentuser.userid[0];
		ch = (ch >= 'A' && ch <= 'Z') ? ch :
		    (ch >= 'a' && ch <= 'z') ? (ch + 'A' - 'a') : '\0';
		sprintf(buf,
			"0Announce/groups/GROUP_0/PersonalCorpus/%c/%.15s",
			ch, currentuser.userid);
		break;
	case '$':
		ch = cmd[1];
		ch = (ch >= 'A' && ch <= 'Z') ? ch :
		    (ch >= 'a' && ch <= 'z') ? (ch + 'A' - 'a') : '\0';
		sprintf(buf,
			"0Announce/groups/GROUP_0/PersonalCorpus/%c/%.15s",
			ch, &(cmd[1]));
		break;
	default:
		ch = cmd[0];
		ch = (ch >= 'A' && ch <= 'Z') ? ch :
		    (ch >= 'a' && ch <= 'z') ? (ch + 'A' - 'a') : '\0';
		sprintf(buf, "0Announce/groups/GROUP_0/PersonalCorpus/%c", ch);
		break;
	}
	if (dashd(buf)) {
		a_menu(genbuf, buf, (HAS_PERM(PERM_ANNOUNCE)
				     || HAS_PERM(PERM_SYSOP)) ?
		       PERM_BOARDS : 0, 0);
	} else if (cmd[0] == '*') {
		a_prompt(-1,
			 "û���ҵ����ĸ����ļ�, �뵽Personal_Corpus�����룬��<Enter>����...",
			 genbuf, 2);
	} else if (cmd[0] == '$') {
		a_prompt(-1, "û���ҵ��ø����ļ�����<Enter>����...", genbuf, 2);
	}
	clear();
}

// end 
/*chk_currBM_Personal���ڶԸ��˾�������֧��, by ylsdd*/
int
chk_currBM_Personal(BMstr)
char *BMstr;
{
	char *ptr;
	char BMstrbuf[BM_LEN];
	int chk1 = 0, chk2 = 0;
	strncpy(BMstrbuf, BMstr, BM_LEN - 1);
	BMstrbuf[BM_LEN - 1] = 0;

	ptr = strtok(BMstrbuf, ",: ;|&()\n");
	while (1) {
		if (ptr == NULL)
			break;
		if (!strcmp(ptr, currentuser.userid))
			chk1 = 1;
		if (!strcmp(ptr, "_Personal"))
			chk2 = 1;
		if (chk1 && chk2)
			break;
		ptr = strtok(NULL, ",: ;|&()\n");
	}
	if (chk2 == 0) {
		if (!HAS_PERM(PERM_BOARDS))
			return 0;
		if (chk1 == 1 || HAS_PERM(PERM_BLEVELS))
			return 2;
		return 0;
	}
	//add by bjgyt for _Personal
	if (HAS_PERM(PERM_SYSOP))
		return 1;
	//end add
	if (chk1 == 0 && HAS_PERM(PERM_BLEVELS))
		return 3;
	if (chk1 == 1 && HAS_PERM(PERM_SPECIAL8))
		return 1;
	return -1;
}

/* logvisit, ������¼�������ķ��ʴ�����ʱ�䳤�� */
int
logvisit(time_t timein, const char *path)
{
	char fn[PATH_MAX + 1];
	int fd, t, n[2], len;
	t = time(NULL) - timein;
	if (t < 2)
		return 0;
	len = snprintf(fn, PATH_MAX + 1, "%s/.logvisit", path);
	if (len > PATH_MAX)
		return -1;
	fd = open(fn, O_RDWR | O_CREAT, 0660);
	if (fd < 0)
		return -1;
	if (read(fd, n, sizeof (int) * 2) <= 0) {
		n[0] = 0;
		n[1] = 0;
	}
	n[0] += 1;
	n[1] += t;
	lseek(fd, 0, SEEK_SET);
	write(fd, n, sizeof (int) * 2);
	close(fd);
	return 0;
}

int
getvisit(int n[2], const char *path)
{
	char fn[PATH_MAX + 1];
	int fd;
	int len;
	n[0] = 0;
	n[1] = 0;
	len = snprintf(fn, PATH_MAX + 1, "%s/.logvisit", path);
	if (len > PATH_MAX)
		return -1;
	fd = open(fn, O_RDONLY | O_CREAT, 0660);
	if (fd < 0)
		return -1;
	if (read(fd, n, sizeof (int) * 2) <= 0) {
		n[0] = 0;
		n[1] = 0;
	}
	close(fd);
	return 0;
}

/* ����ĺ���a_repair�����ռ���ʧ����Ŀ, �����ǷŽ���������Ŀ¼��  by ylsdd*/

static int
a_repair(pm)
MENU *pm;
{
	DIR *dirp;
	struct dirent *direntp;
	int i, changed;
	changed = 0;

	dirp = opendir(pm->path);
	if (dirp == NULL)
		return -1;

	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] == '.')
			continue;
		for (i = 0; i < pm->num; i++) {
			if (strcmp(pm->item[i]->fname, direntp->d_name)
			    == 0) {
				i = -1;
				break;
			}
		}
		if (i != -1) {
			a_additem(pm, direntp->d_name, direntp->d_name,
				  NULL, 0);
			changed++;
		}
	}
	closedir(dirp);
	if (changed > 0)
		a_savenames(pm);
	return changed;
}

/* ����ĺ���a_rjunk�����ָ��Ӿ�������ɾ����Ŀ¼��������,�����ǷŽ���������Ŀ¼��  by lepton*/

static int
a_rjunk(pm)
MENU *pm;
{
	DIR *dirp;
	struct dirent *direntp;
	int changed = 0, len, count;
	char buf[PATH_MAX + 1], rpath[PATH_MAX + 1], fpath[PATHLEN],
	    fname[STRLEN];
	time_t now;
	if (realpath(pm->path, rpath) == NULL)
		return -1;
	strcpy(buf, rpath + sizeof (MY_BBS_HOME "/0Announce"));
	normalize(buf);
	len = strlen(buf);
	dirp = opendir(MY_BBS_HOME "/0Announce/.junk");
	if (dirp == NULL)
		return -2;
	time(&now);
	while ((direntp = readdir(dirp)) != NULL) {
		if (strncmp(buf, direntp->d_name, len)
		    || !strlen(direntp->d_name + len))
			continue;
		sprintf(rpath, MY_BBS_HOME "/0Announce/.junk/%s",
			direntp->d_name);
		count = 0;
		do {
			if (count++ > MAX_POSTRETRY)
				goto out;
			sprintf(fname, "M%d", (int) now++);
			if (snprintf
			    (fpath, PATHLEN, "%s/%s", pm->path,
			     fname) > PATHLEN - 1)
				goto out;
		} while (!access(fpath, F_OK));
		rename(rpath, fpath);
		a_additem(pm, fname, fname, NULL, 0);
		changed++;
	}
      out:
	closedir(dirp);
	if (changed)
		a_savenames(pm);
	return changed;
}

int
add_anpath(char *title, char *path)
{
	char titles[20][STRLEN], paths[20][PATHLEN], *ptr;
	int i;
	int index = 0, nindex = 0;
	read_anpath(titles, paths);
	move(t_lines - 22, 0);
	clrtobot();
	prints
	    ("����ǰ·������Ϊ˿·����һ��? (��A-T����ѡ��, ' '��س�ȷ��', ����'X'ȡ��)");
	for (i = 0; i < 20; i++) {
		move(t_lines - 22 + 1 + i, 0);
		prints(" %s(%c) %s\033[0m",
		       (i == index) ? ">\033[1;7m" : " ",
		       'A' + i,
		       (titles[i][0] !=
			0) ? titles[i] : "\033[32m��δ�趨\033[0m");
	}
	while (1) {
		i = igetkey();
		i = toupper(i);
		if (i == KEY_LEFT || i == 'X')
			return -1;
		if (i == '\n' || i == '\r' || i == ' ')
			break;
		if (i == KEY_UP || i == KEY_DOWN) {
			if (i == KEY_UP) {
				nindex = index - 1;
				if (nindex < 0)
					nindex = 19;
			} else {
				nindex = index + 1;
				if (nindex >= 20)
					nindex = 0;
			}
		} else {
			i = i - 'A';
			if (i >= 0 && i < 20) {
				index = i;
				break;
			}
		}
		if (nindex != index) {
			move(t_lines - 22 + 1 + index, 0);
			prints("  (%c) %s\033[0m",
			       'A' + index,
			       (titles[index][0] !=
				0) ? titles[index] : "\033[32m��δ�趨\033[0m");
			clrtoeol();
			move(t_lines - 22 + 1 + nindex, 0);
			prints(" >\033[1;7m(%c) %s\033[0m",
			       'A' + nindex,
			       (titles[nindex][0] !=
				0) ? titles[nindex] :
			       "\033[32m��δ�趨\033[0m");
			clrtoeol();
			index = nindex;
		}
	}
	if ((ptr = strchr(title, '\n')) != NULL)
		*ptr = 0;
	if ((ptr = strchr(path, '\n')) != NULL)
		*ptr = 0;
	strncpy(titles[index], title, sizeof (titles[index]));
	titles[index][sizeof (titles[index]) - 1] = 0;
	strncpy(paths[index], path, sizeof (paths[index]));
	paths[index][sizeof (paths[index]) - 1] = 0;
	move(t_lines - 22 + 1 + index, 0);
	clrtoeol();
	prints("  (%c) %s", 'A' + index,
	       (titles[index][0] !=
		0) ? titles[index] : "\033[32m��δ�趨\033[0m");
	if (save_anpath(titles, paths) < 0) {
		prints("����˿·����! �����������");
		igetkey();
	} else
		pressreturn();
	return 0;
}

int
select_anpath()
{
	char titles[20][STRLEN], paths[20][PATHLEN];
	int i;
	static int index = 0, nindex;
	Importname[0] = 0;
	move(t_lines - 22, 0);
	clrtobot();
	if (read_anpath(titles, paths) <= 0) {
		prints("˿·δ�����û����, ����������˿·");
		pressreturn();
		return -1;
	}
	prints
	    ("���ĵ����浽�ĸ�˿·? (��A-H����ѡ��, ' '��س�ȷ��', ����'X'ȡ��)");
	for (i = 0; i < 20; i++) {
		move(t_lines - 22 + 1 + i, 0);
		prints(" %s(%c) %s\033[0m",
		       (i == index) ? ">\033[1;7m" : " ",
		       'A' + i,
		       (titles[i][0] !=
			0) ? titles[i] : "\033[32m��δ�趨\033[0m");
	}
	while (1) {
		i = igetkey();
		i = toupper(i);
		if (i == KEY_LEFT || i == 'X')
			return -1;
		if (i == '\n' || i == '\r' || i == ' ') {
			if (titles[index][0] != 0)
				break;
			continue;
		}
		if (i == KEY_UP || i == KEY_DOWN) {
			if (i == KEY_UP) {
				nindex = index - 1;
				if (nindex < 0)
					nindex = 19;
			} else {
				nindex = index + 1;
				if (nindex >= 20)
					nindex = 0;
			}
		} else {
			i = i - 'A';
			if (i >= 0 && i < 20) {
				if (titles[i][0] != 0) {
					index = i;
					break;
				}
				nindex = i;
			}
		}
		if (nindex != index) {
			move(t_lines - 22 + 1 + index, 0);
			prints("  (%c) %s\033[0m",
			       'A' + index,
			       (titles[index][0] !=
				0) ? titles[index] : "\033[32m��δ�趨\033[0m");
			clrtoeol();
			move(t_lines - 22 + 1 + nindex, 0);
			prints(" >\033[1;7m(%c) %s\033[0m",
			       'A' + nindex,
			       (titles[nindex][0] !=
				0) ? titles[nindex] :
			       "\033[32m��δ�趨\033[0m");
			clrtoeol();
			index = nindex;
		}
	}
	strcpy(Importname, paths[index]);
	return 0;
}

int
save_anpath(char titles[20][STRLEN], char paths[20][PATHLEN])
{
	int i;
	FILE *fp;
	char pathfile[PATHLEN];
	sprintf(pathfile, MY_BBS_HOME "/tmp/%s.path8", currentuser.userid);
	if ((fp = fopen(pathfile, "w")) == NULL)
		return -1;
	for (i = 0; i < 20; i++)
		if (titles[i][0] != 0)
			fprintf(fp, "%s\n%s\n", titles[i], paths[i]);
		else
			fprintf(fp, "\n\n");
	fclose(fp);
	return 0;
}

int
read_anpath(char titles[20][STRLEN], char paths[20][PATHLEN])
{
	int i, j = 0;
	FILE *fp;
	char pathfile[PATHLEN], *ptr;
	sprintf(pathfile, MY_BBS_HOME "/tmp/%s.path8", currentuser.userid);
	if ((fp = fopen(pathfile, "r")) == NULL) {
		for (i = 0; i < 20; i++)
			titles[i][0] = 0;
		return -1;
	}
	for (i = 0; i < 20; i++) {
		if (fgets(titles[i], STRLEN, fp) == NULL
		    || fgets(paths[i], PATHLEN, fp) == NULL) {
			while (i < 20)
				titles[i++][0] = 0;
			break;
		}
		if ((ptr = strchr(titles[i], '\n')) != NULL)
			*ptr = 0;
		if ((ptr = strchr(paths[i], '\n')) != NULL)
			*ptr = 0;
		if (strlen(paths[i]) < 10)
			titles[i][0] = 0;
		if (titles[i][0] != 0)
			j++;
	}
	fclose(fp);
	return j;
}
