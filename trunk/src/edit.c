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
#include "edit.h"

struct textline *firstline = NULL;
struct textline *lastline = NULL;
struct boardmem *getbcache();

extern int local_article;
struct textline *currline = NULL;
int first_mark_line;
int currpnt = 0;
char searchtext[80];
int editansi = 0;
int scrollen = 2;
int marknum;
int moveln = 0;
int shifttmp = 0;
int ismsgline;
int tmpline;

struct textline *top_of_win = NULL;
int curr_window_line, currln;
int redraw_everything;
int insert_character = 1;
/* for copy/paste */
#define CLEAR_MARK()  mark_on = 0; mark_begin = mark_end = NULL;
struct textline *mark_begin, *mark_end;
int mark_on;
/* copy/paste */

/*for autocrlf*/
int savecursor;
/*autocrlf*/

/*for auto delete double char*/
int enabledbchar = 1;
/*auto delete double char*/

/* for input special char */
int inputspecial = 0;
int inputtype = 0;
int lineidx = 0;
extern int numofsig;

/* input special char */
/* add by KCN 1998.11*/
char specialcharbuf[10];
static int ishowspecialchar(void);
static void msgline(int ck);
static void msg(void);
static void indigestion(int i);
static struct textline *back_line(struct textline *pos, int num);
static struct textline *forward_line(struct textline *pos, int num);
static void countline(void);
static int getlineno(void);
static char *killsp(char *s);
static struct textline *alloc_line(void);
static void goline(int n);
static void go(void);
static void searchline(char text[STRLEN]);
static void search(void);
static void append(register struct textline *p, register struct textline *line);
static void delete_line(register struct textline *line);
static void split(register struct textline *line, register int pos);
static int join(register struct textline *line);
static void insert_char(register int ch);
static void ve_insert_str(char *str);
static void input_tools(void);
static void delete_char(void);
static void vedit_init(void);
static void strnput(char *str);
static void cstrnput(char *str);
static int Origin(struct textline *text);
static void display_buffer(void);
static int vedit_process_ESC(int arg);
static int mark_block(void);
static void process_MARK_action(int arg, char *msg);
static int process_ESC_action(int action, int arg);
static void vedit_key(int ch);
static int vedit_getspecialchar(int ch);
static int raw_vedit(char *filename, int saveheader, int modifyheader);
static void insert_to_fp(FILE * fp);
static void insert_from_fp(FILE * fp);
static void autolfline(char *line, FILE * fp);

static int
ishowspecialchar()
{
	char filename[80];
	FILE *fp;
	char tablebuf[100];
	int i;

	sprintf(filename, "table/big5.%d", inputtype);
	move(t_lines - 1, 18);
	if ((fp = fopen(filename, "r")) != NULL) {
		if (fgets(tablebuf, 99, fp) != NULL) {
			prints("%7s", tablebuf);
			move(t_lines - 1, 30);
			for (i = 0; i < lineidx + 1; i++)
				if (fgets(tablebuf, 99, fp) == NULL)
					return 0;
			for (i = 0; i < 5; i++) {
				prints(" %d.%2s", i + 1, &tablebuf[i * 13 + 8]);
				specialcharbuf[i * 2] = tablebuf[i * 13 + 8];
				specialcharbuf[i * 2 + 1] =
				    tablebuf[i * 13 + 9];
			}
			return 1;
		}
		return 0;
	}
	return 0;
}

static void
msgline(int ck)
{
	char buf[256], buf2[STRLEN * 2];
	extern int talkrequest;
	int tmpshow;
	time_t now;
	static int c = 0, lc = 0;
	extern int have_msg_unread;

	if (inputspecial) {
		if (editansi) {
			prints(ANSI_RESET);
		}
		move(t_lines - 1, 0);
		clrtoeol();
		//standout();
		prints("�������뷨���룺");
		ishowspecialchar();
		//standend();
		return;
	}

	if (ismsgline <= 0)
		return;
	now = time(0);
	tmpshow = showansi;
	showansi = 1;
	if (talkrequest) {
		talkreply();
		clear();
		showansi = 0;
		display_buffer();
		showansi = 1;
	}
	strcpy(buf, "[1;33;44m");
	if (++c >= 10) {
		ck = 1;
		c = 0;
	}
	if (ck)
		lc = chkmail();
	if (lc)
		strcat(buf, "��[5;32m��[m[1;33;44m��");
	else
		strcat(buf, "��  ��");

	if (have_msg_unread)
		strcat(buf, "��[5;32mMSG[m[1;33;44m��");
	else
		strcat(buf, "��   ��");
	strcat(buf, " [31mCtrl-Q[33m ��� ");
	sprintf(buf2,
		"״̬ [[32m%s[33m][[32m%4.4d[33m,[32m%3.3d[33m][[32m%s[33m] ʱ��",
		insert_character ? "Ins" : "Rep", currln + 1, currpnt + 1,
		enabledbchar ? "X" : " ");
	strcat(buf, buf2);
	sprintf(buf2, "[1;33;44m��[1;32m%.16s[33m��     [m", ctime(&now));
	strcat(buf, buf2);
	move(t_lines - 1, 0);
	clrtoeol();
	prints("%s", buf);
	showansi = tmpshow;
}

static void
msg()
{
	int x, y;
	int tmpansi;
	if (!inputspecial) {
		tmpansi = showansi;
		showansi = 1;
		getyx(&y, &x);
		msgline(1);

		signal(SIGALRM, (void *) msg);
		move(y, x);
		alarm(60);
		showansi = tmpansi;
	} else {
		signal(SIGALRM, (void *) msg);
		alarm(60);
	}
	return;
}

static void
indigestion(i)
int i;
{
	prints("SERIOUS INTERNAL INDIGESTION CLASS %d\n", i);
}

static struct textline *
back_line(pos, num)
struct textline *pos;
int num;
{
	moveln = 0;
	while (num-- > 0)
		if (pos && pos->prev) {
			pos = pos->prev;
			moveln++;
		}

	return pos;
}

static struct textline *
forward_line(pos, num)
struct textline *pos;
int num;
{
	moveln = 0;
	while (num-- > 0)
		if (pos && pos->next) {
			pos = pos->next;
			moveln++;
		}
	return pos;
}

static void
countline()
{
	struct textline *pos;
	pos = firstline;
	moveln = 0;
	while (pos != lastline)
		if (pos->next) {
			pos = pos->next;
			moveln++;
		}
}

static int
getlineno()
{
	int cnt = 0;
	struct textline *p = currline;

	while (p != top_of_win) {
		if (p == NULL)
			break;
		cnt++;
		p = p->prev;
	}
	return cnt;
}

static char *
killsp(s)
char *s;
{
	while (*s == ' ')
		s++;
	return s;
}

static struct textline *
alloc_line()
{
	register struct textline *p;

	p = (struct textline *) malloc(sizeof (*p));
	if (p == NULL) {
		indigestion(13);
		abort_bbs();
	}
	p->next = NULL;
	p->prev = NULL;
	p->data[0] = '\0';
	p->len = 0;
	p->attr = 0;		/* for copy/paste */
	return p;
}

/*
  Appends p after line in list.  keeps up with last line as well.
 */

static void
goline(n)
int n;
{
	register struct textline *p = firstline;
	int count;

	if (n < 0)
		n = 1;
	if (n == 0)
		return;
	for (count = 1; count < n; count++) {
		if (p) {
			p = p->next;
			continue;
		} else
			break;
	}
	if (p) {
		currln = n - 1;
		curr_window_line = 0;
		top_of_win = p;
		currline = p;
	} else {
		top_of_win = lastline;
		currln = count - 2;
		curr_window_line = 0;
		currline = lastline;
	}
	if (Origin(currline)) {
		currline = currline->prev;
		top_of_win = currline;
		curr_window_line = 0;
		currln--;
	}
	if (Origin(currline->prev)) {
		currline = currline->prev->prev;
		top_of_win = currline;
		curr_window_line = 0;
		currln -= 2;
	}

}

static void
go()
{
	char tmp[8];
	int line;
	signal(SIGALRM, SIG_IGN);
	getdata(23, 0, "����Ҫ�����ڼ���: ", tmp, 7, DOECHO, YEA);
	msg();
	if (tmp[0] == '\0')
		return;
	line = atoi(tmp);
	goline(line);
	return;
}

static void
searchline(text)
char text[STRLEN];
{
	int tmpline;
	int addr;
	int tt;

	register struct textline *p = currline;
	int count = 0;
	tmpline = currln;
	for (;; p = p->next) {
		count++;
		if (p) {
			if (count == 1)
				tt = currpnt;
			else
				tt = 0;
			if (strstr(p->data + tt, text)) {
				addr =
				    (int) (strstr(p->data + tt, text) -
					   p->data) + strlen(text);
				currpnt = addr;
				break;
			}
		} else
			break;
	}
	if (p) {
		currln = currln + count - 1;
		curr_window_line = 0;
		top_of_win = p;
		currline = p;
	} else {
		goline(currln + 1);
	}
	if (Origin(currline)) {
		currline = currline->prev;
		top_of_win = currline;
		curr_window_line = 0;
		currln--;
	}
	if (Origin(currline->prev)) {
		currline = currline->prev->prev;
		top_of_win = currline;
		curr_window_line = 0;
		currln -= 2;
	}

}

static void
search()
{
	char tmp[STRLEN];

	signal(SIGALRM, SIG_IGN);
	getdata(23, 0, "��Ѱ�ִ�: ", tmp, 65, DOECHO, YEA);
	msg();
	if (tmp[0] == '\0')
		return;
	else
		strcpy(searchtext, tmp);

	searchline(searchtext);
	return;
}

static void
append(p, line)
register struct textline *p, *line;
{
	p->next = line->next;
	if (line->next)
		line->next->prev = p;
	else
		lastline = p;
	line->next = p;
	p->prev = line;
}

/*
  delete_line deletes 'line' from the list and maintains the lastline, and 
  firstline pointers.
 */

static void
delete_line(line)
register struct textline *line;
{
	/* if single line */
	if (!line->next && !line->prev) {
		line->data[0] = '\0';
		line->len = 0;
		CLEAR_MARK();
		return;
	}
#define ADJUST_MARK(p, q) if(p == q) p = (q->next) ? q->next : q->prev

	ADJUST_MARK(mark_begin, line);
	ADJUST_MARK(mark_end, line);

	if (line->next)
		line->next->prev = line->prev;
	else
		lastline = line->prev;	/* if on last line */

	if (line->prev)
		line->prev->next = line->next;
	else
		firstline = line->next;	/* if on first line */

	if (line)
		free(line);
}

/*
  split splits 'line' right before the character pos
 */

static void
split(line, pos)
register struct textline *line;
register int pos;
{
	register struct textline *p = alloc_line();

	if (pos > line->len) {
		free(p);
		return;
	}
	p->len = line->len - pos;
	line->len = pos;
	strcpy(p->data, (line->data + pos));
	p->attr = line->attr;	/* for copy/paste */
	*(line->data + pos) = '\0';
	append(p, line);
	if (line == currline && pos <= currpnt) {
		currline = p;
		currpnt -= pos;
		curr_window_line++;
		currln++;
	}
	redraw_everything = YEA;
}

/*
  join connects 'line' and the next line.  It returns true if:
  
  1) lines were joined and one was deleted
  2) lines could not be joined
  3) next line is empty

  returns false if:

  1) Some of the joined line wrapped
 */

static int
join(line)
register struct textline *line;
{
	register int ovfl;
	if (!line->next)
		return YEA;
	/*if(*killsp(line->next->data) == '\0')
	   return YEA ; */
	ovfl = line->len + line->next->len - WRAPMARGIN;
	if (ovfl < 0) {
		strcat(line->data, line->next->data);
		line->len += line->next->len;
		delete_line(line->next);
		return YEA;
	} else {
		register char *s;
		register struct textline *p = line->next;

		s = p->data + p->len - ovfl - 1;
		while (s != p->data && *s == ' ')
			s--;
		while (s != p->data && *s != ' ')
			s--;
		if (s == p->data)
			return YEA;
		split(p, (s - p->data) + 1);
		if (line->len + p->len >= WRAPMARGIN) {
			indigestion(0);
			return YEA;
		}
		join(line);
		p = line->next;
		if (p->len >= 1 && p->len + 1 < WRAPMARGIN) {
			if (p->data[p->len - 1] != ' ') {
				strcat(p->data, " ");
				p->len++;
			}
		}
		return NA;
	}
}

static void
insert_char(ch)
register int ch;
{
	register int i;
	register char *s;
	char *ptr;
	register struct textline *p = currline;
	int wordwrap = YEA;

	if (currpnt > p->len) {
		indigestion(1);
		return;
	}
	if (currpnt < p->len && !insert_character) {
		p->data[currpnt++] = ch;
	} else {
		for (i = p->len; i >= currpnt; i--)
			p->data[i + 1] = p->data[i];
		p->data[currpnt] = ch;
		p->len++;
		currpnt++;
	}
	if (p->len < WRAPMARGIN)
		return;
	s = p->data + (p->len - 1);
	while (s != p->data && *s == ' ')
		s--;
	while (s != p->data && *s != ' ')
		s--;
	if (s == p->data) {
		wordwrap = NA;
		s = p->data + (p->len - 2);
	}
	if (!wordwrap) {
		ptr = p->data;
		i = 0;
		while (ptr <= s) {
			if (i)
				i = 0;
			else if ((unsigned char) *ptr >= 128)
				i = 1;
			ptr++;
		}
		if (i)
			s--;
	}
	split(p, (s - p->data) + 1);
	p = p->next;
	if (wordwrap && p->len >= 1) {
		i = p->len;
		if (p->data[i - 1] != ' ') {
			p->data[i] = ' ';
			p->data[i + 1] = '\0';
			p->len++;
		}
		{
		}
	}
	while (!join(p)) {
		p = p->next;
		if (p == NULL) {
			indigestion(2);
			break;
		}
	}
	if (Origin(currline)) {
		currline = p->prev;
		curr_window_line--;
		currln--;
	}
}

static void
ve_insert_str(str)
char *str;
{
	while (*str)
		insert_char(*(str++));
}

/* ----------------------------------------------------- */
/* �༭��������ʽ�����̴���				 */
/* ----------------------------------------------------- */

/* input tool */
static void
input_tools()
{
	char msg[] = { "1.�Ӽ��˳�  2.һ������  3.���£ã�  4.����б�� ��[Q]" };
	char *ansi1[7][10] = {
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"}
	};

	char *ansi2[7][10] = {
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
	};

	char *ansi3[7][10] = {
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "�J"},
		{"�Y", "�K", "�L", "�M", "�N", "�O", "�P", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"}
	};

	char *ansi4[7][10] = {
		{"�x", "�y", "�z", "�{", "�|", "�}", "�~", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "�u", "�v", "�w", "��"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"},
		{"��", "�I", "�J", "�K", "�L", "��", "��", "��", "��", "��"},
		{"��", "��", "��", "��", "��", "��", "�q", "�r", "�s", "�t"},
		{"��", "��", "��", "��", "��", "��", "��", "�H", "��", "�I"},
		{"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"}
	};

	char buf[128] = "�������빤��:", tmp[5];
	char *show[7][10];
	int ch, i, page;

	strcpy(buf + 13, msg);
	move(t_lines - 2, 0);
	clrtoeol();
	prints("%s", buf);
	ch = igetkey();

	if (ch < '1' || ch > '4')
		return;

	switch (ch) {
	case '1':
		memcpy(show, ansi1, sizeof (show));
		break;
	case '2':
		memcpy(show, ansi2, sizeof (show));
		break;
	case '3':
		memcpy(show, ansi3, sizeof (show));
		break;
	case '4':
		memcpy(show, ansi4, sizeof (show));
		break;
	}

	page = 0;
	for (;;) {
		buf[0] = '\0';

		sprintf(buf, "��%dҳ:", page + 1);
		for (i = 0; i < 10; i++) {
			sprintf(tmp, "%d%s%s ", i, ".", show[page][i]);
			strcat(buf, tmp);
		}
		strcat(buf, "(P:��  N:��)[Q]\0");
		move(t_lines - 2, 0);
		clrtoeol();
		prints("%s", buf);
		ch = igetkey();

		if (ch == 'p') {
			if (page)
				page -= 1;
		} else if (ch == 'n') {
			if (page != 6)
				page += 1;
		} else if (ch < '0' || ch > '9') {
			buf[0] = '\0';
			break;
		} else {
			ve_insert_str(show[page][ch - '0']);
			break;
		}
		buf[0] = '\0';
	}
}

/* end */

static void
delete_char()
{
	register int i;
	register int dbchar = 0;

	if (currline->len == 0)
		return;
	if (currpnt >= currline->len) {
		indigestion(1);
		return;
	}
	if (enabledbchar && (currline->data[currpnt] & 0x80)) {
		for (i = 0; i < currpnt; i++)
			if (dbchar)
				dbchar = 0;
			else if (currline->data[i] & 0x80)
				dbchar = 1;
		/* �������������1999��Ԫ��8���޸Ĺ� */
		if (currpnt < currline->len - 1) {
			if (currline->data[currpnt + 1] & 0x80) {
				/*  ��������ȷ��ɾ������һ������ */
				if (dbchar) {
					for (i = currpnt - 1;
					     i != currline->len - 1; i++)
						currline->data[i] =
						    currline->data[i + 2];
					currpnt--;
				} else
					for (i = currpnt;
					     i != currline->len - 1; i++)
						currline->data[i] =
						    currline->data[i + 2];
				currline->len -= 2;
				return;
			} else {	/* ��ǰλΪ��λ������λΪ��λ */
				if (dbchar) {	/* ��ǰ������ */
					for (i = currpnt - 1;
					     i != currline->len - 1; i++)
						currline->data[i] =
						    currline->data[i + 2];
					currpnt--;
					currline->len -= 2;
				} else {	/* ��ǰΪ�����֣�ֻɾ���ֽ� */
					for (i = currpnt; i != currline->len;
					     i++)
						currline->data[i] =
						    currline->data[i + 1];
					currline->len--;
				}
				return;
			}
		}
		if (dbchar && currpnt == currline->len - 1) {
			for (i = currpnt - 1; i != currline->len - 1; i++)
				currline->data[i] = currline->data[i + 2];
			currline->len -= 2;
			currpnt--;
		} else {
			for (i = currpnt; i != currline->len; i++)
				currline->data[i] = currline->data[i + 1];
			currline->len--;
		}
	} else {
		for (i = currpnt; i != currline->len; i++)
			currline->data[i] = currline->data[i + 1];
		currline->len--;
	}
}

static void
vedit_init()
{
	register struct textline *p = alloc_line();
	first_mark_line = 0;
	firstline = p;
	lastline = p;
	currline = p;
	currpnt = 0;
	marknum = 0;

	process_ESC_action('M', '0');
	top_of_win = p;
	curr_window_line = 0;
	currln = 0;
	redraw_everything = NA;

	CLEAR_MARK();
}

static void
insert_to_fp(FILE * fp)
{
	int ansi = 0;
	struct textline *p;

	for (p = firstline; p; p = p->next)
		if (p->data[0]) {
			fprintf(fp, "%s\n", p->data);
			if (strchr(p->data, '\033'))
				ansi++;
		}
	if (ansi)
		fprintf(fp, "%s\n", ANSI_RESET);
}

static void
insert_from_fp(fp)
FILE *fp;
{
	int ch;
	while ((ch = getc(fp)) != EOF)
		if (isprint2(ch) || ch == 27) {
			if (currpnt < 254)
				insert_char(ch);
			else if (currpnt < 255)
				insert_char('.');
		} else if (ch == Ctrl('I')) {
			do {
				insert_char(' ');
			} while (currpnt & 0x7);
		} else if (ch == '\n')
			split(currline, currpnt);
}

static void
read_file(filename, saveheader, modifyheader)
char *filename;
int saveheader;
int modifyheader;
{
	FILE *fp;

	if (currline == NULL)
		vedit_init();
	if ((fp = fopen(filename, "r+")) == NULL) {
		if ((fp = fopen(filename, "w+")) != NULL) {
			fclose(fp);
			return;
		}
		indigestion(4);
		abort_bbs();
	}
	/* Added by deardragon 1999.11.13  ��ֹ�޸�����ͷ����Ϣ */
	/* ������޸�����, �Ȱ�����ͷ����Ϣ��������, ��ʹָ��ָ�±༭�� */
	/* Patched by lepton 2002.11.23 */
	if (!saveheader && !modifyheader)
		keepoldheader(fp, KEEPHEADER);
	insert_from_fp(fp);
	fclose(fp);
}

char save_title[STRLEN];
int in_mail;

int
write_header(fp, mode)
FILE *fp;
int mode;
{
	int noname = 0;
	extern char fromhost[];
	extern struct postheader header;
	struct boardmem *bp;
	char uid[20];
	char uname[NAMELEN];
	time_t now;

	now = time(0);
	strsncpy(uid, currentuser.userid, 20);
	if (in_mail)
		strsncpy(uname, currentuser.username, NAMELEN);
	else
		strsncpy(uname, currentuser.username, NAMELEN);
	save_title[STRLEN - 10] = '\0';
	bp = getbcache(currboard);
	//����bp�᲻����ô��fp�����Ǹ�Ŀ¼���ذ�
	//��ʱ��Ͳ�����ڰ�,������ɾ���˰��� ����Ŀ¼����...
	if (bp)
		noname = bp->header.flag & ANONY_FLAG;
	if (in_mail)
		fprintf(fp, "������: %s (%s)\n", uid, uname);
	else {
		fprintf(fp, "������: %s (%s), ����: %s\n",
			(noname
			 && header.chk_anony) ? "Anonymous" : uid, (noname
								    &&
								    header.
								    chk_anony) ?
			"����������ʹ" : uname, currboard);
	}
	fprintf(fp, "��  ��: %s\n", save_title);
	fprintf(fp, "����վ: %s (%24.24s), %s", MY_BBS_NAME, ctime(&now),
		local_article ? "��վ(" MY_BBS_DOMAIN ")" : "ת��("
		MY_BBS_DOMAIN ")");
	if (in_mail)
		fprintf(fp, "\n��  Դ: %s", fromhost);
	fprintf(fp, "\n\n");
	return 0;
}

void
addsignature(fp, blank)
FILE *fp;
int blank;
{
	FILE *sigfile;
	int i, valid_ln = 0;
	int sig;
	char tmpsig[MAXSIGLINES][256];
	char inbuf[256];
	char fname[STRLEN];

	if (blank)
		fputs("\n", fp);
	fputs("--\n", fp);
	if (HAS_PERM(PERM_DENYSIG))
		return;
	setuserfile(fname, "signatures");
	if ((sigfile = fopen(fname, "r")) == NULL) {
		return;
	}
	sig =
	    (currentuser.signature ==
	     -1) ? (numofsig ? (rand() % numofsig +
				1) : 0) : currentuser.signature;
	for (i = 1; i <= (sig - 1) * MAXSIGLINES && sig != 1; i++) {
		if (!fgets(inbuf, sizeof (inbuf), sigfile)) {
			fclose(sigfile);
			return;
		}
	}
	for (i = 1; i <= MAXSIGLINES; i++) {
		if (fgets(inbuf, sizeof (inbuf), sigfile)) {
			if (inbuf[0] != '\n')
				valid_ln = i;
			strcpy(tmpsig[i - 1], inbuf);
		} else
			break;
	}
	fclose(sigfile);
	for (i = 1; i <= valid_ln; i++)
		fputs(tmpsig[i - 1], fp);
}

#define KEEP_EDITING -2

static void
valid_article(pmt, abort)
char *pmt, *abort;
{
	struct textline *p = firstline;
	char ch;
	int total, lines, len, sig, y;
	int w;

	w = NA;

	if (uinfo.mode == POSTING) {
		total = lines = len = sig = 0;
		while (p != NULL) {
			if (!sig) {
				ch = p->data[0];
				if (strcmp(p->data, "--") == 0)
					sig = 1;
				else if (ch != ':' && ch != '>' && ch != '=' &&
					 !strstr(p->data, "�Ĵ������ᵽ: ��")) {
					lines++;
					len += strlen(p->data);
				}
			}
			total++;
			p = p->next;
		}
		y = 2;
		if (lines < total * 0.35 - MAXSIGLINES) {
			move(y, 0);
			prints
			    ("ע�⣺��ƪ���µ����Թ���, ������ɾ��һЩ����Ҫ������.\n");
			y += 3;
		}
		if (len < 40 || lines < 1) {
			move(y, 0);
			prints("ע�⣺��ƪ���¹��ڼ��, ϵͳ��Ϊ�ǹ�ˮ����.\n");
			y += 3;
		}
		if (w) {
			strcpy(pmt,
			       "(E)�ٱ༭, (F)�����з�����(S)����, (A)ȡ�� or (T)���ı���? [E]: ");
		}
	}

	getdata(0, 0, pmt, abort, 3, DOECHO, YEA);
	if (w && abort[0] == '\0')
		abort[0] = 'E';
	switch (abort[0]) {
	case 'A':
	case 'a':		/* abort */
	case 'E':
	case 'e':		/* keep editing */
		return;
	}

}

/* Add By KCN for auto crlf*/
static void
autolfline(line, fp)
char *line;
FILE *fp;
{
	char *p1, *p2;
	int dbchar = 0;
	int n = 0;
	int curlinelen = 0;
	int inESC = 0;
	p2 = p1 = line;
	while (*p2 != 0) {
		switch (inESC) {
		case 0:
/*data mode*/
			if (*p2 == KEY_ESC)
				inESC = 1;
			else {
				curlinelen++;
				if (curlinelen > 78 && *p1 != ':') {
					if (dbchar) {
						fwrite(p1, 1, p2 - p1 - 1, fp);
						p1 = p2 - 1;
					} else {
						fwrite(p1, 1, p2 - p1, fp);
						p1 = p2;
					}
					fprintf(fp, "\n");
					//curlinelen=0;
					curlinelen = p2 - p1 + 1;	//by ylsdd
				}
				if (dbchar)
					dbchar = 0;
				else if (*p2 & 0x80)
					dbchar = 1;
			}
			break;
		case 1:
/*ESC mode            */
			if (*p2 == '[')
				inESC = 2;
			else {
				inESC = 0;
/*Error ESC format.Need to correct curlinelen;    				*/
			}
			n = 0;
			dbchar = 0;
			break;
		case 2:
/* '*[' begin */
			if (*p2 == ';') {
				n = 0;
				break;
			}
			if (isdigit(*p2)) {
				n = n * 10 + (int) *p2 - '0';
				break;
			}
			switch (*p2) {
			case 'C':
				curlinelen += n;
				break;
			case 'D':
				curlinelen -= n;
				break;
			case 'H':
			case 'f':
				if (n == 0)
					n = 1;
				curlinelen = n - 1;
				break;
			case 's':
				savecursor = curlinelen;
				break;
			case 'u':
				if (savecursor != -1)
					curlinelen = savecursor;
				break;
/*Need add other esc command.			  */
			};
			inESC = 0;
		}
		p2++;
	}
	fprintf(fp, "%s\n", p1);
}

static int
write_file(filename, saveheader, modifyheader)
char *filename;
int saveheader;
int modifyheader;
{
	struct textline *p = firstline;
	FILE *fp = NULL;
	char abort[6];
	int aborted = 0;
	int autolf = 0;
	int save_stat;
	char p_buf[100];
	signal(SIGALRM, SIG_IGN);
	clear();

	save_stat = local_article;
	//local_article: -1 ��ת�Ű���  0 ת�Ű���Ĭ�ϲ�ת��  1 ת��
	if (uinfo.mode != CCUGOPHER) {
		if (uinfo.mode == POSTING) {
			if (local_article == 1)
				strcpy(p_buf,
				       "(L)��ת��, (S)����, (F)�Զ����з���, (A)ȡ��, (T)���ı��� or (E)�ٱ༭? [L]: ");
			else {
				if (local_article == -1)
					strcpy(p_buf,
					       "(L)��ת��, (F)�Զ����з���, (A)ȡ��, (T)���ı��� or (E)�ٱ༭? [L]: ");
				else
					strcpy(p_buf,
					       "(S)����, (L)��ת�ţ�(F)�Զ����з���, (A)ȡ��, (T)���ı��� or (E)�ٱ༭? [S]: ");
			}
		} else if (uinfo.mode == SMAIL)
			strcpy(p_buf,
			       "(S)�ĳ�, (F)�Զ����з�����(A)ȡ��, or (E)�ٱ༭? [S]: ");
		else
			strcpy(p_buf,
			       "(S)���浵��, (F)�Զ����з�����(A)�����༭, (E)�����༭? [S]: ");
		valid_article(p_buf, abort);

		if ((local_article == 0)
		    && ((abort[0] == 'l')
			|| (abort[0] == 'L')))
			local_article = 1;
		if ((local_article == 1)
		    && ((abort[0] == 's')
			|| (abort[0] == 'S')))
			local_article = 0;
		if (local_article == -1)
			local_article = 1;
		if (abort[0] == 0)
			abort[0] = 's';

		if (abort[0] == 'F' || abort[0] == 'f') {
			abort[0] = 's';
			autolf = 1;
		}
	}
	if (abort[0] == 'a' || abort[0] == 'A') {
		struct stat stbuf;
		clear();
		if (uinfo.mode != CCUGOPHER) {
			prints("ȡ��...\n");
			refresh();
			sleep(1);
		}
		if (stat(filename, &stbuf) || stbuf.st_size == 0)
			unlink(filename);
		aborted = -1;
	} else if (abort[0] == 'e' || abort[0] == 'E') {
		msg();
		local_article = save_stat;
		return KEEP_EDITING;
	} else if (abort[0] == 't' || abort[0] == 'T') {
		char buf[STRLEN];

		move(1, 0);
		prints("�ɱ���: %s", save_title);
		sprintf(buf, "%s", save_title);
		getdata(2, 0, "�±���: ", buf, 50, DOECHO, NA);
		if (strcmp(save_title, buf) && strlen(buf) != 0) {
			strncpy(save_title, buf, STRLEN);
		}
	} else if (abort[0] == 's' || abort[0] == 'S') {
	}

	firstline = NULL;
	if (!aborted) {
		if ((fp = fopen(filename, "w")) == NULL) {
			indigestion(5);
			abort_bbs();
		}
		if (!strncmp(filename, "2nd/", 4)) {
			sprintf(genbuf, "%s(%s), ������Ϣʱip: %s\n\n",
				currentuser.userid, currentuser.username,
				currentuser.lasthost);
			fputs(genbuf, fp);
		} else if (saveheader)
			write_header(fp, 0);
		/* Added by deardragon 1999.11.13 ��ֹ�޸�����ͷ����Ϣ */
		/* ������޸�����, ���ڱ���ʱ, �ָ�����ͷ��Ϣ. */
		/* Patched by lepton 2002.11.23 */
		else if (!modifyheader)
			keepoldheader(fp, RESTOREHEADER);
	}
	savecursor = -1;
	while (p != NULL) {
		struct textline *v = p->next;
		if (!aborted)
			if (p->next != NULL || p->data[0] != '\0') {
				if (autolf)
					autolfline(p->data, fp);
				else
					fprintf(fp, "%s\n", p->data);
			}
		free(p);
		p = v;
	}
	if (!aborted)
		fclose(fp);
	currline = NULL;
	lastline = NULL;
	firstline = NULL;
	return aborted;
}

void
keep_fail_post()
{
	char filename[STRLEN];
	struct textline *p = firstline;
	FILE *fp;

	sprintf(filename, "home/%c/%s/%s.deadve",
		mytoupper(currentuser.userid[0]), currentuser.userid,
		currentuser.userid);
	if ((fp = fopen(filename, "w")) == NULL) {
		indigestion(5);
		return;
	}
	while (p != NULL) {
		struct textline *v = p->next;
		if (p->next != NULL || p->data[0] != '\0')
			fprintf(fp, "%s\n", p->data);
		free(p);
		p = v;
	}
	fclose(fp);
	return;
}

static void
strnput(str)
char *str;
{
	int count = 0;

	while ((*str != '\0') && (++count < STRLEN)) {
		if (*str == KEY_ESC) {
			outc('*');
			str++;
			continue;
		}
		outns(str++, 1);
	}
}

static void
cstrnput(str)
char *str;
{
	int count = 0;

	prints("%s", ANSI_REVERSE);
	while ((*str != '\0') && (++count < STRLEN)) {
		if (*str == KEY_ESC) {
			outc('*');
			str++;
			continue;
		}
		outns(str++, 1);
	}
	while (++count < STRLEN)
		outc(' ');
	clrtoeol();
	prints("%s", ANSI_RESET);
}

/*Function Add by SmallPig*/
static int
Origin(text)
struct textline *text;
{
	char tmp[STRLEN];

	if (uinfo.mode != EDIT)
		return 0;
	if (!text)
		return 0;
	sprintf(tmp, ":��%s %s��[FROM:", MY_BBS_NAME, email_domain());
	if (strstr(text->data, tmp) && *text->data != ':')
		return 1;
	else
		return 0;
}

static void
display_buffer()
{
	register struct textline *p;
	register int i;
	int shift;
	int temp_showansi;

	temp_showansi = showansi;

	for (p = top_of_win, i = 0; i < t_lines - 1; i++) {
		move(i, 0);
		if (p) {
			shift = (currpnt + 2 > STRLEN) ?
			    (currpnt / (STRLEN - scrollen)) * (STRLEN -
							       scrollen) : 0;
			if (editansi) {
				showansi = 1;
				prints("%s", p->data);
			} else if ((p->attr & M_MARK)) {
				showansi = 1;
				clear_whole_line(i);
				cstrnput(p->data + shift);
			} else {
				if (p->len >= shift) {
					showansi = 0;
					strnput(p->data + shift);
				} else
					clrtoeol();
			}
			p = p->next;
		} else
			prints("%s~", editansi ? ANSI_RESET : "");
		clrtoeol();
	}

	showansi = temp_showansi;
	return;
}

static int
vedit_process_ESC(arg)
int arg;			/* ESC + x */
{
	int ch2, action;

#define WHICH_ACTION_COLOR    \
"(M)���鴦�� (I/E)��ȡ/д������� (C)ʹ�ò�ɫ (F/B/R)ǰ��/����/��ԭɫ��"
#define WHICH_ACTION_MONO    \
"(M)���鴦�� (I/E)��ȡ/д������� (C)ʹ�õ�ɫ (F/B/R)ǰ��/����/��ԭɫ��"

#define CHOOSE_MARK     "(0)ȡ����� (1)�趨��ͷ (2)�趨��β (3)���Ʊ������ "
#define FROM_WHICH_PAGE "��ȡ�������ڼ�ҳ? (0-7) [Ԥ��Ϊ 0]"
#define SAVE_ALL_TO     "����ƪ����д��������ڼ�ҳ? (0-7) [Ԥ��Ϊ 0]"
#define SAVE_PART_TO    "�ѱ�ǿ�д��������ڼ�ҳ? (0-7) [Ԥ��Ϊ 0]"
#define FROM_WHICH_SIG  "ȡ��ǩ�����ڼ�ҳ? (0-7) [Ԥ��Ϊ 0]"
#define CHOOSE_FG       "ǰ����ɫ? 0)�� 1)�� 2)�� 3)�� 4)���� 5)�ۺ� 6)ǳ�� 7)�� "
#define CHOOSE_BG       "������ɫ? 0)�� 1)�� 2)�� 3)�� 4)���� 5)�ۺ� 6)ǳ�� 7)�� "
#define CHOOSE_BIG5     "�����? 0)��� 1)���� 2)��� 3)ע�� 4)ƽ�� 5)Ƭ�� 6)ϣ�� 7)����"
#define CHOOSE_ERROR    "ѡ�����"

	switch (arg) {
	case 'M':
	case 'm':
		ch2 = ask(CHOOSE_MARK);
		action = 'M';
		break;
	case 'I':
	case 'i':		/* import */
		ch2 = ask(FROM_WHICH_PAGE);
		action = 'I';
		break;
	case 'E':
	case 'e':		/* export */
		ch2 = ask(mark_on ? SAVE_PART_TO : SAVE_ALL_TO);
		action = 'E';
		break;
	case 'S':
	case 's':		/* signature */
		ch2 = '0';
		action = 'S';
		break;
	case 'F':
	case 'f':
		ch2 = ask(CHOOSE_FG);
		action = 'F';
		break;
	case 'B':
	case 'b':
		ch2 = ask(CHOOSE_BG);
		action = 'B';
		break;
	case 'R':
	case 'r':
		ch2 = '0';	/* not used */
		action = 'R';
		break;
	case 'D':
	case 'd':
		ch2 = '4';
		action = 'M';
		break;
	case 'N':
	case 'n':
		ch2 = '0';
		action = 'N';
		break;
	case 'G':
	case 'g':
		ch2 = '1';
		action = 'G';
		break;
	case 'L':
	case 'l':
		ch2 = '0';	/* not used */
		action = 'L';
		break;
	case 'C':
	case 'c':
		ch2 = '0';	/* not used */
		action = 'C';
		break;
	case 'O':
	case 'o':
/* Modified by KCN 1998.11 for input special char.*/
/*             ch2 = ask(CHOOSE_BIG5);     action = 'O'; break;                     */
		ch2 = '0';
		action = 'O';
		break;
	case '=':
		ch2 = '0';	/* not used */
		action = '=';
		break;
	case 'Q':
	case 'q':
		ch2 = '0';	/* not used */
		action = 'M';
		break;
		marknum = 0;
		break;
	default:
		return 0;
	}

	if (strchr("IES", action) && (ch2 == '\n' || ch2 == '\r'))
		ch2 = '0';

	if (ch2 >= '0' && ch2 <= '7')
		return process_ESC_action(action, ch2);
	else {
		return ask(CHOOSE_ERROR);
	}
}

static int
mark_block()
{
	struct textline *p;
	int pass_mark = 0;

	first_mark_line = 0;
	if (mark_begin == NULL && mark_end == NULL)
		return 0;
	if (mark_begin == mark_end) {
		mark_begin->attr |= M_MARK;
		return 1;
	}
	if (mark_begin == NULL || mark_end == NULL) {
		if (mark_begin != NULL)
			mark_begin->attr |= M_MARK;
		else
			mark_end->attr |= M_MARK;
		return 1;
	} else {
		for (p = firstline; p != NULL; p = p->next) {
			if (p == mark_begin || p == mark_end) {
				pass_mark++;
				p->attr |= M_MARK;
				continue;
			}
			if (pass_mark == 1)
				p->attr |= M_MARK;
			else {
				first_mark_line++;
				p->attr &= ~(M_MARK);
			}
			if (pass_mark == 2)
				first_mark_line--;
		}
		return 1;
	}
}

static void
process_MARK_action(arg, msg)
int arg;			/* operation of MARK */
char *msg;			/* message to return */
{
	struct textline *p;
	int dele_1line;

	switch (arg) {
	case '0':		/* cancel */
		for (p = firstline; p != NULL; p = p->next)
			p->attr &= ~(M_MARK);
		CLEAR_MARK();
		break;
	case '1':		/* mark begin */
		mark_begin = currline;
		mark_on = mark_block();
		if (mark_on)
			strcpy(msg, "������趨���");
		else
			strcpy(msg, "���趨��ͷ���, ���޽�β���");
		break;
	case '2':		/* mark end */
		mark_end = currline;
		mark_on = mark_block();
		if (mark_on)
			strcpy(msg, "������趨���");
		else
			strcpy(msg, "���趨��β���, ���޿�ͷ���");
		break;
	case '3':		/* copy mark */
		if (mark_on && !(currline->attr & M_MARK)) {
			for (p = firstline; p != NULL; p = p->next) {
				if (p->attr & M_MARK) {
					ve_insert_str(p->data);
					split(currline, currpnt);
				}
			}
		} else
			bell();
		strcpy(msg, "��Ǹ������");
		break;
	case '4':		/* delete mark */
		dele_1line = 0;
		if (mark_on) {	/*&&(currline->attr & M_MARK)) { */
			if (currline == firstline)
				dele_1line = 1;
			else
				dele_1line = 2;
			for (p = firstline; p != NULL;) {
				if (p->attr & M_MARK) {
					currline = p;
					vedit_key(Ctrl('Y'));
					p = currline;
				} else
					p = p->next;
			}
			process_ESC_action('M', '0');
			marknum = 0;
			if (dele_1line == 0 || dele_1line == 2) {
				if (first_mark_line == 0)
					first_mark_line = 1;
				goline(first_mark_line);
			} else
				goline(1);
		}
		break;
	default:
		strcpy(msg, CHOOSE_ERROR);
	}
	strcpy(msg, "\0");
}

static int
process_ESC_action(action, arg)
int action, arg;
/* valid action are I/E/S/B/F/R/C/O/= */
/* valid arg are    '0' - '7' */
{
	int newch = 0;
	int savemode;
	char msg[80], buf[80];
	char filename[80];
	FILE *fp;

	msg[0] = '\0';
	switch (action) {
	case 'L':
		if (ismsgline >= 1) {
			ismsgline = 0;
			move(t_lines - 1, 0);
			clrtoeol();
		} else
			ismsgline = 1;
		break;
	case 'M':
		process_MARK_action(arg, msg);
		break;
	case 'I':
		sprintf(filename, "home/%c/%s/clip_%c",
			mytoupper(currentuser.userid[0]), currentuser.userid,
			arg);
		if ((fp = fopen(filename, "r")) != NULL) {
			insert_from_fp(fp);
			fclose(fp);
			sprintf(msg, "��ȡ���������� %c ҳ", arg);
		} else
			sprintf(msg, "�޷�ȡ���������� %c ҳ", arg);
		break;
	case 'G':
		go();
		redraw_everything = YEA;
		break;
	case 'E':
		sprintf(filename, "home/%c/%s/clip_%c",
			mytoupper(currentuser.userid[0]), currentuser.userid,
			arg);
		if ((fp = fopen(filename, "w")) != NULL) {
			if (mark_on) {
				struct textline *p;
				for (p = firstline; p != NULL; p = p->next)
					if (p->attr & M_MARK)
						fprintf(fp, "%s\n", p->data);
			} else
				insert_to_fp(fp);
			fclose(fp);
			sprintf(msg, "�������������� %c ҳ", arg);
		} else
			sprintf(msg, "�޷������������� %c ҳ", arg);
		break;
	case 'N':
		searchline(searchtext);
		redraw_everything = YEA;
		break;
	case 'S':
		search();
		redraw_everything = YEA;
		break;
	case 'F':
		sprintf(buf, "%c[3%cm", 27, arg);
		ve_insert_str(buf);
		break;
	case 'B':
		sprintf(buf, "%c[4%cm", 27, arg);
		ve_insert_str(buf);
		break;
	case 'R':
		ve_insert_str(ANSI_RESET);
		break;
	case 'C':
		editansi = showansi = 1;
		redraw_everything = YEA;
		clear();
		display_buffer();
		redoscr();
		strcpy(msg, "����ʾ��ɫ�༭�ɹ��������лص�ɫģʽ");
		break;
	case 'O':
/*add by KCN 1998.11*/
		inputspecial = 1;
/*		while (igetspecialchar(buf))
		{
                	ve_insert_str(buf);
			display_buffer(); redoscr();
		}
         	inputspecial=0;*/
		display_buffer();
		redoscr();
		break;
	case '=':
		savemode = uinfo.mode;
		showansi = 1;
		x_dict();
		modify_user_mode(savemode);
		display_buffer();
		redoscr();
		break;
	}

	if (strchr("FBRCM", action))
		redraw_everything = YEA;

	if (msg[0] != '\0') {
		if (action == 'C') {	/* need redraw */
			move(t_lines - 2, 0);
			clrtoeol();
			prints("[1m%s%s%s[m", msg,
			       ", �밴��������ر༭����...", ANSI_RESET);
			igetkey();
			newch = '\0';
			editansi = showansi = 0;
			clear();
			display_buffer();
		} else
			newch = ask(strcat(msg, "��������༭��"));
	} else
		newch = '\0';
	return newch;
}

static void
vedit_key(ch)
int ch;
{
	int i;
#define NO_ANSI_MODIFY  if(no_touch) { warn++; break; }

	static int lastindent = -1;
	int no_touch, warn, shift;

	if (ch == Ctrl('P') || ch == KEY_UP ||
	    ch == Ctrl('N') || ch == KEY_DOWN) {
		if (lastindent == -1)
			lastindent = currpnt;
	} else
		lastindent = -1;

	no_touch = (editansi && strchr(currline->data, '\033')) ? 1 : 0;
	warn = 0;

	if (ch < 0x100 && isprint2(ch)) {
		if (no_touch)
			warn++;
		else
			insert_char(ch);
	} else
		switch (ch) {
		case Ctrl('I'):
			NO_ANSI_MODIFY;
			do {
				insert_char(' ');
			} while (currpnt & 0x7);
			break;
		case '\r':
		case '\n':
			NO_ANSI_MODIFY;
			split(currline, currpnt);
			break;
			//case Ctrl('G'): /* redraw screen */
			//    clear() ;
			//    redraw_everything = YEA ;
			//    break ;
		case Ctrl('R'):	/* toggle double char */
			enabledbchar = ~enabledbchar & 1;
			break;
		case Ctrl('Q'):	/* call help screen */
			{
				int tmpansi;
				tmpansi = showansi;
				showansi = 1;
				show_help("help/edithelp");
				showansi = tmpansi;
			}
			redraw_everything = YEA;
			break;
		case KEY_LEFT:	/* backward character */
			if (currpnt > 0) {
				currpnt--;
			} else if (currline->prev) {
				curr_window_line--;
				currln--;
				currline = currline->prev;
				currpnt = currline->len;
			}
			if (enabledbchar) {
				int i, j = 0;
				for (i = 0; i < currpnt; i++)
					if (j)
						j = 0;
					else if (currline->data[i] < 0)
						j = 1;
				if (j)
					if (currpnt > 0)
						currpnt--;
			}
			break;
		case Ctrl('C'):
			process_ESC_action('M', '3');
			break;
		case Ctrl('U'):
			if (marknum == 0) {
				marknum = 1;
				process_ESC_action('M', '1');
			} else
				process_ESC_action('M', '2');
			clear();
			break;
		case Ctrl('V'):
		case KEY_RIGHT:	/* forward character */
			if (currline->len != currpnt) {
				currpnt++;
			} else if (currline->next) {
				currpnt = 0;
				curr_window_line++;
				currln++;
				currline = currline->next;
				if (Origin(currline)) {
					curr_window_line--;
					currln--;
					currline = currline->prev;
				}
			}
			if (enabledbchar) {
				int i, j = 0;
				for (i = 0; i < currpnt; i++)
					if (j)
						j = 0;
					else if (currline->data[i] < 0)
						j = 1;
				if (j)
					if (currline->len != currpnt)
						currpnt++;
			}
			break;
		case Ctrl('P'):
		case KEY_UP:	/* Previous line */
			if (currline->prev) {
				currln--;
				curr_window_line--;
				currline = currline->prev;
				currpnt =
				    (currline->len >
				     lastindent) ? lastindent : currline->len;
			}
			if (enabledbchar) {
				int i, j = 0;
				for (i = 0; i < currpnt; i++)
					if (j)
						j = 0;
					else if (currline->data[i] < 0)
						j = 1;
				if (j)
					if (currpnt > 0)
						currpnt--;
			}
			break;
		case Ctrl('N'):
		case KEY_DOWN:	/* Next line */
			if (currline->next) {
				currline = currline->next;
				curr_window_line++;
				currln++;
				if (Origin(currline)) {
					currln--;
					curr_window_line--;
					currline = currline->prev;
				}
				currpnt =
				    (currline->len >
				     lastindent) ? lastindent : currline->len;
			}
			if (enabledbchar) {
				int i, j = 0;
				for (i = 0; i < currpnt; i++)
					if (j)
						j = 0;
					else if (currline->data[i] < 0)
						j = 1;
				if (j)
					if (currpnt > 0)
						currpnt--;
			}
			break;
		case Ctrl('B'):
		case KEY_PGUP:	/* previous page */
			top_of_win = back_line(top_of_win, 22);
			currline = back_line(currline, 22);
			currln -= moveln;
			curr_window_line = getlineno();
			if (currpnt > currline->len)
				currpnt = currline->len;
			redraw_everything = YEA;
			break;
		case Ctrl('F'):
		case KEY_PGDN:	/* next page */
			top_of_win = forward_line(top_of_win, 22);
			currline = forward_line(currline, 22);
			currln += moveln;
			curr_window_line = getlineno();
			if (currpnt > currline->len)
				currpnt = currline->len;
			if (Origin(currline->prev)) {
				currln -= 2;
				curr_window_line = 0;
				currline = currline->prev->prev;
				top_of_win = lastline->prev->prev;
			}
			if (Origin(currline)) {
				currln--;
				curr_window_line--;
				currline = currline->prev;
			}
			redraw_everything = YEA;
			break;
		case Ctrl('A'):
		case KEY_HOME:	/* begin of line */
			currpnt = 0;
			break;
		case Ctrl('E'):
		case KEY_END:	/* end of line */
			currpnt = currline->len;
			break;
		case Ctrl('S'):	/* start of file */
			top_of_win = firstline;
			currline = top_of_win;
			currpnt = 0;
			curr_window_line = 0;
			currln = 0;
			redraw_everything = YEA;
			break;
		case Ctrl('T'):	/* tail of file */
			top_of_win = back_line(lastline, 22);
			countline();
			currln = moveln;
			currline = lastline;
			curr_window_line = getlineno();
			currpnt = 0;
			if (Origin(currline->prev)) {
				currline = currline->prev->prev;
				currln -= 2;
				curr_window_line -= 2;
			}
			redraw_everything = YEA;
			break;
		case Ctrl('G'):
		case KEY_INS:	/* Toggle insert/overwrite */
			insert_character = !insert_character;
			/*move(0,73);
			   prints( " [%s] ", insert_character ? "Ins" : "Rep" ); */
			break;
		case Ctrl('H'):
		case '\177':	/* backspace */
			NO_ANSI_MODIFY;
			if (currpnt == 0) {
				struct textline *p;

				if (!currline->prev) {
					break;
				}
				currln--;
				curr_window_line--;
				currline = currline->prev;
				currpnt = currline->len;
				/* According to period */
				if (curr_window_line < 0) {
					top_of_win = currline;
					curr_window_line = 0;
				}
				/* */
				if (*killsp(currline->next->data) == '\0') {
					delete_line(currline->next);
					redraw_everything = YEA;
					break;
				}
				p = currline;
				while (!join(p)) {
					p = p->next;
					if (p == NULL) {
						indigestion(2);
						abort_bbs();
					}
				}
				redraw_everything = YEA;
				break;
			}
			currpnt--;
			delete_char();
			break;
		case Ctrl('D'):
		case KEY_DEL:	/* delete current character */
			NO_ANSI_MODIFY;
			if (currline->len == currpnt) {
				struct textline *p = currline;
				if (!Origin(currline->next)) {
					while (!join(p)) {
						p = p->next;
						if (p == NULL) {
							indigestion(2);
							abort_bbs();
						}
					}
				} else if (currpnt == 0)
					vedit_key(Ctrl('K'));
				redraw_everything = YEA;
				break;
			}
			delete_char();
			break;
		case Ctrl('Y'):	/* delete current line */
			/* STONGLY coupled with Ctrl-K */
			no_touch = 0;	/* ANSI_MODIFY hack */
			currpnt = 0;
			if (currline->next) {
				if (Origin(currline->next) && !currline->prev) {
					currline->data[0] = '\0';
					currline->len = 0;
					break;
				}
			} else if (currline->prev != NULL) {
				currline->len = 0;
			} else {
				currline->len = 0;
				currline->data[0] = '\0';
				break;
			}
			currline->len = 0;
			vedit_key(Ctrl('K'));
			break;
		case Ctrl('K'):	/* delete to end of line */
			NO_ANSI_MODIFY;
			if (currline->prev == NULL && currline->next == NULL) {
				currline->data[0] = '\0';
				currpnt = 0;
				break;
			}
			if (currline->next) {
				if (Origin(currline->next)
				    && currpnt == currline->len && currpnt != 0)
					break;
				if (Origin(currline->next)
				    && currline->prev == NULL) {
					vedit_key(Ctrl('Y'));
					break;
				}
			}
			if (currline->len == 0) {
				struct textline *p = currline->next;
				if (!p) {
					p = currline->prev;
					if (!p) {
						break;
					}
					if (curr_window_line > 0)
						curr_window_line--;
					currln--;
				}
				if (currline == top_of_win)
					top_of_win = p;
				delete_line(currline);
				currline = p;
				if (Origin(currline)) {
					currline = currline->prev;
					curr_window_line--;
					currln--;
				}
				redraw_everything = YEA;
				break;
			}
			if (currline->len == currpnt) {
				struct textline *p = currline;

				while (!join(p)) {
					p = p->next;
					if (p == NULL) {
						indigestion(2);
						abort_bbs();
					}
				}
				redraw_everything = YEA;
				break;
			}
			currline->len = currpnt;
			currline->data[currpnt] = '\0';
			break;
		default:
			break;
		}

	if (curr_window_line < 0) {
		curr_window_line = 0;
		if (!top_of_win->prev) {
			indigestion(6);
		} else {
			top_of_win = top_of_win->prev;
			rscroll();
		}
	}
	if (curr_window_line >= t_lines - 1) {
		for (i = curr_window_line - t_lines + 1; i >= 0; i--) {
			curr_window_line--;
			if (!top_of_win->next) {
				indigestion(7);
			} else {
				top_of_win = top_of_win->next;
				scroll();
			}
		}
	}
	if (editansi /*|| mark_on */ )
		redraw_everything = YEA;
	shift = (currpnt + 2 > STRLEN) ?
	    (currpnt / (STRLEN - scrollen)) * (STRLEN - scrollen) : 0;
	msgline(0);
	if (shifttmp != shift || redraw_everything == YEA) {
		redraw_everything = YEA;
		shifttmp = shift;
	} else
		redraw_everything = NA;

	move(curr_window_line, 0);
	if (currline->attr & M_MARK) {
		showansi = 1;
		cstrnput(currline->data + shift);
		showansi = 0;
	} else
		strnput(currline->data + shift);
	clrtoeol();
}

static int
vedit_getspecialchar(ch)
char ch;
{
	char buf[3];
	switch (ch) {
	case ',':
		if (inputtype == 0)
			return 1;
		inputtype--;
		lineidx = 0;
		ishowspecialchar();
		return 1;
	case '.':
		if (inputtype == 5)
			break;
		inputtype++;
		lineidx = 0;
		ishowspecialchar();
		return 1;
	case '-':
		if (lineidx == 0)
			return 1;
		lineidx--;
		ishowspecialchar();
		return 1;
	case '=':
		lineidx++;
		if (!ishowspecialchar()) {
			lineidx--;
			ishowspecialchar();
		}
		return 1;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		buf[0] = specialcharbuf[(ch - '1') * 2];
		buf[1] = specialcharbuf[(ch - '1') * 2 + 1];
		ve_insert_str(buf);
		display_buffer();

		return 1;
	}
	return 0;
}

static int
raw_vedit(filename, saveheader, modifyheader)
char *filename;
int saveheader;
int modifyheader;
{
	int newch, ch = 0, foo, shift;

	read_file(filename, saveheader, modifyheader);
	top_of_win = firstline;
	currline = firstline;
	curr_window_line = 0;
	currln = 0;
	currpnt = 0;
	clear();
	display_buffer();
	msgline(0);
	while (ch != EOF) {
		newch = '\0';
		switch (ch) {
		case Ctrl('O'):
			input_tools();
			redraw_everything = YEA;
			break;
		case Ctrl('X'):
		case Ctrl('W'):
			foo = write_file(filename, saveheader, modifyheader);
			if (foo != KEEP_EDITING)
				return foo;
			redraw_everything = YEA;
			break;
		case KEY_ESC:
			if (inputspecial) {
				inputspecial = 0;
				ismsgline = 1;
				msgline(0);
			} else {
				if (KEY_ESC_arg == KEY_ESC)
					insert_char(KEY_ESC);
				else {
					newch = vedit_process_ESC(KEY_ESC_arg);
					clear();
				}
			}
			redraw_everything = YEA;
			break;
		default:
			if (inputspecial) {
				if (!vedit_getspecialchar(ch))
					vedit_key(ch);
			} else
				vedit_key(ch);
		}
		if (redraw_everything) {
			display_buffer();
		}
		redraw_everything = NA;
		shift = (currpnt + 2 > STRLEN) ?
		    (currpnt / (STRLEN - scrollen)) * (STRLEN - scrollen) : 0;
		move(curr_window_line, currpnt - shift);

		ch = (newch != '\0') ? newch : igetkey();
	}
	return 1;
}

int
vedit(filename, saveheader, modifyheader)
char *filename;
int saveheader;
int modifyheader;
{
	int ans, t;
	/*
	struct mmapfile mf = { ptr:NULL };
	char *retv;
	if (file_size(filename) > 0) {
		MMAP_TRY {
			if (mmapfile(filename, &mf) < 0)
				MMAP_RETURN(-1);
			retv = memchr(mf.ptr, '\0', mf.size);
		}
		MMAP_CATCH {
			MMAP_RETURN(-1);
		}
		MMAP_END mmapfile(NULL, &mf);
		if (retv)
			if (askyn
			    ("���İ����������༭�󽫶�ʧ������ȷ��Ҫ�༭��",
			     NA, NA) == NA)
				return -1;
	}*/
	t = showansi;
	showansi = 0;
	init_alarm();
	ismsgline = (DEFINE(DEF_EDITMSG)) ? 1 : 0;
	if (DEFINE(DEF_POSTNOMSG)) {
		block_msg();
	}
	msg();
	ans = raw_vedit(filename, saveheader, modifyheader);
	showansi = t;
	signal(SIGALRM, SIG_IGN);
	if (DEFINE(DEF_POSTNOMSG))
		unblock_msg();
	return ans;
}
