#include "bbslib.h"

struct user_info *
query_f(int uid)
{
	int i, uent, testreject = 0;
	struct user_info *uentp;
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &shm_utmp->uinfo[uent - 1];
		if (!uentp->active || !uentp->pid || uentp->uid != uid)
			continue;
		if (!testreject) {
			if (isbad(uentp->userid))
				return 0;
			testreject = 1;
		}
		if (shm_utmp->uinfo[uent - 1].invisible
		    && !HAS_PERM(PERM_SYSOP | PERM_SEECLOAK))
			continue;
		return uentp;
	}
	return 0;
}


int
bbsfoot_main()
{

	int dt = 0, mail_total = 0, mail_unread = 0, lasttime = 0;
	int count_friends=0, i=0;
	char *id = "guest";
	static int r = 0;
	html_header(2);
	
	printf("<script>function t(){return (new Date()).valueOf();}</script>\n");
	printf("<body bgcolor=#efefef leftmargin=0 topmargin=0>\n");
	printf("<table width=100%% height=18 border=0 cellpadding=0 cellspacing=0><tr> <td>&nbsp; </td><td><table border=0 cellpadding=0 cellspacing=0><tr><td width=1></td><td width=10 align=right valign=top><img src=\"/images/bmy_arrow_black.gif\" width=6 height=5></td>");
    if (loginok) {
		id = currentuser.userid;
		dt = abs(now_t - w_info->login_start_time) / 60;
	}
	printf("<td>");
        printf("时间[<span class=0011>%16.16s</span>]", Ctime(now_t));
        printf("</td>\n");
        printf("<td width=1></td>");
        printf("<td valign=top>");
	if (loginok && !isguest){
		for (i = 0; i < u_info->fnum; i++)
			count_friends += query_f(u_info->friend[i]) ? 1 : 0;
		printf("在线/好友[<a href=bbsufind?search=A&limit=20 target=f3 class=1011>%d</a> ",
		       count_online());
		printf("/<a href=bbsfriend target=f3 class=1011>%d</a>] ",
			count_friends);
	}else
		printf("在线[<a href=bbsufind?search=A&limit=20 target=f3 class=1011>%d</a>] ",
	       	count_online());
	//add by macintosh 050619
	printf("</td>\n");
	printf("<td width=1></td>");
	printf("<td valign=top>");
	printf("帐号[<a href=bbsqry?userid=%s target=f3 class=1011>%s</a>]", id, id);
	printf("</td>\n");
	printf("<td width=1></td>");
	printf("<td valign=top>");
	if (loginok && !isguest) {
		int thistime;
		lasttime = atoi(getparm("lt"));
		thistime = mails_time(id);
		if (thistime <= lasttime) {
			mail_total = atoi(getparm("mt"));
			mail_unread = atoi(getparm("mu"));
		} else {
			mail_total = mails(id, &mail_unread);
			lasttime = thistime;
		}
		if (mail_unread == 0) {
			printf("信箱[<a href=bbsmail target=f3 class=1011>%d封</a>] ",
			       mail_total);
		} else {
			printf
			    ("信箱[<a href=bbsmail target=f3 class=1011>%d(<font color=red>新信%d</font>)</a>] ",
			     mail_total, mail_unread);
		}
	}
	printf("</td>\n");
	printf("<td width=1></td>");
	printf("<td>");
	printf("停留[<font style=\"font-size:10px\" color=#ff6600>%d</font>小时<font style=\"font-size:10px\" color=#ff6600>%d</font>分钟]", dt / 60, dt % 60);
	printf("</td></tr></table></td></tr></table>\n");
	printf("<script>setTimeout('self.location.replace("
	       "\"bbsfoot?lt=%d&mt=%d&mu=%d&sn='+t()+'\")', %d);</script>",
	       lasttime, mail_total, mail_unread, 300000 + r * 1000);
	r = (r + dt + now_t) % 30;
	printf("</body>");
	return 0;
}

int
mails_time(char *id)
{
	char path[80];
	if (!loginok || isguest)
		return 0;
	sprintf(path, "mail/%c/%s/.DIR", mytoupper(id[0]), id);
	return file_time(path);
}

int
mails(char *id, int *unread)
{
	struct fileheader *x;
	char path[80];
	int total = 0, i;
	struct mmapfile mf = { ptr:NULL };
	*unread = 0;
	if (!loginok || isguest)
		return 0;
	setmailfile(path, id, ".DIR");
	MMAP_TRY {
		if (mmapfile(path, &mf) < 0) {
			MMAP_UNTRY;
			MMAP_RETURN(0);
		}
		total = mf.size / sizeof (struct fileheader);
		x = (struct fileheader*)mf.ptr;
		for (i = 0; i < total; i++) {
			if (!(x->accessed & FH_READ))
				(*unread)++;
			x++;
		}
	}
	MMAP_CATCH {
		total = 0;
		*unread = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	return total;

}
