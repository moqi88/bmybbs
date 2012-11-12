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

// ��½�ʼ������������������֤�� added by interma@BMY 2005.5.12
// ����ֵΪ1��ʾ��Ч��0��ʾ��Ч, -1��ʾ��pop���������ӳ��� 
/*
static int test_mail_valid(char *user, char *pass, char *popip)
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


static char * str_to_upper(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0)
	{
		*str = toupper(*str);
		str++;
	}
	return h;
}

*/
int
bbsresetpass_main()
{
	FILE *fp;
	struct userec* x;
	char buf[256], filename[80], pass1[80], pass2[80], dept[80], phone[80],
	    assoc[80], salt[3], words[1024], userid[32],  *ub = FIRST_PAGE;
	int lockfd;
	int count;
	struct active_data act_data;
	html_header(1);
	printf("<body>");

	char user[USER_LEN + 1];
    char pass[PASS_LEN + 1];
	char popserver[512];
	strsncpy(popserver, getparm("popserver"), 512);	
	strsncpy(user, getparm("user"), USER_LEN);
	strsncpy(pass, getparm("pass"), PASS_LEN);

	strsncpy(userid, getparm("userid"), 13);
	strsncpy(pass1, getparm("pass1"), 13);
	strsncpy(pass2, getparm("pass2"), 13);
	char delims[] = "+";
    	char *popname;
	char *popip;


	popname = strtok(popserver, delims);
	popip = strtok(NULL, delims);

	// ��ֹע��©��
	struct stat temp;

	int vaild = 0;
	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // ע�⣺�������10��pop��������Ҫ��������ˣ�
	char ippop[10][256];

	char email[60];
	sprintf(email, "%s@%s", user, popname);  // ע�ⲻҪ��emailŪ�����
	str_to_lowercase(email);




	if (strlen(pass1) < 4)
		http_fatal("����̫��(����4�ַ�)");
	if (strcmp(pass1, pass2))
		http_fatal("������������벻һ��, ��ȷ������");
	if (strlen(user) == 0)
		http_fatal("�����û���û��");
	if (strlen(pass) == 0)
		http_fatal("��������û��");




	printf("<center><table><td><td><pre>\n");
	count=read_active(userid, &act_data);
	if (count<1) {
		http_fatal("���֣����޴��ˣ�������û�id��?");
	}
	if (strcmp(act_data.email, email)) {
		http_fatal("���û������ǲ����������������֤��ѽ&gt__&lt");
	}

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
		  case -1:
		  case 0:
		  printf("<tr><td>%s<br></table><br>\n", 
			  "�ʼ�������������ʧ�ܣ��޷��������룬���Ƿ��������������?");
		  break;

		  case 1:			  

		x=getuser(userid);
		getsalt(salt);
		strcpy(x->passwd, crypt1(pass2, salt));
		save_user_data(x);
		
		
			printf("�������óɹ�!\n"); 
			break;

     
    }

	printf
	    ("<center><form><input type=button onclick='window.close()' value=�رձ�����></form></center>\n");

	return 0;
}


