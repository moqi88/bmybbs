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
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#define VOTE_YN         (1)
#define VOTE_SINGLE     (2)
#define VOTE_MULTI      (3)
#define VOTE_VALUE      (4)
#define VOTE_ASKING     (5)
#define VOTE_SMULTI		(6)

#define VOTE_FLAG_OPENED 0x1
#define VOTE_FLAG_LIMITED 0x2

#define MAX_VOTERLIST_NUM 5

struct ballot
{
        char    uid[IDLEN];                   /* ͶƱ��       */
        unsigned int voted;                  /* ͶƱ������   */
        char    msg[3][STRLEN];         /* ��������     */
};

struct votelog
{
	char uid[IDLEN+1];
	char ip[46];
	time_t votetime;
	unsigned int voted;
};

struct votebal
{
        char            userid[IDLEN+1];
	char            title[STRLEN];
        char            type;
        char            items[32][38];
        int             maxdays;
        int             maxtkt;
        short int	totalitems;
	short int	flag;
        time_t          opendate;
	char		listfname[STRLEN];
}
;

struct voterlist
{
	char		listname[80];
	int		voternum;
	char		listfname[80];
	char		authorid[80];
}
;
