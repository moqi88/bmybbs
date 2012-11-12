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

#define PUTCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(">");move(3+locmem->crs_line-locmem->top_line,0);
//#define PUTCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(">");
#define RMVCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(" ");

int can_R_endline = 0;
struct fileheader SR_fptr;
int SR_BMDELFLAG = NA;
//char *pnt;
extern int t_friends();
extern struct onebrc brc;
extern char IScurrBM;
struct keeploc {
	char *key;
	int top_line;
	int crs_line;
	struct keeploc *next;
};

/*struct fileheader *files = NULL;*/
char currdirect[STRLEN];
int screen_len;
int last_line;
extern int digestmode;

static void modify_locmem(struct keeploc *locmem, int total);
static int move_cursor_line(struct keeploc *locmem, int mode);
static int draw_title(int (*dotitle)(void));
static void draw_entry(char *(*doentry)(int, void *, char *), struct keeploc *locmem, int num, int ssize, char *pnt);
static void doquickview(int i);
static int i_read_key(const struct one_key *rcmdlist, struct keeploc *locmem, int ch, int ssize, char *pnt);
static int search_author(struct keeploc *locmem, int offset, char *powner);
static int search_post(struct keeploc *locmem, int offset);
static int search_title(struct keeploc *locmem, int offset);
static int search_thread(struct keeploc *locmem, int offset, char *title);
static int search_articles(struct keeploc *locmem, char *query, int offset, int aflag);
static int cursor_pos(struct keeploc *locmem, int val, int from_top);
static int search_threadid(struct keeploc *locmem, int offset, int thread, int mode);
static int digest_mode(void);


//getkeep����: �ʼ��һҳ��1,
//���def_topline>=1, ���û���ҵ��Ļ�, Ĭ�ϵĵ�һҳҳ��
//���def_topline==-1, ���û���ҵ��Ļ�, ����Ĭ��ҳ, ���Ƿ��ؿ�ָ��
struct keeploc *
getkeep(s, def_topline, def_cursline)
char *s;
int def_topline;
int def_cursline;
{
	static struct keeploc *keeplist = NULL;
	struct keeploc *p;

	for (p = keeplist; p != NULL; p = p->next) {
		if (!strcmp(s, p->key)) {
			if (p->crs_line < 1)
				p->crs_line = 1;	/* DAMMIT! - rrr */
			return p;
		}
	}
	if (def_topline == -1)
		return NULL;
	p = (struct keeploc *) malloc(sizeof (*p));
	p->key = (char *) malloc(strlen(s) + 1);
	strcpy(p->key, s);
	p->top_line = def_topline;
	p->crs_line = def_cursline;
	p->next = keeplist;
	keeplist = p;
	return p;
}

void
fixkeep(s, first, last)
char *s;
int first, last;
{
	struct keeploc *k;
	k = getkeep(s, 1, 1);
	if (k->crs_line >= first) {
		k->crs_line = (first == 1 ? 1 : first - 1);
		k->top_line = (first < 11 ? 1 : first - 10);
	}
}

static void
modify_locmem(locmem, total)
struct keeploc *locmem;
int total;
{
	if (locmem->top_line > total) {
		locmem->crs_line = total;
		locmem->top_line = total - t_lines / 2;
		if (locmem->top_line < 1)
			locmem->top_line = 1;
	} else if (locmem->crs_line > total) {
		locmem->crs_line = total;
	}
}

static int
move_cursor_line(locmem, mode)
struct keeploc *locmem;
int mode;
{
	int top, crs;
	int reload = 0;

	top = locmem->top_line;
	crs = locmem->crs_line;
	if (mode == READ_PREV) {
		if (crs <= top) {
			top -= screen_len - 1;
			if (top < 1)
				top = 1;
			reload = 1;
		}
		crs--;
		if (crs < 1) {
			crs = 1;
			reload = -1;
		}
	} else if (mode == READ_NEXT) {
		if (crs + 1 >= top + screen_len) {
			top += screen_len - 1;
			reload = 1;
		}
		crs++;
		if (crs > last_line) {
			crs = last_line;
			reload = -1;
		}
	}
	locmem->top_line = top;
	locmem->crs_line = crs;
	return reload;
}

static int
draw_title(dotitle)
int (*dotitle) ();
{
	clear();
	return (*dotitle) ();
}

static void
draw_entry(doentry, locmem, num, ssize, pnt)
char *(*doentry) (int, void *, char *);
struct keeploc *locmem;
int num, ssize;
char *pnt;
{
	char *str, buf[512];
	int base, i;

	base = locmem->top_line;
	move(3, 0);
	clrtobot();
	for (i = 0; i < num; i++) {
		str = (*doentry) (base + i, &pnt[i * ssize], buf);
		prints("%s\n", str);
	}
	move(t_lines - 1, 0);
	clrtoeol();
	update_endline();
}

void (*quickview) () = NULL;
struct {
	int crs_line;
	char *data;
	char *currdirect;
} quickviewdata;
static void
doquickview(int i)
{
	if (quickview != NULL)
		quickview(quickviewdata.crs_line, quickviewdata.data,
			  quickviewdata.currdirect);
}

void
i_read(cmdmode, direct, dotitle, doentry, rcmdlist, ssize)
int cmdmode;
char *direct;
int (*dotitle) ();
char *(*doentry) (int, void *, char *);
const struct one_key *rcmdlist;
int ssize;
{
	extern int talkrequest;
	extern int friendflag;
	extern time_t login_start_time;
	struct keeploc *locmem;
	char lbuf[11];
	int lbc, recbase, mode, ch;
	int num, entries;
	int savet_lines = 0;
	char *pnt;
	time_t now;
	int allstay;
	extern int endlineoffset;

	screen_len = t_lines - 4;
	modify_user_mode(cmdmode);
	pnt = calloc(screen_len, ssize);
	strcpy(currdirect, direct);
	if (draw_title(dotitle) == -1) {
		free(pnt);
		return;
	}
	last_line = get_num_records(currdirect, ssize);

	if (last_line == 0) {
		if (cmdmode == RMAIL) {
			prints("û���κ����ż�...");
			pressreturn();
			clear();
		} else if (cmdmode == SELBACKNUMBER) {
			if (!IScurrBM) {
				prints("���޹���");
				pressreturn();
				clear();
			} else {
				getdata(t_lines - 1, 0,
					"���޹��� (P)�������� (Q)�뿪��[Q] ",
					genbuf, 4, DOECHO, YEA);
				if (genbuf[0] == 'p' || genbuf[0] == 'P')
					new_backnumber();
			}
		} else if (cmdmode == BACKNUMBER) {
			prints("���������������");
			pressreturn();
			clear();
		} else if (cmdmode == DO1984) {
			prints("�������޴��������");
			pressreturn();
			clear();
		} else if (cmdmode == GMENU) {
			char desc[5];
			char buf[40];

			if (friendflag)
				strcpy(desc, "����");
			else
				strcpy(desc, "����");
			sprintf(buf, "û���κ�%s (A)����%s (Q)�뿪��[Q] ", desc,
				desc);
			getdata(t_lines - 1, 0, buf, genbuf, 4, DOECHO, YEA);
			if (genbuf[0] == 'a' || genbuf[0] == 'A')
				override_add();
		} else {
			if (!(IScurrBM) || !club_board(currboard)) {
				getdata(t_lines - 1, 0,
					"�����³��� (P)�������� (Q)�뿪��[Q] ",
					genbuf, 4, DOECHO, YEA);
				if (genbuf[0] == 'p' || genbuf[0] == 'P')
					do_post();
			} else {
				getdata(t_lines - 1, 0,
					"�����³��� (P)�������� (N)���þ��ֲ���Ա (Q)�뿪�� [Q] ",
					genbuf, 4, DOECHO, YEA);
				if (genbuf[0] == 'p' || genbuf[0] == 'P')
					do_post();
				else if (genbuf[0] == 'n' || genbuf[0] == 'N')
					clubmember();
			}
		}
		free(pnt);
		return;
	}
	num = last_line - screen_len + 2;
	locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
	modify_locmem(locmem, last_line);
	if (locmem->crs_line - locmem->top_line >= screen_len)	/*copy from "code by bad@smth 2002.9.2" */
		locmem->crs_line = locmem->top_line;
	recbase = locmem->top_line;
	entries = get_records(currdirect, pnt, ssize, recbase, screen_len);
	draw_entry(doentry, locmem, entries, ssize, pnt);
	PUTCURS;
	lbc = 0;
	lbuf[0]=0;
	mode = DONOTHING;
	now = time(NULL);
	allstay = (now - login_start_time) / 60;
	while (1) {
		if (savet_lines != 0) {
			quickviewdata.crs_line = locmem->crs_line;
			quickviewdata.data =
			    &pnt[(locmem->crs_line - locmem->top_line) * ssize];
			quickviewdata.currdirect = currdirect;
			signal(SIGALRM, doquickview);
			ualarm(300000, 0);
		}
		can_R_endline = 1;
		if ((ch = egetch()) == EOF)
			break;
		can_R_endline = 0;
		if (talkrequest) {
			talkreply();
			mode = FULLUPDATE;
		} else if ((ch >= '0' && ch <= '9') || (lbc > 0
							&& ch == Ctrl('H'))
			   || ch == '\177') {
			if (lbc < 9 || ch == Ctrl('H') || ch == '\177') {
				char buf[30];
				if (ch != Ctrl('H') && ch != '\177') {
					lbuf[lbc++] = ch;
					lbuf[lbc] = '\0';
				} else
					lbuf[--lbc] = '\0';
				move(t_lines - 1, 0);
				clrtoeol();
				sprintf(buf, "[\033[36m%.12s\033[33m]",
					currentuser.userid);
				prints
				    ("\033[1;44;33mת��:[\033[36m%10s\033[33m]"
				     " ״̬:[\033[36m%1s%1s%1s%1s%1s%1s\033[33m]"
				     " ʹ����:%-24s ͣ��:[\033[36m%3d\033[33m:\033[36m%2d\033[33m]"
				     " \033[m", lbuf,
				     (uinfo.pager & ALL_PAGER) ? "P" : "p",
				     (uinfo.pager & FRIEND_PAGER) ? "O" : "o",
				     (uinfo.pager & ALLMSG_PAGER) ? "M" : "m",
				     (uinfo.pager & FRIENDMSG_PAGER) ? "F" :
				     "f", (DEFINE(DEF_MSGGETKEY)) ? "X" : "x",
				     (uinfo.invisible == 1) ? "C" : "c", buf,
				     (allstay / 60) % 1000, allstay % 60);
			}
		} else if (lbc > 0 && (ch == '\n' || ch == '\r')) {
			lbuf[lbc] = '\0';
			lbc = atoi(lbuf);
			if (cursor_pos(locmem, lbc, 10))
				mode = PARTUPDATE;
			lbc = 0;
		} else {
			if (lbc != 0) {
				update_endline();
				lbc = 0;
			}
			endlineoffset = 0;
			mode = i_read_key(rcmdlist, locmem, ch, ssize, pnt);
			modify_user_mode(cmdmode);
			while (mode == READ_NEXT || mode == READ_PREV) {
				int reload;

				reload = move_cursor_line(locmem, mode);
				if (reload == -1) {
					mode = FULLUPDATE;
					break;
				} else if (reload) {
					recbase = locmem->top_line;
					entries =
					    get_records(currdirect, pnt, ssize,
							recbase, screen_len);
					if (entries <= 0) {
						last_line = -1;
						break;
					}
				}
				num = locmem->crs_line - locmem->top_line;
				mode =
				    i_read_key(rcmdlist, locmem, ch, ssize,
					       pnt);
				modify_user_mode(cmdmode);
			}
		}
		if (mode == DOQUIT)
			break;
		if (mode == GOTO_NEXT) {
			cursor_pos(locmem, locmem->crs_line + 1, 1);
			mode = PARTUPDATE;
		}
		if (savet_lines == 0)
			endlineoffset = 0;
		else
			endlineoffset = -8;
		switch (mode) {
		case NEWDIRECT:
		case DIRCHANGED:
		case 999:
		case UPDATETLINE:
			if (1)
				if (mode == UPDATETLINE) {
					if (savet_lines == 0) {
						endlineoffset = -8;
						screen_len = t_lines - 4 - 8;
						savet_lines = 1;
					} else {
						endlineoffset = 0;
						screen_len = t_lines - 4;
						savet_lines = 0;
					}
				}
			recbase = -1;
			if (mode == 999) {
				if (uinfo.mode == SELBACKNUMBER
				    || uinfo.mode ==
				    BACKNUMBER
				    || uinfo.mode == DO1984 || cmdmode == GMENU)
					strcpy(currdirect, direct);
				//sprintf(currdirect, "backnumbers/%s/%s", currboard, DOT_DIR);
				else {
					digestmode = 0;
					setbdir(currdirect, currboard,
						digestmode);

				}
			}
			last_line = get_num_records(currdirect, ssize);
			if (last_line == 0 && digestmode > 0) {
				switch (digestmode) {
				case YEA:
					digest_mode();
					break;
				case 2:
					thread_mode();
					break;
				case 3:
					marked_mode();
					break;
				case 4:
					deleted_mode();
					break;
				case 5:
					junk_mode();
					break;
				}
			}
			if (mode == NEWDIRECT) {
				num = last_line - screen_len + 1;
				locmem =
				    getkeep(currdirect, num < 1 ? 1 : num,
					    last_line);
			}
		case FULLUPDATE:
			draw_title(dotitle);
		case PARTUPDATE:
			if (last_line < locmem->top_line + screen_len) {
				num = get_num_records(currdirect, ssize);
				if (last_line != num) {
					last_line = num;
					recbase = -1;
				}
			}
			if (last_line == 0) {
				prints("No Messages\n");
				entries = 0;
			} else if (recbase != locmem->top_line) {
				recbase = locmem->top_line;
				if (recbase > last_line) {
					recbase = last_line - screen_len / 2;
					if (recbase < 1)
						recbase = 1;
					locmem->top_line = recbase;
				}
				entries = get_records(currdirect, pnt, ssize,
						      recbase, screen_len);
			}
			if (locmem->crs_line > last_line)
				locmem->crs_line = last_line;
			draw_entry(doentry, locmem, entries, ssize, pnt);
			PUTCURS;
			break;
		default:
			break;
		}
		mode = DONOTHING;
		if (entries == 0)
			break;
	}
	alarm(0);
	clear();
	free(pnt);
}

static int
i_read_key(rcmdlist, locmem, ch, ssize, pnt)
const struct one_key *rcmdlist;
struct keeploc *locmem;
int ch, ssize;
char *pnt;
{
	int i, mode = DONOTHING;

	switch (ch) {
	case 'q':
	case KEY_LEFT:
		if (uinfo.mode != RMAIL && uinfo.mode != SELBACKNUMBER
		    && uinfo.mode != BACKNUMBER && uinfo.mode != DO1984) {
			switch (digestmode) {
			case YEA:
				return digest_mode();
			case 2:
				return thread_mode();
			case 3:
				return marked_mode();
			case 4:
				return deleted_mode();
			case 5:
				return junk_mode();
			default:
				return DOQUIT;
			}
		} else
			return DOQUIT;
	case Ctrl('L'):
		redoscr();
		break;
	case 'k':
	case KEY_UP:
		if (cursor_pos(locmem, locmem->crs_line - 1, screen_len - 2))
			return PARTUPDATE;
		break;
	case 'j':
	case KEY_DOWN:
		if (cursor_pos(locmem, locmem->crs_line + 1, 0))
			return PARTUPDATE;
		break;
	case 'L':		/* ppfoong */
		show_allmsgs();
		return FULLUPDATE;
	case 'N':
	case Ctrl('F'):
	case KEY_PGDN:
	case ' ':
		if (last_line >= locmem->top_line + screen_len) {
			locmem->top_line += screen_len - 1;
			locmem->crs_line = locmem->top_line;
			return PARTUPDATE;
		}
		RMVCURS;
		locmem->crs_line = last_line;
		PUTCURS;
		break;
	case 'P':
	case Ctrl('B'):
	case KEY_PGUP:
		if (locmem->top_line > 1) {
			locmem->top_line -= screen_len - 1;
			if (locmem->top_line <= 0)
				locmem->top_line = 1;
			locmem->crs_line = locmem->top_line;
			return PARTUPDATE;
		} else {
			RMVCURS;
			locmem->crs_line = locmem->top_line;
			PUTCURS;
		}
		break;
	case KEY_HOME:
		locmem->top_line = 1;
		locmem->crs_line = 1;
		return PARTUPDATE;
	case '$':
	case KEY_END:
		if (last_line >= locmem->top_line + screen_len) {
			locmem->top_line = last_line - screen_len + 1;
			if (locmem->top_line <= 0)
				locmem->top_line = 1;
			locmem->crs_line = last_line;
			return PARTUPDATE;
		}
		RMVCURS;
		locmem->crs_line = last_line;
		PUTCURS;
		break;
	case 'S':		/* youzi */
		if (!HAS_PERM(PERM_PAGE))
			break;
		s_msg();
		return FULLUPDATE;
		break;
		/*      case 'c': *//* youzi */
/*		if (!HAS_PERM(PERM_BASIC)) break;
		t_friends();
		return FULLUPDATE;
		break;*/
	case 'w':
		if ((in_mail != YEA) && (HAS_PERM(PERM_READMAIL))) {
			m_read();
			return 999;
		} else
			break;

	case '!':		/* youzi leave */
		return Q_Goodbye();
		break;
	case '\n':
	case '\r':
	case KEY_RIGHT:
		ch = 'r';
		/* lookup command table */
	default:
		for (i = 0; rcmdlist[i].fptr != NULL; i++) {
			if (rcmdlist[i].key != ch)
				continue;
			if (rcmdlist[i].fptr == t_friends) {
				if (!HAS_PERM(PERM_BASIC))
					break;
				t_friends();
				return FULLUPDATE;
			}
			if (locmem->crs_line < locmem->top_line) {
				//errlog("i_read_key crs error:%d, %d", locmem->crs_line, locmem->top_line);
				return FULLUPDATE;
			}
			mode =
			    (*(rcmdlist[i].fptr)) (locmem->crs_line,
						   &pnt[(locmem->crs_line -
							 locmem->top_line) *
							ssize], currdirect);
			break;
		}
	}
	return mode;
}

int
auth_search_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, 1, fileinfo->owner))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
auth_search_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, -1, fileinfo->owner))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
post_search_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_post(locmem, 1))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
post_search_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_post(locmem, -1))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
show_author(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	t_query(fileinfo->owner);
	return FULLUPDATE;
}

int
friend_author(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	extern int friendflag;
	char uident[STRLEN];
	char *q_id = fileinfo->owner;
	if (!HAS_PERM(PERM_BASIC)) {
		return 0;
	}
	friendflag = YEA;
	if (*q_id == '\0')
		return 0;
	if (strchr(q_id, ' '))
		strtok(q_id, " ");
	strncpy(uident, q_id, sizeof (uident));
	uident[sizeof (uident) - 1] = '\0';
	if (searchuser(uident) > 0) {
		sprintf(genbuf, "�� %s Ϊ����ô", uident);
		if (YEA == askyn(genbuf, NA, YEA)) {
			clear();
			addtooverride(uident);
		}
	}
	return FULLUPDATE;
}

int
SR_BMfunc(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	int i, dotype=0;			//add by mintbaggio
	char buf[STRLEN * 2], ch[4], BMch;
	static const char *SR_BMitems[] =
	    { "����������ɾ��", "����", "��ժ", "���뾫����", "�����ݴ浵",
		"��Ϊ���ɻظ�", "�ϼ�" ,"��������ɾ��"//add heji by bjgyt
	};
	static const char *subBMitems[] = 
	{"��ͬ����", "��ͬ����"};		//add by mintbaggio

	if (!IScurrBM) {
		return DONOTHING;
	}
	saveline(t_lines - 2, 0, NULL);			
	move(t_lines - 2, 0);			
	clrtoeol();
	
	//add by mintbaggio for BMfunc 'b'
	getdata(t_lines-2, 0, "ִ��: (0) ȡ��  (1) ��ͬ����  (2) ��ͬ����  [0]: ", ch, 3, DOECHO, YEA);	//mint
	dotype =  atoi(ch);
	if(dotype<1 || dotype>2){
		saveline(t_lines - 2, 1, NULL);
		return FULLUPDATE;
	}
	//end

//	strcpy(buf, "��ͬ���� (0)ȡ�� ");			mint
	//add by mintbaggio for BMfunc 'b'
	sprintf(buf, "%s (0)ȡ��", subBMitems[dotype-1]);
	saveline(t_lines - 2, 0, NULL);
	move(t_lines -2 , 0);
	clrtoeol();
	//end

	for (i = 0; i < 5; i++)
		sprintf(buf, "%s(%d)%s ", buf, i + 1, SR_BMitems[i]);
	move(t_lines - 3, 0);
	prints("%s", buf);
	buf[0] = 0;
	for (i = 5; i < 8; i++)//change 6 to 8 by mintbaggio
		sprintf(buf, "%s(%d)%s ", buf, i + 1, SR_BMitems[i]);
	strcat(buf, "? [0]: ");
	getdata(t_lines - 2, 0, buf, ch, 3, DOECHO, YEA);
	BMch = atoi(ch);
	if (BMch <= 0 || BMch > 8) {//change 6 to 8 by mintbaggio
		saveline(t_lines - 2, 1, NULL);
		return FULLUPDATE;
	}
	move(t_lines - 2, 0);
	clrtoeol();
	move(t_lines - 3, 0);
	clrtoeol();
	sprintf(buf, "ȷ��Ҫִ��%s[%s]��", subBMitems[dotype-1], SR_BMitems[BMch - 1]);
	if (askyn(buf, NA, NA) == 0) {
		saveline(t_lines - 2, 1, NULL);
		return FULLUPDATE;
	}
	if ((digestmode == 2 || digestmode == 3 || digestmode == 4
	     || digestmode == 5) && (BMch != 4 && BMch != 5))
		return FULLUPDATE;
	move(t_lines - 2, 0);
	sprintf(buf, "�Ƿ��%s��һƪ��ʼ%s (Y)��һƪ (N)Ŀǰ��һƪ", (dotype == 2) ? "������" : "������",
		SR_BMitems[BMch - 1]);
	if (askyn(buf, YEA, NA) == 1) {
		if(dotype == 1)		//add by mintbaggio 040322 for same author in BMfunc 'b'
			ent = sread(2, 0, ent, 0, fileinfo);
		else	ent = sread(2, 0, ent, 1, fileinfo);
		fileinfo = &SR_fptr;
	}
	if(BMch == 7){			//add by mintbaggio 040321 for heji
		sprintf(buf, "tmp/%s.combine", currentuser.userid);
		if(dashf(buf)) unlink(buf);
	}
	if(dotype == 1)			//add by mintbaggio 040322 for same author in BMfunc 'b'
		sread(BMch + SR_BMBASE, 0, ent, 0, fileinfo);
	else	sread(BMch + SR_BMBASE, 0, ent, 1, fileinfo);

	if(BMch == 7){			//add by mintbaggio 040321 for heji
		char title_combine[STRLEN], title[STRLEN];
		if(dotype == 1){
			strcpy(title, fileinfo->title);
			if (!strncmp(title, "Re: ", 4) | !strncmp(title, "RE: ", 4))
				strcpy(title, title + 4);
		}
		else    strcpy(title, fileinfo->owner);
		sprintf(title_combine, "���ϼ���%s", title);
		sprintf(buf, "tmp/%s.combine", currentuser.userid);
		postfile(buf, currboard, title_combine, 2);
		unlink(buf);
	}
	snprintf(genbuf, 256, "%s sametitle %s %s", currentuser.userid,
		 currboard, fileinfo->title);
	newtrace(genbuf);
	return DIRCHANGED;
}

/*���ҵ�һƪ, ���ҵ�һƪ�µ�, ����ҵ���, �Ͷ�֮, ����ֱ�ӷ���*/
int
SR_first_new(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	SR_first(ent, fileinfo, direct);
	if (sread(3, 0, 0, 0, &SR_fptr) == -1) {	/*Found The First One */
		sread(0, 1, 0, 0, &SR_fptr);
		return FULLUPDATE;
	}
	return PARTUPDATE;
}

int
SR_last(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	sread(1, 0, ent, 0, fileinfo);
	return PARTUPDATE;
}

int
SR_first(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	sread(2, 0, ent, 0, fileinfo);
	return PARTUPDATE;
}

int
SR_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	sread(0, 1, 0, 0, fileinfo);
	return FULLUPDATE;
}

int
SR_author(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	sread(0, 1, 0, 1, fileinfo);
	return FULLUPDATE;
}

static int
search_author(locmem, offset, powner)
struct keeploc *locmem;
int offset;
char *powner;
{
	static char author[IDLEN + 1];
	char ans[IDLEN + 2], pmt[STRLEN];
	char currauth[STRLEN];

	strcpy(currauth, powner);

	sprintf(pmt, "%s��������Ѱ���� [%s]: ",
		offset > 0 ? "������" : "����ǰ", currauth);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, IDLEN + 1, DOECHO, YEA);
	if (ans[0] != '\0')
		strcpy(author, ans);
	else
		strcpy(author, currauth);

	return search_articles(locmem, author, offset, 1);
}
#if 0
static int
auth_post_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, 1, fileinfo->owner))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
auth_post_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, -1, fileinfo->owner))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}
#endif 
int
t_search_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_title(locmem, 1))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
t_search_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (search_title(locmem, -1))
		return PARTUPDATE;
	else
		update_endline();
	return DONOTHING;
}

int
thread_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (uinfo.mode != RMAIL) {
		if (search_threadid(locmem, -1, fileinfo->thread, 0)) {
			update_endline();
			return PARTUPDATE;
		}
	} else {
		if (search_thread(locmem, -1, fileinfo->title)) {
			update_endline();
			return PARTUPDATE;
		}
	}
	update_endline();
	return DONOTHING;
}

int
thread_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;

	locmem = getkeep(direct, 1, 1);
	if (uinfo.mode != RMAIL) {
		if (search_threadid(locmem, 1, fileinfo->thread, 0)) {
			update_endline();
			return PARTUPDATE;
		}
	} else {
		if (search_thread(locmem, 1, fileinfo->title)) {
			update_endline();
			return PARTUPDATE;
		}
	}
	update_endline();
	return DONOTHING;
}

static int
search_post(locmem, offset)
struct keeploc *locmem;
int offset;
{
	static char query[STRLEN];
	char ans[STRLEN], pmt[STRLEN];

	strcpy(ans, query);
	sprintf(pmt, "��Ѱ%s������ [%s]: ",
		offset > 0 ? "������" : "����ǰ", ans);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, 50, DOECHO, YEA);
	if (ans[0] != '\0')
		strcpy(query, ans);

	return search_articles(locmem, query, offset, -1);
}

static int
search_title(locmem, offset)
struct keeploc *locmem;
int offset;
{
	static char title[STRLEN];
	char ans[STRLEN], pmt[STRLEN];

	strcpy(ans, title);
	sprintf(pmt, "%s��Ѱ���� [%.16s]: ", offset > 0 ? "����" : "��ǰ", ans);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, 46, DOECHO, YEA);
	if (*ans != '\0')
		strcpy(title, ans);
	return search_articles(locmem, title, offset, 0);
}

static int
search_thread(locmem, offset, title)
struct keeploc *locmem;
int offset;
char *title;
{

	if (title[0] == 'R' && (title[1] == 'e' || title[1] == 'E')
	    && title[2] == ':')
		title += 4;
	setqtitle(title);
	return search_articles(locmem, title, offset, 2);
}

/*Add by SmallPig*/
int
sread(passonly, readfirst, pnum, auser, ptitle)
int passonly, readfirst, auser, pnum;
struct fileheader *ptitle;
{
	struct keeploc *locmem;
	int rem_top, rem_crs;	/* youzi 1997.7.7 */
	extern int readingthread;
	int istest = 0, isstart = 0, isnext = 1;
	int previous;
	int add_anno_flag = 0;
	char genbuf[STRLEN], title[STRLEN];
	char tmpboard[STRLEN], anboard[STRLEN];

	previous = pnum;

	if (passonly == SR_BMIMPORT) {
		if (select_anpath() < 0 || check_import(anboard) < 0)
			return 1;
		else if (!strcmp(anboard, currboard))
			add_anno_flag = 1;
		else
			add_anno_flag = 0;
	}
	if (passonly == 0 || passonly == 4) {
		if (readfirst)
			isstart = 1;
		else {
			isstart = 0;
			move(t_lines - 1, 0);
			clrtoeol();
			prints
			    ("[1;44;31m[%8s] [33m��һ�� <Space>,<Enter>,��,n����һ�� ��,U,l                          [m",
			     auser ? "��ͬ����" : "�����Ķ�");
			switch (egetch()) {

			case 'n':
			case ' ':
			case '\n':
			case KEY_DOWN:
				isnext = 1;
				break;
			case 'l':
			case KEY_UP:
			case 'u':
			case 'U':
				isnext = -1;
				break;
			default:
				break;
			}
		}
	} else if (passonly == 1 || passonly >= 3)
		isnext = 1;
	else
		isnext = -1;
	locmem = getkeep(currdirect, 1, 1);
	rem_top = locmem->top_line;
	rem_crs = locmem->crs_line;
	if (auser == 0) {
		strcpy(title, ptitle->title);
		setqtitle(title);
	} else {
		strcpy(title, ptitle->owner);
		setqtitle(ptitle->title);
	}
	readingthread = ptitle->thread;
	if (!strncmp(title, "Re: ", 4) | !strncmp(title, "RE: ", 4)) {
		strcpy(title, title + 4);
	}
	memcpy(&SR_fptr, ptitle, sizeof (SR_fptr));
	while (!istest) {
		switch (passonly) {
		case 0:
		case 1:
		case 2:
		case 4:
			break;
		case 3:
			if (UNREAD(&SR_fptr, &brc))
				return -1;
			else
				break;
		case SR_BMDEL:
			if (digestmode)
				return -1;
			if (!(ptitle->accessed & FH_MARKED)) {
				SR_BMDELFLAG = YEA;
				markdel_post(locmem->crs_line,
					     &SR_fptr, currdirect);
				SR_BMDELFLAG = NA;
			}
			break;
		case SR_BMMARK:
			if (digestmode == 2 || digestmode == 3
			    || digestmode == 4 || digestmode == 5)
				return -1;
			mark_post(locmem->crs_line, &SR_fptr, currdirect);
			break;
		case SR_BMDIGEST:
			if (digestmode == YEA || digestmode == 4
			    || digestmode == 5)
				return -1;
			digest_post(locmem->crs_line, &SR_fptr, currdirect);
			break;
		case SR_BMIMPORT:
			strcpy(tmpboard, currboard);
			strcpy(currboard, anboard);
			a_Import(currdirect, &SR_fptr, YEA + add_anno_flag);
			change_dir(currdirect, &SR_fptr,
				   (void *) DIR_do_import,
				   locmem->crs_line, digestmode, 1);
			strcpy(currboard, tmpboard);
			break;
		case SR_BMTMP:
			a_Save("0Announce", currboard, &SR_fptr, YEA);
			break;
		case SR_BMNOREPLY:
			if (digestmode != 0)
				return -1;
			underline_post(locmem->crs_line, &SR_fptr, currdirect);
			break;
                case SR_BMCOMBINE:	//add by bjgyt, modify by mintbaggio
                        Add_Combine(currboard,&SR_fptr);
                        break;
		case SR_BMMINUSDEL:	//add by mintbaggio@BMY for minus-numposts delete
			/*SR_BMDELFLAG = YEA;
			result = del_post(locmem->crs_line, &SR_fptr, currdirect);
			SR_BMDELFLAG = NA;
			if(result == -1)
				return DIRCHANGED;
			if(result != DONOTHING){
				last_line--;
				locmem->crs_line--;
			}*/
			/*if (!(ptitle->accessed & FH_MARKED)) {
				SR_BMDELFLAG = YEA;
				del_post(locmem->crs_line,
					     &SR_fptr, currdirect);
				SR_BMDELFLAG = NA;
			}*/
			if (!(ptitle->accessed & FH_MARKED)) {
                                SR_BMDELFLAG = YEA;
                                mark_minus_del_post(locmem->crs_line,
                                             &SR_fptr, currdirect);
                                SR_BMDELFLAG = NA;
			}
			break;
		}
		if (!isstart) {
			if (uinfo.mode != RMAIL && auser == 0) {
				search_threadid(locmem, isnext, ptitle->thread,
						(passonly == 1)
						|| (passonly == 2));
				if (passonly == 1 || passonly == 2) {
					previous = locmem->crs_line;
					break;
				}
			}
			else{		//modify by mintbaggio 040322 for same author in BMfunc 'b'
				if(auser == 0)
					search_articles(locmem, title, isnext, auser + 2);
				else if(auser == 1)
					search_articles(locmem, title, isnext, auser);
			}
		}
		if (previous == locmem->crs_line) {
			break;
		}
		if (uinfo.mode == RMAIL)
			setmailfile(genbuf, currentuser.userid,
				    fh2fname(&SR_fptr));
		else if (uinfo.mode == BACKNUMBER)
			setbacknumberfile(genbuf, fh2fname(&SR_fptr));
		else if (uinfo.mode == DO1984)
			set1984file(genbuf, fh2fname(&SR_fptr));
		else
			setbfile(genbuf, currboard, fh2fname(&SR_fptr));
		previous = locmem->crs_line;
		setquotefile(genbuf);
		if (passonly == 0 || passonly == 4) {
			int ch;
			isnext = 1;
			ch = ansimore_withzmodem(genbuf, NA, SR_fptr.title);
			SETREAD(&SR_fptr, &brc);
			isstart = 0;
		      redo:
			if (ch != KEY_UP && ch != KEY_DOWN
			    && (ch <= 0 || strchr("RrEexp", ch) == NULL)) {
				move(t_lines - 1, 0);
				clrtoeol();
				prints
				    ("\033[1;44;31m[%8s] \033[33m���� R �� ���� Q,�� ����һ�� ��,n,Enter����һ�� ��,l�� ^R �ظ�����   \033[m",
				     auser ? "��ͬ����" : "�����Ķ�");
				ch = egetch();
			}
			switch (ch) {
			case Ctrl('Y'):
				zmodem_sendfile(0, &SR_fptr, currdirect);
				clear();
				goto redo;
			case 'Q':
			case 'q':
			case KEY_LEFT:
				istest = 1;
				break;
			case 'Y':
			case 'R':
			case 'y':
			case 'r':
/* Added by deardragon 1999.11.21 ���Ӳ��� RE ���� */
				if (in_mail) {
					mail_reply(0, &SR_fptr, (char *)
						   NULL);
					in_mail = YEA;
					break;
				}
				if (!(SR_fptr.accessed & FH_NOREPLY))
					do_reply(&SR_fptr);
				else {
					move(3, 0);
					clrtobot();
					prints
					    ("\n\n ����Re,���ǲ���Re!������!//grin");
					pressreturn();
					clear();
				}
/* Added End. */
				break;
			case ' ':
			case '\n':
			case 'n':
			case KEY_DOWN:
				isnext = 1;
				break;
			case Ctrl('A'):
				clear();
				show_author(0, &SR_fptr, currdirect);
				isnext = 1;
				break;
			case 'C':
				friend_author(0, &SR_fptr, currdirect);
				isnext = 1;
				break;
			case 'l':
			case KEY_UP:
			case 'u':
			case 'U':
			case KEY_PGUP:
				isnext = -1;
				break;
			case Ctrl('R'):
				if (in_mail) {
					mail_reply(0, &SR_fptr, (char *)
						   NULL);
					in_mail = YEA;
				} else
					post_reply(0, &SR_fptr, (char *)
						   NULL);
				break;
			case 'g':
				digest_post(0, &SR_fptr, currdirect);
				break;
			default:
				break;
			}
		}
	}
	if (passonly == 4 && readfirst == 0) {	/* youzi 1997.7.9 */
		RMVCURS;
		locmem->top_line = rem_top;
		locmem->crs_line = rem_crs;
		PUTCURS;
	}
	if ((passonly == 2) && (readfirst == 0) && (auser == 0))	/*��ͬ����ɾ��ʱ,�ܹ����ص�һƪ���� Bigman:2000.8.20 */
		return previous;
	else
		return 1;
}

#include <sys/mman.h>

int
searchpattern(filename, query)
char *filename;
char *query;
{
	char *ptr = NULL;
	int fd, ret;
	struct stat st;
	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return 0;
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)
	    || st.st_size <= 0) {
		close(fd);
		return 0;
	}
#define SKIP_ATTACH_LIMIT 65536
	if (st.st_size >= SKIP_ATTACH_LIMIT) {
		FILE *fp;
		char buf[256];
		fp = fdopen(fd, "r");
		while (fgets(buf, sizeof (buf), fp)) {
			if (strstr(buf, query)) {
				fclose(fp);
				close(fd);
				return 1;
			}
			if (!strncmp(buf, "begin 644 ", 10)) {
				fakedecode(fp);
				continue;
			}
			if (!strncmp(buf, "beginbinaryattach ", 18)) {
				unsigned int len;
				char ch;
				fread(&ch, 1, 1, fp);
				if (ch != 0) {
					ungetc(ch, fp);
					continue;
				}
				fread(&len, 4, 1, fp);
				len = ntohl(len);
				fseek(fp, len, SEEK_CUR);
				continue;
			}
		}
		fclose(fp);
		close(fd);
		return 0;
	}
	MMAP_TRY {
		ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		if (ptr == NULL)
			MMAP_RETURN(0);
		if (strncasestr(ptr, query, st.st_size))
			ret = 1;
		else
			ret = 0;
	}
	MMAP_CATCH {
		close(fd);
		ret = 0;
	}
	MMAP_END munmap(ptr, st.st_size);
	return ret;
}

static int
search_articles(locmem, query, offset, aflag)
struct keeploc *locmem;
char *query;
int offset, aflag;
{
	char *ptr;
	int now, match = 0;
	int complete_search;
	int ssize = sizeof (struct fileheader);
	int fd;

	if (aflag >= 2) {
		complete_search = 1;
		aflag -= 2;
	} else {
		complete_search = 0;
	}

	if (*query == '\0') {
		return 0;
	}
	if ((fd = open(currdirect, O_RDONLY, 0)) == -1)
		return 0;
	now = locmem->crs_line;
	while (1) {
		if (offset > 0) {
			if (++now > last_line)
				break;
		} else {
			if (--now < 1)
				break;
		}
		if (now == locmem->crs_line)
			break;
//              get_record(currdirect, &SR_fptr, ssize, now);
		if (lseek(fd, ssize * (now - 1), SEEK_SET) == -1) {
			close(fd);
			return 0;
		}
		if (read(fd, &SR_fptr, ssize) != ssize) {
			close(fd);
			return 0;
		}
		if (aflag == -1) {
			char p_name[256];
			if (uinfo.mode == RMAIL)
				setmailfile(p_name,
					    currentuser.userid,
					    fh2fname(&SR_fptr));
			else if (uinfo.mode == BACKNUMBER)
				setbacknumberfile(p_name, fh2fname(&SR_fptr));
			else if (uinfo.mode == DO1984)
				set1984file(p_name, fh2fname(&SR_fptr));
			else
				setbfile(p_name, currboard, fh2fname(&SR_fptr));
			if (searchpattern(p_name, query)) {
				match = cursor_pos(locmem, now, 10);
				break;
			} else
				continue;
		}
		ptr = aflag ? SR_fptr.owner : SR_fptr.title;
		if (complete_search == 1) {
			if (!strncasecmp(ptr, "RE: ", 4))
				ptr = ptr + 4;
			if (!strncmp(ptr, query, 45)) {
				match = cursor_pos(locmem, now, 10);
				break;
			}
		} else {
			if (strcasestr(ptr, query)) {
				match = cursor_pos(locmem, now, 10);
				break;
			}
		}
	}
//      move(t_lines - 1, 0);
//      clrtoeol();
	if (lseek(fd, ssize * (locmem->crs_line - 1), SEEK_SET)
	    == -1) {
		close(fd);
		return 0;
	}
	if (read(fd, &SR_fptr, ssize) != ssize) {
		close(fd);
		return 0;
	}
	close(fd);
	return match;
}

/* calc cursor pos and show cursor correctly -cuteyu */
static int
cursor_pos(locmem, val, from_top)
struct keeploc *locmem;
int val;
int from_top;
{
	if (val > last_line) {
		val = DEFINE(DEF_CIRCLE) ? 1 : last_line;
	}
	if (val <= 0) {
		val = DEFINE(DEF_CIRCLE) ? last_line : 1;
	}
	if (val >= locmem->top_line && val < locmem->top_line + screen_len - 1) {
		RMVCURS;
		locmem->crs_line = val;
		PUTCURS;
		return 0;
	}
	locmem->top_line = val - from_top;
	if (locmem->top_line <= 0)
		locmem->top_line = 1;
	locmem->crs_line = val;
	return 1;
}

static int
search_threadid(struct keeploc *locmem, int offset, int thread, int mode)
//�� locmem λ������offset��������thread��ͬ��fileheader��
//mode == 0 ������һ�����˳����������������һ������������
{
	int now, match = 0, start, sorted, i;
	struct fileheader *pFh;
      struct mmapfile mf = { ptr:NULL };
	now = locmem->crs_line;
	memset(&SR_fptr, 0, sizeof (struct fileheader));
	match = 0;
	if (digestmode == 0 || digestmode == 3 || uinfo.mode == BACKNUMBER) {	//�⼸��.DIR�������
		sorted = 1;
	} else {
		sorted = 0;
	}
	MMAP_TRY {
		if (mmapfile(currdirect, &mf) == -1)
			MMAP_RETURN(match);
		last_line = mf.size / sizeof (struct fileheader);
		if (now > last_line) {
			mmapfile(NULL, &mf);
			MMAP_RETURN(match);
		}
		if (mode == 0) {	//�����ѣ��ѵ���ͣ
			pFh = (struct fileheader *) (mf.ptr) + now - 1;
			while (1) {
				if (offset > 0) {
					if (++now > last_line)
						break;
					pFh++;
				} else {
					if (--now < 1)
						break;
					pFh--;
					if (sorted && pFh->filetime < thread)	//����.DIR�����ѵ�����Ϳ���ͣ��
						break;
				}
				if (pFh->thread == thread) {
					match =
					    cursor_pos(locmem, now,
						       screen_len / 2);
					break;
				}
			}
		} else {
			if (sorted && offset == -1) {	//�������ٵĲ���
				start = Search_Bin(mf.ptr, thread, 0, now - 1);	//���ַ��ҵ��������
				if (start >= 0) {	//�ƺ���ֱ���ҵ���Ŷ
					match =
					    cursor_pos(locmem, start + 1,
						       screen_len / 2);
					goto END;
				}
				start = -(start + 1);
				if (start >= now)
					goto END;
				pFh = (struct fileheader *) (mf.ptr) + start;
				for (i = start; i < now; i++) {
					if (pFh->thread == thread) {
						match =
						    cursor_pos(locmem, i + 1,
							       screen_len / 2);
						break;
					}
					pFh++;
				}

			} else {
				pFh = (struct fileheader *) (mf.ptr) + now - 1;
				while (1) {
					if (offset > 0) {
						if (++now > last_line)
							break;
						pFh++;
					} else {
						if (--now < 1)
							break;
						pFh--;
					}
					if (pFh->thread == thread) {
						match = now;
					}
				}
				if (match)	//�ҵ���
					match =
					    cursor_pos(locmem, match,
						       screen_len / 2);
			}
		}
	      END:{
		}
	}
	MMAP_CATCH {
		match = 0;
	}
	MMAP_END {
		memcpy(&SR_fptr,
		       (struct fileheader *) (mf.ptr) + locmem->crs_line - 1,
		       sizeof (struct fileheader));
		mmapfile(NULL, &mf);
	}
	move(t_lines - 1, 0);
	clrtoeol();
	return match;
}

static int
digest_mode()
{
	extern char currdirect[STRLEN];

	if (digestmode == YEA) {
		digestmode = NA;
		setbdir(currdirect, currboard, digestmode);
	} else {
		digestmode = YEA;
		setbdir(currdirect, currboard, digestmode);
		if (!dashf(currdirect)) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
			return DONOTHING;
		}
	}
	return NEWDIRECT;
}

