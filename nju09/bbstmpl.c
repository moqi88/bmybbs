#include "bbslib.h"
#include "tmpl.h"

extern struct boardmem *bcache;
extern struct BCACHE *brdshm;
extern int numboards;

struct a_template * ptemplate = NULL ;
int template_num = 0;
int t_now = 0;

static int 
tmpl_init(char * nboard, struct a_template ** pptemp){
	int fd;
	char tmpldir[STRLEN];
	struct s_template tmpl;
	struct s_content * cont;
	int templ_num;

	setbfile(tmpldir, nboard, TEMPLATE_DIR);
	if( pptemp == NULL ){
		return -1;
	}

	if( * pptemp == NULL ){
		* pptemp = (struct a_template *) malloc( sizeof( struct a_template ) * MAX_TEMPLATE );
		if( * pptemp == NULL )
			return -1;
	}
	bzero( * pptemp, sizeof( struct a_template ) * MAX_TEMPLATE );
	templ_num = 0;

	if( (fd = open( tmpldir, O_RDONLY ) ) == -1 ){
		return 0;
	}
	while( read(fd, &tmpl, sizeof( struct s_template )) == sizeof(struct s_template) ){
		if( tmpl.version > TMPL_NOW_VERSION ){
			close(fd);
			return -1;
		}
		if((!has_BM_perm(&currentuser, getboard(nboard))) && tmpl.flag & TMPL_BM_FLAG ) {
			lseek( fd, sizeof(struct s_content) * tmpl.content_num , SEEK_CUR );
			continue;
		}
		cont = (struct s_content *) malloc( sizeof( struct s_content ) * tmpl.content_num );
		if( cont == NULL )
			break;
		bzero(cont, sizeof(struct s_content) * tmpl.content_num );
		if(read(fd, cont, sizeof(struct s_content)*tmpl.content_num) != sizeof(struct s_content)*tmpl.content_num)
		 	continue;
		(* pptemp)[templ_num].tmpl = (struct s_template *)malloc(sizeof(struct s_template));
		if( (* pptemp)[templ_num].tmpl == NULL ){
			free(cont);
			break;
		}
		bzero( (* pptemp)[templ_num].tmpl , sizeof(struct s_template) );
		memcpy( (* pptemp)[templ_num].tmpl, &tmpl, sizeof(struct s_template) );
		(* pptemp)[templ_num].cont = cont;
		templ_num ++;
		if( templ_num >= MAX_TEMPLATE )
			break;
	}
	close(fd);
	return templ_num;
}


static void 
tmpl_free(){
	int i;
	for (i=0; i<template_num; i++){
		if (ptemplate[i].tmpl)
			free(ptemplate[i].tmpl);
		if (ptemplate[i].cont)
			free(ptemplate[i].cont);
	}
	free(ptemplate);
	ptemplate= NULL;
	template_num = 0;
	return;
}


static int 
tmpl_post(int ent, char *nboard){
	int i;
	printf("<hr>\n");
	printf("<form action=bbstmpl?action=send&board=%s&num=%d method=post>\n", nboard, ent);
	for(i=0; i< ptemplate[ent].tmpl->content_num; i++){
		printf("����: %s      (����󳤶�:    %d)<br>\n",
			ptemplate[ent].cont[i].text, ptemplate[ent].cont[i].length);
		if (ptemplate[ent].cont[i].length < 80)
			printf("<input type=text name=text%d size=%d maxlength=%d><br><br><br>\n",
			i, ptemplate[ent].cont[i].length, ptemplate[ent].cont[i].length);
		else
			printf("<textarea name=text%d rows=5 cols=80 wrap=virtual></textarea><br><br><br>\n", i);
		}
	printf("<input type=submit name=submit value=\"ȷ��\"></form>\n");
	return 0;
}

static int 
tmpl_show(char *nboard){
	printf("<center><table border=1><tr>");
	printf("<td>���</td>");
	printf("<td>������</td>");
	printf("<td>ģ������</td>");
	printf("<td>������</td>");
	printf("<td>����</td>");
	printf("</tr>");
	int i;
	for (i = 0; i < template_num; i++) {
		printf("<tr>");
		printf("<td>%2d</td>", i+1);
		printf("<td><a href=bbsqry?userid=%s>%-13s</a></td>",
		       ptemplate[i].tmpl->authorid, ptemplate[i].tmpl->authorid);
		printf("<td>%-50s</td>", ptemplate[i].tmpl->title);
		printf("<td>%3d</td>", ptemplate[i].tmpl->content_num);
		printf("<td><a href=bbstmpl?board=%s&action=post&num=%d>%s<a></td>",
		     nboard, i, "�ô�ģ�巢��");
		printf("</tr>");
	}
	printf("</table></center>");
	printf("<p><a href=javascript:history.go(-1)>������һҳ</a>");

	tmpl_free();
	return 1;
}


int
bbstmpl_main()
{
	char buf1[STRLEN];
	int i;
	struct boardmem *x;
	char board[80], action[80];
	int tmplnum;
	FILE *fp;
	FILE *fpsrc;
	char filepath[STRLEN], fname[STRLEN];

	int write_ok = 0;
	char *tmp[MAX_CONTENT];
	char newtitle[STRLEN], title[STRLEN];
	
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(action, getparm("action"), 32);
	tmplnum = atoi(getparm("num"));
	if (getboard(board) == NULL)
		http_fatal("�����������");

	printf("<body><center><a href=%s%s><h2>%s������</h2></a></center>", showByDefMode(),
	     board, board);
	if (!loginok || isguest) {
		printf("<script src=/function.js></script>\n");
		printf("�Ҵҹ��Ͳ��ܷ���,���ȵ�¼!<br><br>");
		printf("<script>openlog();</script>");
		http_quit();
	}
	changemode(POSTING);
	if (!HAS_PERM(PERM_POST))
		http_fatal("�Բ�������Ȩ����");
	x = getboard(board);
	if (!x || !has_vote_perm(&currentuser, x))
		http_fatal("�Բ�������Ȩ����");

	template_num = tmpl_init(board, & ptemplate);
	if( template_num < 0 )
		http_fatal("��ʼ��ʧ��");
	if ( template_num == 0 ){
		http_fatal("����û��ģ��ɹ�ʹ��");
		tmpl_free();
	}
	if(tmplnum < 0 || tmplnum >= template_num)
		http_fatal("����Ĳ���");
	if( ptemplate[tmplnum].tmpl->content_num <= 0 )
		http_fatal("ģ�廹û����������");
	
	action[4]=0;
	if (strstr(action, "show"))
		tmpl_show(board);
	else if (strstr(action, "post"))
		tmpl_post(tmplnum, board);
	else if (strstr(action, "send"))
	{
	for(i=0; i< ptemplate[tmplnum].tmpl->content_num; i++){
		sprintf(buf1, "text%d", i);
		tmp[i]=getparm(buf1);
	}
	sprintf(fname, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d", currentuser.userid, getpid());
	if((fp = fopen(fname, "w"))==NULL)
		http_fatal("�ļ�����");

	if( ptemplate[tmplnum].tmpl->filename[0] ){
    	struct stat st;
		setbfile(filepath, board, ptemplate[tmplnum].tmpl->filename);
		if( stat(filepath, &st) == 0 && S_ISREG(st.st_mode) && st.st_size>2){
			if((fpsrc = fopen(filepath,"r"))!=NULL){
				char buf[256];
				while(fgets(buf,255,fpsrc)){
					int l;
					int linex = 0;
					char *pn,*pe;
					for(pn = buf; *pn!='\0'; pn++){
						if( *pn != '[' || *(pn+1)!='$' ){
							fputc(*pn, fp);
							linex++;
						}else{
							pe = strchr(pn,']');
							if(pe == NULL){
								fputc(*pn, fp);
								continue;
							}
							l = atoi(pn+2);
							if (l<=0){
								if (strncmp(pn, "[$userid]", 9) == 0){
									fprintf(fp,"%s", currentuser.userid);
									pn = pe;
									continue;
								} else {
									fputc('[', fp);
									continue;
								}
							} else if( l > ptemplate[tmplnum].tmpl->content_num ){
								fputc('[', fp);
								continue;
							}
							fprintf(fp,"%s",tmp[l-1]);
							pn = pe;
							continue;
						}
					}
				}
				fclose(fpsrc);
				write_ok = 1;
			}
		}
	}
	if(write_ok == 0){
		for(i=0; i< ptemplate[tmplnum].tmpl->content_num; i++)
			fprintf(fp,"\033[1;32m%s:\033[m\n%s\n\n", ptemplate[tmplnum].cont[i].text, tmp[i]);
	}
	fclose(fp);

	if( ptemplate[tmplnum].tmpl->title_tmpl[0] ){
		char *pn,*pe;
		char *buf;
		int l;
		int newl = 0;

		newtitle[0]='\0';
		title[0]='\0';
		buf = ptemplate[tmplnum].tmpl->title_tmpl;

		for(pn = buf; *pn!='\0' && newl < STRLEN-1; pn++){
			if( *pn != '[' || *(pn+1)!='$' ){
				if( newl < STRLEN - 1 ){
					newtitle[newl] = *pn ;
					newtitle[newl+1]='\0';
					newl ++;
				}
			}else{
				pe = strchr(pn,']');
				if(pe == NULL){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				l = atoi(pn+2);
				if( l<0 || l > ptemplate[tmplnum].tmpl->content_num ){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				if ((l == 0 ) && (strncmp(pn, "[$userid]", 9) == 0)){
					int ti;
					for( ti=0; currentuser.userid[ti]!='\0' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = currentuser.userid[ti] ;
						newtitle[newl+1]='\0';
					}
				} else				
				if( l == 0 ){
					int ti;
					for( ti=0; title[ti]!='\0' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = title[ti] ;
						newtitle[newl+1]='\0';
					}
				}
				else{
					int ti;
					for( ti=0; tmp[l-1][ti]!='\0' && tmp[l-1][ti]!='\n' && tmp[l-1][ti]!='\r' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = tmp[l-1][ti] ;
						newtitle[newl+1]='\0';
					}
				}
				pn = pe;
				continue;
			}
		}
		strncpy(title, newtitle, STRLEN);
		title[STRLEN-1]='\0';
	}

	for(i=0; i< ptemplate[tmplnum].tmpl->content_num; i++)
		free( tmp[i] );

	printf("<form name=form1 method=post action=bbssnd?board=%s&th=-1>\n", board);
	if( ptemplate[tmplnum].tmpl->title_tmpl[0] == 0 )
		sprintf(title,"%s","Ĭ��ģ�����");
	printf("<center>����: %s </center><hr>\n", title);
	printf("<input type=hidden name=title size=40 maxlength=100 value='%s'>\n", title);

	showcon(fname);

	printf("<textarea style=\"display:none\" name=text>\n");
	showfile(fname);
	printf("</textarea>\n");
	printselsignature();
	printuploadattach();
	if (innd_board(board))
		printf("<table><tr><td>ת��<input type=checkbox name=outgoing %s>\n",
                       "checked");
        if (anony_board(board))
                printf("����<input type=checkbox name=anony>\n");
	printf("</td></tr>\n");
	printf
	    ("<tr><td><a href=home/boards/BBSHelp/html/itex/itexintro.html target=_blank>ʹ��Tex������ѧ��ʽ</a><input type=checkbox name=usemath>\n");
	printf
	    ("��Ϊ���ɻظ�<input type=checkbox name=nore></td></tr></table>\n");

	printf("<hr><input type=submit value=���� "
		"onclick=\"this.value='�����ύ�У����Ժ�...';"
		"this.disabled=true;form1.submit();\">\n</form>\n");

	unlink(fname);
	}
	http_quit();
	return 0;
}
