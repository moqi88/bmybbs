#include "bbslib.h"

int
bbsupload_main()
{
	int i;
	html_header(1);
	if (!loginok || isguest)
		http_fatal("���¼!");
	changemode(POSTING);
	i = u_info - &(shm_utmp->uinfo[0]);
	printf("<body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0>\n"
	       "<form method=POST ENCTYPE=\"multipart/form-data\" action=\"/cgi-bin/bbs/bbsupload.py/%c%c%c%s\"\n"
	       "<table width=100%%>\n"
	       "<td><INPUT NAME=\"userfile\" TYPE=\"file\" size=20><input type=submit value=\"ճ��\"></td>\n"
	       "<td><font color=red>��������bmp��ʽ���ļ����������ϴ�ͼƬ�ĸ������������ڷ���ͼ������ͼ������ᱻȡ��ȫվpostȨ�ޡ�</font>���������Ϊ���¸��ӵ�СͼƬС����ɶ��, ��Ҫ��̫��Ķ���Ŷ, �ļ�������Ҳ��Ҫ�������ʺ�ʲô�ĵ�, �����ճ��ʧ��.</td></table>\n"
	       "</form>\n</body>", i / (26 * 26) + 'A', i / 26 % 26 + 'A',
	       i % 26 + 'A', u_info->sessionid);

	http_quit();
	return 0;
}
