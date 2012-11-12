#include "bbslib.h"

int
bbsselstyle_main()
{
	char name[STRLEN], *p, *url;
	int i, n= NWWWSTYLE;
	url = getenv("SCRIPT_URL");
	if (NULL == url)
		return -1;
	name[0] = 0;
	if (!strcmp(currentuser.userid,"guest")) 
		n=NWWWSTYLE-1;
	if (!strncmp(url, "/" SMAGIC, sizeof (SMAGIC))) {
		snprintf(name, STRLEN, "%s", url + sizeof (SMAGIC));
		p = strchr(name, '/');
		if (NULL != p) {
			*p = 0;
		} else {
			http_fatal("Incorrect url");
			return -1;
		}
	}
	html_header(1);
	check_msg();
	printf("<body><center><br><h2>ѡ�������</h2><table>");
	for (i = 0; i < n; i++) {
		addextraparam(name, sizeof (name), 0, i);
		printf("<tr><td><li><a href=/" SMAGIC
		       "%s/ target=_top>%s</a></td></tr>", name,
		       wwwstyle[i].name);
	}
	if ( n==NWWWSTYLE )
		printf("<tr><td><li><a href=bbsdefcss>�����Լ��Ľ���</a></td></tr>");
	printf("</table></body>"); 
	http_quit();
	return 0;
}
