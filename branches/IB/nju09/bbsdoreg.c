#include "bbslib.h"
#include "identify.h"

#ifdef POP_CHECK
// ��½�ʼ��������õ�ͷ�ļ� added by interma@BMY 2005.5.12
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
// �ʼ����������û���������ĳ��ȣ� added by interma@BMY 2005.5.12
#define USER_LEN 20
#define PASS_LEN 20

#endif

#ifdef POP_CHECK
// ��½�ʼ������������������֤�� added by interma@BMY 2005.5.12
// ����ֵΪ1��ʾ��Ч��0��ʾ��Ч, -1��ʾ��pop���������ӳ��� 
int test_mail_valid(char *user, char *pass, char *popip)
{
    char buffer[512]; 
    int sockfd;
    struct sockaddr_in server_addr; 
    struct hostent *host; 

	if (user[0] == ' ' || pass[0] == ' ')
		return 0;

    // �ͻ�����ʼ���� sockfd������
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) 
    {  
        return -1;
    } 
    if (strcmp(user, "test")==0) {
	return -2;
    }
    int i;
    for ( i = 0; i < 8; i++)
    server_addr.sin_zero[i] = 0;
    server_addr.sin_family=AF_INET; 
    server_addr.sin_port=htons(110);
    // 202.117.1.22 == stu.xjtu.edu.cn 
    if(inet_aton(popip, &server_addr.sin_addr) == 0) 
    {  
        return -1;
    }

    // �ͻ���������������
    if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
    {  
        return -1; 
    } 

    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
        return -1;
    
    sprintf(buffer, "USER %s\r\n\0", user);
    if (write(sockfd, buffer, strlen(buffer)) == -1)
    { 
        return -1; 
    }   
     
    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
    {   
        return 0;
    }   
     
    sprintf(buffer, "PASS %s\r\n\0", pass);
    if (write(sockfd, buffer, strlen(buffer)) == -1)
    { 
        return -1; 
    }
     
    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
    {
        return 0;
    }
            
    write(sockfd, "QUIT\r\n", strlen("QUIT\r\n"));          
    return 1;
}    

//void securityreport(char *str, char *content);

/*
char *
sethomefile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}
*/

int
substitute_record(filename, rptr, size, id)
char *filename;
void *rptr;
int size, id;
{
#ifdef LINUX
	struct flock ldata;
#endif
	int retv = 0;
	int fd;
	//add by hace
	struct stat st;
	if(stat(filename,&st)==-1)
	    return -1;
	else{
	    if(st.st_size/size <id)
		return -1;
	}
	//end 
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0660)) == -1)
		return -1;
#ifdef LINUX
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (id - 1);
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		errlog("reclock error %d", errno);
		return -1;
	}
#else
	flock(fd, LOCK_EX);
#endif
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) {
		errlog("subrec seek err %d", errno);
		retv = -1;
		goto FAIL;
	}
	if (safewrite(fd, rptr, size) != size) {
		errlog("subrec write err %d", errno);
		retv = -1;
		goto FAIL;
	}
      FAIL:
#ifdef LINUX
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &ldata);
#else
	flock(fd, LOCK_UN);
#endif
	close(fd);
	return retv;
}

// ��username�û�ͨ����֤�� added by interma@BMY 2005.5.12
void register_success(int usernum, char *userid, char *realname, char *dept, 
char *addr, char *phone, char *assoc, char *email)
{
	struct userec uinfo;
	FILE *fout, *fn;
	char buf[STRLEN];
	int n;
	char genbuf[512];

	//int id = getuser(userid);
	struct userec *u = getuser(userid);

	sethomefile(genbuf, userid, "mailcheck");
	//http_fatal(genbuf);
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}

		
	
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);

	memcpy(&uinfo, u, sizeof (uinfo));

			strsncpy(uinfo.userid, userid,
				 sizeof (uinfo.userid));
			strsncpy(uinfo.realname, realname,
				 sizeof (uinfo.realname));
			strsncpy(uinfo.address, addr,
				 sizeof (uinfo.address));
			sprintf(genbuf, "%s$%s@%s", dept, phone, userid);
			strsncpy(uinfo.realmail, genbuf,
				 sizeof (uinfo.realmail));

			strsncpy(uinfo.email, email, sizeof (uinfo.email));

			uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
			substitute_record(PASSFILE, &uinfo, sizeof (struct userec), usernum);

			sethomefile(buf, uinfo.userid, "sucessreg");
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "\n");
				fclose(fout);
			}

			sethomefile(buf, uinfo.userid, "register");
	
			if ((fout = fopen(buf, "w")) != NULL) 
			{
				
				fprintf(fout, "%s: %d\n", "usernum", usernum);
				fprintf(fout, "%s: %s\n", "userid", userid);
				fprintf(fout, "%s: %s\n", "realname", realname);
				fprintf(fout, "%s: %s\n", "dept", dept);
				fprintf(fout, "%s: %s\n", "addr", addr);
				fprintf(fout, "%s: %s\n", "phone", phone);
				fprintf(fout, "%s: %s\n", "assoc", assoc);

				n = time(NULL);
				fprintf(fout, "Date: %s",
					ctime((time_t *) & n));
				fprintf(fout, "Approved: %s\n", userid);
				fclose(fout);
			}

			mail_file("etc/s_fill", uinfo.userid,
				  "������ͨ�������֤", "SYSOP"); 

			mail_file("etc/s_fill2", uinfo.userid,
				  "��ӭ����" MY_BBS_NAME "���ͥ", "STSOP");
			sethomefile(buf, uinfo.userid, "mailcheck");
			unlink(buf);
			sprintf(genbuf, "�� %s ͨ�����ȷ��.", uinfo.userid);
			securityreport(genbuf, genbuf);
	return ;
}


char * str_to_upper(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0)
	{
		*str = toupper(*str);
		str++;
	}
	return h;
}

extern char fromhost[256];
// ��¼pop�������ϵ��û�������ֹ�ظ�ע����id�� added by interma@BMY 2005.5.16
// ����ֵΪ0��ʾ�Ѽ�¼��1��ʾ�Ѵ��� 
// ��¼pop�������ϵ��û�������ֹ�ظ�ע����id�� added by interma@BMY 2005.5.16
/* ����ֵΪ0��ʾ�Ѽ�¼��δ���ڣ���1��ʾ�Ѵ��� */
int write_pop_user(char *user, char *userid, char *pop_name)
{	
	FILE *fp;
	char buf[256];
	char path[256];
	int isprivilege = 0; 

	char username[USER_LEN + 2];
	sprintf(username, "%s\n", user);

	// ���Ƚ�����Ȩ�û���privilege������
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s_privilege" , pop_name);

	fp = fopen(path, "r");
	if (fp != NULL)
	{
		while(fgets(buf, 256, fp) != NULL)
		{
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0)
			{
				isprivilege = 1;
				break;
			}
		}
			
		fclose(fp);
	}


	// ���½�����ͨ�û�����
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s", pop_name);

	int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX); // ��������֤�������
	
	fp = fopen(path, "a+");

	if (fp == NULL) 
	{	
		close(lockfd);
		return 0;
	}

	if (isprivilege == 0)
	{
		fseek(fp, 0, SEEK_SET);
		while(fgets(buf, 256, fp) != NULL)
		{
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0)
			{
				fclose(fp);
				close(lockfd);
				return 1;
			}
			fgets(buf, 256, fp);
		}
	}

	fseek(fp, 0, SEEK_END);
	fputs(user, fp);

	time_t t;
	time(&t);

	sprintf(buf, "\n%s : %s : %s", userid, fromhost, ctime(&t));
	fputs(buf, fp);

	fclose(fp);
	close(lockfd);
	return 0;
}
#endif
// -------------------------------------------------------------------------------


#if 0
int
badymd(int y, int m, int d)
{
	int max[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
		max[2] = 29;
	if (y < 10 || y > 100 || m < 1 || m > 12)
		return 1;
	if (d < 0 || d > max[m])
		return 1;
	return 0;
}
#endif

int id_with_num(userid)
char    userid[IDLEN + 1];
{
   char   *s;
   for (s = userid; *s != '\0'; s++)
      if (*s < 1 || !isalpha(*s)) return 1;
   return 0;            
} 

int
bbsdoreg_main()
{
	FILE *fp;
	struct userec x;
	char buf[256], filename[80], pass1[80], pass2[80], dept[80], phone[80],
	    assoc[80], salt[3], words[1024], *ub = FIRST_PAGE;
	int lockfd;
	struct active_data act_data;
	html_header(1);
	printf("<body>");
	bzero(&x, sizeof (x));
//      xz=atoi(getparm("xz"));

#ifdef POP_CHECK
	char user[USER_LEN + 1];
    char pass[PASS_LEN + 1];
	char popserver[512];
	strsncpy(popserver, getparm("popserver"), 512);	
	strsncpy(user, getparm("user"), USER_LEN);
	strsncpy(pass, getparm("pass"), PASS_LEN);
#endif

	strsncpy(x.userid, getparm("userid"), 13);
	strsncpy(pass1, getparm("pass1"), 13);
	strsncpy(pass2, getparm("pass2"), 13);
	strsncpy(x.username, getparm("username"), 32);
	strsncpy(x.realname, getparm("realname"), 32);
	strsncpy(dept, getparm("dept"), 60);
	strsncpy(x.address, getparm("address"), 60);

#ifndef POP_CHECK
	strsncpy(x.email, getparm("email"), 60);
#else
	char delims[] = "+";
    	char *popname;
	char *popip;
	//char popname[256];
	//char popip[256];

	popname = strtok(popserver, delims);
	popip = strtok(NULL, delims);

	// ��ֹע��©��
	struct stat temp;
	/*
	if (stat(MY_BBS_HOME "/etc/pop_register/pop_list", &temp) == -1)
	{
		http_fatal("Ŀǰû�п������ε��ʼ��������б�, ����޷���֤�û�\n");
	}
	
	fp = fopen(MY_BBS_HOME "/etc/pop_register/pop_list", "r");
	if (fp == NULL)
	{
		http_fatal("�򿪿������ε��ʼ��������б����, ����޷���֤�û�\n");
	}
	*/

	//if (!seek_in_file(MY_BBS_HOME "/etc/pop_register/pop_list", popname)) {
	//	http_fatal("���ǿ����ε��ʼ��������б�!");
	//}
	int vaild = 0;
	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // ע�⣺�������10��pop��������Ҫ��������ˣ�
	char ippop[10][256];
	/*
	while(fgets(bufpop, 256, fp) != NULL)
	{
		if (strcmp(bufpop, "") == 0 || strcmp(bufpop, " ") == 0 || strcmp(bufpop, "\n") == 0)
			break;
		strcpy(namepop[numpop], bufpop);
		fgets(bufpop, 256, fp);
		strcpy(ippop[numpop], bufpop);
		
		namepop[numpop][strlen(namepop[numpop]) - 1] = 0;
		ippop[numpop][strlen(ippop[numpop]) - 1] = 0;

		if (strcmp(namepop[numpop], popname) == 0 &&
			strcmp(ippop[numpop], popip) == 0 )
		{
			vaild = 1;
			break;
		}
		
		numpop ++;
	}
	fclose(fp);	
	*/
	
	//if (!vaild)
	//	http_fatal("-_-bb \n");
	//
	

	char email[60];
	sprintf(email, "%s@%s", user, popname);  // ע�ⲻҪ��emailŪ�����
	str_to_lowercase(email);
	strsncpy(x.email, email, 60);
#endif	
	
	strsncpy(phone, getparm("phone"), 60);
	strsncpy(assoc, getparm("assoc"), 60);
	strsncpy(words, getparm("words"), 1000);


//      x.gender='M';
//      if(atoi(getparm("gender"))) x.gender='F';
//      x.birthyear=atoi(getparm("year"))-1900;
//      x.birthmonth=atoi(getparm("month"));
//      x.birthday=atoi(getparm("day"));

	//if (!goodgbid(x.userid))  by bjgyt
        if (id_with_num(x.userid))
		http_fatal("�ʺ�ֻ����Ӣ����ĸ���");
	if (strlen(x.userid) < 2)
		http_fatal("�ʺų���̫��(2-12�ַ�)");
	if (strlen(pass1) < 4)
		http_fatal("����̫��(����4�ַ�)");
	if (strcmp(pass1, pass2))
		http_fatal("������������벻һ��, ��ȷ������");
	if (strlen(x.username) < 2)
		http_fatal("�������ǳ�(�ǳƳ�������2���ַ�)");
	if (strlen(x.realname) < 4)
		http_fatal("��������ʵ����(��������, ����2����)");
//      if(strlen(dept)<6) http_fatal("������λ�����Ƴ�������Ҫ6���ַ�(��3������)");
	if (strlen(x.address) < 6)
		http_fatal("ͨѶ��ַ��������Ҫ6���ַ�(��3������)");
	if (badstr(x.passwd) || badstr(x.username) || badstr(x.realname))
		http_fatal("����ע�ᵥ�к��зǷ��ַ�");
	if (badstr(x.address) || badstr(x.email))
		http_fatal("����ע�ᵥ�к��зǷ��ַ�");
//      if(badymd(x.birthyear, x.birthmonth, x.birthday)) http_fatal("���������ĳ�������");
	if (is_bad_id(x.userid))
		http_fatal("�����ʺŻ��ֹע���id, ������ѡ��");
	if (getuser(x.userid))
		http_fatal("���ʺ��Ѿ�����ʹ��,������ѡ��");
//      sprintf(salt, "%c%c", 65+rand()*26, 65+rand()*26);
//add by lepton


#ifdef POP_CHECK
	if (strlen(user) == 0)
		http_fatal("�����û���û��");
	if (strlen(pass) == 0)
		http_fatal("��������û��");
#endif


	getsalt(salt);
	strsncpy(x.passwd, crypt1(pass1, salt), 14);
	strncpy(x.lasthost, fromhost,15);	//ipv6 by leoncom 
						//���ܸ�ֵ̫�࣬��Ӱ����������
	x.userlevel = PERM_BASIC;
	x.firstlogin = now_t;
	x.lastlogin = now_t - 3600;  //ipv6 by leoncom ע����ֶ���¼
	x.userdefine = -1;
	x.flags[0] = CURSOR_FLAG | PAGER_FLAG;
//      if(xz==1) currentuser.userdefine ^= DEF_COLOREDSEX;
//      if(xz==2) currentuser.userdefine ^= DEF_S_HOROSCOPE;
	adduser(&x);

#ifndef POP_CHECK
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	fp = fopen("new_register", "a");
	if (fp) {
		fprintf(fp, "usernum: %d, %s\n", getusernum(x.userid) + 1,
			Ctime(now_t));
		fprintf(fp, "userid: %s\n", x.userid);
		fprintf(fp, "realname: %s\n", x.realname);
		fprintf(fp, "dept: %s\n", dept);
		fprintf(fp, "addr: %s\n", x.address);
		fprintf(fp, "phone: %s\n", phone);
		fprintf(fp, "assoc: %s\n", assoc);
		fprintf(fp, "----\n");
		fclose(fp);
	}
	close(lockfd);
#endif

sprintf(filename, "home/%c/%s", mytoupper(x.userid[0]), x.userid);
mkdir(filename, 0755);

#ifndef POP_CHECK
	printf("<center><table><td><td><pre>\n");
	printf("�װ�����ʹ���ߣ����ã�\n\n");
	printf("��ӭ���� ��վ, �������ʺ��Ѿ��ɹ����Ǽ��ˡ�\n");
	printf("��Ŀǰӵ�б�վ������Ȩ��, �����Ķ����¡������ķ�������˽��\n");
	printf("�ż����������˵���Ϣ�����������ҵȵȡ�����ͨ����վ�����ȷ\n");
	printf("������֮���������ø����Ȩ�ޡ�Ŀǰ����ע�ᵥ�Ѿ����ύ\n");
	printf("�ȴ����ġ�һ�����24Сʱ���ھͻ��д𸴣������ĵȴ���ͬʱ��\n");
	printf("��������վ�����䡣\n");
	printf
	    ("��������κ����ʣ�����ȥsysop(վ���Ĺ�����)�淢��������\n\n</pre></table>");
	printf("<hr><br>���Ļ�����������:<br>\n");
	printf("<table border=1 width=400>");
	printf("<tr><td>�ʺ�λ��: <td>%d\n", getusernum(x.userid));
	printf("<tr><td>ʹ���ߴ���: <td>%s (%s)\n", x.userid, x.username);
	printf("<tr><td>��  ��: <td>%s<br>\n", x.realname);
	printf("<tr><td>��  ��: <td>%s<br>\n", x.username);
	printf("<tr><td>��վλ��: <td>%s<br>\n", x.lasthost);
	printf("<tr><td>�����ʼ�: <td>%s<br></table><br>\n", x.email);

	printf
	    ("<center><form><input type=button onclick='window.close()' value=�رձ�����></form></center>\n");
#else
	printf("<center><table><td><td><pre>\n");
	memset(&act_data, 0, sizeof(act_data));
	strcpy(act_data.name, x.realname);
	strcpy(act_data.userid, x.userid);
	strcpy(act_data.dept, dept);
	strcpy(act_data.phone, phone);
	strcpy(act_data.email, email);
	strcpy(act_data.ip, fromhost);
	strcpy(act_data.operator, x.userid);
	
	act_data.status=0;
	write_active(&act_data);


	int result;
	//int result = test_mail_valid(user, pass, popip);
	if (strstr(popname, "idp.xjtu6.edu.cn")) {
		if (!strcmp(fromhost, "202.117.1.190") || !strcmp(fromhost, "2001:250:1001:2::ca75:1be"))
			result=1;
		else {
			http_fatal("�ǿ��ŵ���֤��!");
			result=0;
		}
	}
	else {
		result = test_mail_valid(user, pass, popip);
	}

	switch (result)
    {
		case -2:
		printf("<tr><td>%s<br></table><br>\n", 
			"��ӭ�����뽻����������ٸBBS��<br>��������������������ע�ᣬĿǰ���������û���ݡ�"
			"Ŀǰ��û�з��ġ��ż�����Ϣ��Ȩ�ޡ�<br><br>"
			"���ڿ�ѧȡ��stu.xjtu.edu.cn�����<br>�������������дע�ᵥ��������������֤��������Ϊ��վ��ʽ�û���");	
		  break;
		  case -1:
		  case 0:
		  printf("<tr><td>%s<br></table><br>\n", 
			  "�ʼ�������������ʧ�ܣ�����ֻ��ʹ�ñ�bbs����������ܣ�ʮ�ֱ�Ǹ��");
		  break;

		  case 1:			  
		   if (query_record_num(email, MAIL_ACTIVE)>=MAX_USER_PER_RECORD ) {
        		printf("���������Ѿ���֤�� %d ��id���޷���������֤��!\n", MAX_USER_PER_RECORD);
			break;
		  }
		int response;
		strcpy(act_data.email, email);
		act_data.status=1;
		response=write_active(&act_data);
		if (response==WRITE_SUCCESS || response==UPDATE_SUCCESS)  {
			printf("�����˳ɹ������Ѿ�����ʹ�����ù����ˣ�\n"); 
			register_success(getusernum(x.userid) + 1, x.userid, x.realname, dept, x.address, phone, assoc, email);
			break;
		}
    		printf("  ��֤ʧ��!");
			break;

     
    }

	printf
	    ("<center><form><input type=button onclick='window.close()' value=�رձ�����></form></center>\n");
#endif
	
	// �������У�newcomer�����ܽ�������www��ia64���µ����⡣interma@BMY
	newcomer(&x, words); 


//      sprintf(buf, "%s %-12s %d\n", Ctime(now_t)+4, x.userid, getusernum(x.userid));
//      f_append("wwwreg.log", buf);
	sprintf(buf, "%s newaccount %d %s www", x.userid, getusernum(x.userid),
		fromhost);
	newtrace(buf);
	//wwwstylenum = 1;

	//don't login with reg by leoncom for ipv6
	//ub = wwwlogin(&x,0);
	//sprintf(buf, "%s enter %s www", x.userid, fromhost);
	//newtrace(buf);
	//printf("<script>opener.top.location.href=\"%s\";</script>", ub);
	return 0;
}

int
badstr(unsigned char *s)
{
	int i;
	for (i = 0; s[i]; i++)
		if (s[i] != 9 && (s[i] < 32 || s[i] == 255))
			return 1;
	return 0;
}

void
newcomer(struct userec *x, char *words)
{
	FILE *fp;
	char filename[80];
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(filename, "w");
	fprintf(fp, "��Һ�, \n\n");
	fprintf(fp, "���� %s(%s), ���� %s\n", x->userid, x->username, fromhost);
	fprintf(fp, "��������˵ر���, ���Ҷ��ָ��.\n\n");
	fprintf(fp, "���ҽ���:\n\n");
	fprintf(fp, "%s", words);
	fclose(fp);
	post_article("newcomers", "WWW������·", filename, x->userid,
		     x->username, fromhost, -1, 0, 0, x->userid, -1);
	unlink(filename);
}

void
adduser(struct userec *x)
{
	int i;
	FILE *fp;
	fp = fopen(".PASSWDS", "r+");
	flock(fileno(fp), LOCK_EX);
	for (i = 0; i < MAXUSERS; i++) {
		if (shm_ucache->userid[i][0] == 0) {
			if (i + 1 > shm_ucache->number)
				shm_ucache->number = i + 1;
			strncpy(shm_ucache->userid[i], x->userid, 13);
			insertuseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE,
					 x->userid, i + 1);
			save_user_data(x);
			break;
		}
	}
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	//utime(FLUSH, NULL);
}
