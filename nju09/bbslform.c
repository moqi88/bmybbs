#include "bbslib.h"
#define LOGTMP HTMPATH "/login.htm"

int
bbslform_main()
{
	static struct mmapfile mf = { ptr:NULL };
	html_header(1);
	if (loginok && !isguest)
		http_fatal("���Ѿ���¼�˰�...");
	if (mmapfile(LOGTMP, &mf) < 0)
		http_fatal("�޷���ģ���ļ�");
	fwrite(mf.ptr, 1, mf.size, stdout);
	printf("<center><a href='/" SMAGIC "/con?B=sysop&F=M.1328175493.A&N=2&T=0'> ��ȴ�����ˣ�</a></center>\n");
	return 0;
}
