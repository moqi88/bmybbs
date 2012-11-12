/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                         G    struct
    {
      char author[IDLEN + 1];
      char board[18];
      char title[66];
      time_t date;
      int number;
    }      postlog;

uy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

struct wwwsession {
	unsigned char used:1, show_reg:1, att_mode:1, ipmask:4, doc_mode:1;
	unsigned char link_mode:1, def_mode:1, t_lines:6;
	char iskicked;
	char unused;
	time_t login_start_time;
	time_t lastposttime;
	time_t lastinboardtime;
};

//modify by mintbaggio 040326 for front page commend
struct commend{
        char board[24];         //the board that the article be commened is in
        char userid[14];        //the author of the article be commended
        char com_user[14];      //the user who commend this article
        char title[80];         //the title of the article be commended
        char filename[80];      //the filename of the article be commended
        //int time;
        time_t time;            //the time when the com_user commend this article
        //int flag;
        unsigned int accessed;
};

struct user_info {		/* Structure used in UTMP file */
	int active;		/* When allocated this field is true */
	int uid;		/* Used to find user name in passwd file */
	int pid;		/* kill() to notify user of talk request */
	int invisible;		/* Used by cloaking function in Xyz menu */
	int sockactive;		/* Used to coordinate talk requests */
	int sockaddr;		/* ... */
	int destuid;		/* talk uses this to identify who called */
	int mode;		/* UL/DL, Talk Mode, Chat Mode, ... */
	int pager;		/* pager toggle, YEA, or NA */
	int in_chat;		/* for in_chat commands   */
	int fnum;		/* number of friends */
	short ext_idle;		/* has extended idle time, YEA or NA */
	short isssh;		/* login from ssh */
	time_t lasttime;	/* time of the last action */
	unsigned int userlevel;	//change by lepton for www
	//time_t     login_start_time; //change by lepton for www
	int nouse1;
	char chatid[10];	/* chat id, if in chat mode */
	char from[20];		/* machine name the user called in from */
	char sessionid[40];	/* add by leptin for www use */
	char userid[20];
	char realname[20];
	char username[NAMELEN];
	unsigned int unreadmsg;
	//time_t        lastposttime;
	int nouse2;
	short curboard;
	//time_t  lastinboardtime;
	int nouse3;
	int clubrights[4];	//add by ylsdd
	unsigned friend[MAXFRIENDS];
	unsigned reject[MAXREJECTS];
	struct wwwsession wwwinfo;
	struct onebrc brc;
	char user_state_temp[16];  //add by leoncom 
};

#define BM_LEN 60
struct oldboardheader {
	char filename[STRLEN];
//	int stocknum;
//	int score;
//	int clubnum;
//	int total;
//	int lastpost;
	char BM[BM_LEN - 1];
	char flag;
	char title[STRLEN];
	unsigned level;
//	short inboard;
	unsigned char unused[10];
};

struct oldfileheader {		/* This structure is used to hold data in */
	char filename[STRLEN];	/* the DIR files */
	char owner[STRLEN];
//	time_t edittime;
//	char unused[3];
//	char realauthor[IDLEN + 1];
	char title[STRLEN];
//	unsigned char star_avg:3, hasvoted:5;
//	unsigned char sizebyte;
//	short viewtime;
//add follow one line
        unsigned level;
	unsigned char accessed[12];	/* struct size = 256 bytes */
};

struct one_key {		/* Used to pass commands to the readmenu */
	int key;
	int (*fptr) ();
	char func[33];		//add by gluon for self-define menu
};

struct moneyCenter {
        int ave_score;
        int prize777;
        int prize367;
        int prizeSoccer;
        unsigned char transfer_rate;
        unsigned char deposit_rate;
        unsigned char lend_rate;
        unsigned char isSalaryTime:1,isSoccerSelling:1,isMCclosed:1,unused:5;
};

#define USHM_SIZE       (MAXACTIVE + 10)
struct UTMPFILE {
	struct user_info uinfo[USHM_SIZE];
	time_t uptime;
	unsigned short activeuser;
	unsigned short maxuser;	//add by gluon
	unsigned short maxtoday;
	unsigned short wwwguest;
	time_t activetime;	//time of updating activeuser
	int ave_score;
	int allprize;
	time_t watchman;
	unsigned int unlock;
	int nouse[5];
	struct moneyCenter mc;
};

struct BCACHE {
	struct boardmem bcache[MAXBOARD];
	int number;
	time_t uptime;
	time_t pollvote;
};

struct UCACHE {
	char userid[MAXUSERS][IDLEN + 1];
	int number;
	int usersum;
	time_t uptime;
	int nouse[10];
};

#define UCACHE_HASH_SIZE (MAXUSERS*2)
struct useridhashitem {
	int num;
	char userid[IDLEN + 1];
};

struct UCACHEHASH {
	struct useridhashitem uhi[UCACHE_HASH_SIZE];
	time_t uptime;
};

struct UINDEX {
	int user[MAXUSERS][6];	//�����www�ж϶��¼�Ļ����Ƿ�ʹ���޳���telnet�е�5, ���6
};

struct postheader {
	char title[STRLEN];
	char ds[40];
	int reply_mode;
	char include_mode;
	int chk_anony;
	int postboard;
	int canreply;
	int mailreply;
};

struct postlog {
	char author[IDLEN + 1];
	char board[18];
	char title[66];
	time_t date;
	int number;
};

struct fivechess {
	int winner;
	int hand, tdeadf, tlivef, livethree, threefour;
	int playboard[15][15];
};

