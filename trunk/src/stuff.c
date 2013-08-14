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
    Copyright (C) 1999	KCN,Zhou lin,kcn@cic.tsinghua.edu.cn
    
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

extern char fromhost[];
extern int convcode;

#ifdef CAN_EXEC
char tempfile[MAXPATHLEN];
#endif

extern void output(char *s, int len);

int
dashf(fname)
const char *fname;
{
	struct stat st;

	return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

int
dashd(fname)
char *fname;
{
	struct stat st;

	return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

int
dashl(fname)
char *fname;
{
	struct stat st;

	return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}

int
pressanykey()
{
	extern int showansi;

	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	prints
	    ("[m                                [5;1;33m���κμ�����...[m");
	egetch();
	move(t_lines - 1, 0);
	clrtoeol();
	return 0;
}

int
pressreturn()
{
	extern int showansi;
	char buf[3];

	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0,
		"                              [1;33m�밴 ��[5;36mEnter[m[1;33m�� ����\033[m",
		buf, 2, NOECHO, YEA);
	move(t_lines - 1, 0);
	clrtoeol();
	return 0;
}

int
askyn(str, defa, gobottom)
char str[STRLEN];
int defa, gobottom;
{
	int x, y;
	char realstr[280];
	char ans[3];

	snprintf(realstr, sizeof (realstr), "%s (Y/N)? [%c]: ", str,
		 (defa) ? 'Y' : 'N');
	if (gobottom)
		move(t_lines - 1, 0);
	getyx(&x, &y);
	clrtoeol();
	getdata(x, y, realstr, ans, 2, DOECHO, YEA);
	if (ans[0] != 'Y' && ans[0] != 'y' && ans[0] != 'N' && ans[0] != 'n') {
		return defa;
	} else if (ans[0] == 'Y' || ans[0] == 'y')
		return 1;
	else
		return 0;
}

void
printdash(mesg)
char *mesg;
{
	char buf[80], *ptr;
	int len;

	memset(buf, '=', 79);
	buf[79] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > 76)
			len = 76;
		ptr = &buf[40 - len / 2];
		ptr[-1] = ' ';
		ptr[len] = ' ';
		strncpy(ptr, mesg, len);
	}
	prints("%s\n", buf);
}

void
bell()
{
	char sound;

	sound = Ctrl('G');
	output(&sound, 1);
}

void
touchnew()
{
	int fd;

	sprintf(genbuf, "touch by: %s\n", currentuser.userid);
	if ((fd = open(FLUSH, O_WRONLY | O_CREAT, 0644)) == -1)
		return;
	write(fd, genbuf, strlen(genbuf));
	close(fd);
}

/* rrr - Snagged from pbbs 1.8 */

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"

char *bbsenv[MAXENVS];
int numbbsenvs = 0;

void
strtolower(dst, src)
char *dst, *src;
{
	for (; *src; src++)
		*dst++ = tolower(*src);
	*dst = '\0';
}

int
deltree(dst)
char *dst;
{
	char rpath[PATH_MAX + 1 + 10], buf[PATH_MAX + 1];
	int i = 0, j = 0, isdir = 0, fd;
	static char *const (disks[]) = {
		MY_BBS_HOME "/0Announce/",
		MY_BBS_HOME "/boards/",
		MY_BBS_HOME "/mail/", MY_BBS_HOME "/", NULL
	};

	if (dashl(dst)) {
		unlink(dst);
		return 1;
	}
	if (realpath(dst, rpath) == NULL)
		return 0;
	j = strlen(rpath);
	if (rpath[j - 1] == '/')
		rpath[j - 1] = 0;
	if (strncmp(rpath, MY_BBS_HOME "/", sizeof (MY_BBS_HOME)))
		return 0;
	strcpy(buf, rpath);
	if (dashd(dst))
		isdir = 1;
	for (i = 0; disks[i]; i++) {
		j = strlen(disks[i]);
		if (!strncmp(rpath, disks[i], j))
			break;
	}
	memmove(rpath + j + 6, rpath + j, sizeof (rpath) - j - 6);
	memcpy(rpath + j, ".junk/", 6);
	j += 6;
	normalize(rpath + j);
	rpath[PATH_MAX - 10] = 0;
	j = strlen(rpath);
	i = 0;
	if (isdir) {
		while (mkdir(rpath, 0770) != 0 && i < 1000) {
			sprintf(rpath + j, "+%d", i);
			i++;
		}
		if (i == 1000)
			return 0;
	} else {
		while (((fd = open(rpath, O_CREAT | O_EXCL | O_WRONLY, 0660)) <
			0) && i < 1000) {
			sprintf(rpath + j, "+%d", i);
			i++;
		}
		if (i == 1000)
			return 0;
		close(fd);
	}
	rename(buf, rpath);
	return 1;
}

#if 0
static int
bbssetenv(env, val)
char *env, *val;
{
	int i, len;

	if (numbbsenvs == 0)
		bbsenv[0] = NULL;
	len = strlen(env);
	for (i = 0; bbsenv[i]; i++)
		if (!strncasecmp(env, bbsenv[i], len))
			break;
	if (i >= MAXENVS)
		return -1;
	if (bbsenv[i])
		free(bbsenv[i]);
	else
		bbsenv[++numbbsenvs] = NULL;
	bbsenv[i] = malloc(strlen(env) + strlen(val) + 2);
	strcpy(bbsenv[i], env);
	strcat(bbsenv[i], "=");
	strcat(bbsenv[i], val);
	return 0;
}
#endif

int
do_exec(com, wd)
char *com, *wd;
{
#if defined(CAN_EXEC) && ! defined(SSHBBS)
	char exec_param[256];
	char path[MAXPATHLEN];
	char pcom[MAXCOMSZ];
	char *arglist[MAXARGS];
	char *tz;
	register int i, len;
	register int argptr;
	int status, pid, w;
	int pmode;
	void (*isig) (), (*qsig) ();

	strncpy(path, BINDIR, MAXPATHLEN);
	strncpy(pcom, com, MAXCOMSZ);
	len = min(strlen(com) + 1, MAXCOMSZ);
	pmode = LOOKFIRST;
	for (i = 0, argptr = 0; i < len; i++) {
		if (pcom[i] == '\0')
			break;
		if (pmode == QUOTEMODE) {
			if (pcom[i] == '\001') {
				pmode = LOOKFIRST;
				pcom[i] = '\0';
				continue;
			}
			continue;
		}
		if (pcom[i] == '\001') {
			pmode = QUOTEMODE;
			arglist[argptr++] = &pcom[i + 1];
			if (argptr + 1 == MAXARGS)
				break;
			continue;
		}
		if (pmode == LOOKFIRST)
			if (pcom[i] != ' ') {
				arglist[argptr++] = &pcom[i];
				if (argptr + 1 == MAXARGS)
					break;
				pmode = LOOKLAST;
			} else
				continue;
		if (pcom[i] == ' ') {
			pmode = LOOKFIRST;
			pcom[i] = '\0';
		}
	}
	arglist[argptr] = NULL;
	if (argptr == 0)
		return -1;
	if (*arglist[0] == '/')
		strncpy(path, arglist[0], MAXPATHLEN);
	else
		strncat(path, arglist[0], MAXPATHLEN);
	alarm(0);
#ifdef IRIX
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif
		if (wd)
			if (chdir(wd)) {
				prints("Unable to chdir to '%s'\n", wd);
				refresh();
				exit(-1);
			}
		bbssetenv("PATH", "/bin:.");
		bbssetenv("TERM", "vt100");
		bbssetenv("USER", currentuser.userid);
		bbssetenv("USERNAME", currentuser.username);
		bbssetenv("REPLYTO", currentuser.email);
		bbssetenv("FROMHOST", fromhost);
		if ((tz = getenv("TZ")) != NULL)
			bbssetenv("TZ", tz);
		if (numbbsenvs == 0)
			bbsenv[0] = NULL;
/*
        execve(path,arglist,bbsenv) ;
*/
		strcpy(exec_param, path);
		tz = exec_param + strlen(exec_param) + 1;
		for (i = 1; i < argptr; i++) {
			strcpy(tz, arglist[i]);
			tz = tz + strlen(tz) + 1;
		}
		*tz = 0;
		max_timeout = 0;
		term_convert = convcode;
		sprintf(tempfile, "tmp/telnet.%d", getpid());

		if (nsave(tempfile) == 0) {
			prints("save comm fail");
			refresh();
			exit(-1);
		};

		execle("bin/ptyexec", "ptyexec", tempfile, "0", NULL, bbsenv);
		sprintf(genbuf, "EXECV FAILED... path = '%s %s'\n", exec_param,
			tempfile);
		write(0, genbuf, strlen(genbuf));
		sleep(4);
		exit(-1);
	}
	if (pid < 0)
		return -1;
	isig = signal(SIGINT, SIG_IGN);
	qsig = signal(SIGQUIT, SIG_IGN);
	while ((w = waitpid(pid, &status, WUNTRACED)) != pid && w != -1 && w)
		/* NULL STATEMENT */ ;
	signal(SIGINT, isig);
	signal(SIGQUIT, qsig);
	unlink(tempfile);
#ifdef DOTIMEOUT
	alarm(IDLE_TIMEOUT);
#endif
	return ((w == -1) ? w : status);
#endif				/* CAN_EXEC */
	prints("����ִ���ⲿ�������SSH�û�?\n\r");
	pressanykey();
	return -1;
}

char   *
horoscope(month, day)
char    month, day;
{
        char   *name[12] = {
                "Ħ��", "ˮƿ", "˫��", "ĵ��", "��ţ", "˫��",
                "��з", "ʨ��", "��Ů", "���", "��Ы", "����"
        };
        switch (month) {
        case 1:
                if (day < 21)
                        return (name[0]);
                else
                        return (name[1]);
        case 2:
                if (day < 19)
                        return (name[1]);
                else
                        return (name[2]);
        case 3:
                if (day < 21)
                        return (name[2]);
                else
                        return (name[3]);
        case 4:
                if (day < 21)
                        return (name[3]);
                else
                        return (name[4]);
        case 5:
                if (day < 21)
                        return (name[4]);
                else
                        return (name[5]);
        case 6:
                if (day < 22)
                        return (name[5]);
                else
                        return (name[6]);
        case 7:
                if (day < 23)
                        return (name[6]);
                else
                        return (name[7]);
        case 8:
                if (day < 23)
                        return (name[7]);
                else
                        return (name[8]);
        case 9:
                if (day < 23)
                        return (name[8]);
                else
                        return (name[9]);
        case 10:
                if (day < 24)
                        return (name[9]);
                else
                        return (name[10]);
        case 11:
                if (day < 23)
                        return (name[10]);
                else
                        return (name[11]);
        case 12:
                if (day < 22)
                        return (name[11]);
                else
                        return (name[0]);
        }
        return ("����");
}
