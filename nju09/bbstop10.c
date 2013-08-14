#include "bbslib.h"

/*20121014 New top10 page  Edited by liuche*/

void showTop10Table();

/*
|20121014 �Ա����ʽ��ʾ���ĺ�֪ͨ����
|kind==1 ��ʾ���ģ���Ӧԭ���commend��
|kind==2 ��ʾ֪ͨ����Ӧԭ���commend2��
|��ʵ����Ȥ���԰����е�commend�ֵܸ���д��һ������������
*/
void showCommend(int kind); 

int
bbstop10_main()
{
	html_header(1);
	check_msg();
//main frame
	printf("<div style=\"width=100%; min-height:350px;\">");
	//<!--No.1 Top 10 -->	
	showTop10Table();
	printf("</div>");


	printf("<div style=\"width:98%;margin-top:50px\">");
	printf("<center>");
	printf("<div style=\"width:48%;min-height:100px;float:left;overflow:hidden;\">");
	printf("	<!--No.2  COMMEND -->");
	showCommend(1);
	printf("</div>");

	printf("	<!--No.3 COMMEND2-->");
	printf("<div style=\"width:48%; min-height:100px;float:left; overflow:auto;\">");
	showCommend(2);
	printf("</div>");
	printf("</center>");
	printf("</div>");

	http_quit();
	return 0;
}

void showTop10Table(){
	struct mmapfile mf = {ptr:NULL};
	MMAP_TRY {
		if (mmapfile("wwwtmp/ctopten", &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("�ļ�����");
		}
		fwrite(mf.ptr, mf.size, 1, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
}

void showCommend(int kind){
	FILE *fp;
	struct commend x;
	char allcanre[256];
	char *head[3];
	int i;//, total;
	head[1]="���������Ƽ�";
	head[2]="����֪ͨ����";
	printf("	<body><center><div class=rhead>����ٸ BBS --<span class=h11> %s</span></div>",head[kind]);
	printf("	<hr>");
	if(1==kind)
		fp=fopen(COMMENDFILE,"r");
	else if(2==kind)
		fp=fopen(COMMENDFILE2,"r");
	if (!fp)
		 http_fatal("Ŀǰû���κ��Ƽ�����");	
	printf("	<table border=1>");
	printf("	<tr><td>No.</td><td>������</td><td>����</td><td>����</td></tr>");
	fseek(fp, -20*sizeof(struct commend), SEEK_END);	
	for(i=20; i>0; i--) {
		strcpy(allcanre, "");
		if(fread(&x, sizeof(struct commend), 1, fp)<=0) break;
		if(x.accessed & FH_ALLREPLY)
 			strcpy(allcanre," style='color:red;' ");
		printf("<tr><td>  %d  </td> <td><a href=\"%s%s\" >%-13s</a></td> <td><a href=con?B=%s&F=%s%s>%-30s</a></td> <td><a href=qry?U=%s >%-12s</a></td> </tr>",
 			21-i, showByDefMode(), x.board, x.board, x.board, x.filename, allcanre, x.title,x.userid,  x.userid );

	}
	fclose(fp);
	printf("	</table></center></body>	");
}
