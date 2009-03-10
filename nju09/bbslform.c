#include "bbslib.h"
#define LOGTMP HTMPATH "/login.htm"

int
bbslform_main()
{
	static struct mmapfile mf = { ptr:NULL };
	html_header(1);
	if (loginok && !isguest)
		http_fatal("你已经登录了啊...");
	if (mmapfile(LOGTMP, &mf) < 0)
		http_fatal("无法打开模板文件");
	fwrite(mf.ptr, 1, mf.size, stdout);
	return 0;
}
