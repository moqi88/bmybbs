// ��PASSWD�ļ��е������û�����Զ�������Ȩ��(H)
// author: leoncom@bmy

#include <stdio.h> 

int main() 
{ 
	FILE *fp,*fd; 
	if((fp=fopen(".PASSWDS","rb")) == NULL) 
		exit(1); 
	if((fd=fopen("result","wb")) == NULL) 
		exit(1); 
	unsigned c; 
	int count=0; 
	while(1) 
	{ 
		c=fgetc(fp); 
		if(feof(fp)) 
			exit(1); 
		count++; 
		if((count-224)%452 == 0) //�������Ǻ�2�ֽ�Ȩ��λ 
		{ 
			c=c | 0x80 ;         //���HȨ�� 
			//printf("%d\n",c); 
		} 
		fputc(c,fd); 
	} 
	fclose(fp); 
	fclose(fd); 
	return 0; 
} 
