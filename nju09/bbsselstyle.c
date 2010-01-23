#include "bbslib.h"

int
bbsselstyle_main()
{
	char name[STRLEN], *p, *url;
	int i, n= NWWWSTYLE;

	if (!strcmp(currentuser.userid,"guest")) 
		n=NWWWSTYLE-1;

	html_header(1);
	check_msg();
	printf("<body><center><br><h2>选择界面风格</h2><table>");
	for (i = 0; i < n; i++) {
		printf("<tr><td><li><a href=/" SMAGIC
		       "/changestyle?color=%d target=_top>%s</a></td></tr>", i,
		       wwwstyle[i].name);
	}
	if ( n==NWWWSTYLE )
		printf("<tr><td><li><a href=bbsdefcss>定义自己的界面</a></td></tr>");
	printf("</table></body>"); 
	http_quit();
	return 0;
}
