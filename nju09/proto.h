/* bbsmain.c */
void logtimeused(void);
void wantquit(int signal);
struct cgi_applet *get_cgi_applet(char *needcgi);
void no_outcache(void);
void end_outcache(void);
void get_att_server(void);
int main(int argc, char *argv[]);
/* bbstop10.c */
int bbstop10_main(void);
/* bbsdoc.c */
void printdocform(char *cginame, char *board);
void nosuchboard(char *board, char *cginame);
void printboardhot(struct boardmem *x);
void printboardtop(struct boardmem *x, int num);
int getdocstart(int total, int lines);
void bbsdoc_helper(char *cgistr, int start, int total, int lines);
int printkeywords(char *keywordstr);
void printrelationboards(char *buf);
int bbsdoc_main(void);
char *size_str(int size);
int top_file(void);
int show_rec(void);
/* bbscon.c */
int showbinaryattach(char *filename);
void fprintbinaryattachlink(FILE *fp, int ano, char *attachname, int pos, int size, char *alt, char *alt1);
int fshowcon(FILE *output, char *filename, int show_iframe);
int showcon(char *filename);
int showconxml(char *filename, int viewertype);
int showcon_cache(char *filename, int level, int edittime);
int testmozilla(void);
int testxml(void);
void processMath(void);
int bbscon_main(void);
/* bbsbrdadd.c */
int bbsbrdadd_main(void);
/* bbsboa.c */
int bbsboa_main(void);
int showsecpage(const struct sectree *sec, struct boardmem *(data[]), int total, char *secstr);
int showdefaultsecpage(const struct sectree *sec, struct boardmem *(data[]), int total, char *secstr);
int showsechead(const struct sectree *sec);
int showstarline(char *str);
int showsecnav(const struct sectree *sec);
int showhotboard(const struct sectree *sec, char *s);
int showfile(char *fn);
int showsecintro(const struct sectree *sec);
int showboardlist(struct boardmem *(data[]), int total, char *secstr, const struct sectree *sec);
int board_read(char *board, int lastpost);
void printlastmark(char *board);
int showsecmanager(const struct sectree *sec);
int show_commend(void);
void show_banner(void);
void title_begin(char *title);
void title_end(void);
int show_content(void);
void show_sec(struct sectree *sec);
void show_boards(char *secstr);
void show_sec_boards(struct boardmem *(data[]), int total);
void show_top10(void);
void show_area_top(char c);
void show_right_click_header(int i);
/* bbsall.c */
int bbsall_main(void);
/* bbsanc.c */
int bbsanc_main(void);
/* bbs0an.c */
int anc_readtitle(FILE *fp, char *title, int size);
int anc_readitem(FILE *fp, char *path, int sizepath, char *name, int sizename);
int anc_hidetitle(char *title);
int bbs0an_main(void);
int getvisit(int n[2], const char *path);
/* bbslogout.c */
int bbslogout_main(void);
/* bbsleft.c */
void printdiv(int *n, char *str);
void printsectree(const struct sectree *sec);
int bbsleft_main(void);
/* bbslogin.c */
int bbslogin_main(void);
char *wwwlogin(struct userec *user, int ipmask);
/* bbsbadlogins.c */
int bbsbadlogins_main(void);
/* bbsqry.c */
int show_special_web(char *id2);
int bbsqry_main(void);
void show_special(char *id2);
int bm_printboard(struct boardmanager *bm, void *farg);
/* bbsnot.c */
int bbsnot_main(void);
/* bbsfind.c */
int bbsfind_main(void);
int search(char *id, char *pat, char *pat2, char *pat3, int dt);
/* bbsfadd.c */
int bbsfadd_main(void);
/* bbsfdel.c */
int bbsfdel_main(void);
/* bbsfall.c */
int bbsfall_main(void);
/* bbsfriend.c */
int bbsfriend_main(void);
/* bbsfoot.c */
struct user_info *query_f(int uid);
int bbsfoot_main(void);
int mails_time(char *id);
int mails(char *id, int *unread);
/* bbsform.c */
int bbsform_main(void);
void check_if_ok(void);
void check_submit_form(void);
/* bbspwd.c */
char *random_str(char *buf, int len);
int vaild_mail(const char *email);
int bbspwd_main(void);
/* bbsplan.c */
int bbsplan_main(void);
int save_plan(char *plan);
/* bbsinfo.c */
int bbsinfo_main(void);
int check_info(void);
/* bbsmybrd.c */
int bbsmybrd_main(void);
int readmybrd(char *userid);
int ismybrd(char *board);
int read_submit(void);
/* bbssig.c */
int bbssig_main(void);
void save_sig(char *path);
/* bbspst.c */
int bbspst_main(void);
void printselsignature(void);
void printuploadattach(void);
void printusemath(int checked);
/* bbsgcon.c */
int bbsgcon_main(void);
/* bbsgdoc.c */
int bbsgdoc_main(void);
/* bbsmmdoc.c */
int bbsmmdoc_main(void);
/* bbsdel.c */
int bbsdel_main(void);
/* bbsdelmail.c */
int bbsdelmail_main(void);
/* bbsmailcon.c */
int bbsmailcon_main(void);
/* bbsmail.c */
int bbsmail_main(void);
/* bbsdelmsg.c */
int bbsdelmsg_main(void);
/* bbssnd.c */
int testmath(char *ptr);
int bbssnd_main(void);
/* bbsnotepad.c */
int bbsnotepad_main(void);
/* bbsmsg.c */
int bbsmsg_main(void);
/* bbssendmsg.c */
int bbssendmsg_main(void);
int checkmsgbuf(char *msg);
/* bbsreg.c */
int bbsreg_main(void);
/* bbsmailmsg.c */
int bbsmailmsg_main(void);
void mail_msg(struct userec *user);
/* bbssndmail.c */
int bbssndmail_main(void);
/* bbsnewmail.c */
int bbsnewmail_main(void);
/* bbspstmail.c */
int bbspstmail_main(void);
/* bbsgetmsg.c */
int print_emote_table(char *form, char *input);
int emotion_print(char *msg);
int bbsgetmsg_main(void);
void check_msg(void);
/* bbscloak.c */
int bbscloak_main(void);
/* bbsmdoc.c */
int bbsmdoc_main(void);
/* bbsnick.c */
int bbsnick_main(void);
/* bbstfind.c */
int bbstfind_main(void);
/* bbsadl.c */
int bbsadl_main(void);
/* bbstcon.c */
char *userid_str_class(char *s, char *class);
int bbstcon_main(void);
int fshow_file(FILE *output, char *board, struct fileheader *x, int n);
int show_file(char *board, struct fileheader *x, int n);
/* bbstdoc.c */
int bbstdoc_main(void);
char *stat1(struct fileheader *data, int from, int total);
/* bbsdoreg.c */
int test_mail_valid(char *user, char *pass, char *popip);
int substitute_record(char *filename, void *rptr, int size, int id);
void register_success(int usernum, char *userid, char *realname, char *dept, char *addr, char *phone, char *assoc, char *email);
char *str_to_upper(char *str);
int write_pop_user(char *user, char *userid, char *pop_name);
int id_with_num(char userid[12 + 1]);
int bbsdoreg_main(void);
int badstr(unsigned char *s);
void newcomer(struct userec *x, char *words);
void adduser(struct userec *x);
/* bbsmywww.c */
int bbsmywww_main(void);
int save_set(int t_lines, int link_mode, int def_mode, int att_mode, int doc_mode);
/* bbsccc.c */
int bbsccc_main(void);
int do_ccc(struct fileheader *x, struct boardmem *brd1, struct boardmem *brd);
/* bbsufind.c */
void printkick(char *buf, int size, struct user_info *x);
int bbsufind_main(void);
/* bbsclear.c */
int bbsclear_main(void);
/* bbsstat.c */
int search_stat(void *ptr, int size, int key);
int bbsstat_main(void);
/* bbsedit.c */
int bbsedit_main(void);
int Origin2(char text[256]);
int update_form(char *board, char *file, char *title);
int getpathsize(char *path);
/* bbsman.c */
int bbsman_main(void);
int do_del(char *board, char *file);
int do_set(char *dirptr, int size, char *file, int flag, char *board);
/* bbsparm.c */
int bbsparm_main(void);
int read_form(void);
/* bbsfwd.c */
int bbsfwd_main(void);
int do_fwd(struct fileheader *x, char *board, char *target);
/* bbsmnote.c */
int bbsmnote_main(void);
void save_note(char *path);
/* bbsdenyall.c */
int bbsdenyall_main(void);
/* bbsdenydel.c */
int bbsdenydel_main(void);
/* bbsdenyadd.c */
int bbsdenyadd_main(void);
/* bbstopb10.c */
int bbstopb10_main(void);
/* bbsbfind.c */
int bbsbfind_main(void);
/* bbsx.c */
int bbsx_main(void);
/* bbseva.c */
int bbseva_main(void);
int set_eva(char *board, char *file, int star, int result[2], char *buf);
int do_eva(char *board, char *file, int star);
/* bbsvote.c */
int bbsvote_main(void);
int addtofile(char filename[80], char str[256]);
/* bbsshownav.c */
int bbsshownav_main(void);
int shownavpart(int mode, const char *secstr);
void shownavpartline(char *buf, int mode);
/* bbsbkndoc.c */
int bbsbkndoc_main(void);
/* bbsbknsel.c */
int bbsbknsel_main(void);
/* bbsbkncon.c */
int bbsbkncon_main(void);
/* bbshome.c */
char *userid_str2(char *s);
char *get_mime_type(char *name);
int bbshome_main(void);
/* bbsindex.c */
int checkfile(char *fn, int maxsz);
char *get_login_link(void);
char *get_login_pic(void);
int loadoneface(void);
int showannounce(void);
void loginwindow(void);
void shownologin(void);
int bbsindex_main(void);
/* bbssechand.c */
void short_stamp(char *str, time_t *chrono);
int showheader(char *grp);
void showwelcome(void);
void showgroup(char *grp);
void showitem(char *grp, char *item);
void postnewslot(char *grp);
int savenewslot(char *grp);
int replymail(char *grp, char *item);
int bbssechand_main(void);
/* bbsupload.c */
int bbsupload_main(void);
/* bbslform.c */
int bbslform_main(void);
/* regreq.c */
int regreq_main(void);
/* bbsselstyle.c */
int bbsselstyle_main(void);
/* bbscon1.c */
int bbscon1_main(void);
/* bbsattach.c */
int bbsattach_main(void);
/* bbskick.c */
int bbskick_main(void);
/* bbsscanreg.c */
int bbsscanreg_main(void);
/* bbsshowfile.c */
int bbsshowfile_main(void);
/* bbst.c */
int bbst_main(void);
/* bbsdt.c */
int bbsdt_main(void);
/* bbslt.c */
void print_radio(char *cname, char *name, char *str[], int len, int select);
int bbslt_main(void);
/* bbsincon.c */
int bbsincon_main(void);
/* bbssetscript.c */
int bbssetscript_main(void);
/* bbscccmail.c */
int bbscccmail_main(void);
int do_cccmail(struct fileheader *x, struct boardmem *brd);
/* bbsfwdmail.c */
int bbsfwdmail_main(void);
int do_fwdmail(char *fn, struct fileheader *x, char *target);
/* bbstmpl.c */
int bbstmpl_main(void);
/* bbsrss.c */
int bbsrss_main(void);
/* bbsucss.c */
int bbsucss_main(void);
/* bbsdefcss.c */
int bbsdefcss_main(void);
/* bbssecfly.c */
int bbssecfly_main(void);
/* bbssbs.c */
int bbssbs_main(void);
/* bbseditmail.c */
int bbseditmail_main(void);
/* BBSLIB.c */
void getsalt(char salt[3]);
void filter(char *line);
int junkboard(char *board);
int file_has_word(char *file, char *word);
int f_write(char *file, char *buf);
int f_append(char *file, char *buf);
int get_record(void *buf, int size, int num, char *file);
int put_record(void *buf, int size, int num, char *file);
int del_record(char *file, int size, int num);
long get_num_records(char *filename, int size);
int insert_record(char *fpath, void *data, int size, int pos, int num);
char *noansi(char *s);
char *nohtml(const char *s);
char *strright(char *s, int len);
char *getsenv(char *s);
int http_quit(void);
void http_fatal(char *fmt, ...);
void strnncpy(char *s, int *l, char *s2);
void strnncpy2(char *s, int *l, char *s2, int len);
void hsprintf(char *s, char *s0);
char *titlestr(char *str);
int hprintf(char *fmt, ...);
void fqhprintf(FILE *output, char *str);
int fhhprintf(FILE *output, char *fmt, ...);
void parm_add(char *name, char *val);
int isaword(char *dic[], char *buf);
int url_parse(void);
void http_parm_free(void);
void http_parm_init(void);
int cache_header(time_t t, int age);
void html_header(int mode);
void xml_header(void);
int __to16(char c);
void __unhcode(char *s);
char *getparm(char *var);
char *getparm2(char *v1, char *v2);
int shm_init(void);
int ummap(void);
int addextraparam(char *ub, int size, int n, int param);
void extraparam_init(unsigned char *extrastr);
int user_init(struct userec *x, struct user_info **y, unsigned char *ub);
int mail_file(char *filename, char *userid, char *title, char *sender);
int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark);
int post_mail_buf(char *userid, char *title, char *buf, char *id, char *nickname, char *ip, int sig, int mark);
int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);
int post_article_1984(char *board, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark, int outgoing, char *realauthor, int thread);
int post_article(char *board, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark, int outgoing, char *realauthor, int thread);
int securityreport(char *title, char *content);
void sig_append(FILE *fp, char *id, int sig);
char *anno_path_of(char *board);
int has_BM_perm(struct userec *user, struct boardmem *x);
int has_read_perm(struct userec *user, char *board);
int has_read_perm_x(struct userec *user, struct boardmem *x);
int hideboard(char *bname);
int hideboard_x(struct boardmem *x);
int innd_board(char *bname);
int political_board(char *bname);
int anony_board(char *bname);
int noadm4political(char *bname);
int has_post_perm(struct userec *user, struct boardmem *x);
int has_vote_perm(struct userec *user, struct boardmem *x);
struct boardmem *getbcache(char *board);
struct boardmem *getboard(char board[80]);
int findnextutmp(char *id, int from);
int send_msg(char *myuserid, int i, char *touserid, int topid, char *msg, int offline);
char *horoscope(int month, int day);
int life_special_web(char *id);
int count_life_value(struct userec *urec);
int modify_mode(struct user_info *x, int newmode);
int save_user_data(struct userec *x);
int is_bansite(char *ip);
int user_perm(struct userec *x, int level);
int useridhash(char *id);
int insertuseridhash(struct useridhashitem *ptr, int size, char *userid, int num);
int finduseridhash(struct useridhashitem *ptr, int size, char *userid);
int getusernum(char *id);
struct userec *getuser(char *id);
int checkuser(char *id, char *pw);
int count_id_num(char *id);
int count_online(void);
int count_online2(void);
int loadfriend(char *id);
int cmpfuid(unsigned *a, unsigned *b);
int initfriends(struct user_info *u);
int isfriend(char *id);
int loadbad(char *id);
int isbad(char *id);
int changemode(int mode);
char *encode_url(unsigned char *s);
char *noquote_html(unsigned char *s);
char *void1(unsigned char *s);
char *flag_str_bm(int access);
char *flag_str(int access);
char *flag_str2(int access, int has_read);
char *userid_str(char *s);
int fprintf2(FILE *fp, char *s);
char *getbfroma(char *path);
int set_my_cookie(void);
int has_fill_form(void);
int cmpboard(struct boardmem **b1, struct boardmem **b2);
int cmpboardscore(struct boardmem **b1, struct boardmem **b2);
int cmpboardinboard(struct boardmem **b1, struct boardmem **b2);
int cmpuser(struct user_info *a, struct user_info *b);
struct fileheader *findbarticle(struct mmapfile *mf, char *file, int *num, int mode);
char *utf8_decode(char *src);
void fdisplay_attach(FILE *output, FILE *fp, char *currline, char *nowfile);
void printhr(void);
void updatelastboard(void);
void updateinboard(struct boardmem *x);
int getlastpost(char *board, int *lastpost, int *total);
int updatelastpost(char *board);
int readuserallbrc(char *userid, int must);
void brc_update(char *userid);
int brc_initial(char *userid, char *boardname);
void brc_add_read(struct fileheader *fh);
void brc_add_readt(int t);
int brc_un_read(struct fileheader *fh);
void brc_clear(void);
int brc_un_read_time(int ftime);
void loaddenyuser(char *board);
void savedenyuser(char *board);
char *bbsred(char *command);
int max_mail_size(void);
int get_mail_size(void);
int check_maxmail(char *currmaildir);
int countln(char *fname);
double *system_load(void);
int setbmstatus(struct userec *u, int online);
int setbmhat(struct boardmanager *bm, int *online);
void add_uindex(int uid, int utmpent);
void remove_uindex(int uid, int utmpent);
int count_uindex(int uid);
int cachelevel(int filetime, int attached);
int dofilter(char *title, char *fn, int level);
int dofilter_edit(char *title, char *buf, int level);
int search_filter(char *pat1, char *pat2, char *pat3);
char *setbfile(char *buf, char *boardname, char *filename);
/* boardrc.c */
int readuserallbrc(char *userid, int must);
void brc_update(char *userid);
int brc_initial(char *userid, char *boardname);
void brc_add_read(struct fileheader *fh);
void brc_add_readt(int t);
int brc_un_read(struct fileheader *fh);
void brc_clear(void);
int brc_un_read_time(int ftime);
/* deny_users.c */
void loaddenyuser(char *board);
void savedenyuser(char *board);
/* bbsupdatelastpost.c */
int getlastpost(char *board, int *lastpost, int *total);
int updatelastpost(char *board);
/* bbsred.c */
char *bbsred(char *command);
