#ifndef __YTHTLIB_H
#define __YTHTLIB_H
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>

#ifndef sizeof
#define sizeof(x) ((int)sizeof(x))
#endif

#ifndef ATTACHCACHE
#define ATTACHCACHE "/home/bbsattach/cache"
#endif

#ifndef ERRLOG
#define ERRLOG	"/home/bbs/deverrlog"
#endif

struct hword {
	char str[80];
	void *value;
	struct hword *next;
};

typedef struct hword *diction[26 * 26];

int getdic(diction dic, size_t size, void **mem);
struct hword *finddic(diction dic, char *key);
struct hword *insertdic(diction dic, struct hword *cell);
int mystrtok(char *str, int delim, char *result[], int max);
struct stat *f_stat(char *file);
struct stat *l_stat(char *file);
int uudecode(FILE * fp, char *outname);
int fakedecode(FILE * fp);
char *attachdecode(FILE * fp, char *articlename, char *filename);
void uuencode(FILE * fr, FILE * fw, int len, char *filename);
void _errlog(char *fmt, ...);
#define errlog(format, args...) _errlog(__FILE__ ":%s line %d " format, __FUNCTION__,__LINE__ , ##args)

#define file_size(x) (f_stat(x)->st_size)
#define file_time(x) (f_stat(x)->st_mtime)
#define file_rtime(x) (f_stat(x)->st_atime)
//#define file_exist(x) (file_time(x)!=0)
#define file_exist(x) (access(x, F_OK)==0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)
#define lfile_isdir(x) ((l_stat(x)->st_mode & S_IFDIR)!=0)

unsigned char numbyte(int n);
int bytenum(unsigned char c);

void normalize(char *buf);
void strsncpy(char *s1, const char *s2, int n);
char *strltrim(char *s);
char *strrtrim(char *s);
#define strtrim(s) strltrim(strrtrim(s))
char *strnstr(const char *haystack, const char *needle, size_t haystacklen);
char *strncasestr(const char *haystack, const char *needle, size_t haystacklen);

void *try_get_shm(int key, int size, int flag);
void *get_shm(int key, int size);
#define get_old_shm(x,y) try_get_shm(x,y,0)
//Copy from Linux 2.4 kernel...
#define min(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })
//end
#include "fileop.h"
#include "named_socket.h"
#include "crypt.h"
#include "limitcpu.h"
#include "timeop.h"

#define BAD_WORD_NOTICE "�������¿��ܺ��а��չ����йع涨" \
                       "�������ڹ������Ϸ��������\n������" \
                       "�����й���ʿ���󷢱�\n"            \
                       "����������ʣ�����ϵ���ߵ�վ����˸�����"
#define DO1984_NOTICE  "�����йز���Ҫ��,�������±��뾭���й�" \
                       "��ʿ���󷢱�,�����ĵȺ�!\n"           \
                       "����������ʣ�����ϵ���ߵ�վ����˸�����"

int reload_badwords(char *wordlistf, char *imgf);
int filter_file(char *checkfile, struct mmapfile *badword_img);
int filter_string(char *string, struct mmapfile *badword_img);
int filter_article(char *title, char *filename, struct mmapfile *badword_img);

#endif
