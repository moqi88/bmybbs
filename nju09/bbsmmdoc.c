#include "bbslib.h"

int
bbsmmdoc_main()
{	//developed by macintosh 050519 for new www
	FILE *fp, *fp1, *fp2;
	char board[80], dir[80],buf[128],name[80];
	struct boardmem *x1;
	struct fileheader x;
	int i, start, total=0;
	html_header(1);
	check_msg();
	printf("<script src=/function.js></script>\n");
	changemode(READING);
	strsncpy(board, getparm2("B", "board"), 32);
	x1 = getboard(board);
	if (x1 == 0)
		nosuchboard(board, "bbsmmdoc");
	updateinboard(x1);
	strcpy(board, x1->header.filename);

	/*��ʼ��.DIR�ļ�*/
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(buf, "boards/%s/.POWER.%s_mark", board, currentuser.userid);
	fp1 = fopen(dir, "r");
	fp2 = fopen(buf, "w");
	if (fp1== 0)
		http_fatal("��������Ŀǰû������");
	/*��ʼ����m����, д��.POWER�ļ�*/
	while (1) {
		if (fread(&x, sizeof (x), 1, fp1) == 0)
			break;
		if (!(x.accessed & FH_MARKED))
			continue;
		fwrite(&x, sizeof (x), 1, fp2);
		total++;
	}
	fclose(fp1);
	fclose(fp2);
	
	fp = fopen(buf, "r");	
	start = getdocstart(total, w_info->t_lines);
	printf("<body topmargin=0 leftmargin=0>\n");
	printf("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
		"<td><form name=form1 action=bbsmmdoc>\n");
	printboardtop(x1, 4);

	printf("<tr><td><a href=\"pst?B=%s\" class=\"btnsubmittheme\" title=\"�������� accesskey: p\" accesskey=\"p\">��������</a>\n", board);
	printf("������&lt;%d&gt; ����&lt;%d&gt;</td>", total, x1->inboard);
	sprintf(buf, "bbsmmdoc?board=%s", board);
	printf("<td align=right><a href=\"mmdoc?B=%s&S=%d\" title=\"��һҳ accesskey: 1\" accesskey=\"1\">��һҳ</a>\n", board, 1);
	if(start > w_info->t_lines+1) printf("<a href=\"mmdoc?B=%s&S=%d\" title=\"��һҳ accesskey: f\" accesskey=\"f\">��һҳ</a>\n", board, (start-w_info->t_lines));
	if(start < total-w_info->t_lines+1) printf("<a href=\"mmdoc?B=%s&S=%d\" title=\"��һҳ accesskey: n\" accesskey=\"n\">��һҳ</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"mmdoc?B=%s&S=%d\" title=\"���һҳ accesskey: l\" accesskey=\"l\">���һҳ</a>\n", board, (total-w_info->t_lines+1)); 
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit1 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=S type=text style=\"font-size:11px;font-family:verdana;\" size=4></td>\n"
		"</tr></table></td></tr>\n");
	printf("</form></td>");
	if (total <= 0) {
		fclose(fp);
		http_fatal("��������Ŀǰû�б�m����");
	}

	printf("%s", "<tr><td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td class=\"level1\"><TABLE width=\"95%\" cellpadding=2 cellspacing=0>\n"
		"<TBODY>\n");
	printf("%s", "<TR>\n"
		"<TD class=tdtitle>���</TD>\n"
		"<TD class=tdtitle>״̬</TD>\n"
		"<TD class=tduser>����</TD>\n"
		"<TD align=center class=tdtitle>����</TD>\n"
		"<TD align=center class=tdtitle>����</TD>\n"
		"<TD class=tdtitle>�Ǽ�</TD>\n"
		"<TD class=tdtitle>����</TD>\n"
		"</TR>\n");
	
	fseek(fp, (start - 1) * sizeof (struct fileheader), SEEK_SET);
	for (i = 0; i < w_info->t_lines; i++) {
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;
		printf("<tr><td class=tdborder>%d</td><td class=tdborder> </td><td class=tduser>%s</td>",
		       start + i, 
		       //flag_str(x.accessed) , 
		       userid_str(x.owner));
		printf("<td align=center class=tdborder>%12.12s</td>", Ctime(x.filetime) + 4);
		printf
		    ("<td class=tdborder><a href=con?B=%s&F=%s>%s%s</a></td><td class=tdborder>%d</td><td class=tdborder>%d��</td></tr>\n",
		     board, fh2fname(&x),  strncmp(x.title,
								 "Re: ",
								 4) ? "�� " :
		     "", void1(titlestr(x.title)), x.staravg50 / 50,
		     x.hasvoted);
	}
	printf("</TR> </TBODY></TABLE></td></tr>\n");
	printf("<tr><td height=40 class=\"level1\">&nbsp;</td>\n"
		"<td height=40 class=\"level1\">\n"
		"<table width=\"95%\" height=\"100%\"  border=0 cellpadding=0 cellspacing=0 class=\"level1\">\n"
		"<td><form name=form2 action=bbsmmdoc>\n");
	printf("<tr><td><a href=\"pst?B=%s\" class=\"btnsubmittheme\" title=\"�������� accesskey: p\" accesskey=\"p\">��������</a>\n", board);
	printf("������&lt;%d&gt; ����&lt;%d&gt;</td>", total, x1->inboard);
	sprintf(buf, "bbsmmdoc?board=%s", board);
	printf("<td align=right><a href=\"mmdoc?B=%s&S=%d\" title=\"��һҳ accesskey: 1\" accesskey=\"1\">��һҳ</a>\n", board, 1);
	if(start > w_info->t_lines+1) printf("<a href=\"mmdoc?B=%s&S=%d\" title=\"��һҳ accesskey: f\" accesskey=\"f\">��һҳ</a>\n", board, (start-w_info->t_lines));
	if(start < total-w_info->t_lines+1) printf("<a href=\"mmdoc?B=%s&S=%d\" title=\"��һҳ accesskey: n\" accesskey=\"n\">��һҳ</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"mmdoc?B=%s&S=%d\" title=\"���һҳ accesskey: l\" accesskey=\"l\">���һҳ</a>\n", board, (total-w_info->t_lines+1)); 
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit2 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=S type=text style=\"font-size:11px;font-family:verdana;\" size=4></td>\n"
		"</tr></table></td></tr>\n");
	printf("</form></td>");
	fclose(fp);
	printf("</table></td></tr></table></body>\n");
	http_quit();
	return 0;
}
