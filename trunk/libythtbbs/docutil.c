#include <stdio.h>
#include "ythtbbs.h"

int
eff_size(char *file)
{
	FILE *fp;
	char buf[1000];
	int i, size, size2 = 0;
	size = file_size(file);
	if (size > 3000 || size == 0)
		goto E;
	size = 0;
	fp = fopen(file, "r");
	if (fp == 0)
		return 0;
	for (i = 0; i < 10; i++) {
		char *ptr;
		if (fgets(buf, sizeof (buf), fp) == 0)
			break;
		if ((ptr = strchr(buf, '\r')))
			*ptr = 0;
		if ((ptr = strchr(buf, '\n')))
			*ptr = 0;
		if (!strlen(strtrim(buf)))
			break;
	}
	while (1) {
		if (fgets(buf, sizeof (buf), fp) == 0)
			break;
		if (!strcmp(buf, "--\n"))
			break;
		if (!strncmp(buf, ": ", 2))
			continue;
		if (!strncmp(buf, "�� �� ", 4))
			continue;
		if (strstr(buf, "�� ��Դ:��"))
			continue;
		for (i = 0; buf[i]; i++)
			if (buf[i] < 0)
				size2++;
		size += strlen(strtrim(buf));
	}
	fclose(fp);
      E:
	size = size - size2 / 2;
	if (size == 0)
		size = 1;
	return size;
}

char *
getdocauthor(char *filename, char *author, int len)
{
	char buf[256], *ptr, *f1, *f2;
	int i = 0;
	FILE *fp;
	author[0] = 0;
	fp = fopen(filename, "r");
	if (!fp)
		return author;
	while (i++ < 5) {
		if (!fgets(buf, sizeof (buf), fp))
			break;
		if (strncmp(buf, "������: ", 8) && strncmp(buf, "������: ", 8))
			continue;
		ptr = buf + 8;
		f1 = strsep(&ptr, " ,\n\r\t");
		if (f1)
			strsncpy(author, f1, len);
		f2 = strsep(&ptr, " ,\n\r\t");
		if (f2 && f2[0] == '<' && f2[strlen(f2) - 1] == '>'
		    && strchr(f2, '@')) {
			f2[strlen(f2) - 1] = 0;
			strsncpy(author, f2 + 1, len);
		}
		ptr = strpbrk(author, "();:!#$\"\'");
		if (ptr)
			*ptr = 0;
		break;
	}
	fclose(fp);
	return author;
}

int
keepoldheader(FILE * fp, int dowhat)
{
	static char (*tmpbuf)[STRLEN] = NULL;
	static int hash = 0;
	int i;
	switch (dowhat) {
	case SKIPHEADER:
	case KEEPHEADER:
		hash = i = 0;
		if (NULL == tmpbuf)
			tmpbuf = malloc(5 * STRLEN);
		if (NULL == tmpbuf)
			return -1;
		while (fgets(tmpbuf[i], STRLEN, fp)) {
			i++;
			if (!strcmp(tmpbuf[i - 1], "\n")
			    || !strcmp(tmpbuf[i - 1], "\r\n") || i > 4)
				break;
		}
		if (i < 4 || (strncmp(tmpbuf[0], "������: ", 8) &&
			      strncmp(tmpbuf[0], "������: ", 8)) ||
		    strncmp(tmpbuf[1], "��  ��: ", 8)) {
			fseek(fp, 0, SEEK_SET);
			i = 0;
			goto RET1;
		}
	      RET1:if (SKIPHEADER == dowhat) {
			free(tmpbuf);
			tmpbuf = NULL;
		}
		hash = i;
		return 0;
	case RESTOREHEADER:
		if (!tmpbuf)
			return -2;
		for (i = 0; i < hash; i++)
			fputs(tmpbuf[i], fp);
		free(tmpbuf);
		tmpbuf = NULL;
		hash = 0;
		return 0;
	}
	return -3;
}
