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
	printf("<center><a href='/" SMAGIC "/bbsfindpass' target='_blank'> �һ��û���������</a></center>\n");
	return 0;
}
