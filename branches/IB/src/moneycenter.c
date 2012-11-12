#include "bbs.h"
#include "bbstelnet.h"
#include <sys/mman.h>	// for mmap
#include <math.h>

#define MC_BOARD        "millionaires"
#define DIR_MC          MY_BBS_HOME "/etc/moneyCenter/"
#define DIR_MC_TEMP     MY_BBS_HOME "/bbstmpfs/dynamic/"
#define MC_ADMIN_FILE   DIR_MC "mc_admin"
#define MC_BOSS_FILE    DIR_MC "mc_boss"
#define MC_ASS_FILE     DIR_MC "mc_ass"
#define MC_RATE_FILE    DIR_MC "mc_rate"
#define MC_STOCK_FILE   DIR_MC "mc_stock"
#define MC_PRICE_FILE   DIR_MC "mc_price"
#define MC_DENY_FILE	DIR_MC "mc_deny"
#define MC_MARRYADMIN_FILE DIR_MC "mc_marryadmin"
#define MC_STOCK_BOARDS  DIR_MC "stockboards" //���а�������
#define MC_STOCK_STOPBUY  DIR_MC "st_stopbuy" //��ͣ���׵İ�������
#define DIR_CONTRIBUTIONS  DIR_MC "contributions/" //������ļ���������Ŀ¼
#define MC_JIJIN_CTRL_FILE  DIR_MC "jijin_ctrl" //�����ھ���еĻ���id

#define MAX_RECORD_LINE 100	//��¼�ļ�����󳤶�
#define MAX_BET_LENGTH  80	//��ע������󳤶�

//���Ҵ洢����
#define MONEY_NAME	"bmy_money"
#define CREDIT_NAME	"bmy_credit"
#define LEND_NAME       "lend_money"
#define INTEREST_NAME   "interest"

//���ֽ������
#define PRIZE_PER     3000000
#define MAX_POOL_MONEY 90000000
#define RUMOR_MONEY    500000
#define MAX_MONEY_NUM 500000000
#define MAX_CTRBT_NUM 1000000000
#define BIG_PRIZE             0.7
#define I_PRIZE               0.18
#define II_PRIZE              0.12
#define III_PRIZE             0.00
#define CMFT_PRIZE        20000
#define SALARY_I         10000000
#define SALARY_II        300000
#define SALARY_III       500000

#define MAX_STOCK_NUM 16
#define MAX_STOCK_NUM2 8
/*�޸�MAX_STOCK_NUM Ҫ�ǵ���money_stock_board������
  ��stock_name������Ӧ�Ĺ�Ʊ����*/

//�������ݽṹ������
/*
��������������:
�з�ѡ��"���"��Ů�����,��ʱ�Ǽǽ���¼��״̬Ϊ MAR_COURT��һ���ں��Զ�ʧ��
Ů��ȥ���ã��Զ���ʾ�Ƿ������飿
    ��������ܣ�����¼enable��Ϊ0,����ʧ��
	������ܣ�����¼״̬��Ϊ MAR_MARRYING �����,�������Ĭ����Ϊ1���
Ȼ����Ů�������Ե�����׼�����񣬰������ý�����ڣ�д�����������ñ�����
��ʱ�����ڲμӻ���ʱ�Ļ���ȼ����п���������¼��ʱ��һ���������Զ���ʼ
�����ǿ������μ����ǵĻ����ˣ��μ�����ʱ����������ͻ����ͺؿ�
��������Сʱ���Զ����������ŵ��������İ���
��MC_MARRY_RECORDS(100����¼)�б������COURT���ڻ�MARRYING�ļ�¼
��ˢ�¼�¼ʱ�����ʧ�ܺͽ��ɹ��ļ�¼ת��MC_MARRY_RECORDS_ALL

*/
#define DIR_MC_MARRY			MY_BBS_HOME"/etc/moneyCenter/marry"
#define MC_MARRY_RECORDS        MY_BBS_HOME"/etc/moneyCenter/marryrecords"
#define MC_MARRY_RECORDS_ALL	MY_BBS_HOME"/etc/moneyCenter/marryrecords_all"
#define MC_MARRIED_LIST 	MY_BBS_HOME"/etc/moneyCenter/marriedlist"
//Ĭ�����뺯
#define MC_MAEEY_INVITATION		MY_BBS_HOME"/0Announce/groups/GROUP_0/" MC_BOARD "/system/welcome/invitation"
//Ĭ�ϻ��񲼾�
#define MC_MAEEY_SET			MY_BBS_HOME"/0Announce/groups/GROUP_0/" MC_BOARD "/system/welcome/frontpage"
#define MAR_COURT		1	//���
#define MAR_MARRIED	2	//�ѻ�
#define MAR_MARRYING	3	//�����		//������marry_t��Сʱ�����
#define MAR_DIVORCE		4	//���
#define MAR_COURT_FAIL	5	//���ʧ��

struct MC_Marry{
	int enable;					//�Ƿ���Ч
	char bride[14];				//����
	char bridegroom[14];		//����
	int status;					//����״��MAR_...
	int giftmoney;				//���
	int attendmen;				//�μ�����
	time_t court_t;				//���ʱ��
	time_t marry_t;				//���ʱ��
	time_t divorce_t;			//���ʱ��
	char subject[60];			//������30����
	int setfile;			//�����õ���ʾ�ļ�	ʱ��ֵ
	int invitationfile;		//����ļ�	ʱ��ֵ
	int visitfile;			//������Ա�洢�ļ�
	int visitcount;			//�μ�����
	char unused[18];
}; // 150 bytes


extern struct UTMPFILE *utmpshm;
extern struct boardmem *bcache;
extern int numboards;
char marry_status[][20] = {"δ֪","���","�ѻ�","������","�����","���ʧ��",""}; 
time_t now_t;
int multex=0;

void *loadData(char *filepath, void *buffer, size_t filesize);
void saveData(void *buffer, size_t filesize);
static int loadValue(char *user, char *valueName, int sup);
static int saveValue(char *user, char *valueName, int valueToAdd, int sup);
int show_welcome(char *filepath,int startline,int endline);
static int shop_present(int order, char *kind, char *touserid);
static int buy_present(int order, char *kind, char *cardname, char *filepath, int price_per,char *touserid);

static void moneycenter_welcome(void);
static void moneycenter_byebye(void);
static int millionairesrec(char *title, char *str, char *owner);
static int limitValue(int value, int sup);
static int money_bank(void);
static int money_lottery(void);
static int money_shop(void);
static int money_check_guard(void);
static int money_dice(void);
static int money_robber(void);
static int money_killer(void);
static int money_stock(void);
static int money_stock_board(void);
//static int money_stock_board2(void);
//static int money_stock_change(void);//slow
static void money_show_stat(char *position);
static void nomoney_show_stat(char *position);
static int money_gamble(void);
static int money_777(void);
static int calc777(int t1, int t2, int t3);
static int guess_number(void);
static int an(char *a, char *b);
static int bn(char *a, char *b);
static void itoa(int i, char *a);
static void time2string(int num, char *str);
static int money_police(void);
static void persenal_stock_info(int stock_num[15], int stock_price[15],
				int money, char stockboard[STRLEN][MAX_STOCK_NUM],
				int stock_board[15]);
/*atic void persenal_stock_info2(int stock_num[15], int stock_price[15],
				int money, char *stockboard[],
				int stock_board[15]);*/
//static int shop_card_show(char *card[][2], int group);
//static int buy_card(char *cardname, int cardnumber);
static int forq(char *a, char *b);
static void p_gp(void);
static void russian_gun();
static void show_card(int isDealer, int c, int x);
static void money_cpu(void);
static int gp_win(void);
static int complex(char *cc, char *x, char *y);
static void money_suoha_tran(char *a, char *b, char *c);
static void money_suoha_check(char *p, char *q, char *r, char *cc);
static void show_style(int my, int cpu);
static int valid367Bet(char *buf);
static int make367Prize(char *bet, char *prizeSeq);
static void make367Seq(char *prizeSeq);
static int open_36_7();
static int validSoccerBet(char *buf);
static int computeSum(char *complexBet);
static void saveSoccerRecord(char *complexBet);
static int makeSoccerPrize(char *bet, char *prizeSeq);
static int open_soccer(char *prizeSeq);
static int makeInterest(int credit, char *valueName, float rate);
static int makeRumor(int num);
static int newSalary();
static int money_admin();
static void policereport(char *str);
static int money_cop();
static int check_allow_in();
static int money_beggar();
static void whoTakeCharge(int pos, char *boss);
static void whoTakeCharge2(int pos, char *boss);
static void sackOrAppoint(int pos, char *boss, int msgType, char *msg);
static void sackOrAppoint2(int pos, char *boss, int msgType, char *msg);
static int Allclubtest(char *id);
static int slowclubtest(char *board,char *id);
static int stop_buy();//slow
//���
static int money_marry();
static int PutMarryRecord(struct MC_Marry *marryMem, int n, struct MC_Marry *new_mm);
static int marry_attend(struct MC_Marry *marryMem, int n);
static int marry_court(struct MC_Marry *marryMem, int n);
static int marry_perpare(struct MC_Marry *marryMem, int n);
static int marry_divorce();
char *get_date_str(time_t *tt);
char *get_simple_date_str(time_t *tt);
static int marry_refresh(struct MC_Marry *marryMem, int n);
static int marry_recordlist(struct MC_Marry *marryMem, int n);
static int marry_all_records();
static int marry_active_records(struct MC_Marry *marryMem, int n);
static int marry_query_records(char *id);
static int marry_admin(struct MC_Marry *marryMem, int n);
//������
static int money_deny();
static int mc_addtodeny(char *uident, char *msg, int ischange);
static int mc_denynotice(int action, char *user, char *msgbuf);
static int mc_autoundeny(void);
static int addstockboard(char *sbname, char *fname);
static int delstockboard(char *sbname, char *fname);
static int stockboards();
//static int calc_ticket_price();

static int money_office();

static void 
showAt(int line, int col, char *str, int flag) 
{
	move(line, col);
	prints("%s", str);
	if (flag == 1)
		pressanykey();
	else if (flag == 2)
		pressreturn();
}

int
moneycenter()  //moneycenter�������
{
	int ch;
	int quit = 0;
	modify_user_mode(MONEY);
	strcpy(currboard, MC_BOARD);
	if (!file_exist(DIR_MC"MoneyValues"))
		mkdir(DIR_MC"MoneyValues", 0770);
	if (!file_exist(DIR_CONTRIBUTIONS))
		mkdir(DIR_CONTRIBUTIONS, 0770);	
	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
		&& !(currentuser.userlevel & PERM_SYSOP)
		&& strcmp(currentuser.userid, "macintosh")) 
		if (utmpshm->mc.isMCclosed){
			clear();
			move(6, 4);
			prints("\033[1;31m����ٸ�������Ľ�����Ϣ\033[m");
			pressanykey();
			return 0;
		}			
	moneycenter_welcome();
	multex = loadValue(currentuser.userid, "multex", 9);
	if (!check_allow_in()) 
		return 0;
	saveValue(currentuser.userid,"multex",1, 9);

	if (seek_in_file(DIR_MC "jijin", currentuser.userid))
       {
		money_bank();
		moneycenter_byebye();
		return 0;
	}
	clear();
	while (!quit) {
		nomoney_show_stat("����ٸ��������");
		move(6, 4);
		prints("����ٸ�������ľ������죬��Ȼһ�£���ӭ����������");
		move(8, 4);
		prints("ѧϰ������Ϸ������ȥ��վϵͳ��millionaires�档");
		move(t_lines - 2, 0);
		prints("\033[1;44m ѡ \033[1;46m [1]���� [2]��Ʊ [3]�ĳ� [4]�ڰ� [5]ؤ�� [6]���� [7]�̳� [8]����            \n"
			   "\033[1;44m �� \033[1;46m [9]ɱ�� [C]���� [A]���̹������� [Q]�뿪                                                 ");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_bank();
			break;
		case '2':
			money_lottery();
			break;
		case '3':
			money_gamble();
			break;
		case '4':
			money_robber();
			break;
		case '5':
			money_beggar();
			break;
		case '6':
			money_stock();
			break;
		case '7':
			money_shop();
			break;
		case '8':
			money_cop();
			break;
		case '9':
			money_killer();
			break;
		case '0':
			money_admin();	//���أ���Ȩ�޼��
			break;
		case 'c':
		case 'C':
			money_marry(); //added by macintosh 20051203
			break;
		case 'a':
		case 'A':
			money_office(); //added by macintosh 20071105
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	moneycenter_byebye();
	return 0;
}  // moneycenter�������

static void
moneycenter_welcome()
{
	clear();
	move(4, 4);
	prints("����ٸ���������ſ�����һ������ӣ�");
	move(6, 4);
	prints("\033[1;31m���Ͷ������\033[m \033[1;31m�ȶ���������\033[m");
	move(8, 4);
	prints("\033[1;33m�������Ļ�Ȼһ�£�����������\033[0m");
	pressanykey();
} // ��ӭ����

static void
moneycenter_byebye()
{
	clear();
	saveValue(currentuser.userid, "multex", -9,9);
	move(5, 14);
	prints("\033[1;32m��ӭ�ٴι��ٽ������ģ����ĸ��������ǵ����ҡ�\033[m");
	pressanykey();
} //�뿪����

//added by macintosh 20051202
static int
millionairesrec(char *title, char *str, char *owner)
{
	struct fileheader postfile;
	char filepath[STRLEN], fname[STRLEN];
	char buf[256];
	time_t now;
	FILE *inf, *of;

	now = time(0);
	sprintf(fname, "tmp/deliver.millionairesrec.%d", (int)now);
	if ((inf = fopen(fname, "w")) == NULL) 
		return -1;
	fprintf(inf, "%s", str);
	fclose(inf);
	
	//postfile(fname, owner, "millionairesrec", title);
	memset(&postfile, 0, sizeof (postfile));
	sprintf(filepath, MY_BBS_HOME "/boards/%s/", "millionairesrec");
	now = trycreatefile(filepath, "M.%d.A", now, 100);
	if (now < 0)
		return -1;
	postfile.filetime = now;
	postfile.thread = now;
	fh_setowner(&postfile, owner[0] ? owner : "millionaires", 0);
	strsncpy(postfile.title, title, sizeof (postfile.title));

	//getcross(filepath, fname, "millionairesrec", title);
	now = time(0);
	inf = fopen(fname, "r");
	if (inf == NULL)
		return -2;
	of = fopen(filepath, "w");
	if (of == NULL) {
		fclose(inf);
		return -3;
	}
	fprintf(of, "������: %s (����ٸ����ϵͳ��¼), ����: millionairesrec\n", owner[0] ? owner : "millionaires");
	fprintf(of, "��  ��: %s\n", title);
	fprintf(of, "����վ: ����ٸBBS (%24.24s), ��վ(bbs.xjtu.edu.cn)\n\n", ctime(&now));
	fprintf(of, "����ƪ�����ɱ���ٸ�����Զ�����ϵͳ����\n\n");
	while (fgets(buf, 256, inf) != NULL)
		fprintf(of, "%s", buf);
	fprintf(of, "\n\n");
	fprintf(of, "�������IP: %s\n\n", currentuser.lasthost);
	fclose(inf);
	fclose(of);

	sprintf(buf, MY_BBS_HOME "/boards/%s/%s", "millionairesrec", DOT_DIR);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		//errlog("Posting '%s' on '%s': append_record failed!", postfile.title, "millionairesrec");
		return 0;
	}
	updatelastpost("millionairesrec");
	unlink(fname);
	return 1;
}//����ϵͳ��¼������

static int
limitValue(int value, int sup)
{
	if (value > sup)
		return sup;
	if (value < 0)
		return 0;
	return value;
}  

static int
savemoneyvalue(char *userid, char *key, char *value)
{
	char path[256];
	sprintf(path, DIR_MC"MoneyValues/%s", userid);
	return savestrvalue(path, key, value);
}

static int
readmoneyvalue(char *userid, char *key, char *value, int size)
{
	char path[256];
	sprintf(path, DIR_MC"MoneyValues/%s", userid);
	return readstrvalue(path, key, value, size);
}

static int
loadValue(char *user, char *valueName, int sup)
{
	char value[20];
	if (readmoneyvalue(user, valueName, value, 20) != 0)
		return 0;
	else
		return limitValue(atoi(value), sup);
}  //��ȡ�����ֵ

static int
saveValue(char *user, char *valueName, int valueToAdd, int sup)
{
	int valueInt;
	int retv;
	char value[20];
	valueInt = loadValue(user, valueName, sup);
	valueInt += valueToAdd;
	valueInt = limitValue(valueInt, sup);
	snprintf(value, 20, "%d", valueInt);
	if ((retv = savemoneyvalue(user, valueName, value)) != 0) {
		errlog("save %s %s %s retv=%d err=%s", currentuser.userid,
		       valueName, value, retv, strerror(errno));
	}
	return 0;
}  //���������ֵ

void *loadData(char *filepath, void *buffer, size_t filesize) {
	int fd;

	if ((fd = open(filepath, O_RDWR, 0660)) == -1)
		return (void *)-1;
	buffer = mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd , 0);
	close(fd);
	return buffer;
}

void saveData(void *buffer, size_t filesize) {
	if (buffer != NULL)
		munmap(buffer, filesize);
}

static void // ����ְ��ϵͳ
whoTakeCharge(int pos, char *boss)
{
	const char feaStr[][20] =
	    { "bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office"
	};
	if (readstrvalue(MC_BOSS_FILE, feaStr[pos - 1], boss, IDLEN + 1) != 0)
		*boss = '\0';
}
static void//slowaction ����ϵͳ
whoTakeCharge2(int pos, char *boss)
{
	const char feaStr[][20] =
	    { "bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office"
	};
	if (readstrvalue(MC_ASS_FILE, feaStr[pos - 1], boss, IDLEN + 1) != 0)
		*boss = '\0';
}

static int //������Ȩ��
check_allow_in()  
{
	int backTime;
	int freeTime;
	int currentTime = time(0);
	int num,money;
	int robTimes;

	mc_autoundeny();	
	if (seek_in_file(MC_DENY_FILE, currentuser.userid)){
		clear();
		move(10, 10);
		prints("���Ѿ������벻�ܴ��̻�ӭ����������Ǹ\n");
		pressanykey();
		return 0;
	}

	/* ����ര��*/
	if (multex && count_uindex_telnet(usernum) > 1) {
		clear();
		move(10, 10);
		prints("���Ѿ��ڽ�����������!\n");
		pressanykey();
		return 0;
	}
	set_safe_record();
	if (currentuser.dietime > 0) {
		clear();
		move(10, 10);
		prints("���Ѳ����˼䣬���ǲ��������~~\n");
		pressanykey();
		return 0;
	}
	
	/* ���ﱻ��� */
	freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
	if (currentTime < freeTime) {
		clear();
		move(10, 10);
        if((freeTime - currentTime) / 86400>50)
         saveValue(currentuser.userid, "freeTime",
					  time(0) + 86400 *1,
					  2000000000);
	prints("���Ѿ�������ٸ�������ˡ�����%d��ļ����\n",
	       (freeTime - currentTime) / 86400);
	num=((freeTime - currentTime) / 86400)*25000+25000;
	sprintf(genbuf, "�Ƿ�Ҫ���ͣ����ͽ�%d",num);
	if (askyn(genbuf, NA, NA) == YEA) {
		money =	loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		 if (money < num) {
			move(8, 4);
			prints ("����Ǯ���������� %d ����ٸ��", num);
			pressanykey();
			return 0;
		 	}
		 else {
		 	saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
		  	saveValue("BMYpolice", MONEY_NAME, +num, MAX_MONEY_NUM);
			move(8, 4);
			prints("���ͳɹ�");
			robTimes = loadValue(currentuser.userid, "rob", 50);
                	saveValue(currentuser.userid, "rob", -robTimes, 50);
			saveValue(currentuser.userid, "freeTime", -2000000000, 2000000000);
			del_from_file(DIR_MC "imprison_list", currentuser.userid);
		 	} 
		}else{
		pressanykey();
		return 0;
		}
	} else 
	if (currentTime > freeTime && freeTime > 0) {
		clear();
		move(10, 10);
		prints("�����������ϲ�����»�����ɣ�");
		saveValue(currentuser.userid, "freeTime", -2000000000, 2000000000);
		del_from_file(DIR_MC "imprison_list", currentuser.userid);
		pressanykey();
		}

	/* Ƿ��� */
	int total_num, lendMoney;
	backTime = loadValue(currentuser.userid, "back_time", 2000000000);
	if((backTime - (int) time(0)) / 3600>5000)
	saveValue(currentuser.userid, "back_time",
					  time(0) + 1* 86400,
					  2000000000);
	lendMoney = loadValue(currentuser.userid, LEND_NAME,
				  		  MAX_MONEY_NUM);
	if (backTime < 0 || lendMoney < 0 ) {
		saveValue(currentuser.userid, LEND_NAME, -lendMoney, MAX_MONEY_NUM); 
		saveValue(currentuser.userid, "lend_time", -2000000000, 2000000000); 
		saveValue(currentuser.userid, "back_time", -2000000000, 2000000000);
		}
	if (currentTime > backTime && backTime > 0) {
		clear();
		move(10, 10);
		if (askyn("��Ƿ���еĴ�����ˣ��Ͻ����ɣ�", YEA, NA) == YEA) {
			total_num =
			    lendMoney + makeInterest(lendMoney, "lend_time",
						     utmpshm->mc.lend_rate/10000.0);
			money = loadValue(currentuser.userid, MONEY_NAME,
					  MAX_MONEY_NUM);
			if (money < total_num) {
				move(11, 10);
				prints("���Ǯ�����������");
				pressanykey();
				return 0;
			}
			saveValue(currentuser.userid,
				  MONEY_NAME, -total_num, MAX_MONEY_NUM);
			saveValue(currentuser.userid, LEND_NAME,
				  -MAX_MONEY_NUM, MAX_MONEY_NUM);
			saveValue(currentuser.userid,
				  "lend_time", -2000000000, 2000000000);
			saveValue(currentuser.userid,
				  "back_time", -2000000000, 2000000000);
			del_from_file(DIR_MC "special_lend",
				      currentuser.userid);
			move(12, 10);
			prints("���ˣ���������ծһ��������");
			pressanykey();
			return 1;
		}
		return 0;
	}

	/* �������ý���������� */

	return 1;
}

static int  //������Ϣ
makeInterest(int basicMoney, char *valueName, float rate)
{				
	int lastTime;
	int day;
	int maxDay;

	if (basicMoney <= 0 || rate <= 0) {
		return 0;
	}
	maxDay = MAX_MONEY_NUM / (1 + rate * basicMoney);

	lastTime = loadValue(currentuser.userid, valueName, 2000000000);
	if (lastTime > 0 && time(0) > lastTime) {
		day = (time(0) - lastTime) / 86400;
		day = limitValue(day, maxDay);
		return basicMoney * rate * day;
	}
	return 0;
}

static int
makeRumor(int num)
{
	if (random() % 3) {
		num += (random() % num) / 5;
	} else {
		num -= (random() % num) / 5;
	}
	return limitValue(num, MAX_MONEY_NUM);
}

static void
time2string(int num, char *str)
{
	int i;
	for (i = 0; num > 0; i++, num /= 10) {
		str[9 - i] = num % 10 + '0';
	}
	str[10] = '\0';
}

static int //�����Ƿ���ȡнˮ��ʱ��
newSalary()
{
	char lastSalaryTime[20];
	return 1;//��ʱ����

	if (!readstrvalue(DIR_MC "etc_time", "salary_time", lastSalaryTime, 20)) {
		if (time(0) < atoi(lastSalaryTime) + 30 * 86400) 
			return 0;
		return 1;
/*
		time2string(time(0), genbuf);
		if (savestrvalue(DIR_MC "etc_time", "salary_time", genbuf) == 0) {
			return 1;
		}
		return 0;
*/
	}
	return 0;
}

static int //����нˮ
makeSalary()
{
	if (currentuser.userlevel & PERM_SYSOP) {
                return SALARY_I;
		}
	if (currentuser.userlevel & PERM_BOARDS) {
		return SALARY_III;
		}
	if (currentuser.userlevel & PERM_OBOARDS ||
            currentuser.userlevel & PERM_ACCOUNTS ||
            currentuser.userlevel & PERM_ARBITRATE ||
            currentuser.userlevel & PERM_SPECIAL7 ||
            currentuser.userlevel & PERM_ACBOARD) {
                return SALARY_II;
		}
	return 0;
}

static void //ְ������ϵͳ
sackOrAppoint(int pos, char *boss, int msgType, char *msg)
{

	char head[10];
	char in[10];
	char end[10];
	char posDesc[][40] =
	    { "����ٸ�����г�", "����ٸ���ʹ�˾����", "����ٸ�ĳ�����",
		"����ٸ�ڰ����", "����ٸؤ�����", "����ٸ֤�����ϯ",
		"����ٸ�̳�����", "����ٸ������","����ٸɱ�ְ���",
		"����ٸ��������칫������", "����ٸ���̹�����������"
	};
	if (msgType == 0) {
		strcpy(head, "����");
		strcpy(in, "Ϊ");
		strcpy(end, "");
	} else {
		strcpy(head, "��ȥ");
		strcpy(in, "��");
		strcpy(end, "ְ��");
	}
	sprintf(msg, "%s %s %s%s%s", head, boss, in, posDesc[pos - 1], end);

}
static void //��������ϵͳ
sackOrAppoint2(int pos, char *boss, int msgType, char *msg)
{

	char head[10];
	char in[10];
	char end[10];
	char posDesc[][40] =
	    { "����ٸ�����г�����", "����ٸ���ʹ�˾��������", "����ٸ�ĳ���������",
		"����ٸ�ڰ��������", "����ٸؤ���������", "����ٸ֤�����ϯ����",
		"����ٸ�̳���������", "����ٸ����������","����ٸɱ�ְ�������",
		"����ٸ��������칫����������", "����ٸ���̹�����������"
	};
	if (msgType == 0) {
		strcpy(head, "����");
		strcpy(in, "Ϊ");
		strcpy(end, "");
	} else {
		strcpy(head, "��ȥ");
		strcpy(in, "��");
		strcpy(end, "ְ��");
	}
	sprintf(msg, "%s %s %s%s%s", head, boss, in, posDesc[pos - 1], end);

}

static int //����ϵͳ
money_bank()
{
	int ch;
	int quit = 0;
	int num, money, credit, total_num;
	char uident[IDLEN + 1];
	char title[80];
	char buf[100], letter[256];
	int convert_rate;
	int lendTime;
	int lendMoney, salary;
	int inputValid, withdrawAll;
	float transfer_rate, deposit_rate, lend_rate;
	double backTime;

	while (!quit) {
		money_show_stat("����ٸ����");
		move(8, 16);
		prints("����ٸ���л�ӭ���Ĺ��٣�");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��Ǯ [2]ת�� [3]���� [4]���� [5]���� [6]�г��칫�� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			money_show_stat("����ٸ���жһ�����");
//			convert_rate = utmpshm->mc.ave_score / 50;
			convert_rate = 100;
			move(4, 4);
			prints
			    ("������ͨ��������������ñ���ٸ�ҡ�����Ļ����� 1:%d",
			     convert_rate);
			move(5, 4);
			prints("\033[1;31mע��:������һ�������Ų��˻�!\033[0m");
			getdata(6, 4, "��Ҫ����������������[0]: ", genbuf, 7,
				DOECHO, YEA);
			num = atoi(genbuf);
			if (num <= 0) {
				break;
			}
			move(7, 4);
			sprintf(genbuf,
				"ȷ��Ҫ���� %d ����������ȡ %d ����ٸ����",
				num, num * convert_rate);
			if (askyn(genbuf, NA, NA) == YEA) {
				set_safe_record();
				if (currentuser.numposts < num) {
					move(8, 4);
					prints("��û����ô��������...");
					pressanykey();
					break;
				}
				currentuser.numposts -= num;
				substitute_record(PASSFILE, &currentuser,
						  sizeof (currentuser),
						  usernum);
				saveValue(currentuser.userid, MONEY_NAME,
					  num * convert_rate, MAX_MONEY_NUM);
				move(8, 4);
				prints("���׳ɹ������������� %d ����ٸ�ҡ�",
				       num * convert_rate);
				sprintf(genbuf, "%s�������н���(������)", currentuser.userid);
				sprintf(buf, "%s����%d������, ���� %d����ٸ��", currentuser.userid, num, num * convert_rate);
				millionairesrec(genbuf, buf, "���н���");				
				pressanykey();
			}
			break;
		case '2':
			money_show_stat("����ٸ����ת�˴���");
			move(4, 4);
            if(utmpshm->mc.transfer_rate == 0){
                // ����bbsd�����¶�ȡת�������ѵ��ڴ��� by IronBlood@bmy 20120118
                char tmp_transfer_rate[512];
                readstrvalue(MC_RATE_FILE, "transfer_rate", tmp_transfer_rate, sizeof(512));
                utmpshm->mc.transfer_rate = atoi(tmp_transfer_rate);            
            }
			transfer_rate = utmpshm->mc.transfer_rate / 10000.0;
			sprintf(genbuf,
				"��Сת�˽�� 1000 ����ٸ�ҡ������� %.2f���������ȡ 100000 ����ٸ�ң�",
				transfer_rate * 100);
			prints("%s", genbuf);
			move(5, 4);
			usercomplete("ת�˸�˭��", uident);
			if (uident[0] == '\0')
				break;
			if (!getuser(uident)) {
				move(6, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				move(6, 4);
				prints("�����Ǯֻ���ղ��ܸ�����...");
				pressreturn();
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				if (seek_in_file(DIR_MC "jijin", currentuser.userid));
				else if (!seek_in_file(DIR_MC "mingren", uident)) {
					move(6, 4);
					prints
					    ("�Բ������в�������������ת�ʡ�");
					pressreturn();
					break;
				}
			}
			getdata(6, 4, "ת�˶��ٱ���ٸ�ң�[0]", buf, 10,
				DOECHO, YEA);
			if (buf[0] == '\0') {
				break;
			}
			num = atoi(buf);
			if (num < 1000) {
				move(7, 4);
				prints("�Բ���δ�ﵽ��С���׽�");
				pressanykey();
				break;
			}
			if (currentuser.stay < 86400) {
				move(7, 4);
				prints
				    ("�Բ������в���δ�������ṩת��ҵ��");
				pressanykey();
				break;
			}
			move(7, 4);
			sprintf(genbuf, "ȷ��ת�˸� %s %d ����ٸ����", uident,
				num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				if (num * transfer_rate >= 100000) {
					total_num = num + 100000;
				} else {
					total_num = num * (1.0 + transfer_rate);
				}
				if (money < total_num) {
					move(8, 4);
					prints
					    ("����Ǯ�������������Ѵ˴ν��׹��� %d ����ٸ��",
					     total_num);
					pressanykey();
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME,
					  -total_num, MAX_MONEY_NUM);
				saveValue(uident, MONEY_NAME, num,
					  MAX_MONEY_NUM);

				char notebuf[512];
		 		char note[3][STRLEN];
				int i, j;
				move(9, 0);
				prints("����ʲô��Ҫ������[����д3��]");
				bzero(note, sizeof (note));
				for (i = 0; i < 3; i++) {
					getdata(10 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
					if (note[i][0] == '\0')
						break;
				}
		 		move(15, 4);
				prints("ת�ʳɹ��������Ѿ�֪ͨ���������ѡ�");
				sprintf(title, "�������� %s ������Ǯ����",
					currentuser.userid);
				sprintf(notebuf,
					"�������� %s ͨ������ٸ���и���ת���� %d ����ٸ�ң�����ա�"
					"\n\n������ %s �ĸ���:\n",
					currentuser.userid, num, currentuser.userid);
				for (j = 0; j < i; j++){
					strcat(notebuf, note[j]);
					strcat(notebuf, "\n");
				}
				mail_buf(notebuf, uident, title);
				if (seek_in_file(DIR_MC "mingren", currentuser.userid))
				{
					sprintf(title, "%s �� %s ת��", currentuser.userid, uident);
					sprintf(buf, " %s ͨ������ٸ������ %s ת���� %d ����ٸ��", currentuser.userid, uident, num);
					mail_buf(buf, "millionaires", title);
				}
				if (num >= RUMOR_MONEY && random() % 2) {
					sprintf(genbuf,
						"��˵ %s �յ���һ�� %d ����ٸ�ҵ�ת�ʣ�",
						uident, makeRumor(num));
					deliverreport
					    ("[ҥ��]���Ա���ٸ���е���Ϣ",
					     genbuf);
				}
				sprintf(genbuf, "%s�������н���(ת��)", currentuser.userid);
				sprintf(buf,"%sת�ʸ�%s %d����ٸ��", currentuser.userid, uident, num);
				millionairesrec(genbuf, buf, "���н���");
				pressanykey();
			}
			break;
		case '3':
			clear();
			money_show_stat("����ٸ���д����");
			move(4, 4);
            if(utmpshm->mc.deposit_rate == 0){
                // ����bbsd�����¶�ȡ������ʵ��ڴ��� by IronBlood@bmy 20120118
                char tmp_deposit_rate[512];
                readstrvalue(MC_RATE_FILE, "deposit_rate", tmp_deposit_rate, sizeof(512));
                utmpshm->mc.deposit_rate = atoi(tmp_deposit_rate);
            }
			deposit_rate = utmpshm->mc.deposit_rate / 10000.0;
			sprintf(genbuf,
				"��С��ȡ���� 1000 ����ٸ�ҡ�������ʣ��գ�Ϊ %.2f��",
				deposit_rate * 100);
			prints("%s", genbuf);
			move(t_lines - 1, 0);
			prints
			    ("\033[1;44m ѡ�� \033[1;46m [1]��� [2]ȡ�� [Q]�뿪\033[m");
			ch = igetkey();
			switch (ch) {
			case '1':
				getdata(6, 4, "��Ҫ����ٱ���ٸ��?[0]", buf,
					10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);
				if (num < 1000) {
					move(7, 4);
					prints("�Բ���δ�ﵽ��С���׽�");
					pressanykey();
					break;
				}
				move(7, 4);
				sprintf(genbuf, "ȷ���� %d ����ٸ����?", num);
				if (askyn(genbuf, NA, NA) == NA) {
					break;
				}
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				credit =
				    loadValue(currentuser.userid, CREDIT_NAME,
					      MAX_MONEY_NUM);
				if (money < num) {
					move(8, 4);
					prints("��û����ô��Ǯ���Դ档");
					pressanykey();
					break;
				}
				if (credit + num > MAX_MONEY_NUM) {
					move(8, 4);
					prints("��������ô������ʲô�أ�");
					pressanykey();
					break;
				}
				/* ��Ǯ */
				saveValue(currentuser.userid, MONEY_NAME, -num,
					  MAX_MONEY_NUM);
				/* ����ԭ�ȴ�����Ϣ */
				saveValue(currentuser.userid, INTEREST_NAME,
					  makeInterest(credit, "deposit_time",
						       deposit_rate),
					  MAX_MONEY_NUM);
				/* ��� */
				saveValue(currentuser.userid, CREDIT_NAME,
					  num, MAX_MONEY_NUM);
				saveValue(currentuser.userid,
					  "deposit_time", -2000000000,
					  2000000000);
				/* �µĴ�ʼʱ�� */
				saveValue(currentuser.userid, "deposit_time",
					  time(0), 2000000000);
				move(8, 4);
				prints
				    ("���׳ɹ��������ڴ��� %d ����ٸ�ң���Ϣ���� %d ����ٸ�ҡ�",
				     loadValue(currentuser.userid, CREDIT_NAME,
					       MAX_MONEY_NUM),
				     loadValue(currentuser.userid,
					       INTEREST_NAME, MAX_MONEY_NUM));
				if (num >= RUMOR_MONEY && random() % 2) {
					sprintf(genbuf,
						"����Ŀ�� %s �ڱ���ٸ���д����� %d �ı���ٸ�ң�",
						currentuser.userid,
						makeRumor(num));
					deliverreport
					    ("[ҥ��]���Ա���ٸ���е���Ϣ",
					     genbuf);
				}
				pressanykey();
				break;
			case '2':
				getdata(6, 4, "��Ҫȡ���ٱ���ٸ��?[0]", buf,
					10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);
				if (num < 1000) {
					move(7, 4);
					prints("�Բ���δ�ﵽ��С���׽�");
					pressanykey();
					break;
				}
				move(7, 4);
				sprintf(genbuf, "ȷ��ȡ %d ����ٸ����?", num);
				if (askyn(genbuf, NA, NA) == NA) {
					break;
				}
				credit =
				    loadValue(currentuser.userid, CREDIT_NAME,
					      MAX_MONEY_NUM);
				if (num > credit) {
					move(8, 4);
					prints("��û����ô���");
					pressanykey();
					break;
				}
				withdrawAll = 0;
				total_num = num;
				if (num == credit) {
					move(8, 4);
					sprintf(genbuf,	"�Ƿ�һ��ȡ�� %d ����ٸ�ҵĴ����Ϣ��",
						loadValue(currentuser.userid, INTEREST_NAME, MAX_MONEY_NUM)
                        + makeInterest(num, "deposit_time", deposit_rate));
					if (askyn(genbuf, NA, NA) == YEA) {
						/* ������Ϣ */
						total_num =
						    num + makeInterest(num,
								       "deposit_time",
								       deposit_rate)
						    +
						    loadValue(currentuser.
							      userid,
							      INTEREST_NAME,
							      MAX_MONEY_NUM);
						withdrawAll = 1;
					}
				}

				credit =
				    loadValue(currentuser.userid, CREDIT_NAME,
					      MAX_MONEY_NUM);
				if (num > credit) {
					move(9, 4);
					prints("��û����ô���");
					pressanykey();
					break;
				}
				/* ��ȥȡ�� */
				saveValue(currentuser.userid, CREDIT_NAME,
					  -num, MAX_MONEY_NUM);
				/* ȡ���ֽ� */
				saveValue(currentuser.userid, MONEY_NAME,
					  total_num, MAX_MONEY_NUM);
				/* ������ȡ��Ǯ����Ϣ */
				if (withdrawAll) {
					saveValue(currentuser.userid,
						  INTEREST_NAME, -MAX_MONEY_NUM,
						  MAX_MONEY_NUM);
				} else {
					saveValue(currentuser.userid,
						  INTEREST_NAME,
						  makeInterest(num,
							       "deposit_time",
							       deposit_rate),
						  MAX_MONEY_NUM);
				}
				move(8, 4);
				prints
				    ("���׳ɹ��������ڴ��� %d ����ٸ�ң������Ϣ���� %d ����ٸ�ҡ�",
				     loadValue(currentuser.userid, CREDIT_NAME,
					       MAX_MONEY_NUM),
				     loadValue(currentuser.userid,
					       INTEREST_NAME, MAX_MONEY_NUM));
				pressanykey();
				break;
			case 'Q':
			case 'q':
				break;
			}
			break;
		case '4':
			clear();
			money_show_stat("����ٸ���д����");
			move(4, 4);
            if(utmpshm->mc.lend_rate == 0){
                // ����bbsd�����¶�ȡ�������ʵ��ڴ��� by IronBlood@bmy 20120118
                char tmp_lend_rate[512];
                readstrvalue(MC_RATE_FILE, "lend_rate", tmp_lend_rate, sizeof(512));
                utmpshm->mc.lend_rate = atoi(tmp_lend_rate);            
            }
			lend_rate = utmpshm->mc.lend_rate / 10000.0;
			sprintf(genbuf,
				"��С���׽�� 1000 ����ٸ�ҡ��������ʣ��գ�Ϊ %.2f��",
				lend_rate * 100);
			prints("%s", genbuf);
			move(5, 4);
			lendMoney =
			    loadValue(currentuser.userid, LEND_NAME,
				      MAX_MONEY_NUM);
			total_num =
			    lendMoney + makeInterest(lendMoney, "lend_time",
						     lend_rate);
			lendTime =
			    loadValue(currentuser.userid, "lend_time",
				      2000000000);
			if (lendTime > 0) {
				sprintf(genbuf,
					"������ %d ����ٸ�ң���ǰ��Ϣ���� %d ����ٸ�ң��ൽ�� %d Сʱ��",
					lendMoney,
					total_num,
					(loadValue
					 (currentuser.userid, "back_time",
					  2000000000) - (int) time(0)) / 3600);
			} else {
				sprintf(genbuf, "��Ŀǰû�д��");
			}
			prints("%s", genbuf);
			move(t_lines - 1, 0);
			prints
			    ("\033[1;44m ѡ�� \033[1;46m [1]���� [2]���� [Q]�뿪\033[m");
			ch = igetkey();
			switch (ch) {
			case '1':
				move(6, 4);
				sprintf(genbuf,
					"�������еĹ涨����Ŀǰ������������� %d ����ٸ�ҡ�",
					countexp(&currentuser) * 100);
				prints("%s", genbuf);
				getdata(7, 4, "��Ҫ������ٱ���ٸ��?[0]", buf,
					10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);

				if (lendMoney > 0) {
					move(8, 4);
					prints("���Ȼ�����");
					pressanykey();
					break;
				}
				if (num < 1000) {
					move(8, 4);
					prints("�Բ���δ�ﵽ��С���׽�");
					pressanykey();
					break;
				}
				if (num > countexp(&currentuser) * 100) {
					move(8, 4);
					prints
					    ("�Բ�����Ҫ�����Ľ������й涨��");
					pressanykey();
					break;
				}
				inputValid = 0;
				while (!inputValid) {
					getdata(8, 4,
						"��Ҫ��������죿[3-30]: ", buf,
						3, DOECHO, YEA);
					if (atoi(buf) > 2 && atoi(buf) < 31) {
						inputValid = 1;
					}
				}
				saveValue(currentuser.userid, MONEY_NAME, num,
					  MAX_MONEY_NUM);
				saveValue(currentuser.userid, LEND_NAME, num,
					  MAX_MONEY_NUM);
				saveValue(currentuser.userid, "lend_time",
					  time(0), 2000000000);
				saveValue(currentuser.userid, "back_time",
					  time(0) + atoi(buf) * 86400,
					  2000000000);
				move(9, 4);
				prints("���Ĵ��������Ѿ���ɡ��뵽�ڻ��");
				sprintf(genbuf, "%s�������н���(����)", currentuser.userid);
				sprintf(buf, "%s����%d����ٸ��%d��", currentuser.userid, num, atoi(buf));
				millionairesrec(genbuf, buf, "���н���");
				pressanykey();
				break;
			case '2':
				move(6, 4);
				backTime =
				    loadValue(currentuser.userid, "back_time", 2000000000);
				if((backTime - (int) time(0)) / 3600>5000||(backTime - (int) time(0)) / 3600<-30)
                			saveValue(currentuser.userid, "back_time", time(0) + 1* 86400, 2000000000);
				lendTime =
				    loadValue(currentuser.userid, "lend_time", 2000000000);
				if (lendTime == 0) {
					prints("���Ǵ��˰ɣ�û���ҵ����Ĵ����¼����");
					pressanykey();
					break;
				}
				if (time(0) - lendTime < 86400) {
					prints ("�Բ������в�����δ������Ϣ�Ļ�����");
					pressanykey();
					break;
				}
				if (askyn("��Ҫ���ڳ���������", NA, NA) == YEA) {
					money = loadValue(currentuser.userid,
						      MONEY_NAME,
						      MAX_MONEY_NUM);
					move(7, 4);
					if (money < total_num) {
						prints ("�Բ�������Ǯ�����������");
						pressanykey();
						break;
					}
					saveValue(currentuser.userid,
						  MONEY_NAME, -total_num,
						  MAX_MONEY_NUM);
					saveValue(currentuser.userid, LEND_NAME,
						  -MAX_MONEY_NUM,
						  MAX_MONEY_NUM);
					saveValue(currentuser.userid,
						  "lend_time", -2000000000,
						  2000000000);
					saveValue(currentuser.userid,
						  "back_time", -2000000000,
						  2000000000);
					del_from_file(DIR_MC "special_lend",
						      currentuser.userid);
					sprintf(genbuf, "%s�������н���(����)", currentuser.userid);
					sprintf(buf,"%s�����������%d����ٸ��", currentuser.userid, total_num);
					millionairesrec(genbuf, buf, "���н���");
					prints
					    ("���Ĵ����Ѿ����塣�����ּ����������ĳ��š�");
					pressanykey();
				}
				break;
			case 'q':
			case 'Q':
				break;
			}
			break;
		case '5':
			clear();
			money_show_stat("����ٸ���й��ʴ��촰��");
			salary = makeSalary();
			if (salary == 0) {
				move(10, 10);
				prints("�Բ��������Ǳ�վ����Ա��û�й��ʡ�");
				pressanykey();
				break;
			}
			if (utmpshm->mc.isSalaryTime == 0) {
				move(10, 10);
				prints
				    ("�Բ������л�û���յ����ʻ���������������");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "salary_list", currentuser.userid)) {
				move(10, 10);
				prints("���Ѿ�������¹��ʡ������ڷܹ����ɣ�");
				pressanykey();
				break;
			}
			move(6, 4);
			sprintf(genbuf, "�����µĹ��� %d ����ٸ���Ѿ��������С�������ȡ��",	salary);
			if (askyn(genbuf, NA, NA) == YEA) {
				saveValue(currentuser.userid,
					  MONEY_NAME, salary, MAX_MONEY_NUM);
				addtofile(DIR_MC "salary_list",
					  currentuser.userid);
				move(8, 4);
				prints
				    ("���������Ĺ��ʡ���л��Ϊ����ٸ�����Ĺ���!");
				pressanykey();
				break;
			}
			break;
		case '6':
			clear();
			money_show_stat("����ٸ�����г��칫��");
			move(6, 4);
			char name[20];
			whoTakeCharge2(1, name);
			whoTakeCharge(1, uident);
			if (strcmp(currentuser.userid, uident)) {
				prints
				    ("����%s��ס���㣬����˵��:���г�%s��������칫��������š���",
				    name,uident);
				pressanykey();
				break;
			} else {
				prints("��ѡ���������:");
				move(7, 6);
				prints
				    ("1. �����������                  2. ������������");
				move(8, 6);
				prints
				    ("3. ����ת�ʷ���                  4. ��������");
				move(9, 6);
				prints
				    ("5. �����û��ʻ�                  6. �ر��������");
				move(10, 6);
				prints
				    ("7. ���Ź���                      8. ���Ⲧ��");
				move(11,6);
				prints
				    ("9. ��ְ                          Q. �˳�");
				ch = igetkey();
				switch (ch) {
				case '1':
					getdata(12, 4,
						"�趨�µĴ������[10-250]: ",
						buf, 4, DOECHO, YEA);
					if (atoi(buf) < 10 || atoi(buf) > 250) {
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"�µĴ�������� %.2f����ȷ����",
						atoi(buf) / 100.0);
					if (askyn(genbuf, NA, NA) == YEA) {
						savestrvalue(MC_RATE_FILE,
							     "deposit_rate",
							     buf);
						utmpshm->mc.deposit_rate =
						    atoi(buf);
						move(15, 4);
						prints("������ϡ�");
						sprintf(genbuf,
							"�µĴ������Ϊ %.2f�� ��",
							utmpshm->mc.
							deposit_rate / 100.0);
						deliverreport
						    ("[����]����ٸ���е����������",
						     genbuf);
						sprintf(genbuf, "%s��ʹ����Ȩ��",currentuser.userid);
						sprintf(buf, "�����µĴ������Ϊ %.2f�� ��", utmpshm->mc.
							deposit_rate / 100.0);
						millionairesrec(genbuf, buf, "");
						pressanykey();
					}
					break;
				case '2':
					getdata(12, 4,
						"�趨�µĴ�������[10-250]: ",
						buf, 4, DOECHO, YEA);
					if (atoi(buf) < 10 || atoi(buf) > 250) {
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"�µĴ��������� %.2f����ȷ����",
						atoi(buf) / 100.0);
					if (askyn(genbuf, NA, NA) == YEA) {
						savestrvalue(MC_RATE_FILE,
							     "lend_rate", buf);
						utmpshm->mc.lend_rate =
						    atoi(buf);
						move(15, 4);
						prints("������ϡ�");
						sprintf(genbuf,
							"�µĴ�������Ϊ %.2f�� ��",
							utmpshm->mc.lend_rate /
							100.0);
						deliverreport
						    ("[����]����ٸ���е�����������",
						     genbuf);
						sprintf(genbuf, "%s��ʹ���й���Ȩ��",currentuser.userid);
						sprintf(buf, "�����µĴ�������Ϊ %.2f�� ��", utmpshm->mc.
							lend_rate / 100.0);
						millionairesrec(genbuf, buf, "");
						pressanykey();
					}
					break;
				case '3':
					getdata(12, 4,
						"�趨�µ�ת�ʷ���[10-250]: ",
						buf, 4, DOECHO, YEA);
					if (atoi(buf) < 10 || atoi(buf) > 250) {
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"�µ�ת�ʷ����� %.2f����ȷ����",
						atoi(buf) / 100.0);
					if (askyn(genbuf, NA, NA) == YEA) {
						savestrvalue(MC_RATE_FILE,
							     "transfer_rate",
							     buf);
						utmpshm->mc.transfer_rate =
						    atoi(buf);
						move(15, 4);
						prints("������ϡ�");
						sprintf(genbuf,
							"�µ�ת�ʷ���Ϊ %.2f�� ��",
							utmpshm->mc.
							transfer_rate / 100.0);
						deliverreport
						    ("[����]����ٸ���е���ת�ʷ���",
						     genbuf);
						sprintf(genbuf, "%s��ʹ���й���Ȩ��",currentuser.userid);
						sprintf(buf, "�����µ�ת�ʷ���Ϊ %.2f�� ��", utmpshm->mc.
							transfer_rate / 100.0);
						millionairesrec(genbuf, buf, "");
						pressanykey();
					}
					break;
				case '4':
					move(12, 4);
					usercomplete("��˭�ṩ�ر���",
						     uident);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(13, 4);
						prints("�����ʹ���ߴ���...");
						pressreturn();
						break;
					}
					if (lookupuser.dietime > 0) {
						move(13, 4);
						prints("���������...");
						pressreturn();
						break;
					}
					if (loadValue
					    (uident, "lend_time", 2000000000) > 0) {
						move(13, 4);
						prints
						    ("�ÿͻ��Ѿ��������׷�Ӵ��");
						pressanykey();
						break;
					}
					getdata(13, 4, "������[0]: ", buf, 10, DOECHO, YEA);
					if (buf[0] == '\0') 
						break;
					
					if (atoi(buf) < 100000) {
						move(14, 4);
						prints
						    ("��ô��Ǯ��Ӫҵ���Ϳ��԰���");
						pressanykey();
						break;
					}
					if (atoi(buf) > 100000000) {
						move(14, 4);
						prints
						    ("�������޴�Ĵ�����¶��»᲻��ͬ��ġ�");
						pressanykey();
						break;
					}
					num = atoi(buf);
					getdata(14, 4, "��������[3-30]: ", buf, 3, DOECHO, YEA);
					if (atoi(buf) < 1 || atoi(buf) > 30) 
						break;
					move(15, 4);
					if (askyn("ȷ����ÿͻ��ṩ������", NA, NA) == NA)
						break;
					time_t t = time(0) + 86400 * atoi(buf);
					sprintf(genbuf, "%s\t%s", uident, ctime(&t));
					addtofile(DIR_MC "special_lend", genbuf);
					saveValue(uident, MONEY_NAME, num,
						  MAX_MONEY_NUM);
					saveValue(uident, LEND_NAME, num,
						  MAX_MONEY_NUM);
					saveValue(uident, "lend_time", time(0),
						  2000000000);
					saveValue(uident, "back_time",
						  time(0) + atoi(buf) * 86400,
						  2000000000);
					sprintf(genbuf,
						"������ %d ����ٸ�ң�������� %s ���ڳ������",
						num, buf);
					mail_buf(genbuf, uident,
						 "����ٸ�����г�ͬ�������Ĵ�������");
					move(16, 4);
					prints
					    ("����������ɡ���ȷ���ͻ���ʱ���");
					sprintf(buf, "��%s�ر���%s",uident, genbuf);
					sprintf(genbuf, "%s��ʹ���й���Ȩ��",currentuser.userid);
					millionairesrec(genbuf, buf, "");
					pressanykey();
					break;
				case '5':
					move(12, 4);
					usercomplete("����˭���ʻ���", uident);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(13, 4);
						prints("�����ʹ���ߴ���...");
						pressreturn();
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"�ÿͻ����ֽ�%d ����ٸ�ң���� %d ����ٸ��,���� %d ����ٸ�ҡ�",
						loadValue(uident, MONEY_NAME,
							MAX_MONEY_NUM),
						loadValue(uident, CREDIT_NAME,
							MAX_MONEY_NUM),
						loadValue(uident, LEND_NAME,
							MAX_MONEY_NUM));
					prints("%s", genbuf);
					pressanykey();
					break;
				case '6':
					clear();
					move(1, 0);
					prints("Ŀǰ���е��ر���������");
					move(2, 0);
					prints("�ͻ�ID\t����ʱ��");
					listfilecontent(DIR_MC "special_lend");
					pressanykey();
					break;
				case '7':
					move(12, 4);
					if (newSalary()) {
						if (askyn("ȷ�����ű��¹�����", NA, NA) == YEA) {
							time2string(time(0), genbuf);
							if (savestrvalue(DIR_MC "etc_time", "salary_time", genbuf)){
								move(14, 4);
								prints("����!����д�ļ�!");
								pressanykey();
								break;
							}
							strcpy(currboard, "sysop");
							deliverreport
							    ("[����]��վ����Ա��ȡ���¹���",
							     "����7���ڵ�����ٸ������ȡ��������Ϊ������");
							strcpy(currboard,	 MC_BOARD);
							remove(DIR_MC "salary_list");
							utmpshm->mc.isSalaryTime = 1;
							move(14, 4);
							prints("������ɡ�");
							sprintf(genbuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
							millionairesrec(genbuf, "���Ź���", "");
							pressanykey();
						}
					} else {
						prints("��δ������ʱ�䡣");
						pressanykey();
					}
					break;
				case '8':
					move(12, 4);
					usercomplete("��˭�ṩ�ر𲦿",
						     uident);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(13, 4);
						prints("�����ʹ���ߴ���...");
						pressreturn();
						break;
					}
					if (lookupuser.dietime > 0) {
						move(13, 4);
						prints("���������...");
						pressreturn();
						break;
					}
					getdata(13, 4,"������[0]: ", buf, 10, DOECHO, YEA);
					if (buf[0] == '\0') {
						break;
					}
					if (atoi(buf) < 100000) {
						move(14, 4);
						prints
						    ("��ô��Ǯ��Ӫҵ���Ϳ��԰���");
						pressanykey();
						break;
					}
					if (atoi(buf) > 100000000) {
						move(14, 4);
						prints
						    ("�������޴�Ĵ�����¶��»᲻��ͬ��ġ�");
						pressanykey();
						break;
					}
					num = atoi(buf);
					getdata(15, 4, "����ԭ��", buf, 50, DOECHO, YEA);
					sprintf(letter, "������������Ľ��跢չ�����䰴�չ涨ʹ�ã����ý���Υ���Ҽͻ��\n\n����ԭ��%s", buf);
					move(16, 4);
					if (askyn("ȷ����ÿͻ�������", NA, NA) == NA) 
						break;
					saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);
					sprintf(genbuf,"����%s %d����ٸ��Ԯ������",uident, num);
					deliverreport(genbuf, letter);
					mail_buf(genbuf, uident,
						 "����ٸ�����г�ͬ�������Ĳ�������");
					move(17, 4);
					prints("������ɡ�");
					sprintf(buf, "��%s�ر𲦿�%d����ٸ��",uident, num);
					sprintf(genbuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
					millionairesrec(genbuf, buf, "");
					pressanykey();
					break;
				case '9':
					move(12, 4);
					if (askyn("�����Ҫ��ְ��", NA, NA) ==
					    YEA) {
					/*	del_from_file(MC_BOSS_FILE, "bank");
						sprintf(genbuf,
							"%s ������ȥ����ٸ�����г�ְ��",
							currentuser.userid);
						deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
						move(14, 4);
						prints
						    ("�ðɣ���Ȼ�����Ѿ������»�Ҳ����ǿ�����ټ���");
						quit = 1;
					*/
						sprintf(genbuf, "%s Ҫ��ȥ����ٸ�����г�ְ��",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
						pressanykey();
					}
					break;
				case 'q':
				case 'Q':
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int//��Ʊϵͳ
money_lottery()
{
	int ch, money;
	int quit = 0, quitRoom = 0;
	int inputValid;
	char buf[100], uident[IDLEN + 1];
	char letter[200];
	char title[80];
	FILE *fp;
	long openTime;
	char name[20];

	clear();
	while (!quit) {
		nomoney_show_stat("����ٸ��Ʊ����");
		move(6, 4);
		prints("��Ʊ����¡�ؿ��ţ���ӭ���ӻԾ�����Ʊ����");
		move(8, 4);
		prints("��Ʊ�����뵽millionaires���ѯ��");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]36ѡ7 [2]��� [3]������ [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("36ѡ7");
			if (access(DIR_MC_TEMP "36_7_start", 0)) {
				move(6, 4);
				prints("��Ǹ����һ�ڵ�36ѡ7��Ʊ��δ��ʼ���ۡ�");
				pressanykey();
				break;
			}
			move(5, 4);
			prints("���ּ���-���������� 08-13-01-25-34-17-18");
			move(7, 4);
			sprintf(genbuf,
				"��ǰ������ۻ�����\033[1;31m%d\033[m   �̶�����\033[1;31m%d\033[m",
				utmpshm->mc.prize367, PRIZE_PER);
			prints("%s", genbuf);
			move(9, 4);
			sprintf(genbuf, "ÿע 10000 ����ٸ�ҡ�ȷ����ע��?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				move(10, 4);
				if (money < 10000) {
					prints
					    ("û��Ǯ�ͱ��ң�һ��ȥ����һ����");
					pressanykey();
					break;
				}

				saveValue(currentuser.userid, MONEY_NAME, -10000, MAX_MONEY_NUM);	//��Ǯ
				utmpshm->mc.prize367 += 10000;
				utmpshm->mc.prize367 =
				    limitValue(utmpshm->mc.prize367, MAX_POOL_MONEY);
				inputValid = 0;
				while (!inputValid) {
					getdata(10, 4, "����д��ע��: ", buf, 21, DOECHO, YEA);	/* 2��7��6��1��21 */

					if (!valid367Bet(buf)) {	// ������ע�ĺ�����
						move(11, 4);
						prints
						    ("�Բ���������ע����д����������Ү��������һ�Ρ�");
						pressanykey();
					} else {
						sprintf(genbuf, "%s %s", currentuser.userid, buf);
						addtofile(DIR_MC_TEMP "36_7_list", genbuf);
						move(11, 4);
						prints
						    ("                                                             ");
						move(11, 4);
						prints
						    ("����ɹ���ף���д󽱣�");
						inputValid = 1;
						sprintf(letter,
							"��������һע36ѡ7��ע���ǣ�%s�������Ʊ��棬���ڶҽ���", buf);
						sprintf(title,
							"��Ʊ���Ĺ���ƾ֤");
						mail_buf(letter, currentuser.userid, title);
						pressanykey();
					}

				}

			}
			break;
		case '2':
			nomoney_show_stat("���");
			move(6, 4);
			if (access(DIR_MC_TEMP "soccer_start", 0)) {
				prints("��Ǹ����һ�ڵ������Ʊ��δ��ʼ���ۡ�");
				pressanykey();
				break;
			}
			if (utmpshm->mc.isSoccerSelling == 0) {
				prints("��Ǹ���������Ѿ���������ȴ�������");
				pressanykey();
				break;
			}
			move(4, 4);
			prints("������±������millionaires�湫�����¡�");
			move(5, 4);
			prints
			    ("����ʤΪ3������ƽΪ1��������Ϊ0���������������-������֧�ָ�ʽ��ע��");
			move(6, 4);
			prints
			    ("�����6������ʱ��һ�����ܵ���ע����Ϊ�� 1-310-1-10-3-0");
			move(8, 4);
			sprintf(genbuf,
				"��ǰ������ۼƽ���\033[1;31m%d\033[m  �̶�����\033[1;31m%d\033[m",
				utmpshm->mc.prizeSoccer, PRIZE_PER);
			prints("%s", genbuf);
			move(10, 4);
			sprintf(genbuf, "ÿע10000����ٸ�ҡ�ȷ����ע��?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				move(11, 4);
				if (money < 10000) {
					prints("û��Ǯ�ͱ��ң�һ��ȥ����һ����");
					pressanykey();
					break;
				}

				inputValid = 0;
				while (!inputValid) {
					int sum;
					getdata(11, 4, "����д��ע��: ", buf, 55, DOECHO, YEA);
					if (!validSoccerBet(buf)) {	/* ������ע�ĺ����� */
						move(12, 4);
						prints
						    ("�Բ���������ע����д����������Ү��������һ�Ρ�");
						pressanykey();
					} else {
						int money;
						inputValid = 1;
						sum = computeSum(buf);	/*���㸴ʽ��ע��ע�� */
						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						if (sum > money / 10000) {
							move(12, 4);
							prints("                                                     ");
							move(12, 4);
							sprintf(genbuf,
								"����Ǯ������ %d ע��������һ�°ɣ�",
								sum);
							prints("%s", genbuf);
							pressanykey();
							break;
						}
						saveValue(currentuser.userid, MONEY_NAME, -sum * 10000, MAX_MONEY_NUM);	/*��Ǯ */
						utmpshm->mc.prizeSoccer += sum * 10000;
						utmpshm->mc.prizeSoccer = limitValue(utmpshm->mc.prizeSoccer, MAX_POOL_MONEY);
						saveSoccerRecord(buf);	/*�������渴ʽ��ע��¼ */
						move(12, 4);
						prints("                                                             ");
						move(12, 4);
						sprintf(genbuf,"��һ��������%dע��ף���д󽱣�", sum);
						prints("%s", genbuf);
						sprintf(letter,
							"��������һע(��ʽ)��ʡ�ע���ǣ�%s�������Ʊ��棬���ڶҽ���",
							buf);
						sprintf(title, "��Ʊ���Ĺ���ƾ֤");
						mail_buf(letter, currentuser.userid, title);
						pressanykey();
					}
				}
			}
			break;
		case '3':
			nomoney_show_stat("���ʹ�˾������");
			whoTakeCharge(2, uident);
			whoTakeCharge2(2, name);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints("����%s��ʾ��:������%s�������ȥ�ˣ�������ֱ�Ӹ�����ϵ����",
				    name, uident);
				pressanykey();
				break;
			}
			quitRoom = 0;
			while (!quitRoom) {
				char strTime[15];
				nomoney_show_stat("���ʹ�˾������");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m ѡ�� \033[1;46m [1]���� [2]�½� [3]ֹͣ������� [4]��ְ [Q]�뿪\033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					nomoney_show_stat("���ʹ�˾������");
					move(6, 10);
					prints("1.  36ѡ7");
					move(7, 10);
					prints("2.  �����Ʊ");
					move(8, 10);
					prints("Q.  �˳�");
					move(4, 4);
					prints("��ѡ��Ҫ�����Ĳ�Ʊ���ţ�");
					move(12, 4);
					ch = igetkey();
					move(t_lines - 5, 15);
					switch (ch) {
					case '1':
						fp = fopen(DIR_MC_TEMP "36_7_start", "r");
						if (fp) {
							fgets(strTime, 15, fp);
							openTime = atoi(strTime);
							fclose(fp);
							if (time(0) >= openTime) //||strcmp(currentuser.userid,"macintosh")==0
								if (open_36_7() == 0) {
									prints("�����ɹ���");
									sprintf(genbuf, "%s��ʹ��Ʊ����Ȩ��",currentuser.userid);
									millionairesrec(genbuf, "36ѡ7����", "");
								}
								else prints("�����������...");	
							else 
								prints("����ʱ�仹û�е�����");
							
						} else 
							prints("û�иò�Ʊ�ļ�¼��");
						pressanykey();
						break;
					case '2':
						fp = fopen(DIR_MC_TEMP "soccer_start", "r");
						if (fp) {
							fgets(strTime, 15, fp);
							fclose(fp);
							openTime =atoi(strTime);
							if (time(0) >= openTime) //||strcmp(currentuser.userid,"macintosh")==0
							{
								getdata(t_lines - 5, 4,
									"������ҽ�����(���� - )[��\033[1;33mENTER\033[m����]: ",
									buf, 55, DOECHO, YEA);
								if (strlen(buf) == 0) 
									break;
								if (open_soccer(buf) ==0)	{
									prints("�����ɹ���");
									sprintf(genbuf, "%s��ʹ��Ʊ����Ȩ��",currentuser.userid);
									millionairesrec(genbuf, "��ʿ���", "");
								}
								else prints("�����������...");
								
							} else 
								prints ("����ʱ�仹û�е�����");	
						} else 
							prints ("û�иò�Ʊ�ļ�¼��");
						pressanykey();
						break;
					case 'q':
					case 'Q':
						break;
					}
					break;
				case '2':
					nomoney_show_stat("���ʹ�˾������");
					move(6, 10);
					prints("1. 36ѡ7 ");
					move(7, 10);
					prints("2. ���");
					move(8, 10);
					prints("Q. �˳�");
					move(4, 4);
					prints("��ѡ�񿪽�����������");
					ch = igetkey();
					switch (ch) {
					case '1':
						nomoney_show_stat
						    ("���ʹ�˾������");
						move(4, 4);
						if (!access(DIR_MC_TEMP "36_7_start",0)) {
							prints("36ѡ7��Ʊ�������ڻ��Ƚ��С�");
							pressanykey();
							break;
						}
						prints("�½�36ѡ7");
						inputValid = 0;
						while (!inputValid) {
							getdata(8, 4,"��Ʊ��������[1-7]: ",buf, 2, DOECHO,YEA);
							if (buf[0] > '0' && buf[0] < '8') 
								inputValid = 1;
						}
						time2string(time(0) + (buf[0] - '0') * 86400, genbuf);
						addtofile(DIR_MC_TEMP "36_7_start", genbuf);

						sprintf(genbuf,
							"���ڲ�Ʊ���� %s ��󿪽�����ӭ���ӻԾ����",
							buf);
						deliverreport
						    ("[����]��һ��36ѡ7��Ʊ��ʼ����", genbuf);

						move(10, 4);
						prints("�����ɹ����뵽ʱ������");
						sprintf(genbuf, "�½�36ѡ7��%s��󿪽���",buf);
						sprintf(buf, "%s��ʹ��Ʊ����Ȩ��",currentuser.userid);
						millionairesrec(buf, genbuf, "");
						pressanykey();
						break;
					case '2':
						nomoney_show_stat("���ʹ�˾������");
						move(4, 4);
						if (!access(DIR_MC_TEMP "soccer_start",0)) {
							prints("�����Ʊ�������ڻ��Ƚ��С�");
							pressanykey();
							break;
						}
						prints("�½����");
						inputValid = 0;
						while (!inputValid) {
							getdata(8, 4,"��Ʊ��������[1-7]: ",buf, 2, DOECHO,YEA);
							if (buf[0] > '0' && buf[0] < '8') 
								inputValid = 1;
						}
						time2string(time(0) +(buf[0] - '0') * 86400, genbuf);
						addtofile(DIR_MC_TEMP "soccer_start", genbuf);
						utmpshm->mc.isSoccerSelling = 1;
						sprintf(genbuf,
							"���ڲ�Ʊ���� %s ��󿪽�����ӭ���ӻԾ����",
							buf);
						deliverreport
						    ("[����]��һ�������Ʊ��ʼ����", genbuf);

						move(10, 4);
						prints("�����ɹ����뵽ʱ������");
						sprintf(genbuf, "�½���ʣ�%s��󿪽���",buf);
						sprintf(buf, "%s��ʹ��Ʊ����Ȩ��",currentuser.userid);
						millionairesrec(buf, genbuf, "");
						pressanykey();
						break;
					case 'q':
					case 'Q':
						break;
					}
					break;
					
				case '3':
					nomoney_show_stat("���ʹ�˾������");
					move(6, 4);
					if (askyn("�����Ҫͣ�������", NA, NA) == YEA) {
						utmpshm->mc.isSoccerSelling = 0;
						deliverreport("[����]���������Ʊֹͣ����",
							      "����������ĵȴ�������");
						sprintf(buf, "%s��ʹ��Ʊ����Ȩ��", currentuser.userid);
						millionairesrec(buf, "ͣ�۱������", "");
						move(8, 4);
						prints("�Ѿ�ͣ�ۣ��뵽ʱ������");
						pressanykey();
					}
					break;
					
				case '4':
					nomoney_show_stat("���ʹ�˾������");
					move(6, 4);
					if (askyn("�����Ҫ��ְ��", NA, NA) ==
					    YEA) {
					/*	del_from_file(MC_BOSS_FILE, "lottery");
						sprintf(genbuf,
							"%s ������ȥ����ٸ���ʹ�˾����ְ��",
							currentuser.userid);
						deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
						move(8, 4);
						prints
						    ("�ðɣ���Ȼ�����Ѿ������»�Ҳ����ǿ�����ټ���");
						pressanykey();
						quitRoom = 1;
					*/
						sprintf(genbuf, "%s Ҫ��ȥ����ٸ���ʹ�˾����ְ��",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(8, 4);
						prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
						pressanykey();
					}
					break;
				case 'q':
				case 'Q':
					quitRoom = 1;
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}


struct MC_Jijin{
	char userid[14];
	char name[18];
};

static int
addOrDel_contrb()
{
	char uident[STRLEN], ans[8];
	int count = 0, tag = 0, i, j, fd, x=0;
	char buf[STRLEN], title[STRLEN];
	void *buffer = NULL;
	size_t filesize;
	FILE *fpw;
	
	struct MC_Jijin JijinTmp, *JijinMem; 

	while (1) {
		clear();
		count = get_num_records(MC_JIJIN_CTRL_FILE, sizeof(struct MC_Jijin));
		if (count == 0){
			if ((fd = open(MC_JIJIN_CTRL_FILE, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
				return -1;
			close(fd);
		}
		filesize = sizeof(struct MC_Jijin) * count;
		JijinMem = loadData(MC_JIJIN_CTRL_FILE, buffer, filesize);
		if (JijinMem == (void *) -1)
			return -1;
		
		prints("�趨�����������Ļ���: \n");
		j = 0;
		for(i = 0; i<count ;i++){
			if (JijinMem[i].userid[0]==0)
				continue;
			sprintf(buf, "%-12.12s  %s", JijinMem[i].userid, JijinMem[i].name);
			if (j < 15)
				showAt(3+j, 0, buf, 0);
			else if (j <30)
				showAt(3+j, 40, buf, 0);
			j++;
		}
		if (j==0){
			unlink(MC_JIJIN_CTRL_FILE);
			count = 0;
		}
			
		if (count)
			getdata(1, 0, "(A)���� (D)ɾ�� (C)�ı� (E)�뿪 [E]: ", ans, 2, DOECHO, YEA);
		else
			getdata(1, 0, "(A)���� (E)�뿪 [E]: ", ans, 2, DOECHO, YEA);

		tag = 0;
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			while (1){
				move(1, 0);
				clrtoeol();
				usercomplete("����id��", uident);
				if (*uident == '\0')
					break;
				if (!getuser(uident)) {
					showAt(2, 0, "��id������", 1);
					tag = -1;
					break;
				}
				if (!seek_in_file(DIR_MC "jijin", uident)) {
					showAt(2, 0, "��id���ǻ���!", 1);
					tag = -1;
					break;
				}
				for(i = 0; i<count ;i++){ 
					if (!strcmp(JijinMem[i].userid, uident)){
						showAt(2, 0, "��id�Ѿ�����", 1);
						tag = -1;
						break;
					}
				}
				if (tag == 0)
					tag = 1;
				break;
			}
			buf[0] = 0;
			memset(&JijinTmp, 0, sizeof(struct MC_Jijin));
			if (tag == 1){
				sprintf(JijinTmp.userid, "%s", uident);
				while (buf[0] == 0)
					getdata(2, 0, "�������������: ", buf, 18, DOECHO, YEA);
				sprintf(JijinTmp.name, "%s", buf);
				append_record(MC_JIJIN_CTRL_FILE, &JijinTmp, sizeof(struct MC_Jijin));
				sprintf(title, "%s��ʹ����Ȩ��(���þ�����)", currentuser.userid);
				sprintf(buf,"%s��%s���Ϊ %s����", currentuser.userid, JijinTmp.userid, JijinTmp.name);
				millionairesrec(title,buf, "");
			} 				
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("�ı��ĸ�id: ", uident);
			if (*uident != '\0') {
				for(i = 0; i<count ;i++)
					if (!strcmp(JijinMem[i].userid, uident)){
						tag = 1;
						break;
					}
			}
			buf[0] = 0;
			if (tag == 1){
				while (buf[0] == 0)
					getdata(2, 0, "�������µ�����: ", buf, 18, DOECHO, YEA);
				sprintf(JijinMem[i].name, "%s", buf);
				sprintf(title, "%s��ʹ����Ȩ��(���þ�����)", currentuser.userid);
				sprintf(buf,"%s�ı�%s������Ϊ %s����", currentuser.userid, JijinMem[i].userid, JijinMem[i].name);
				millionairesrec(title,buf, "");				
				saveData(JijinMem, filesize);
			} else 
				showAt(2, 0, "�������id�����б���", 1);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			usercomplete("ɾ��id: ", uident);
			if (uident[0] != '\0') {
				for(i = 0; i < count ;i++)
					if (!strcmp(JijinMem[i].userid, uident)){
						tag = 1;
						x = i;
						break;
					}
			}
			if (tag == 1){
				fpw = fopen(MC_JIJIN_CTRL_FILE".tmp", "w");
				if (fpw == 0) {
					showAt(2, 0, "�����������!", 1);
					return -1;
				}
				sprintf(title, "%s��ʹ����Ȩ��(���þ�����)", currentuser.userid);
				sprintf(buf,"%sɾ��%s(%s����)", currentuser.userid, JijinMem[i].userid, JijinMem[i].name);
				millionairesrec(title,buf, "");
				for (i = 0; i < x; i++)
					fwrite(&JijinMem[i], sizeof(struct MC_Jijin), 1, fpw);
				for (i = x+1; i < count; i++)
					fwrite(&JijinMem[i], sizeof(struct MC_Jijin), 1, fpw);
				fclose(fpw);
				unlink(MC_JIJIN_CTRL_FILE);
				rename(MC_JIJIN_CTRL_FILE".tmp", MC_JIJIN_CTRL_FILE);
				showAt(2, 0, "ɾ���ɹ�", 1);
			}else 
				showAt(2, 0, "�������id�����б���", 1);
		} else
			break;
	}
	clear();
	return 1;
}


static int 
money_sackOrAppoint(int type) //type1ְλ 2����
{
	int pos, i=0 , j;
	char buf[100], letter[100], report[100], uident[IDLEN + 1], boss[IDLEN + 1];
	const char feaStr[][20] =
	    { "bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office",""
	};
	const char feaStr2[][20] =
	    { "����", "��Ʊ", "�ĳ�", "�ڰ�", "ؤ��", "����", "�̳�",
		"����","ɱ��","����","����", ""
	};

	clear();
	if (type==1)
		showAt(2, 4, "Ŀǰ����ٸ�������ĸ�ְλ�����", 0);
	if (type==2)
		showAt(2, 4, "Ŀǰ����ٸ�������ĸ�����ְλ�����", 0);

	while (feaStr[i][0]){
		if (type == 1)
			whoTakeCharge(i+1, boss);
		else 
			whoTakeCharge2(i+1, boss);
		sprintf(buf, "%d.%s: %s", i+1, feaStr2[i], boss);
		showAt(i+5, 4, buf, 0);
		i++;
	}
	
	getdata(16, 4, "��ѡ��ְ��? ", buf, 3, DOECHO, YEA);
	pos = atoi(buf);
	if (pos > 11 || pos < 1) 
		return 0;
	
	getdata(16, 4, "��ѡ��:  1.����  2.��ְ? ", buf, 2, DOECHO, YEA);
	j = atoi(buf);
	if (j > 2 || j < 1) 
		return 0;

	if (type == 1)
		whoTakeCharge(pos, boss);
	else 
		whoTakeCharge2(pos, boss);
	if (j==1){
		if (boss[0] != '\0') {	//�����ְλ�ǿ�
			prints("%s�Ѿ������ְλ��", boss);
			pressanykey();
			return 0;
		}
		move(16, 4);
		usercomplete("����˭��", uident);
		move(17, 4);
		if (uident[0] == '\0')
			return 0;
		if (!searchuser(uident)) {
			prints("�����ʹ���ߴ���...");
			pressanykey();
			return 0;
		}
		sprintf(genbuf, "ȷ������ %s ְλ %d%s ��", uident, pos, (type==1)?"":"����");
		if (askyn(genbuf, NA, NA) == YEA) {
			if (type==1){
				sackOrAppoint(pos, uident, 0, letter);
				savestrvalue(MC_BOSS_FILE, feaStr[pos - 1], uident);
			}else{ 
				sackOrAppoint2(pos, uident, 0, letter);
				savestrvalue(MC_ASS_FILE, feaStr[pos - 1], uident);
			}
			deliverreport(letter,
				      "������������������Ȩı˽����Ϊ����ٸ������ҵ�ķ�չ�Ϲ����ᡣ");
			mail_buf(letter, uident, letter);
			sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
			sprintf(buf,"%s����%s����%dְλ%s", currentuser.userid, uident, pos, (type==1)?"":"����");
			millionairesrec(genbuf, buf, "");
			move(18, 4);
			prints("�����ɹ���");
			pressanykey();
		}
	}else if (j==2){
		if (boss[0] == '\0') {	//�����ְλΪ��
			prints("Ŀǰ�����˸����ְλ��");
			pressanykey();
			return 0;
		}
		getdata(17, 4, "��ȥԭ��:", genbuf, 50, DOECHO, YEA);
		sprintf(report, "��ȥԭ��%s", genbuf);
		move(17, 4);
		sprintf(genbuf, "ȷ����ȥ %s ��%sְλ��", boss, (type==1)?"":"����");
		if (askyn(genbuf, NA, NA) == YEA) {
			if (type==1){
				sackOrAppoint(pos, boss, 1, letter);
				del_from_file(MC_BOSS_FILE, (char *) feaStr[pos - 1]);
			}else{ 
				sackOrAppoint2(pos, boss, 1, letter);
				del_from_file(MC_ASS_FILE, (char *) feaStr[pos - 1]);
			}
			deliverreport(letter, report);
			mail_buf(letter, boss, letter);
			sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
			sprintf(buf,"%s��ȥ%s��%d%sְλ", currentuser.userid, boss, pos, (type==1)?"":"����");
			millionairesrec(genbuf, buf, "");
			move(18, 4);
			prints("��ְ�ɹ���");
			pressanykey();
		}
	}
	return 1;
}


static int //����ϵͳ  ��Ʊϵͳ
money_admin()
{
	int ch, i, j, quit = 0;
	char buf[100], letter[100], uident[IDLEN + 1];
	char stockboard[STRLEN][MAX_STOCK_NUM];
	FILE *fp1;
	int count;

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
	    && !(currentuser.userlevel & PERM_SPECIAL5) && strcmp(currentuser.userid, "macintosh")) {
		return 0;
	}
	clear();
	while (!quit) {
		clear();
		nomoney_show_stat("����ٸ�������Ĺ���");
		move(5, 4);
		prints("���︺�����ٸ�������ĵ����¹���");
		move(7, 8);
		prints("A. �������������ܹ�             B. ��ȥ���������ܹ�");
		move(8, 8);
		prints("C. �г��ܹ�����           ");
		move(9, 8);
		prints("E. ����ְλ                     F. ��������");
		move(10, 8);
		prints("I. �����ϰ�������");
		move(11, 8);
		prints("J. ���������ó�Ա               K. ȡ���������ʸ�");
		move(12, 8);
		prints("L. ��������                     M. ȡ�������ʸ�");
		move(13, 8);
		prints("N. ����������                   O. ȡ��������");
		move(14, 8);
		prints("Y. ��������id                   Z. ȡ������id");
		move(15, 8);
		prints("R. �г������ó�Ա               S. �г���������");
		move(16, 8);
		prints("T. �г�����������               U. �г�����id����");
		move(17, 8);
		prints("P. ���г�ʼ��");
		move(19, 8);
		prints("X. ����������                   0. ���ؽ�������");
		move(20, 8);
		prints("1. �ı�����ֽ�                 2. �ı���˴��");
		move(22, 8);
		prints("G. ��ְ                         Q. �˳�");

		
		ch = igetkey();
		switch (ch) {
		case 'e':
		case 'E':
			money_sackOrAppoint(1);
			break;

		case 'f':
		case 'F':
			money_sackOrAppoint(2);//����
			break;

		case 'a':
		case 'A':
			clear();
			move(15, 4);
			usercomplete("����˭���������ܹ�Ȩ�ޣ�", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (!seek_in_file(MC_ADMIN_FILE, uident)) {
				if (askyn("ȷ����", NA, NA) == NA) {
					break;
				}
				addtofile(MC_ADMIN_FILE, uident);
				move(17, 4);
				prints("�����ɹ�!");
				sprintf(genbuf,
					"[����]���� %s ����ٸ�������Ĺ���Ȩ��",
					uident);
				deliverreport(genbuf,
					      "������������������Ȩı˽����Ϊ����ٸ������ҵ�ķ�չ�Ϲ����ᡣ");
				sprintf(genbuf,
					"%s �� %s �������ٸ�������Ĺ���Ȩ��",
					uident, currentuser.userid);
				mail_buf(genbuf, uident, genbuf);
				//add by macintosh for system record
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%s����%sΪ���������ܹ�", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
			} else {
				prints("��ID�Ѿ����н������Ĺ���Ȩ��");
			}
			pressanykey();
			break;
		case 'c':
		case 'C':
			clear();
			move(1, 0);
			prints("Ŀǰ���й���Ȩ�޵�ID�б�");
			listfilecontent(MC_ADMIN_FILE);
			pressanykey();
			break;
		case 'b':
		case 'B':
			clear();
			move(15, 4);
			usercomplete("ȡ��˭�Ľ��������ܹ�Ȩ�ޣ�", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (seek_in_file(MC_ADMIN_FILE, uident)) {
				getdata(16, 4, "ȡ��ԭ��", buf, 50,
					DOECHO, YEA);
				move(17, 4);
				if (askyn("ȷ����", NA, NA) == NA) {
					pressanykey();
					break;
				}
				del_from_file(MC_ADMIN_FILE, uident);
				move(18, 4);
				prints("ȡ���ɹ�!");
				sprintf(genbuf,
					"[����]ȡ�� %s �ı���ٸ�������Ĺ���Ȩ��",
					uident);
				sprintf(letter, "ȡ��ԭ�� %s", buf);
				deliverreport(genbuf, letter);
				sprintf(genbuf,
					"%s �� %s ȡ������ٸ�������Ĺ���Ȩ��",
					uident, currentuser.userid);
				mail_buf(genbuf, uident, genbuf);
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%sȡ��%s�Ľ��������ܹ�Ȩ��", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
			} else {
				prints("��IDû�д�Ȩ�ޡ�");
			}
			pressanykey();
			break;

		case 'g':
		case 'G':
			clear();
			move(15, 4);
			if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)) {
				break;
			}
			if (askyn("�����Ҫ��ְ��", NA, NA) == YEA) {
				del_from_file(MC_ADMIN_FILE,
					      currentuser.userid);
				sprintf(genbuf,
					"%s ������ȥ����ٸ���������ܹ�ְ��",
					currentuser.userid);
				deliverreport(genbuf,
					      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%s��ȥ����ٸ���������ܹ�ְ��", currentuser.userid);
				millionairesrec(genbuf, buf, "");
				move(16, 4);
				prints
				    ("�ðɣ���Ȼ�����Ѿ�����������Ҳ����ǿ�����ټ���");
				quit = 1;
				pressanykey();
			}
			break;
			case 'j':
			case 'J':
				clear();
				move(15, 4);
				usercomplete("����˭�������ã�", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!searchuser(uident)) {
					prints("�����ʹ���ߴ���...");
					pressanykey();
					break;
				}
				if (seek_in_file
				    (DIR_MC "mingren", uident)) {
					prints("��ID�Ѿ��������ˡ�");
					pressanykey();
					break;
				}
				if (askyn("ȷ����", NA, NA) == YEA) {
					addtofile(DIR_MC "mingren",
						  uident);
					sprintf(genbuf,
						"��ϲ%s�������ٸ��������������",
						uident);
					deliverreport(genbuf,
				      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
					mail_buf
					    ("��л��Ϊ�˴�����Ϸ�ĸ���",
					     uident, genbuf);
					sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
					sprintf(buf,"%s����%s�����", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");
					
					move(17, 4);
					prints("�����ɹ���");
					pressanykey();
				}
				break;
		case 'k':
		case 'K':
			clear();
			move(12, 4);
			usercomplete("�����λ��", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (!seek_in_file
			    (DIR_MC "mingren", uident)) {
				prints
				    ("��ID���Ǳ���ٸ���ˡ�");
				pressanykey();
				break;
			}
			if (askyn("ȷ����", NA, NA) == YEA) {
				del_from_file(DIR_MC
					      "mingren",
					      uident);
				sprintf(genbuf,
					"%s �س�������",
					uident);
				deliverreport(genbuf,
			      "������Ҫ��һ��Ѫ���ȷ���");
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%s���%s�����", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("��ְ�ɹ���");
				pressanykey();
			}
			break;
		case 'l':
		case 'L':
			clear();
			move(15, 4);
			usercomplete("����˭Ϊ����ٸ�������Ĳ��ѣ�", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "chayou", uident)) {
				prints("��ID�Ѿ��ǲ����ˡ�");
				pressanykey();
				break;
			}
			if (askyn("ȷ����", NA, NA) == YEA) {
				addtofile(DIR_MC "chayou",
					  uident);
				sprintf(genbuf,
					"��ϲ%s��Ϊ����ٸ�������Ĳ���",
					uident);
				deliverreport(genbuf,
			      "������ʱ���������Ȳ����ͣ�");
				mail_buf
				    ("������ʱ���������Ȳ����ͣ�",
				     uident, genbuf);
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%s����%sΪ����", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				
				move(17, 4);
				prints("�����ɹ���");
				pressanykey();
			}
			break;
		case 'm':
		case 'M':
			clear();
			move(12, 4);
			usercomplete("�����λ��", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (!seek_in_file
			    (DIR_MC "chayou", uident)) {
				prints
				    ("��ID���Ǳ���ٸ�������Ĳ��ѡ�");
				pressanykey();
				break;
			}
			if (askyn("ȷ����", NA, NA) == YEA) {
				del_from_file(DIR_MC
					      "chayou",
					      uident);
				sprintf(genbuf,
					"%s �س�������",
					uident);
				deliverreport(genbuf,
			      "��л��һֱ�����Դ��̵Ĺ�ע��");
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%sȡ��%s�Ĳ������", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("��ְ�ɹ���");
				pressanykey();
			}
			break;
	       case 'n':
		case 'N':
			clear();
			move(15, 4);
			usercomplete("����˭Ϊ��������", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "gongji", uident)) {
				prints("��ID�Ѿ����������ˡ�");
				pressanykey();
				break;
			}
			if (askyn("ȷ����", NA, NA) == YEA) {
				addtofile(DIR_MC "gongji",
					  uident);
				sprintf(genbuf,
					"��ϲ%s����������ƺ�",
					uident);
				 deliverreport(genbuf,
			      "����ٸ�������Ķ���һë���ε���Ϊ��ʾ������");
				 //deliverreport(genbuf,
			      //"����ٸ�������Ķ���һ��ļ�����أ��ڼ��Լ��ʾ���ͣ�");
				 mail_buf
				    ("����������ƺ�",
				     uident, genbuf);
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%s����%sΪ������", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				
				move(17, 4);
				prints("�����ɹ���");
				pressanykey();
			}
			break;
		case 'o':
		case 'O':
			clear();
			move(12, 4);
			usercomplete("�����λ��", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (!seek_in_file
			    (DIR_MC "gongji", uident)) {
				prints
				    ("��ID������������");
				pressanykey();
				break;
			}
			if (askyn("ȷ����", NA, NA) == YEA) {
				del_from_file(DIR_MC
					      "gongji",
					      uident);
				sprintf(genbuf,
					"%s ������Ǯ������",
					uident);
				deliverreport(genbuf, "��������������ե����ˮ������������");
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%sȡ��%s���������ƺ�", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("��ְ�ɹ���");
				pressanykey();
			}
			break;

		case 'y':
		case 'Y':
			clear();
			move(13, 4);
			usercomplete("����˭Ϊ����id��", uident);
			move(14, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "jijin", uident)) {
				prints("��ID�Ѿ��ǻ���ID�ˡ�");
				pressanykey();
				break;
			}
			getdata(14, 4, "�������ƣ�", buf, 50, DOECHO, YEA);
			sprintf(genbuf, "[����]����%s����%s", buf, uident);
			getdata(15, 4, "ԭ��", buf, 50, DOECHO, YEA);
			sprintf(letter, "����ԭ��%s\nϣ���������������ְ�أ����������Ч�Ļ�����ϵ��", buf);
			move(16, 4);
			if (askyn("ȷ����", NA, NA) == NA) 
				break;
			addtofile(DIR_MC "jijin",uident);
			if (!seek_in_file(DIR_MC "mingren", uident)) 
				addtofile(DIR_MC "mingren",uident);
			//����id�Ǹ�������Ļ����
			 deliverreport(genbuf, letter);
			 mail_buf (letter, uident, genbuf);
			sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
			sprintf(buf,"%s����%sΪ����ID", currentuser.userid, uident);
			millionairesrec(genbuf, buf, "");
			move(17, 4);
			prints("�����ɹ���");
			pressanykey();
			break;
			
		case 'z':
		case 'Z':
			clear();
			move(12, 4);
			usercomplete("�����λ��", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			if (!seek_in_file(DIR_MC "jijin", uident)) {
				prints
				    ("��ID���ǻ���ID��");
				pressanykey();
				break;
			}
			getdata(15, 4, "ԭ��", buf, 50, DOECHO, YEA);
			sprintf(letter, "����ԭ��%s", buf);
			move(16, 4);
			if (askyn("ȷ����", NA, NA) == NA) 
				break;
			del_from_file(DIR_MC"jijin", uident);
			del_from_file(DIR_MC"mingren", uident);
			//һ��ȡ�������
			sprintf(genbuf, "[����]��������%s", uident);
			 deliverreport(genbuf, letter);
			 mail_buf (letter, uident, genbuf);
			sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
			sprintf(buf,"%s��������%s", currentuser.userid, uident);
			millionairesrec(genbuf, buf, "");
			move(17, 4);
			prints("����ɹ���");
			pressanykey();
			break;
					
		case 'p':
		case 'P':
			clear();
			fp1 = fopen( MC_STOCK_BOARDS, "r" );
			count = listfilecontent(MC_STOCK_BOARDS);
			clear();
			for (j = 0; j < count; j++) {
				fscanf(fp1, "%s", stockboard[j]);
			}
			fclose(fp1);

			move(12, 4);
			if (askyn("ȷ��Ҫ��ʼ��������", NA, NA) == YEA)
			{
				for (i = 0; i < numboards; i++) 
					for (j = 0; j < count; j++) 
						if (!strcmp(bcache[i].header.filename, stockboard[j])) 
						{
//									stock_price[j] = utmpshm->ave_score / 100 + bcache[i].score / 20;
							if (bcache[i].score > 10000)
									bcache[i].stocknum = bcache[i].score * 2000;
								else
									bcache[i].stocknum = bcache[i].score * 1000;
							if (bcache[i].stocknum < 50000)
								bcache[i].stocknum = 50000;
						}
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"%s��ʼ������", currentuser.userid);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("���г�ʼ���ɹ���");
				pressanykey();
			}
			break;
			
		case 'r':
		case 'R':
                        clear();
                        move(1, 0);                                                          
                        prints("Ŀǰ�����õ�ID�б�");                                
                        listfilecontent(DIR_MC "mingren");                                      
                        pressanykey();                                                       
                        break;
						
		case 's':                                                                    
              case 'S':                                                                    
                        clear();
                        move(1, 0);                                                          
                        prints("Ŀǰ���ѵ�ID�б�");                                      
                        listfilecontent(DIR_MC "chayou");                                      
                        pressanykey();                                                       
                        break;
						
                case 't':                                                                    
                case 'T':                                                                    
                        clear();
                        move(1, 0);                                                          
                        prints("Ŀǰ��������ID�б�");                                      
                        listfilecontent(DIR_MC "gongji");                                      
                        pressanykey();                                                       
                        break;
						
		  case 'u':                                                                    
                case 'U':                                                                    
                        clear();
                        move(1, 0);                                                          
                        prints("Ŀǰ����ID�б�");                                      
                        listfilecontent(DIR_MC "jijin");                                      
                        pressanykey();                                                       
                        break;
						
		case 'X':
		case 'x':
			money_deny();
			break;

		int num=0;
		case '1':
			clear();
			move(12, 4);
			usercomplete("����˭���ֽ����", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			prints("%sĿǰ���ֽ�%d����ٸ�ҡ�", uident, 
				loadValue(uident, MONEY_NAME, MAX_MONEY_NUM));
			getdata(14, 4, "��Ϊ����?", genbuf, 10, DOECHO, YEA);
			num = atoi(genbuf);
			sprintf(buf, "ȷ��Ҫ��Ϊ%d��", num);
			move(15, 4);
			if (askyn(buf, NA, NA) == YEA) {
				saveValue(uident ,MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
				saveValue(uident ,MONEY_NAME, num, MAX_MONEY_NUM);
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"����%s�ֽ�����Ϊ%d", uident, num);
				millionairesrec(genbuf, buf, "");
				move(17, 4);
				prints("�޸ĳɹ���");
				pressanykey();
			}
			break;
		case '2':
			clear();
			move(12, 4);
			usercomplete("����˭�Ĵ�����", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressanykey();
				break;
			}
			prints("%sĿǰ�д��%d����ٸ�ҡ�", uident, 
				loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM));
			getdata(14, 4, "��Ϊ����?", genbuf, 10, DOECHO, YEA);
			num = atoi(genbuf);
			sprintf(buf, "ȷ��Ҫ��Ϊ%d��", num);
			move(15, 4);
			if (askyn(buf, NA, NA) == YEA) {
				saveValue(uident, CREDIT_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
				saveValue(uident, CREDIT_NAME, num, MAX_MONEY_NUM);
				sprintf(genbuf, "%s��ʹ����Ȩ��", currentuser.userid);
				sprintf(buf,"����%s�������Ϊ%d", uident, num);
				millionairesrec(genbuf, buf, "");
				move(17, 4);
				prints("�޸ĳɹ���");
				pressanykey();
			}
			break;

		case '0':
			clear();
			move(6, 4);
			sprintf(buf, "ȷ��Ҫ%s����������", 
				(utmpshm->mc.isMCclosed)?"����":"�ر�");
			if (askyn(buf, NA, NA) == YEA)
				utmpshm->mc.isMCclosed = (utmpshm->mc.isMCclosed)?0:1;
			move(9, 4);
			prints("�޸ĳɹ���");
			pressanykey();
			break;

		case 'i':
		case 'I':
			addOrDel_contrb();
			break;
			
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static int//��Ʊ36ѡ7
valid367Bet(char *buf)
{
	int i, j;
	int temp[7];
	int slot = 0;

	if (strlen(buf) != 20) {	/*  ���ȱ���Ϊ20 */
		return 0;
	}
	for (i = 0; i < 20; i++) {	/*  ������ʽ������ȷ   */
		if ((i % 3 == 2) && buf[i] != '-') {
			return 0;
		}
		if ((i % 3 != 2) && !(buf[i] >= '0' && buf[i] <= '9')) {
			return 0;
		}
		if (i % 3 == 0) {
			temp[slot] = (buf[i] - '0') * 10 + (buf[i + 1] - '0');
			if (temp[slot] > 36) {
				return 0;
			}
			slot++;
		}
	}
	for (i = 0; i < 7; i++) {	/* �������ظ� */
		for (j = 0; j < 7; j++) {
			if (temp[j] == temp[i] && i != j) {
				return 0;
			}
		}
	}
	return 1;
}

static int//��Ʊ36ѡ7
make367Prize(char *bet, char *prizeSeq)
{
	int count = 0;
	int i, j;
	int len = strlen(bet);

	if (strlen(bet) != strlen(prizeSeq)) {
		return 0;
	}
	for (i = 0; i + 1 < len; i = i + 3) {
		for (j = 0; j + 1 < len; j = j + 3) {
			if (bet[i] == prizeSeq[j]
			    && bet[i + 1] == prizeSeq[j + 1]) {
				count++;
			}
		}
	}
	return count;
}

static void//��Ʊ36ѡ7
make367Seq(char *prizeSeq)
{
	int i, j;
	int num;
	int temp[7];
	int slot = 0;
	int success;

	srandom(time(0));
	for (i = 0; i < 7; i++) {
		do {		/*  ���ֲ�����ͬ  */
			success = 1;
			num = 1 + random() % 36;
			for (j = 0; j <= slot; j++) {
				if (num == temp[j]) {
					success = 0;
					break;
				}
			}
			if (success) {
				temp[slot++] = num;
			}
		} while (!success);
		prizeSeq[3 * i] = (char) (num / 10 + '0');
		prizeSeq[3 * i + 1] = (char) (num % 10 + '0');
		if (i != 6) {
			prizeSeq[3 * i + 2] = '-';
		} else {
			prizeSeq[3 * i + 2] = '\0';
		}
	}

	sprintf(genbuf, "�����ǣ�  %s  �����н�����", prizeSeq);
	deliverreport("[����]����36ѡ7��Ʊҡ�����", genbuf);
}

static int/*��Ʊ26ѡ7 */
open_36_7(void)
{
	FILE *fp;
	char line[MAX_RECORD_LINE];
	char prizeSeq[MAX_BET_LENGTH];
	char *bet;
	char *userid;
	int prizeType;
	int totalMoney, remainMoney;
	int num_bp = 0, num_1p = 0, num_2p = 0, num_3p = 0, num_cp = 0;

	make367Seq(prizeSeq);	//��������

	fp = fopen(DIR_MC_TEMP "36_7_list", "r");
	if (!fp) {
		return -1;
	}
	while (fgets(line, MAX_RECORD_LINE, fp)) {
		userid = strtok(line, " ");
		bet = strtok(NULL, "\n");
		if (!userid || !bet) {
			continue;
		}
		/*   ---------------------���㽱��----------------------- */
		prizeType = make367Prize(bet, prizeSeq);
		switch (prizeType) {
		case 7:
			addtofile(DIR_MC_TEMP "36_7_bp", userid);
			num_bp++;
			break;
		case 6:
			addtofile(DIR_MC_TEMP "36_7_1p", userid);
			num_1p++;
			break;
		case 5:
			addtofile(DIR_MC_TEMP "36_7_2p", userid);
			num_2p++;
			break;
		case 4:
			addtofile(DIR_MC_TEMP "36_7_3p", userid);
			num_3p++;
			break;
		case 3:
			addtofile(DIR_MC_TEMP "36_7_cp", userid);
			num_cp++;
			break;
		default:
			break;
		}
	}			/* end of while */
	fclose(fp);

	/*  ------------------------ ���� --------------------- */
	totalMoney = utmpshm->mc.prize367 + PRIZE_PER;
	remainMoney = totalMoney;
	if (num_bp > 0) {
		int per_bp = (BIG_PRIZE * totalMoney) / num_bp;
		char buf[1024];
		char title[80];

		remainMoney -= BIG_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_bp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_bp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_bp);
			mail_buf(genbuf, userid, "��ϲ�����36ѡ7�صȽ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "����36ѡ7�صȽ�������ÿע����%d����ٸ�ң�",
			per_bp);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_1p > 0) {
		int per_1p = (I_PRIZE * totalMoney) / num_1p;
		char buf[1024];
		char title[80];

		remainMoney -= I_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_1p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_1p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_1p);
			mail_buf(genbuf, userid, "��ϲ�����36ѡ7һ�Ƚ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "����36ѡ7һ�Ƚ�������ÿע����%d����ٸ�ң�",
			per_1p);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_2p > 0) {
		int per_2p = (II_PRIZE * totalMoney) / num_2p;
		char buf[1024];
		char title[80];

		remainMoney -= II_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_2p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_2p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_2p);
			mail_buf(genbuf, userid, "��ϲ�����36ѡ7���Ƚ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "����36ѡ7���Ƚ�������ÿע����%d����ٸ�ң�",
			per_2p);
		deliverreport(title, buf);
		fclose(fp);
	}
	if (num_3p > 0) {
		int per_3p = (III_PRIZE * totalMoney) / num_3p;
		char buf[2048];
		char title[80];

		remainMoney -= III_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_3p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_3p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_3p);
			mail_buf(genbuf, userid, "��ϲ�����36ѡ7���Ƚ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "����36ѡ7���Ƚ�������ÿע����%d����ٸ�ң�",
			per_3p);
		deliverreport(title, buf);
		fclose(fp);

	}
	if (num_cp > 0) {
		int per_cp = CMFT_PRIZE;
		char buf[2048];
		char title[80];

		remainMoney -= CMFT_PRIZE * num_cp;

		fp = fopen(DIR_MC_TEMP "36_7_cp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_cp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_cp);
			mail_buf(genbuf, userid, "��ϲ�����36ѡ7��ο����");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "����36ѡ7��ο��������ÿע����%d����ٸ�ң�",
			CMFT_PRIZE);
		deliverreport(title, buf);
		fclose(fp);
	}
	remainMoney = limitValue(remainMoney, MAX_POOL_MONEY);
	utmpshm->mc.prize367 = remainMoney;
	remove(DIR_MC_TEMP "36_7_list");
	remove(DIR_MC_TEMP "36_7_bp");
	remove(DIR_MC_TEMP "36_7_1p");
	remove(DIR_MC_TEMP "36_7_2p");
	remove(DIR_MC_TEMP "36_7_3p");
	remove(DIR_MC_TEMP "36_7_cp");
	remove(DIR_MC_TEMP "36_7_start");
	return 0;
}

static int /*��Ʊ--���*/
computeSum(char *complexBet)
{				/*���㸴ʽע������ */
	int i;
	int len;
	int countNum = 0;
	int total = 1;

	len = strlen(complexBet);

	for (i = 0; i < len; i++) {
		if (complexBet[i] == '-') {
			total *= countNum;
			countNum = 0;
		} else {
			countNum++;
		}
	}
	total *= countNum;	/*���һ����Ԫ */
	return total;
}

static void/*��Ʊ--���*/
saveSoccerRecord(char *complexBet)
{				/*���渴ʽעΪ��ע */
	int i, j;
	int len;
	int simple = 1;
	int meet = 0;
	int count = 0;
	int firstDivEnd, firstDivStart;
	char buf[100];

	len = strlen(complexBet);
	firstDivEnd = len;

	for (i = 0; 2 * i + 1 < len; i++) {
		if (complexBet[2 * i + 1] != '-') {
			simple = 0;
			break;
		}
	}
	if (simple) {		/*�򵥱�׼��ʽ��ֱ�Ӵ�ӡ */
		for (i = 0, j = 0; i < len; i++) {
			if (complexBet[i] != '-') {
				genbuf[j++] = complexBet[i];
			}
		}
		genbuf[j] = '\0';
		sprintf(buf, "%s %s", currentuser.userid, genbuf);
		addtofile(DIR_MC_TEMP "soccer_list", buf);
	} else {
		for (i = 0; i < len; i++) {	/*Ѱ�ҵ�һ����ʽ��Ԫ */
			if (complexBet[i] == '-') {
				if (count > 1 && !meet) {
					firstDivEnd = i;
					break;
				} else {
					count = 0;
				}
			} else {
				count++;
			}
		}
		firstDivStart = firstDivEnd - count;
		firstDivEnd--;

		for (i = 0; i < count; i++) {	/*��ÿһ��Ҫ��ֵĵ�Ԫ��Ԫ�� */
			int slot = 0;
			char *temp = malloc(len * sizeof (char));

			/*�õ�ǰ��Ĳ��� */
			if (firstDivStart != 0) {
				for (j = 0; j < firstDivStart; j++, slot++) {
					temp[slot] = complexBet[j];
				}
			}
			temp[slot] = complexBet[firstDivStart + i];
			slot++;
			/*�õ�����Ĳ��� */
			for (j = firstDivEnd + 1; j < len; j++, slot++) {
				temp[slot] = complexBet[j];
			}
			temp[slot] = '\0';

			/*��ÿһ����֣����еݹ���� */
			saveSoccerRecord(temp);
		}

	}
}

static int /*��Ʊ--���*/
validSoccerBet(char *buf)
{
	int count = 0;
	int meetSeperator = 1;
	int i;
	int first = 0, second = 0;

	if (strlen(buf) == 0) {
		return 0;
	}
	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == '-') {
			if (meetSeperator == 1) {	/*�����������-���϶�����ȷ */
				return 0;
			}
			count = 0;
			meetSeperator = 1;
		} else {
			if (buf[i] != '3' && buf[i] != '1' && buf[i] != '0') {	/*����310���϶����� */
				return 0;
			}
			count++;
			if (count > 3) {
				return 0;
			}
			if (count == 1) {
				first = buf[i];
			} else if (count == 2) {
				if (buf[i] == first) {	/*�غ� */
					return 0;
				}
				second = buf[i];
			} else if (count == 3) {
				if (buf[i] == first || buf[i] == second) {	/*�غ� */
					return 0;
				}
			}
			meetSeperator = 0;
		}
	}
	if (buf[strlen(buf) - 1] == '-') {
		return 0;
	}
	return 1;
}

static int /*��Ʊ--���*/
makeSoccerPrize(char *bet, char *prizeSeq)
{
	int diff = 0;
	int i;
	int n1 = strlen(bet);
	int n2 = strlen(prizeSeq);

	if (n1 != n2) {
		return 10;	/*���н� */
	}
	for (i = 0; i < n1; i++) {
		if (bet[i] != prizeSeq[i]) {
			diff++;
		}
	}
	return diff;
}

static int /*��Ʊ--���*/
open_soccer(char *prizeSeq)
{
	FILE *fp;
	char line[MAX_RECORD_LINE];
	char *bet;
	char *userid;
	int prizeType;
	int totalMoney, remainMoney;
	int num_bp = 0, num_1p = 0, num_2p = 0, num_3p = 0, num_cp = 0;

	fp = fopen(DIR_MC_TEMP "soccer_list", "r");

	if (!fp) {
		return -1;
	}
	sprintf(genbuf, "�����ǣ�%s�����н�����", prizeSeq);
	deliverreport("[����]������ʽ��", genbuf);
	while (fgets(line, MAX_RECORD_LINE, fp)) {
		userid = strtok(line, " ");
		bet = strtok(NULL, "\n");
		if (!userid || !bet) {
			continue;
		}
		/*   ---------------------���㽱��----------------------- */
		prizeType = makeSoccerPrize(bet, prizeSeq);
		switch (prizeType) {
		case 0:	/*��ȫ��ͬ */
			addtofile(DIR_MC_TEMP "soccer_bp", userid);
			num_bp++;
			break;
		case 1:	/*��һ����ͬ */
			addtofile(DIR_MC_TEMP "soccer_1p", userid);
			num_1p++;
			break;
		case 2:	/*�ж�����ͬ */
			addtofile(DIR_MC_TEMP "soccer_2p", userid);
			num_2p++;
			break;
		case 3:	/*��������ͬ */
			addtofile(DIR_MC_TEMP "soccer_3p", userid);
			num_3p++;
			break;
		case 4:	/*���ĸ���ͬ */
			addtofile(DIR_MC_TEMP "soccer_cp", userid);
			num_cp++;
			break;
		default:
			break;
		}
	}			/* end of while */
	fclose(fp);
	/*  ------------------------ ���� --------------------- */
	totalMoney = utmpshm->mc.prizeSoccer + PRIZE_PER;
	remainMoney = totalMoney;
	if (num_bp > 0) {
		int per_bp = (BIG_PRIZE * totalMoney) / num_bp;
		char buf[1024];
		char title[80];

		remainMoney -= BIG_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_bp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_bp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_bp);
			mail_buf(genbuf, userid, "��ϲ����������Ʊ�صȽ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "��������صȽ�������ÿע����%d����ٸ�ң�",
			per_bp);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_1p > 0) {
		int per_1p = (I_PRIZE * totalMoney) / num_1p;
		char buf[1024];
		char title[80];

		remainMoney -= I_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_1p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_1p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_1p);
			mail_buf(genbuf, userid, "��ϲ����������Ʊһ�Ƚ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "�������һ�Ƚ�������ÿע����%d����ٸ�ң�",
			per_1p);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_2p > 0) {
		int per_2p = (II_PRIZE * totalMoney) / num_2p;
		char buf[2048];
		char title[80];

		remainMoney -= II_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_2p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_2p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_2p);
			mail_buf(genbuf, userid, "��ϲ����������Ʊ���Ƚ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "������ʶ��Ƚ�������ÿע����%d����ٸ�ң�",
			per_2p);
		deliverreport(title, buf);
		fclose(fp);
	}
	/*if (num_3p > 0) {
		int per_3p = (III_PRIZE * totalMoney) / num_3p;
		char buf[2048];
		char title[80];

		remainMoney -= III_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_3p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_3p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_3p);
			mail_buf(genbuf, userid, "��ϲ����������Ʊ���Ƚ���");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048, fp);
		sprintf(title, "����������Ƚ�������ÿע����%d����ٸ�ң�",
			per_3p);
		deliverreport(title, buf);
		fclose(fp);

	}*/
	if (num_cp > 0) {
		int per_cp = CMFT_PRIZE;
		char buf[2048];
		char title[80];

		remainMoney -= CMFT_PRIZE * num_cp;

		fp = fopen(DIR_MC_TEMP "soccer_cp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_cp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"���õ��� %d ����ٸ�ҵĽ��𡣹�ϲ��ϣ���´λ��к��ˡ�����",
				per_cp);
			mail_buf(genbuf, userid, "��ϲ����������Ʊ��ο����");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "������ʰ�ο��������ÿע����%d����ٸ�ң�",
			CMFT_PRIZE);
		deliverreport(title, buf);
		fclose(fp);
	}
	remainMoney = limitValue(remainMoney, MAX_POOL_MONEY);
	utmpshm->mc.prizeSoccer = remainMoney;
	remove(DIR_MC_TEMP "soccer_list");
	remove(DIR_MC_TEMP "soccer_bp");
	remove(DIR_MC_TEMP "soccer_1p");
	remove(DIR_MC_TEMP "soccer_2p");
	remove(DIR_MC_TEMP "soccer_3p");
	remove(DIR_MC_TEMP "soccer_cp");
	remove(DIR_MC_TEMP "soccer_start");
	return 0;
}


static int/*�̳�--����*/
money_check_guard()
{
	int money, guard;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	guard = loadValue(currentuser.userid, "guard", 8);
	if (guard > 0) {
		saveValue(currentuser.userid, "guard", -guard, 50);
		move(9, 4);
		if (random() % 2 == 0) {
			prints("��ı��������ȥ,��˳�����������ɵ��ֽ�.");
			saveValue(currentuser.userid, MONEY_NAME, -money / 5,
				  MAX_MONEY_NUM);
		} else {
			prints
			    ("��ı���һ������������,������������һ���Ǯ����·�ˡ�");
			saveValue(currentuser.userid, MONEY_NAME, -money / 2,
				  MAX_MONEY_NUM);
			pressanykey();
			Q_Goodbye();
		}
		return 1;
	}
	return 0;
}

static int /*�Ĳ�--����*/
money_dice()
{
	int quit = 0;
	int ch, num = 0, money;
	int target;
	int t1, t2, t3;
	int win;
	int isVIP;
	char slow[IDLEN + 1];
	char title[STRLEN], buf[256];

	isVIP = seek_in_file(DIR_MC "gamble_VIP", currentuser.userid);
       //isVIP=1;
	while (!quit) {
		clear();
		if (isVIP) {
			money_show_stat("����ٸ�ĳ�������VIP��");
		} else {
			money_show_stat("����ٸ�ĳ�������");
		}
		move(4, 4);
		prints
		    ("\033[1;31m�����׬���������⣬�����֣�Ը�ķ���\033[m");
		move(5, 4);
		prints("�ִ�С���ţ�4-10����С��11-17��Ϊ��");
		move(6, 4);
		prints("��ѺС��С������һ���ʽ�Ѻ��ľ�ȫ��ׯ�ҡ�");
		move(7, 4);
		prints("ׯ��Ҫ��ҡ��ȫ�����������ӵ���һ������ͨ�Դ�С�ҡ�");
		move(8, 4);
		if (isVIP) {
			prints("��Сѹ 100000����ٸ��,���� 10000000 ����ٸ�ҡ�");
		} else {
			prints
			    ("��Сѹ 1000 ����ٸ��,���� 500000 ����ٸ�ҡ�Ҫ�������VIP�ҡ�");
		}
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��ע [Q]�뿪                                                   \033[m");
		win = 0;
		ch = igetkey();
		switch (ch) {
		case '1':
			if (isVIP) {
				getdata(9, 4, "��ѹ���ٱ���ٸ�ң�[100000]", genbuf,
					9, DOECHO, YEA);
			} else {
				getdata(9, 4, "��ѹ���ٱ���ٸ�ң�[1000]", genbuf,
					7, DOECHO, YEA);
			}
			num = atoi(genbuf);
			if (!genbuf[0]){
				if (!isVIP)
					num = 1000;
				else
					num = 100000;
			}
			if (isVIP && num < 100000) {
				move(11, 4);
				prints("������VIP�ң�ѹע�����ޡ�");
				pressanykey();
				break;
			}
			if (!isVIP && num > 500000) {
				move(11, 4);
				prints("Ҫ��Ĵ�ģ����VIP�ҡ�");
				pressanykey();
				break;
			}
			if (num < 1000) {
				move(11, 4);
				prints("��û��Ǯ������ô��Ǯ���ǲ�����ġ�");
				pressanykey();
				break;
			}
			if (num > 10000000) {
				move(11,4);
				prints("����������ע����������ע��");
				pressanykey();
				break;
			}
			getdata(10, 4, "��ѹ��(L)����С(S)��[L]", genbuf, 3,
				DOECHO, YEA);
			if (genbuf[0] == 'S' || genbuf[0] == 's')
				target = 1;
			else
				target = 0;
			sprintf(genbuf,
				"�����֣������� \033[1;31m%d\033[m ����ٸ�ҵ� \033[1;31m%s\033[m��ȷ��ô��",
				num, target ? "С" : "��");
			move(11, 4);
			if (askyn(genbuf, YEA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				if (money < num) {
					move(12, 4);
					prints("ȥȥȥ��û��ô��Ǯ��ʲô�ң�      \n");
					pressanykey();
					break;
				}
				//srandom(time(0));
				t1 = random() % 6 + 1;
				t2 = random() % 6 + 1;
				t3 = random() % 6 + 1;
				move(12, 4);
				if ((t1 == t2) && (t2 == t3)) {
					if (num > 2000000)
						utmpshm->mc.prize777 += 1000000;
					else
						utmpshm->mc.prize777 += num * 50 / 100;
					if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
						utmpshm->mc.prize777 = MAX_MONEY_NUM;
					sprintf(genbuf, "\033[1;32mׯ��ͨɱ��\033[m");
				} else if (t1 + t2 + t3 < 11) {
					sprintf(genbuf,
						"%d �㣬\033[1;32mС\033[m",
						t1 + t2 + t3);
					if (target == 1)
						win = 1;
				} else if (t1 + t2 + t3 > 10) {
					sprintf(genbuf,
						"%d �㣬\033[1;32m��\033[m",
						t1 + t2 + t3);
					if (target == 0)
						win = 1;
				}
				prints("���˿��ˣ�%d %d %d��%s", t1, t2, t3, genbuf);
				move(13, 4);
				if (win) {
					prints("��ϲ��������һ�Ѱɣ�");
					saveValue(currentuser.userid,
						  MONEY_NAME, num,
						  MAX_MONEY_NUM);
					whoTakeCharge(3, slow);//slowaction
                     		saveValue(slow,
						  MONEY_NAME, -num,
						  MAX_MONEY_NUM);

					if (num >= RUMOR_MONEY && random() % 2) {
						int rumor = makeRumor(num);
						sprintf(genbuf,
							"����Ŀ�� %s �ڱ���ٸ�ĳ�һ��Ӯ�� %d �ı���ٸ�ң�",
							currentuser.userid,
							rumor);
						deliverreport
						    ("[ҥ��]���Ա���ٸ�ĳ�����Ϣ", genbuf);
					}
					sprintf(title, "%s����Ĳ�(����)(Ӯ)", currentuser.userid);
					sprintf(buf, "%s������Ӯ��%d����ٸ��", currentuser.userid, num);
					millionairesrec(title, buf, "�Ĳ�����");
				} else {
					prints("û�й�ϵ�������Ӯ...");
					saveValue(currentuser.userid,
						  MONEY_NAME, -num,
						  MAX_MONEY_NUM);
					whoTakeCharge(3, slow);//slowaction
                    			saveValue(slow,
						MONEY_NAME, +num,
						MAX_MONEY_NUM);
					sprintf(title, "%s����Ĳ�(����)(��)", currentuser.userid);
					sprintf(buf, "%s����������%d����ٸ��", currentuser.userid, num);
					millionairesrec(title, buf, "�Ĳ�����");
				}
				pressanykey();
			}
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int /*�ڰ�*/
money_robber()
{
	int quit = 0, guard_num = 0;
	int ch, x, y, z, ch2;
	int num, money, r, ra, id, count = 0, rob,credit;
	int freeTime;
	int zhuannum=20;
	int currentTime = time(0);
	char uident[IDLEN + 1], buf[200], title[40];
	double mathtmp;
	srandom(time(0));
	char letter1[] = "�����Сʱ�ڸ��Ҽ�Ǯ����Ȼ����ÿ���\n";
	char letter2[] =
	    "����Ҽ�Ǯ������С������Դ�����ש��\n�һ�ǹ�����İ�ȫ�ģ��ٺ�...";
	char letter3[] = "����Ҽ�Ǯ������С���Ұ����Ǯȫ�����ߣ�";
	while (!quit) {
		clear();
		money_show_stat("������");
		move(4, 4);
		prints
		    ("����ǰ�ı���ٸ�ڰ��޶���������һʱ��������������ϴ򣬻����������");
		move(5, 4);
		prints("һ��������С��˵����Ҫ��שô�����˺��۵ġ���");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��ש [2]͵�� [3]���� [4]���� [5]�ڰ���� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
		     
		       if(!Allclubtest(currentuser.userid)){  
			   	move(5, 4);
				prints("    \033[1;32m  ��ͨ����Ҫ����\033[m");
				pressanykey();
				break;
			}
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				move(5, 4);
				prints("    \033[1;32m  ���Ѳ�Ҫ����\033[m");
				pressanykey();
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)) {
				move(5, 4);
				prints("    \033[1;32m  ��Ҫ����\033[m");
				pressanykey();
				break;
			}
			r = random() % 40;
			if (r < 1)
				money_police();
			money_show_stat("�ڰ��ש��������");
			move(4, 4);
			prints("����İ�ש�ʵ���������ȥ����һ��ʹ�졣");
			move(5, 4);
			prints("һ���ש 1000 ����ٸ�ҡ�");
			move(6, 4);
			if (currentuser.dietime > 0) {
				prints("���Ѿ����˰���ץ����");
				pressanykey();
				Q_Goodbye();
				break;
			}
			usercomplete("��Ҫ��˭:", uident);
			if (uident[0] == '\0')
				break;
			freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
	       	if (currentTime < freeTime){
				pressreturn();
				break;
			}
			if (!(id = getuser(uident))) {
				move(7, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				move(7, 4);
				prints("������Ҳ���Ź���̫���˰ɣ�");
				pressreturn();
				break;
			}
			if ((slowclubtest("Beggar", currentuser.userid) 
				&& slowclubtest("Beggar", uident)) ||
				(slowclubtest("Rober", currentuser.userid) 
				&& slowclubtest("Rober", uident)) ||
				(slowclubtest("Police", currentuser.userid) 
				&& slowclubtest("Police", uident)) ||
				(slowclubtest("killer", currentuser.userid) 
				&& slowclubtest("killer", uident)))
			{
				move(7, 4);
				prints("�����Լ��ֵ�...");
				pressreturn();
				break;
			}
			getdata(7, 4, "��Ҫ�ļ��飿 [0]", genbuf, 4,
				DOECHO, YEA);
			if (genbuf[0] == '\0')
				break;
			count = atoi(genbuf);
			if (count < 1) {
				move(8, 4);
				prints("û�а�ש����ʲô�ģ�");
				pressanykey();
				break;
			}
			if (currentuser.dietime > 0) {
				prints("���Ѿ����˰���ץ����");
				pressanykey();
				Q_Goodbye();
				break;
			}
			move(8, 4);
			num = count * 1000;
			sprintf(genbuf, "�ܹ���Ҫ %d ����ٸ�ҡ�", num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < num) {
					move(9, 4);
					prints("����Ǯ����...");
					pressanykey();
					break;
				}
				if (money_check_guard()) {
					pressanykey();
					break;
				}
				if(seek_in_file(DIR_MC "mingren", uident)) {
					prints ("      ���л���ӣ��㻹�����˰�\n");
					pressanykey();
					break;
				}

				if (seek_in_file(DIR_MC "killer", currentuser.userid))
					zhuannum=40;
                  
				saveValue(currentuser.userid, "last_rob",
					  -2000000000, 2000000000);
                		saveValue(currentuser.userid, "last_rob",
					  time(0), 2000000000);
				saveValue(currentuser.userid, MONEY_NAME, 
					-num,  MAX_MONEY_NUM);
				
				saveValue("BMYRober", MONEY_NAME, 
					+num/2, MAX_MONEY_NUM);
				
				prints
				    ("        ���������͵���͸��٣��㷢��ÿ������7��10��%s��·��Ƨ����\n",
				     uident);
				prints
				    ("    ����԰�ߡ�������������������ٸ��ש��׼���ж��ˡ�\n");
				prints
				    ("        ���˰�ש����������סԺ��Ǯ���ˣ��ٺ�...\n");
				prints
				    ("        ��Ȼ����Ҳ�����⵽����������������\n");
				if (askyn("    �ϻ���˵���㻹����ô��", YEA, NA) == NA) {
					move(15, 0);
					prints
					    ("            ��������ͷ�㺦���ˣ����Բ����ˡ�\n");
					pressanykey();
					break;
				} else {
					if(!seek_in_file(DIR_MC "gongji", uident))
						saveValue(currentuser.userid, "rob", 1, 50);
					if (currentuser.dietime > 0) {
						prints("���Ѿ����˰���ץ����");
						pressanykey();
						Q_Goodbye();
						break;
					}
					sleep(5);
					x = countexp(&currentuser);
					y = countexp(&lookupuser);
					r = random() % 2;
					if (r == 0)
						z = x;
					else
						z = y;
					r = random() % 100;
					num = 1000 + random() % 2000;
					move(16, 4);
					if (r < 100 * z / (x + x + y + y) +zhuannum+ count)	//���˳ɹ�
					{
						guard_num = loadValue(uident, "guard", 8);
						if (guard_num > 0) {
							saveValue(uident, "guard", -1, 50);
							prints("��ɵ�����һ������");
							pressanykey();
							break;
						}

						prints
						    ("       ���⻵��������͵Ϯ������%s��С�Դ��ϡ�\n",
						     uident);
						money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
						if (money == 0) {
							if(!Allclubtest(lookupuser.userid) || seek_in_file(DIR_MC "chayou", lookupuser.userid)){
								showAt(17, 4, "�㶼�ĵ��˼�ûǮ������...�������°ɣ�\n", 0);
								sprintf(buf,
									"�㱻%s���˰�ש����ûǮ���ˣ�ֻ��ҧ����ʹ...",
									currentuser.userid);
							}else{
								saveValue(uident, MONEY_NAME, -money, MAX_MONEY_NUM);
								move(17, 4);
								prints ("       ������%s��ת�������ˡ�", uident);
								sprintf(genbuf, "%s���кڰ�(��ש)", currentuser.userid);
								sprintf(buf,"%s������%s ", currentuser.userid, uident);
								millionairesrec(genbuf, buf, "�ڰ�");
								lookupuser.dietime = lookupuser.stay + 999 * 60;
								substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
								if (seek_in_file(DIR_MC "killer", currentuser.userid)){
									if (random()%3 == 0){
										sprintf(genbuf, "�㱻%s�ð�ש�����ˣ��ò�",
											currentuser.userid);
										mail_buf(genbuf, uident, "�����е�");}
                                  						sprintf(genbuf,
											"��վ��ʿ%s��10����ǰ��ͭ�����\nһ��ǹ���¼�����������\n����͸¶�����а�ᱳ��\n\n"
									 		"Ŀǰ��վ������֯ɱ����������Դ��¸���\n�й��¼��Ľ�һ���������ע��������", uident);
                                   					deliverreport("[����]ͭ���巢��һ��ǹ���¼�", genbuf);
								}
								else if (slowclubtest("Beggar", currentuser.userid)){
									sprintf(genbuf,
										"������ʿ%s��10����ǰ�ڼ�ɳ�׵�\nһ������ͻ�����ز���\n�����ƴ����а�ᱳ��\n\n"
									 	"����Ϣ��ͨ��ʿ͸¶�����¼������\n��ؤ���й�", uident);
                                   				deliverreport("[����]��ɳ�׷���һ�����¼�",genbuf);
								   	sprintf(genbuf,
										"�㱻ؤ�����%s�ð�ש�����ˣ��ò�", currentuser.userid);
									mail_buf(genbuf, uident, "������");
								}
								else if (slowclubtest("Rober",currentuser.userid)){
									sprintf(genbuf,
										"������ʿ%s��10����ǰ�ڰ��ŵ�\nһ��ڰ�е����ɥ��\n�������ɴ����������й���\n\n"
									 	"��һλ��Ը͸¶�����ľ����Ա͸¶\n����¼����ܺͺڰ�Ѱ���й�\n������ʾһ��������ά���ΰ�", uident);
                                   				deliverreport("[����]���ŷ���һ�����ͻ", genbuf);
									sprintf(genbuf,"�㱻%s�ð�ש�����ˣ��ò�", currentuser.userid);
									mail_buf(genbuf, uident,"������");
								}
								else if (slowclubtest("killer",currentuser.userid)){
									sprintf(genbuf,
									"���ںͺڰ�ĳ�ͻ�б�%s�ð�ש�����ˣ��ò�", currentuser.userid);
									mail_buf(genbuf, uident,"�����е�");
                                  					sprintf(genbuf,
										"��վ��ʿ%s��10����ǰ�ھ�����\nһ��ǹ���¼�����������\n����͸¶�����а�ᱳ��\n\n"
									 	"��������������ɱ����˽�˶�Թ��\n�й��¼��Ľ�һ���������ע��������", uident);
                                   				deliverreport("[����]��������һ��ǹ���¼�", genbuf);		
							     }
								else{
									sprintf(genbuf, "�㱻%s�ð�ש�����ˣ��ò�", currentuser.userid);
									mail_buf(genbuf, uident, "������");
								}
								//saveValue(lookupuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
								pressanykey();
							}  
						} else {
							saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
							sprintf(buf,
								"������%s����%dԪ���ˣ����ڳ�Ժ�ˡ�С�ı����㣡\n",
								uident, num);
							move(17, 4);
							prints("%s", buf);
							sprintf(buf,
								"�㱻%s���˰�ש������%d����ٸ�����ˣ���������...",
								currentuser.userid, num);
						}
					} else {
						prints
						    ("      �ܲ��ң���û�����С�����������С�Դ���...");

						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						num *= 3;
						if (money < num) {
							saveValue (currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							showAt(17, 4, "����Ѫֱ��������Ǯ�������ƣ���ҽԺ���˳�����", 0);
							showAt(18, 4, "������ƶ񻯣�������...", 0);
							sprintf(genbuf, "%s���кڰ�(��ש)", currentuser.userid);
							sprintf(buf,"%s��%s, �Լ�����, �� ", currentuser.userid, uident);
							millionairesrec(genbuf, buf, "�ڰ�");
							set_safe_record();
							currentuser.dietime = currentuser.stay + (num - money);
							substitute_record (PASSFILE, &currentuser, sizeof(currentuser), usernum);
							saveValue(currentuser.userid, MONEY_NAME,  -MAX_MONEY_NUM,  MAX_MONEY_NUM);
							saveValue(currentuser.userid, CREDIT_NAME,  -MAX_MONEY_NUM,  MAX_MONEY_NUM);
							pressanykey();
							Q_Goodbye();
						} else {
							saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
							move(17, 4);
							sprintf(buf, "�㻨��%d����ٸ�Ҳ��κ����ˣ������´λ����˲���",
								num);
							prints("%s", buf);
						}
					}
				}
				pressanykey();
			}
			break;
		case '2':
			clear();
			if(!Allclubtest(currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ��ͨ����Ҫ����\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ���Ѳ�Ҫ����\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ��Ҫ����\033[m", 1);
				break;
			}			
			move(6, 4);
			usercomplete("͵˭��", uident);
			if (uident[0] == '\0')
				break;
			/*if(!Allclubtest(uident)){ 
				prints("    \033[1;32m  ��Ҫ��ɱ�޹���\033[m");
				pressanykey();
				break;
			}*/
			freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
			if (currentTime < freeTime){
				pressreturn();
				break;
			}
			if (!getuser(uident)) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt (7, 4, "      ���л���ӣ��㻹�����˰�\n", 1);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "������Ҳ���Ź���̫���˰ɣ�", 1);
				break;
			}
			if(strcmp(lookupuser.userid,"BMYpolice")==0||strcmp(lookupuser.userid,"BMYbeg")==0||
				strcmp(lookupuser.userid,"BMYRober")==0||strcmp(lookupuser.userid,"BMYboss")==0||
                		strcmp(lookupuser.userid,"BMYKillersky")==0){
				showAt(7, 4, "������������ݣ�������", 2);
				break;
			}
			credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
			if(credit<2000){
				showAt(7, 4, "��֤��û�У����ǲ�Ҫ͵��!", 2);
				break;
			}

			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);

			if (currentuser.stay < 86400) {
				showAt(7, 4, "С���Ӽұ�ѧ����!", 2);
				break;
			}
			getdata(7, 4, "�������������: ", buf, PASSLEN, NOECHO, YEA);
			if (*buf == '\0'
			    || !checkpasswd(currentuser.passwd, buf)) {
				showAt(8, 4, "�ܱ�Ǹ, ����������벻��ȷ��", 2);
				break;
			}
			  saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
			  saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
			showAt(9, 4,
				"\033[1;5;31m����\033[0;1;31m�� С�İ�������������ϴ�Ŷ��", 0);
			move(10, 4);
			if (askyn("���Ҫ͵ô��", NA, NA) == NA)
				break;
			set_safe_record();
			if (currentuser.dietime > 0) {
				showAt(11, 4, "���Ѿ����˰���ץ����", 1);
				Q_Goodbye();
				break;
			}
			if (money_check_guard()) {
				pressanykey();
				break;
			}
			//currentuser.stay -= 3600 * 1;
			//substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
			r = random() % 100;
			x = countexp(&currentuser);
			y = countexp(&lookupuser);
			if(random() % x *0.7<random() % y)
			{
			//saveValue(currentuser.userid, CREDIT_NAME, -2000, MAX_MONEY_NUM);
			//saveValue("police", MONEY_NAME, +2000, MAX_MONEY_NUM);
			}

			if (NULL == t_search(uident, NA, 1))
				ra = 1;
			else
				ra = 10;
			if (r < 100 * x / (x + x + y + y) / ra) {
				guard_num = loadValue(uident, "guard", 8);
				if (guard_num > 0) {
					if (loadValue(uident, MONEY_NAME, MAX_MONEY_NUM) > guard_num * 1000000) {
						if (random() % 2 == 0)
							saveValue(uident, "guard", -2, 50);
						else
							saveValue(uident, "guard", -1, 50);
					} else {
							saveValue(uident, "guard", -1, 50);
					}
					showAt(11, 4, "��ɵ�����һ������", 1);
					break;
				}
				if (random() % 2 == 0) {
					money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
					r = random() % 50;
					money = money / 100 * r;
					saveValue(uident, MONEY_NAME, -money, MAX_MONEY_NUM);
					saveValue(currentuser.userid, MONEY_NAME, money,
						  MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31m%s\033[m ��Ǯ��û�źã���������ȥ�������� %d ����ٸ���ֽ𣬿��ܰ�...",
					     uident, money);
					sprintf(title, "%s���кڰ�(͵��)", currentuser.userid);
					sprintf(buf,"%s͵��%s %d����ٸ��", currentuser.userid, uident, money);
					if (money != 0)
						millionairesrec(title, buf, "�ڰ�");
					sprintf(buf,
						"%s ������ע���ʱ��͵���� %d ����ٸ�ҡ�",
						currentuser.userid, money);
					sprintf(title, "�Բ�������͵��");
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					pressanykey();
					break;
				} else {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					r = random() % 70;
					money = money / 100 * r;
					saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
					saveValue(uident, MONEY_NAME, money, MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31m��ȥ�� %s ��Ǯ��,�ۿ��Ѿ�������,����Ȼת��������������",
					     uident);
					move(12, 4);
					prints
					    ("\033[1;31m��ѽѽ,��һ���,����û͵������Ǯ��,�������������� %d ����ٸ�ҡ�",
					     money);
					sprintf(title, "%s���кڰ�(͵��)", currentuser.userid);
					sprintf(buf,"%s͵%s, ��������%d����ٸ��", currentuser.userid, uident, money);
					if (money != 0)
						millionairesrec(title, buf, "�ڰ�");
					sprintf(title, "������С͵");
					sprintf(buf,
						"%s �������ע��͵����Ǯ��,������㷢���ˡ��㷴������ %d ����ٸ�ҡ����׬����,^_^",
						currentuser.userid, money);
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					pressanykey();
					break;
				}

			} else if (r < 90) {
				money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
				rob = loadValue(currentuser.userid, "rob", 50);
				move(11, 4);
				if (rob > 20) {
					saveValue(currentuser.userid, "rob", -rob/2, 50);
					prints
					    ("�����о��죬�������ܵ�ʱ��ֻ��һ��ǹ��...");
					set_safe_record();
					if (money / 200 < 3600)
						currentuser.dietime = currentuser.stay + 1000*60;
					else if (money < 10000000){
						mathtmp = (double)(money)/10000;
						mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//����ϵĶ��κ���
						currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
					}
					else{
						mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
						currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
					}
					substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
					saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
					pressanykey();
					Q_Goodbye();
				} else {
					if (askyn
					    ("�����췢����,��Ҫ����ô?", YEA, NA) == NA) {
						saveValue(currentuser.userid, "rob", 1, 50);
						move(12, 4);
						if (askyn ("�������㻰,��׼��̹�״ӿ�ô?", YEA, NA) == YEA) {
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							saveValue(currentuser.userid, MONEY_NAME, -money * 50 /100, MAX_MONEY_NUM);
							sprintf(title, "%s���кڰ�(͵��)", currentuser.userid);
							sprintf(buf,"%s͵%s, ������û��%d����ٸ��", currentuser.userid, uident, money/2);
							if (money != 0)
								millionairesrec(title, buf, "�ڰ�");
							showAt
							    (13, 4, "�㱻���������,��û�����������е�Ǯ֮��,��Ҫ����ѵ��һ����", 0);
							showAt
							    (14, 4, "�����Ǿ�������15����ѵ��ʱ�䣬����ʵʵ���Űɡ�", 1);
							sleep(15);
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							sprintf(genbuf,
								"���˾����,����˵Ĵ�Ь���ͳ���������%d����ٸ�ҡ�����,һ�ɳ���Ѿ��ζ...",
								money);
							showAt(15, 4, genbuf, 1);							
						} else {
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							if (random() % 2 == 0) {
								saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
								sprintf(title, "%s���кڰ�(͵��)", currentuser.userid);
								sprintf(buf,"%s͵%s, ������û��%d����ٸ��(ȫ��)", currentuser.userid, uident, money);
								if (money != 0)
									millionairesrec(title, buf, "�ڰ�");
								showAt
								    (13, 4, "�����ʻ��㻹����ʵ,��һŭ֮��һ�Ѷ�����Ǯ��,�ﳤ��ȥ��", 0);
								showAt
								    (14, 4, "�����ڵ��ϴ��:\"����һ�Ұ�!�ҵ�Ǯ,�ҵ�Ǯ...\"", 1);
							} else {
								showAt
								    (13, 4, "�����ʻ�ʱ��ٰ����,�������Ҳ����û�취,ֻ�ð������.", 0);
								showAt
								    (14, 4, "����! ���ܴ���,�ؼҹ���", 1);
							}
						}
					} else {
						move(12, 4);
						if (random() % 2 == 0) {
							saveValue(currentuser.userid, "rob", 5, 50);
							prints("��û��������,��ϧ,Ǯ��������·��...");
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							sprintf(title, "%s���кڰ�(͵��)", currentuser.userid);
							sprintf(buf,"%s͵%s, ��������ʧ%d����ٸ��(ȫ��)", currentuser.userid, uident, money);
							if (money != 0)
								millionairesrec(title, buf, "�ڰ�");
							pressanykey();
						} else {
							saveValue(currentuser.userid, "rob", -rob/2, 50);
							prints
							    ("�����������ܵ�ʱ��ֻ��һ��ǹ��...");
							set_safe_record();
							if (money / 200 < 3600)
								currentuser.dietime = currentuser.stay + 1000*60;
							else if (money < 10000000){
								mathtmp = (double)(money)/10000;
								mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//����ϵĶ��κ���
								currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
							}
							else{
								mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
								currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
							}
							substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
							pressanykey();
							saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
							sprintf(title, "%s���кڰ�(͵��)", currentuser.userid);
							sprintf(buf,"%s͵%s, ������, ��ʧ%d����ٸ��(ȫ��)", currentuser.userid, uident, money);
							if (money != 0)
								millionairesrec(title, buf, "�ڰ�");
							Q_Goodbye();
						}
					}
				}
				break;
			} else {
				move(11, 4);
				prints
				    ("\033[1;31m%s\033[m ��Ǯ�����ý����ģ����װ��С��ײ����һ��,��һ��Ǯ��û͵����",
				     uident);
				pressanykey();
				break;
			}
			break;
		case '3':
			clear();
			money_show_stat("����ٸ�ڰ����볡");
			showAt
			    (4, 4, "�ڰ�Ϊ���ṩ�����ż�����ҵ��,ÿ���շ������ζ�����", 0);
			if (currentuser.dietime > 0) {
				showAt(5, 4, "���Ѿ����˰���ץ����", 1);
				Q_Goodbye();
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				clear();
				showAt(5, 4, "    \033[1;32m  ��Ҫ����\033[m", 1);
				break;
			}			
			usercomplete("��Ҫ����˭:", uident);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "����Ҳ��������...", 1);
				break;
			}
			move(8, 4);
			sprintf(genbuf, "ȷ��Ҫ����ô?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < 100) {
					showAt(9, 4, "����Ǯ������", 1);
					break;
				}
				if (money < 1000) {
					saveValue(currentuser.userid, MONEY_NAME, -100, MAX_MONEY_NUM);
					sprintf(title, "%s����Ҫ��ǧ�����ٸ��", currentuser.userid);
					mail_buf(letter1, uident, title);
				} else if (money < 100000) {
					saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
					sprintf(title, "%s����Ҫ��������ٸ��", currentuser.userid);
					mail_buf(letter2, uident, title);
				} else if (money < 10000000) {
					saveValue(currentuser.userid, MONEY_NAME, -100000, MAX_MONEY_NUM);
					sprintf(title, "%s����Ҫһ�������ٸ��", currentuser.userid);
					mail_buf(letter3, uident, title);
				} else {
					saveValue(currentuser.userid, MONEY_NAME, -500000, MAX_MONEY_NUM);
					sprintf(title, "%s����Ҫһǧ�����ٸ��", currentuser.userid);
					mail_buf(letter3, uident, title);
				}
				showAt(10, 4, "�ŷ���ȥ�ˣ���ȥ����Ϣ�ɡ�", 1);
			}
			break;
		case '4':
			clear();
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ���Ѳ�Ҫ����\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ��Ҫ����\033[m", 1);
				break;
			}			
			move(6, 4);
			usercomplete("��˭��", uident);
			if (uident[0] == '\0')
				break;
			/*if(!Allclubtest(uident)){ 
				prints("    \033[1;32m  ��Ҫ��ɱ�޹���\033[m");
				pressanykey();
				break;
			}*/
			freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
			if (currentTime < freeTime){
				pressreturn();
				break;
			}
			if (!getuser(uident)) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt(7, 4, "      ���л���ӣ��㻹�����˰�\n", 1);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "������Ҳ���Ź���̫���˰ɣ�", 1);
				break;
			}
			if(strcmp(lookupuser.userid,"BMYpolice")==0||strcmp(lookupuser.userid,"BMYbeg")==0||
				strcmp(lookupuser.userid,"BMYRober")==0||strcmp(lookupuser.userid,"BMYboss")==0||
                		strcmp(lookupuser.userid,"BMYKillersky")==0){
				showAt(7, 4, "������������ݣ�������", 2);
				break;
			} 
			money = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);

			if (currentuser.stay < 3600 + 86400) {
				showAt(7, 4, "С���ӼҲ�Ҫѧ����!", 2);
				break;
			}
			if (!clubtest("Rober")) {
				showAt(7, 4, "��ô����Ҳ���������鷸�Ƶ��˰���", 2);
				break;
			}
			getdata(7, 4, "�������������: ", buf, PASSLEN, NOECHO, YEA);
			if (*buf == '\0'
			    || !checkpasswd(currentuser.passwd, buf)) {
				showAt(8, 4, "�ܱ�Ǹ, ����������벻��ȷ��", 2);
				break;
			}
			saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
			saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
			showAt(9, 4, 
				"\033[1;5;31m����\033[0;1;31m�� С�İ�������������ϴ�Ŷ��", 0);
			move(10, 4);
			if (askyn("���Ҫ��ô��", NA, NA) == NA)
				break;
			set_safe_record();
			if (currentuser.dietime > 0) {
				showAt(11, 4, "���Ѿ����˰���ץ����", 1);
				Q_Goodbye();
				break;
			}
			if (money_check_guard()) {
				pressanykey();
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(11, 4, "�˶�����,������Ϣ��.", 1);
				break;
			}
			//currentuser.stay -= 3600 * 1;
			//substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
			r = random() % 100;
			x = countexp(&currentuser);
			y = countexp(&lookupuser);
			if (NULL == t_search(uident, NA, 1))
				ra = 1;//ra = 10;
			else
				ra = 2;
			if (r < 100 * x / (x + x + y + y) / ra) {
				guard_num = loadValue(uident, "guard", 8);
				if (guard_num > 0) {
					if (loadValue(uident, MONEY_NAME, MAX_MONEY_NUM) > guard_num * 1000000)
						saveValue(uident, "guard", -guard_num, 50);
					else
						saveValue(uident, "guard", -1, 50);
					prints
					    ("��ɵ�����һ������");
					pressanykey();
					break;
				}

				if (random() % 2 == 0) {
					money = loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM);
					r = random() % 50;
					money = money / 100 * r;
					saveValue(uident, CREDIT_NAME, -money, MAX_MONEY_NUM);
					saveValue(currentuser.userid, MONEY_NAME, money, MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31m%s\033[m ����û���������˽�ȥ,�ҳ�����, ���� %d ����ٸ���ֽ𣬿��ܰɡ�",
					     uident, money);
					sprintf(buf,
						"%s ������ע���ʱ��������ҵĴ���,���㷢�ֹ�ʧ��ʱ���Ѿ���ʧ�� %d ����ٸ�ҡ�",
						currentuser.userid, money);
					sprintf(title, "�Բ�����������");
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					sprintf(title, "%s���кڰ�(����)", currentuser.userid);
					sprintf(buf,"%s��%s  %d����ٸ��", currentuser.userid, uident, money);
					millionairesrec(title, buf, "�ڰ�");		
					pressanykey();
					break;
				} else {
					money = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
					r = random() % 70;
					money = money / 100 * r;
					saveValue(currentuser.userid, CREDIT_NAME, -money, MAX_MONEY_NUM);
					saveValue(uident, CREDIT_NAME, money, MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31m������� %s ����,��������,̧�ۿ����ڶ�����ǹ�ڶ�����...",
					     uident);
					move(12, 4);
					prints
					    ("\033[1;31m��ѽѽ,û�뵽���ڼ�,�㱻��˽��,�Ӵ�����ȡ�� %d ����ٸ�Ҹ�����",
					     money);
					sprintf(title, "����������");
					sprintf(buf,
						"%s �������Ǯ,������㷢����,���������� %d ����ٸ��,�����ŵķ��Ⱑ��",
						currentuser.userid, money);
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					sprintf(title, "%s���кڰ�(����)", currentuser.userid);
					sprintf(buf,"%s��%s , ��������%d����ٸ��", currentuser.userid, uident, money);
					millionairesrec(title, buf, "�ڰ�");	
					pressanykey();
					break;
				}

			} else if (r < 90) {
				money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
				rob = loadValue(currentuser.userid, "rob", 50);
				move(11, 4);
				if (rob > 20) {
					saveValue(currentuser.userid, "rob", -rob/2, 50);
					prints
					    ("�����о��죬�������ܵ�ʱ��ֻ��һ��ǹ��...");
					set_safe_record();
					if (money / 200 < 3600)
						currentuser.dietime = currentuser.stay + 1000*60;
					else if (money < 10000000){
						mathtmp = (double)(money)/10000;
						mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//����ϵĶ��κ���
						currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
					}else{
						mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
						currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
					}
					substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
					saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
					pressanykey();
					Q_Goodbye();
				} else {
					if (askyn("�����췢����,��Ҫ����ô?", YEA, NA) == NA) {
						saveValue(currentuser.userid, "rob", 1, 50);
						money = loadValue(currentuser.userid, MONEY_NAME,  MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, -money * 50 / 100, MAX_MONEY_NUM);
						sprintf(title, "%s���кڰ�(����)", currentuser.userid);
						sprintf(buf,"%s��%s ������û��%d����ٸ��", currentuser.userid, uident, money/2);
						millionairesrec(title, buf, "�ڰ�");
						showAt
						    (12, 4, "�㱻���������,��û�����������е�Ǯ֮��,���ڵȾ������ѵ��", 0);
						showAt
						    (13, 4, "�����Ǿ�������15����ѵ��ʱ�䣬Ӳ��ͷƤ���ɡ�", 1);
						sleep(15);
						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						sprintf(genbuf,
							"���˾������,����˵Ĵ�Ь���ͳ���������%d����ٸ�ҡ�����,һ�ɳ���Ѿ��ζ...",
							money);
						showAt(14, 4, genbuf, 1);
					} else {
						move(12, 4);
						if (random() % 2 == 0) {
							saveValue(currentuser.userid, "rob", 5, 50);
							prints
							    ("���ܳɹ�,��ϧ,���Ǯ��������·��...");
							saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							sprintf(title, "%s���кڰ�(����)", currentuser.userid);
							sprintf(buf,"%s��%s, ������ʧ%d����ٸ��(ȫ��)", currentuser.userid, uident, money);
							millionairesrec(title, buf, "�ڰ�");
							pressanykey();
						} else {
							saveValue(currentuser.userid, "rob", -rob/2, 50);
							prints("�����������ܵ�ʱ��ֻ��һ��ǹ��...");
							set_safe_record();
							if (money / 200 < 3600)
								currentuser.dietime = currentuser.stay + 1000*60;
							else if (money < 10000000){
								mathtmp = (double)(money)/10000;
								mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//����ϵĶ��κ���
								currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
							}else{
								mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
								currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
							}
							substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
							pressanykey();
							saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							sprintf(title, "%s���кڰ�(����)", currentuser.userid);
							sprintf(buf,"%s��%s, ������, ��ʧ%d����ٸ��(ȫ��)", currentuser.userid, uident, money);
							millionairesrec(title, buf, "�ڰ�");
							Q_Goodbye();
						}
					}
				}
				break;
			} else {
				move(11, 4);
				prints
				    ("\033[1;31m%s\033[m �ҵ������Ľ����ģ����װ·��,�����޷�����,ֻ���뿪��",
				     uident);
				pressanykey();
				break;
			}
			break;
			case '5':
				nomoney_show_stat("�ڰ�����칫��");
				whoTakeCharge2(4, buf);
				whoTakeCharge(4, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
				  	  ("����%s��ס����,˵��:���ϴ�%s���ں�æ,ûʱ��Ӵ��㡣��", buf,uident);
					move(8,4);
					if(!slowclubtest("Rober",currentuser.userid)){
					if (askyn("���������ڰ���", NA, NA) == YEA) {
						sprintf(genbuf, "%s Ҫ����ڰ�", currentuser.userid);
						mail_buf(genbuf, "BMYRober", genbuf);
						move(14, 4);
						prints("���ˣ��һ�֪ͨ�ϴ��");
					}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints("��ѡ���������:");
					move(7, 6);
					prints("5. ��ְ                      6. �˳�");
					ch2 = igetkey();
					switch (ch2) {
					case '5':
						move(12, 4);
						if (askyn("�����Ҫ��ְ��", NA,NA) == YEA) {
						/*	del_from_file(MC_BOSS_FILE,"gang");
							sprintf(genbuf, "%s ������ȥ�ڰ����ְ��", currentuser.userid);
							deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
							move(14, 4);
							prints
							    ("�ðɣ���Ȼ�����Ѿ�������Ҳֻ����׼��");
							quit = 1;
							pressanykey();
						*/
						sprintf(genbuf, "%s Ҫ��ȥ�ڰ����ְ��",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
						pressanykey();
						}
						break;
					}
				}
				break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int/*ؤ��*/
money_beggar()
{
	int ch,ch2;
	int quit = 0;
	char uident[IDLEN + 1], buf[STRLEN], title[40];
	int money, credit, num;
	int id;
	while (!quit) {
		money_show_stat("ؤ���ܶ�");
		move(4, 4);
		prints
		    ("ؤ���Թ����µ�һ������Ŀǰ���û��㾰��������ؤ����Ҳ��������");
		move(5, 4);
		prints
		    ("һ����ؤ�߹����ʵ�����Ҫ������Ϣô��ؤ�����ϵ���������֪��������������");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��̽ [2]��Ǯ [3]���� [4]���� [5]ؤ����� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			move(6, 4);
			usercomplete("��˭�ļҵף�", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
                    
			if (money < 1000) {
				showAt(7, 4, "������ֻ������ô��Ǯ��", 0);
				showAt(8, 4, "����ؤ�ӹ�Ǯת������ˣ���Ҳû�����ġ�", 1);
				break;
			}
			saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
			saveValue("BMYbeg", MONEY_NAME, 500, MAX_MONEY_NUM);
			saveValue("millionaires", MONEY_NAME, 500, MAX_MONEY_NUM);
			money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
			if (money >= 100)
				money = makeRumor(money);
			
			credit = loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM);
			if (credit >= 100) 
				credit = makeRumor(credit);
			
			move(7, 4);
			prints
			    ("\033[1;31m%s\033[m ��Լ�� \033[1;31m%d\033[m ����ٸ�ҵ��ֽ��Լ� \033[1;31m%d\033[m ����ٸ�ҵĴ�",
			     uident, money, credit);
			pressanykey();
			break;
		case '2':
			clear();
			money_show_stat("ؤ������");
			move(4, 4);
			prints
			    ("��Ǯ��С��� 1000 ����ٸ�ҡ�����ͨڤ����£������߸��");
			move(5, 4);
			usercomplete("��˭�գ�", uident);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				move(6, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid) &&
				((lookupuser.dietime- lookupuser.stay) > 10000*60) ) {//5000->10000
				showAt(6, 4, "��ɱ������ô������˾ͷ��ĵ�ȥ�ɣ�����", 1);
				break;
			}
			getdata(6, 4, "�������ն��ٱ���ٸ�ң�[0]", genbuf, 10, DOECHO, YEA);
			num = atoi(genbuf);
			if (num < 1000) {
				showAt(7, 4, "��ô��Ǯ����ô��¸ڤ����°���", 1);
				break;
			}
			move(7, 4);
			sprintf(genbuf, "��ȷ�ϸ� %s �� %d ����ٸ�ң�", uident, num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME,  MAX_MONEY_NUM);
				if (money < num) {
					showAt(8, 4, "����Ǯ����", 1);
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
				saveValue("millionaires", MONEY_NAME, +num/2, MAX_MONEY_NUM);
				saveValue("BMYboss", MONEY_NAME, +num/2, MAX_MONEY_NUM);
				if (lookupuser.dietime == 2 || lookupuser.dietime == 0) {
					showAt(8, 4, "������������������...", 1);
					break;
				}
				if (seek_in_file(MC_ADMIN_FILE, currentuser.userid) && 
					((lookupuser.dietime- lookupuser.stay) > 5000*60)){
					sprintf(title,"%s��ʹ��Ǯ��Ȩ", currentuser.userid);
					sprintf(buf,"%s��%s����%d(/60=%d)����ٸ��",
						currentuser.userid, uident, num, num / 60); 
					millionairesrec(title, buf, "");
				}else{
					sprintf(title,"%s��%s��Ǯ", currentuser.userid, uident);
					sprintf(buf,"%s��%s����%d(/60=%d)����ٸ��",
						currentuser.userid, uident, num, num / 60); 
					millionairesrec(title, buf, "��Ǯ");
				}
				if (lookupuser.dietime > lookupuser.stay)
					lookupuser.dietime -= num;
				if (lookupuser.dietime <= lookupuser.stay)
					lookupuser.dietime = 2;
				substitute_record(PASSFILE, &lookupuser, sizeof (lookupuser), id);
				showAt(8, 4, "�����ˣ��߰ɡ�", 1);
				sprintf(title,
					"�������� %s ������Ǯ����",
					currentuser.userid);
				sprintf(buf,
					"�������� %s �������˵�Ǯ����������������%d����",
					currentuser.userid, num / 60); 
				mail_buf(buf, uident, title);
				pressanykey();
			}
			break;
		case '3':
			move(6, 4);
			usercomplete("Ҫ����˭��", uident);
			if (uident[0] == '\0')
				break;
			if (!getuser(uident)) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			money =  loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if (money < 1000) {
				showAt(7, 4, "���������Ǯ��������·�Ѱ���", 0);
				showAt(8, 4, "����ؤ�ӹ�Ǯת������ˣ���Ҳû�����ġ�", 1);
				break;
			}
			saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
			saveValue("BMYbeg", MONEY_NAME, 500, MAX_MONEY_NUM);
			saveValue("millionaires", MONEY_NAME, 500, MAX_MONEY_NUM);
			move(7, 4);
			prints("��������յ�ؤ�����Ϣ˵��");
			move(8, 4);
			prints
			    ("\033[1;31m%s\033[m �� \033[1;31m%s\033[m �ĵ�λ���Լ� \033[1;31m%s\033[m һ��Ĳ��ա�",
			     uident, cexp(countexp(&lookupuser)), cperf(countperf(&lookupuser)));
			pressanykey();
			break;
		case '4':
			clear();
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ���Ѳ�Ҫ����\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ��Ҫ����\033[m", 1);
				break;
			}			
			money_show_stat("����ٸС��");
			showAt(4, 4, "�����Ǳ���ٸ�ĸ����������ֵĺõط���", 0);
			move(6, 4);
			usercomplete("��˭���֣�", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			if (!getuser(uident)) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			/*if(!Allclubtest(uident)){ 
				prints("    \033[1;32m  ��Ҫ��ɱ�޹���\033[m");
				pressanykey();
				break;
			}*/
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt(7, 4, "      ���л���ӣ��㻹�����˰�\n", 1);
				break;
			}
			if (!clubtest("Beggar")) {
				showAt(7, 4, "��ô����Ҳ������ؤ��İ���", 1);
				break;
			}
			if (lookupuser.dietime>0) {
				showAt(7, 4, "�˶�����,������Ϣ��.", 1);
				break;
			}
			money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
			credit = loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM);
			int flag;
			if (money/2>credit/9){
				num=random() % (money/2);
		    		flag=1;
			}else{ 
				num=credit/9;
			if(num>money)
				num=random() % (num/2);
		    		flag=0;
			}
		   	if(num>500000)
				num=random() % 500000;
			/*if (money >= 100 || credit >= 100) {
				prints("�㻹��Ǯ���������֣�");
				pressanykey();
				break;
			}*/
			if (loadValue(currentuser.userid, "begtime", 2000000000) >=12) {
				if(time(0) > 24*3600 + loadValue(currentuser.userid, "last_beg", 2000000000)){
				  	saveValue(currentuser.userid, "begtime", -12, 2000000000);
					saveValue(currentuser.userid, "last_beg", time(0), 2000000000);
					saveValue(currentuser.userid, "begtime", +1, 2000000000);
				}else				  
					prints("%sŭ���ɶ����������������Ҫ���ģ������ˣ������������", uident);
					pressanykey();
					break;
				}
				saveValue(currentuser.userid, "begtime", +1, 2000000000);
				if (!t_search(uident, NA, 1)) {
					if (random() % 5 == 0) {
						prints("�����%s�޺����������������Ұɣ������ҵ�Сǿ��������...��",
							 uident);
						//num = (random() % (1 + 100))*10000 + 500000;
						if(flag==1)
							saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
						else
							saveValue(uident, CREDIT_NAME, -num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);

						sprintf(title, "%s����ؤ��", currentuser.userid);
						sprintf(buf, "%s������%s %d����ٸ��", currentuser.userid, uident, num);
						if (num != 0)
							millionairesrec(title, buf, "ؤ��");

						move(8, 4);
						prints
							("%s��Ȧ��ʱ���ˣ��Ͻ��������ó� %d ����ٸ�Ҹ��㡣",
							 uident, num);
						sprintf(genbuf,
							"��һʱ���ģ�����%s %d����ٸ�ң����������治����ζ��",
							currentuser.userid, num);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
						mail_buf(genbuf, uident, "�������л���");
						pressanykey();
					} else {
						prints("�����%s�������������ˣ���Ҫ����", uident);
						move(8, 4);
						prints("%sһ�Ű������˳�����", uident);
						pressanykey();
					}
				/*prints("%s���ڼң������˰�����Ҳû��Ӧ��",
				       uident);
				pressanykey();
				break;*/
			}
			
			else {
				int begmoney= loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
				if (seek_in_file(DIR_MC "gongji", uident)){
					if(random() % 3 == 0){ 
						saveValue(uident, MONEY_NAME, -begmoney, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, begmoney, MAX_MONEY_NUM);

						sprintf(title, "%s����ؤ��", currentuser.userid);
						sprintf(buf, "%s������%s %d����ٸ��", currentuser.userid, uident, begmoney);
						if (begmoney != 0)
							millionairesrec(title, buf, "ؤ��");
						
						prints
							("%s��Ȧ��ʱ���ˣ��Ͻ��������ó����еı���ٸ��һ�� %d ���㡣",
							 uident, num);
						sprintf(genbuf,
							"��һʱ���ģ�����%s %d����ٸ�ң����������治����ζ��",
							currentuser.userid, num);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
						mail_buf(genbuf, uident, "�������л���");
						pressanykey();
					}
			  	}

				if (random() % 3 == 0) {
					prints
					    ("�����%s�޺����������������Ұɣ������ҵ�Сǿ��������...��",
					     uident);
					//num = (random() % (1 + 100))*10000 + 500000;
					if(flag==1)
						saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
					else
						saveValue(uident, CREDIT_NAME, -num, MAX_MONEY_NUM);
					saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);

					sprintf(title, "%s����ؤ��", currentuser.userid);
					sprintf(buf, "%s������%s %d����ٸ��", currentuser.userid, uident, num);
					if (num != 0)
						millionairesrec(title, buf, "ؤ��");
					
					move(8, 4);
					prints
					    ("%s��Ȧ��ʱ���ˣ��Ͻ��������ó� %d ����ٸ�Ҹ��㡣",
					     uident, num);
					sprintf(genbuf,
						"��һʱ���ģ�����%s %d����ٸ�ң����������治����ζ��",
						currentuser.userid, num);
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(genbuf, uident, "�������л���");
					pressanykey();
				} else {
					prints("�����%s�������������ˣ���Ҫ����", uident);
					move(8, 4);
					prints("%sһ�Ű������˳�����", uident);
					pressanykey();
				}
			}
			break;
			case '5':
				nomoney_show_stat("ؤ������칫��");
				whoTakeCharge2(5, buf);
				whoTakeCharge(5, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
				  	  ("����%s��ס����,˵��:���ϴ�%s���ں�æ,ûʱ��Ӵ��㡣��", buf,uident);
					move(8,4);
					if(!slowclubtest("Beggar",currentuser.userid)){
					if (askyn("���������ؤ����", NA, NA) == YEA) {
						sprintf(genbuf, "%s Ҫ����ؤ��", currentuser.userid);
						mail_buf(genbuf, "BMYbeg", genbuf);
						move(14, 4);
						prints("���ˣ��һ�֪ͨ�ϴ��");
					}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints("��ѡ���������:");
					move(7, 6);
					prints("5. ��ְ                      6. �˳�");
					ch2 = igetkey();
					switch (ch2) {
					case '5':
						move(12, 4);
						if (askyn("�����Ҫ��ְ��", NA,NA) == YEA) {
						/*	del_from_file(MC_BOSS_FILE,"beggar");
							sprintf(genbuf, "%s ������ȥؤ�����ְ��", currentuser.userid);
							deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
							move(14, 4);
							prints
							    ("�ðɣ���Ȼ�����Ѿ�������Ҳֻ����׼��");
							quit = 1;
							pressanykey();
						*/
						sprintf(genbuf, "%s Ҫ��ȥؤ�����ְ��",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
						pressanykey();
						}
						break;
					}
				}
				break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int/*ɱ��rewrite by macintosh 20051204*/
money_killer()
{
	int ch,ch2;
	int guard_num;
	int robTimes;
	int x,y;
	int quit = 0;
	int quit2=0;
	int count=0;
	int freeTime;
	int currentTime = time(0);
	char uident[IDLEN + 1], name[IDLEN + 1], buf[STRLEN];
	int money,num;
	int id;
	char c4_price[10];
	int price;
	while (!quit) {
		quit2=0;
		nomoney_show_stat("ɱ�����");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��Ӷɱ�� [2]���� [3]ɱ�ְ��� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ���Ѳ�Ҫ����\033[m", 1);
				break;
			}		
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ����Ӳ�Ҫ����\033[m", 1);
				break;
			}	
			money_show_stat("ɱ��֮��");
			readstrvalue(MC_PRICE_FILE, "c4_price", c4_price, 10);
			price = atoi(c4_price);
			if (price==0)
				price=300000;
			move(4, 4);
			prints("��������ɱһ�� %d ����ٸ�ҡ�", price);
			move(5, 4);
			prints("ÿ����ÿ��ֻ��ɱһ�Σ�ÿ����������ɱ�����Ρ�");
			if (currentuser.dietime > 0) {
				showAt(7, 4, "���Ѿ����˰���ץ����", 1);
				Q_Goodbye();
				break;
			}
			move(6, 4);
			usercomplete("��Ҫɱ˭:", uident);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				showAt(7, 4, "�����ʹ���ߴ���...", 2);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "������Ҳ���Ź���̫���˰ɣ�", 1);
				break;
			}
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt(7, 4, "���л���ӣ��㻹�����˰�", 1);
				break;
			}
		       if(!Allclubtest(uident)){
			   	showAt(7, 4, "ɱ�ֲ�ɱ�޹�����...", 1);
				break;
			}
 			getdata(7, 4, "��Ҫɱ���Σ� [1-3]", genbuf, 2, DOECHO, YEA);
			if (genbuf[0] == '\0')
				break;
			count = atoi(genbuf);
			if (count < 1) {
				showAt(8, 4, "�ݲ����Ķ����ˣ�", 1);
				break;
			}
			if (count > 3) {
				move(8, 4);
				sprintf(genbuf, "Ҫɱ%d�Σ�ɱ�����ɱ3�Σ���Ҫ��Ǯ˽����", count);
				if (askyn(genbuf, NA, NA) == NA){
					showAt(9, 4, "��������ɣ�", 1);
					break;
				}
			}
			move(8, 4);
			num = count * price;
			sprintf(genbuf, "�ܹ���Ҫ %d ����ٸ�ҡ�", num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < num) {
					showAt(9, 4, "���Ǯ����...", 1);
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
				saveValue("BMYKillersky", MONEY_NAME, num, MAX_MONEY_NUM);
				sprintf(buf,"%s����%d����ٸ��Ҫɱ%s%d��",currentuser.userid,num,uident, count); 
				mail_buf(buf, "BMYKillersky","[����]ɱ����������"); 
				
				if (seek_in_file(DIR_MC "killerlist", uident)){
					FILE *fp;
					char *ptr;
					int count2=0;
					fp = fopen(DIR_MC "killerlist","r");
					while (fgets(buf,sizeof(buf),fp)) {
						ptr= strstr(buf,uident);
						if(ptr){
							count2 = atoi(ptr+strlen(uident)+1);
							break;
						}
					}
					fclose(fp);
					if (count2+count>3) 
						count2 = 3;
					else
						count2 += count;
					del_from_file(DIR_MC "killerlist", uident);
					sprintf(buf, "%s\t%d",uident, count2);
					addtofile(DIR_MC "killerlist",buf);
				}else{
					sprintf(buf, "%s\t%d",uident, (count>3)?3:count);
					addtofile(DIR_MC "killerlist",buf);
				}
				showAt(10, 4, "���Ѿ��ɹ�����������˵���ͷ���뾲�����", 1);
			}
			break;
			
		case '2':
			while (!quit2) {
			nomoney_show_stat("���¾������г�");
			move(t_lines - 1, 0);
		     	prints
			    ("\033[1;44m ѡ�� \033[1;46m [1]c4  [Q]�뿪\033[m");
			ch2 = igetkey();
			switch (ch2) {			
			case 'q':
			case 'Q':
				quit2 = 1;
			  	break;
			case '1':
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  ��Ҫ����\033[m", 1);
					break;
				}	
				if (!seek_in_file(DIR_MC "killer", currentuser.userid) 
					||!slowclubtest("killer", currentuser.userid)){
					showAt(7, 4, "\033[1;31mҪƴ��ȥ��ɱ��\033[m", 1);
					break;
				}
				if (loadValue(currentuser.userid, "guard", 8) > 0) {
					showAt(7, 4, "���ܲ��ܴ����ֵ�һ�����ɣ�^_^", 1);
					break;
				}
				showAt(4, 4,"\033[1;35m�����������ɱʽ������\033[m", 0);
                		money =loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money <10000) {
					showAt(9, 4, "����Ǯ����...", 1);
					break;
				}
				if (currentuser.dietime > 0) {
					showAt(9, 4, "���Ѿ����˰���ץ����", 1);
					Q_Goodbye();
					break;
				}
				usercomplete("��Ҫը˭:", uident);
				if (uident[0] == '\0')
					break;
				if (!(id = getuser(uident))) {
					showAt(7, 4,"�����ʹ���ߴ���...", 1);
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(7, 4,"������Ҳ���Ź���̫���˰ɣ�", 1);
					break;	
				}
				if (seek_in_file(DIR_MC "mingren", uident)){
					showAt(7, 4, "      ���л���ӣ��㻹�����˰�\n", 1);
					break;
				}
				if (!Allclubtest(uident)){
					showAt(7, 4, "    \033[1;32m  ��Ҫ��ɱ�޹���\033[m", 1);
					break;
				}				guard_num =loadValue(uident, "guard", 8);
				if (guard_num > 0) {
					showAt(7, 4, "�Է��б��ڻ���,�㻹�����˰�...", 1);
					break;
				}

				freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
				if (currentTime < freeTime){
					pressreturn();
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -100000, MAX_MONEY_NUM);
				move(6, 4);
				prints
				    ("  \n\033[1;35m  �㱧��ըҩ������һ����С�ձ�,��%s���˹�ȥ\033[m\n", uident);
				sprintf(genbuf, "������ʿ%s��10����ǰ�ھ�����\nһ����ɱʽ����������\n�������ɴ����а�ᱳ��\n\n"
							"��һλ��Ը͸¶�����ľ����Ա͸¶\n����¼�������ְҵɱ����Ϊ", uident);                                
				x = countexp(&currentuser);
				y = countexp(&lookupuser);
				robTimes = loadValue(currentuser.userid, "rob", 50);
				saveValue(currentuser.userid, "rob", -robTimes, 50);
				if(random()/x>(random()/y)/3||(random() % 3==0)){
					lookupuser.dietime = lookupuser.stay + 4500 * 60;
					substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
					deliverreport("[����]��վ������ɱ����",genbuf);
					mail_buf_slow(uident,	 "�����","���˶��㷢������ɱʽ������","BMYKillersky");
					sprintf(buf,"�� %s ��������ɱʽ����",uident); 
					mail_buf(buf, "BMYKillersky","�������"); 
					if (seek_in_file(DIR_MC "killerlist", uident)){
						FILE *fp;
						char *ptr;
						int count2=0;
						fp = fopen(DIR_MC "killerlist","r");
						while (fgets(buf,sizeof(buf),fp)) {
							ptr = strstr(buf,uident);
							if(ptr){
								count2 = atoi(ptr+strlen(uident)+1);
								break;
							}
						}
						fclose(fp);
						del_from_file(DIR_MC "killerlist", uident);
						if (count2==2 || count2==3){
							sprintf(buf, "%s\t%d",uident, count2-1);
							addtofile(DIR_MC "killerlist",buf);
						}
					}
				}
				set_safe_record();
				currentuser.dietime = currentuser.stay +1000 * 60;
				substitute_record (PASSFILE, &currentuser, sizeof(currentuser), usernum);
				pressanykey();
				Q_Goodbye();
				}
			limit_cpu();
			}
			break;

			case '3':
				nomoney_show_stat("ɱ�ְ����칫��");
				whoTakeCharge2(9, name);
				whoTakeCharge(9, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
				  	  ("����%s��ס����,˵��:���ϴ�%s���ں�æ,ûʱ��Ӵ��㡣��", name,uident);
					move(8,4);
					if (!seek_in_file(DIR_MC "killer", currentuser.userid) && 
						!slowclubtest("killer",currentuser.userid)){
					if (askyn("�������Ϊɱ����", NA, NA) == YEA) {
						sprintf(genbuf, "%s Ҫ����ɱ��", currentuser.userid);
						mail_buf(genbuf, "BMYKillersky", genbuf);
						move(14, 4);
						prints("���ˣ��һ�֪ͨ�ϴ��");
					}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints("��ѡ���������:");
					move(7, 6);
					prints("1. ����ɱ��                  2. ��ְɱ��");
					move(8, 6);
					prints("3. ɱ������                  4. ��������");
					move(9, 6);
					prints("5. ��ְ                      6. c4����");
					move(10, 6);
					prints("7. �˳�");
					ch2 = igetkey();
					switch (ch2) {
					case '1':
						move(12, 4);
						usercomplete("����˭Ϊɱ�֣�", uident);
						move(13, 4);
						if (uident[0] == '\0')
							break;
						if (!searchuser(uident)) {
							prints("�����ʹ���ߴ���...");
							pressanykey();
							break;
							}
						if (seek_in_file(DIR_MC "killer", uident)) {
							prints("��ID�Ѿ���ɱ���ˡ�");
							pressanykey();
							break;
							}
						if (askyn("ȷ����", NA, NA) == YEA) {
							addtofile(DIR_MC "killer", uident);
							sprintf(genbuf, "%s ������Ϊɱ��",currentuser.userid);
							mail_buf("ϣ���㲻������ҵ�ϣ�����������",uident, genbuf);
							move(14, 4);
							prints("�����ɹ���");
							sprintf(genbuf, "%s��ʹɱ�ֹ���Ȩ��",currentuser.userid);
							sprintf(buf, "����%sΪɱ��", uident);
							millionairesrec(genbuf, buf, "BMYKillersky");
							pressanykey();
							}
						break;
					case '2':
						move(12, 4);
						usercomplete("��ְ��λɱ�֣�", uident);
						move(13, 4);
						if (uident[0] == '\0')
							break;
						if (!searchuser(uident)) {
							prints("�����ʹ���ߴ���...");
							pressanykey();
							break;
						}
						if (!seek_in_file(DIR_MC "killer", uident)) {
							prints("��ID����ɱ�֡�");
							pressanykey();
							break;
							}
						if (askyn("ȷ����", NA, NA) == YEA) {
							del_from_file(DIR_MC "killer", uident);
							sprintf(genbuf, "%s ������ɱ��ְ��", currentuser.userid);
							mail_buf("��л���������", uident, genbuf);
							move(14, 4);
							prints("��ְ�ɹ���");
							sprintf(genbuf, "%s��ʹɱ�ֹ���Ȩ��",currentuser.userid);
							sprintf(buf, "���%s��ɱ��ְ��", uident);
							millionairesrec(genbuf, buf, "BMYKillersky");
							pressanykey();
							}
						break;
					case '3':
						clear();
						move(1, 0);
						prints("Ŀǰ����ٸɱ��������");
						listfilecontent(DIR_MC "killer");
						pressanykey();
						break;
					case '4':
						clear();
						move(1, 0);
						prints("Ŀǰ����ٸ׷ɱ������");
						move(2, 0);
						prints("Ŀ��ID\t����");
						listfilecontent(DIR_MC "killerlist");
						pressanykey();
						break;
					case '5':
						move(12, 4);
						if (askyn("�����Ҫ��ְ��", NA,NA) == YEA) {
						/*	del_from_file(MC_BOSS_FILE,"killer");
							sprintf(genbuf, "%s ������ȥɱ�ְ���ְ��", currentuser.userid);
							deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
							move(14, 4);
							prints
							    ("�ðɣ���Ȼ�����Ѿ�������Ҳֻ����׼��");
							quit = 1;
							pressanykey();
						*/
							sprintf(genbuf, "%s Ҫ��ȥɱ�ְ���ְ��",
								currentuser.userid);
							mail_buf(genbuf, "millionaires", genbuf);
							move(14, 4);
							prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
							pressanykey();
						}
						break;
					case '6':
						move(12, 4);
						readstrvalue(MC_PRICE_FILE, "c4_price", c4_price, 10);
						price = atoi(c4_price);
						prints("���ڵļ۸���%d", price ? price : 300000);
						getdata(13, 4, "�趨�µļ۸�: ", buf, 10, DOECHO, YEA);
						move(14, 4);
						sprintf(genbuf, "�µļ۸��� %d��ȷ����", atoi(buf));
						if (askyn(genbuf, NA, NA) == YEA) {
							if (atoi(buf)>MAX_MONEY_NUM){
								move(15, 4);
								prints("��Ҫ̫����...");
								pressanykey();
								sprintf(buf, "%d", MAX_MONEY_NUM);
							}
							savestrvalue(MC_PRICE_FILE, "c4_price", buf);
							move(15, 4);
							prints("������ϡ�    ");
							sprintf(genbuf, "����c4�۸�Ϊ%s��", buf);
							sprintf(buf, "%s��ʹɱ�ֹ���Ȩ��", currentuser.userid);
							millionairesrec(buf, genbuf, "BMYKillersky");
							pressanykey();
						}
						break;
					}
				}
				break;

			case 'q':
			case 'Q':
				quit = 1;
				break;
		}
		limit_cpu();
	}
	return 0;
}

static int 
money_postoffice()
{
	int ch2, slownum=0;
	
	nomoney_show_stat("�����ʼ�����");
	slownum=loadValue(currentuser.userid, "mail", 8);
	move(6, 4);
	if(Allclubtest(currentuser.userid)){
		prints("������ʿ�Ͳ�Ҫ����ô����!");
		pressanykey();
		return 0;
	}
	if (slownum==0){
		prints("����δ��ͨ����ٸ�ʾ��ʼ����񣬲����յ�������ɸ������ż���");
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [1]��ͨ���� [Q]�뿪\033[m");
	}
	else{
		prints("���Ѿ������˱���ٸ�ʾֵ��ʼ��������ǽ��ڵ�һʱ�佫�������");
		move(7, 4);
		prints("�������ż��ݸ�����");
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [1]ȡ������ [Q]�뿪\033[m");
	}
	ch2 = igetkey();
	switch (ch2) {			
	  	case '1':
	    		if(slownum==0){
				saveValue(currentuser.userid, "mail", 1, 50);
				nomoney_show_stat("�����ʼ�����");
				move(6, 4);
				prints("��ӭʹ�ñ���ٸ�ʾ��ʼ�����ϵͳ�����ǽ��ڵ�һʱ�佫�������");
				move(7, 4);
				prints("�������ż��ݵ��������䡣�ټ���");
	    		}else{
				saveValue(currentuser.userid, "mail", -slownum, 50);
				nomoney_show_stat("�����ʼ�����");
				move(6,4);
				prints("��ӭ���´μ���ʹ�ñ��ʾֵĸ������лл���Ĺ�ˣ��ټ���");
	    		}
			pressanykey();
			break;
			
		case 'q':
		case 'Q':
	    	 	break;			
		}
	return 0;
}


static int /*�̳�rewrite by macintosh 20051204*/
money_shop()
{
	int ch, money, num, ch2;
	int guard_num;
	char uident[IDLEN + 1], ticket_price[10], buf[STRLEN];
	int quit = 0, quit2= 0, price=0;
	
	while (!quit) {
		quit2=0;
		nomoney_show_stat("����ٸ�̳�");
		move(6, 4);
		prints("����ٸ�̳���������𣬴�Ҿ��ˣ�");
		move(t_lines - 1, 0);
		prints
	    ("\033[1;44m ѡ�� \033[1;46m [1]�ͱ��� [2]��Ʒ�� [3]������ [4]�ʾ� [6]��Ʊ�ۼ��� [Q]�뿪\033[m");
		   // ("\033[1;44m ѡ�� \033[1;46m [1]�ͱ��� [2]�ؿ� [4]������ [5]hell�ι� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("����ٸ���ڹ�˾");
			move(4, 4);
			prints
			    ("����ٸ���ڹ�˾������Ҫ����ʿ�ṩ����ҵ��,�۸������������");
			move(5, 4);
			prints
			    ("���Ǳ���������һ��Ϊ��,�����Զ��뿪,�����ܻ�Թ������кڳԺ�Ŷ��");
			move(7, 4);
			sprintf(genbuf, "��ȷ��Ҫ�ͱ���ô?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				move(8, 4);
				if (money < 10000) {
					prints
					    ("�㻹��ʡʡ�ɣ�û�˻��������ġ�����ô��Ǯ...");
					pressanykey();
					break;
				}
				guard_num =(countexp(&currentuser) / 1000) + 1 >
				    8 ? 8 : (countexp(&currentuser) / 1000) + 1;
				prints
				    ("������Ŀǰ����ݵ�λ����Ӷ%d�����ھ͹��ˡ�",
				     guard_num);
				saveValue(currentuser.userid, MONEY_NAME, -money / 20, MAX_MONEY_NUM);
				move(9, 4);
				if (loadValue(currentuser.userid, "rob", 50) > 0) {
					prints
					    ("�ٺ٣����а��ף�����������Ǯ�ķ��ϣ��Ͻ���·��...");
					pressanykey();
					break;
				}
				if (loadValue(currentuser.userid, "guard", 8) > 0) {
					prints
					    ("���Ѿ��б����ˡ�Ǯ�������£����ڲ����ٸ��ˣ�^_^");
					pressanykey();
				} else {
					saveValue(currentuser.userid, "guard", guard_num, 50);
					prints
					    ("��Ӷ���ڳɹ�,�������һ��ʱ�䰲��̫ƽ�ˡ�");
					pressanykey();
				}
			}
			break;

		case '2':
			while (!quit2) {
				nomoney_show_stat("����ٸ��Ʒ��");
				move(6, 4);
				//prints ("��ӭ���ٱ���ٸ��Ʒ�꣡");
				prints("����ASCII��Ʒ���Ǳ���������������Ʒ��������ԭ��δ�ܱ������ߡ�\n"
					"    ����Ʒ�����߶�����Ʒ���ڱ���������飬���뱾վ�����ܹ���ϵ��\n"
					"    ��վ����ʱ����������Ը����������\n\n"
					"                                                   \33[1;32m����ٸ��Ʒ��\033[0m\n");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m ѡ�� \033[1;46m [1]�ʻ� [2]�ؿ� [Q]�뿪\033[m");
				ch2 = igetkey();
				switch (ch2) {
					case 'q':
					case 'Q':
						quit2=1;
						break;
					case '1':
						shop_present(1, "�ʻ�", NULL);
						break;
					case '2':
						shop_present(2, "�ؿ�", NULL);
						break;				
					}
				limit_cpu();
				}
			break;
			
		case '3':
			nomoney_show_stat("����ٸ�̳�������");
			whoTakeCharge(7, uident);
			char name[20];
			whoTakeCharge2(7, name);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("ֵ������%s��ס���㣬˵��:������%s���ڿ��ᣬ��ʲô�¸���˵Ҳ�С���",
				     name,uident);
				pressanykey();
				break;
			} else {
					move(6, 4);
					prints("��ѡ���������:");
					move(9, 6);
					prints("5. ��ְ                      6. ��Ʊ�۶���");
					move(10, 6);
					prints("7. �˳�");
					ch2 = igetkey();
					switch (ch2) {
					case '5':
						move(12, 4);
						if (askyn("�����Ҫ��ְ��", NA,NA) == YEA) {
							sprintf(genbuf, "%s Ҫ��ȥ�̳�����ְ��",
								currentuser.userid);
							mail_buf(genbuf, "millionaires", genbuf);
							move(14, 4);
							prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
							pressanykey();
						}
						break;
					case '6':
						move(12, 4);
						readstrvalue(MC_PRICE_FILE, "ticket_price", ticket_price, 10);
						price = atoi(ticket_price);
						prints("���ڵļ۸���%d", price);
						getdata(13, 4, "�趨�µļ۸�: ", ticket_price, 10, DOECHO, YEA);
						move(14, 4);
						sprintf(genbuf, "�µļ۸��� %d��ȷ����", atoi(ticket_price));
						if (askyn(genbuf, NA, NA) == YEA) {
							if (atoi(ticket_price)>MAX_MONEY_NUM){
								move(15, 4);
								prints("��Ҫ̫����...");
								pressanykey();
								sprintf(ticket_price, "%d", MAX_MONEY_NUM);
							}
							savestrvalue(MC_PRICE_FILE, "ticket_price", ticket_price);
							move(15, 4);
							prints("������ϡ�    ");
							sprintf(genbuf, "������Ʊ�ۼ۸�Ϊ%s��", ticket_price);
							sprintf(buf, "%s��ʹ�̳��������Ȩ��", currentuser.userid);
							millionairesrec(buf, genbuf, "");
							pressanykey();
						}
						break;
					}
				}
				break;
			
		/*case '5':
			 sprintf(genbuf, "���Ҫȥhell");
			move(11, 4);
			if (askyn(genbuf, NA, NA) == YEA){		
				set_safe_record();
				currentuser.dietime = currentuser.stay + 1;
				substitute_record(PASSFILE,&currentuser,sizeof(currentuser),usernum);					
				pressanykey();
				Q_Goodbye();
			}
			break;
		*/

		case '6':
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			readstrvalue(MC_PRICE_FILE, "ticket_price", ticket_price, 10);
			num = atoi(ticket_price);
			clear();
			move(5, 4);
			if (askyn("�������շѣ�ȷ��Ҫ����? ", YEA, NA) == YEA){
				if (money < num) {
					move(9, 4);
					prints("�Բ������Ľ��㡣");
					pressanykey();
					break;
				}else{
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
					//calc_ticket_price();	
				}
			}
			break;

		case '4':
			money_postoffice();
			break;
			
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}


static int/*��Ʊϵͳ*/
money_stock()
{
//      moneycenter_welcome();
	int quit = 0;
	char ch;
	
	while (!quit) {
		clear();
		money_show_stat("����ٸ����");
      
		if (utmpshm->ave_score == 0) {
			clear();
			move(7, 10);
			prints("\033[1;31m����ٸ���н�������\033[0m");
			pressanykey();
			return 0;
		}
		

		move(4, 4);
		prints("��ȷ�����Ѿ���"MC_BOARD"���Ķ�������ٸ���й���");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��ͳ��� [2]�Ƽ���� [3]֤�����ϯ�칫�� [Q]�뿪   \033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_stock_board();
			break;
		case '2':
			clear();
			move(7, 10);
			prints("\033[1;32m��δ����\033[0m");
			pressanykey();
			break;
		case '3':
			stockboards();
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static int/*��Ʊϵͳ*/
money_stock_board()
{
	char stockname[STRLEN][MAX_STOCK_NUM];
	char stockboard[STRLEN][MAX_STOCK_NUM];
	int ch, i, j, quit = 0, money, count, count1;
	int stock_num[MAX_STOCK_NUM], addto_num[MAX_STOCK_NUM],
	    stock_board[MAX_STOCK_NUM];
	int stock_price[MAX_STOCK_NUM];
	int total_money = 0, temp_sum = 0, total_sum = 0;
	char slow[IDLEN + 1];
	char uident[IDLEN + 1];
	char title[80];
	char buf[200];
	int getnum=0;
	FILE *fp1;

	fp1 = fopen( MC_STOCK_BOARDS, "r" );
	count1= count = listfilecontent(MC_STOCK_BOARDS);
	clear();
	if (count==0){
		move(7, 10);
		prints("\033[1;32m����ٸ������δ����\033[0m");
		pressanykey();
		return 0;
	}
	for (j = 0; j < count; j++) 
		fscanf(fp1, "%s", stockboard[j]);
	fclose(fp1);
	for (j = 0; j < count; j++) 
		sprintf(stockname[j], "St_%s", stockboard[j]);

	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	clear();
	//count = MAX_STOCK_NUM;
	
	bzero(&stock_price, sizeof (stock_price));
	bzero(&stock_num, sizeof (stock_num));
	bzero(&addto_num, sizeof (addto_num));
	for (i = 0; i < numboards; i++) {
		for (j = 0; j < count; j++) {
			if (!strcmp(bcache[i].header.filename, stockboard[j])) {
				stock_price[j] = utmpshm->ave_score / 100 + bcache[i].score / 20;
				if (bcache[i].stocknum <= 0) {
					if (bcache[i].score > 10000)
						bcache[i].stocknum = bcache[i].score * 2000;
					else
						bcache[i].stocknum = bcache[i].score * 1000;
				}
				stock_board[j] = i;
				count1--;
				break;
			}
		}
		if (count1 == 0)
			break;
	}//����ɼ�
	for (i = 0; i < count; i++) {
		stock_num[i] =
		    loadValue(currentuser.userid, stockname[i], 1000000);
	}//ͳ���Լ�������
	//for (i = 0; i < MAX_STOCK_NUM; i++) 
	 i=0;
	while(!quit){
		money =
		    loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		persenal_stock_info(stock_num, stock_price, money, stockboard,
				    stock_board);
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [B]���� [S]���� [C]ת�� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch){
		case 'B':
		case 'b':
			total_money = 0;
			if (stop_buy()) {
				clear();
				move(7, 10);
				prints("\033[1;31m����ٸ������δ����\033[0m");
				pressanykey();
				break;
			}
			getdata(t_lines - 1, 0, "��ѡ����֧��Ʊ?[0]", genbuf, 7,
				DOECHO, YEA);
			getnum=atoi(genbuf);
			if(getnum<0||getnum>count-1)
				break; //�Ƿ�����
			else 
				i=getnum;
			if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i])){
				move(t_lines - 2, 0);
				prints("��֧��Ʊ�ѱ���ͣ����!");
				pressanykey();
				break;
			}
			getdata(t_lines - 1, 0, "��Ҫ����ٹ�?[0]", genbuf, 7,
				DOECHO, YEA);
			
			addto_num[i] = atoi(genbuf);
			if (!genbuf[0])
				addto_num[i] = 0;
			//addto_num[i] = abs(addto_num[i]);
			if (addto_num[i] <= 0){
				move(t_lines - 2, 0);
				prints("������Ҫ������...");
				pressanykey();
				break;
			}				
			stock_num[i] =
			    loadValue(currentuser.userid, stockname[i], 1000000);
			if (stock_num[i] >= 1000000) {
				move(t_lines - 2, 0);
				prints("���Ѿ��кܶ��Ʊ��,��Ҫ������");
				pressanykey();
				break;
			}
			if (bcache[stock_board[i]].stocknum <= 50000) {
				move(t_lines - 2, 0);
				prints("�Բ���,�˹�Ŀǰû�п��Գ��۵Ĺ�Ʊ!");
				pressanykey();
				break;
			}
			if (stock_num[i] + addto_num[i] > 1000000){
				addto_num[i] = 1000000 - stock_num[i];
				move(t_lines - 2, 0);
				prints("�Բ���,���Ѿ��кܶ��Ʊ��!");
				pressanykey();
			}			
			if (bcache[stock_board[i]].stocknum - addto_num[i] < 50000){
				addto_num[i] = bcache[stock_board[i]].stocknum - 50000;
				move(t_lines - 2, 0);
				prints("�Բ���,�˹�Ŀǰû����ô���Ʊ����!");
				pressanykey();				
			}
			move(t_lines - 2, 0);
			sprintf(genbuf, "ȷ������ %d �� %s ��", 
				addto_num[i], stockname[i]);
			if (askyn(genbuf, NA, NA) == YEA) {
				temp_sum = addto_num[i] * stock_price[i];
				total_money += temp_sum;
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money - temp_sum < 0) {
					total_money -= addto_num[i] * stock_price[i];
					addto_num[i] = 0;
					move(t_lines - 2, 0);
					prints("�㵱ǰ�ı���ٸ�Ҳ�����ɴ������!");
					pressanykey();
					break;
				}
				bcache[stock_board[i]].stocknum -= addto_num[i];
				saveValue(currentuser.userid, MONEY_NAME, -temp_sum, MAX_MONEY_NUM);
				stock_num[i] += addto_num[i];
				saveValue(currentuser.userid, stockname[i],
					  addto_num[i], 1000000);
				if (addto_num[i]>0){
					sprintf(genbuf, "%s���й�Ʊ����(����)", currentuser.userid);
					sprintf(buf,"%s������%d��%s��Ʊ(ÿ��%d����ٸ��)������%d����ٸ��\n", 
						currentuser.userid, addto_num[i], stockname[i], stock_price[i], temp_sum);
					millionairesrec(genbuf, buf, "��Ʊ����");
					sprintf(buf,"��������%d��%s��Ʊ���ɽ���%d����ٸ��ÿ�ɣ�����%d����ٸ�ҡ�\n", 
						addto_num[i], stockname[i], stock_price[i], temp_sum);
					sprintf(title,"��Ʊ����ƾ֤");
					mail_buf(buf, currentuser.userid, title);	
					total_sum -= temp_sum;
					sprintf(genbuf, "�㻨����%d����ٸ��", temp_sum);
					move(t_lines - 2, 0);
					clrtoeol();
					prints("%s", genbuf);
					pressanykey();
				}
			}
			sleep(1);
			break;
		case 'S':
		case 's':
			total_money= 0;
			if (stop_buy()) {
				clear();
				move(7, 10);
				prints("\033[1;31m����ٸ������δ����\033[0m");
				pressanykey();
				break;
			}
			getdata(t_lines - 1, 0, "��ѡ����֧��Ʊ?[0]", genbuf, 7,
				DOECHO, YEA);
			getnum=atoi(genbuf);
			if(getnum<0||getnum>count-1)
				break; //�Ƿ�����
			else 
				i=getnum;
			if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i])){
				move(t_lines - 2, 0);
				prints("��֧��Ʊ�ѱ���ͣ����!");
				pressanykey();
				break;
			}
			
			getdata(t_lines - 1, 0, "��Ҫ�����ٹ�?[0]", genbuf, 7,
				DOECHO, YEA);
			stock_num[i] =
			    loadValue(currentuser.userid, stockname[i], 1000000);
			addto_num[i] = atoi(genbuf);
			if (!genbuf[0])
				addto_num[i] = 0;
			//addto_num[i] = abs(addto_num[i]);
			if (addto_num[i] <= 0){
				move(t_lines - 2, 0);
				prints("������Ҫ������...");
				pressanykey();
				break;
			}
			/*
			if (stock_num[i] - addto_num[i] < 0)
				addto_num[i] = stock_num[i];
			*/
			if (stock_num[i] < addto_num[i]) {
				move(t_lines - 2, 0);
				prints
				    ("��û����ô���Ʊ��...���㷸�λ����ҷ���?");
				pressanykey();
				break;
			}
			move(t_lines - 2, 0);
			sprintf(genbuf, "ȷ������ %d �� %s ��", 
				addto_num[i], stockname[i]);
			if (askyn(genbuf, NA, NA) == YEA) {
				addto_num[i] *= -1;
				temp_sum = addto_num[i] * stock_price[i];
				stock_num[i] += addto_num[i];
				saveValue(currentuser.userid, MONEY_NAME, temp_sum/100-temp_sum,
									  MAX_MONEY_NUM);
				whoTakeCharge(6, slow);//slowaction
	                     saveValue(slow, MONEY_NAME, -temp_sum/100, MAX_MONEY_NUM);
				saveValue(currentuser.userid, stockname[i],
					  addto_num[i], 1000000);
				total_money += temp_sum-temp_sum/100;
				bcache[stock_board[i]].stocknum -= addto_num[i];
				temp_sum = bcache[stock_board[i]].score;
				if (temp_sum > 10000) {
					if (bcache[stock_board[i]].stocknum > temp_sum * 2000)
						bcache[stock_board[i]].stocknum = temp_sum * 2000;
				} else {
					if (bcache[stock_board[i]].stocknum > temp_sum * 1000)
						bcache[stock_board[i]].stocknum = temp_sum * 1000;
				}			
				sprintf(genbuf, "%s���й�Ʊ����(����)", currentuser.userid);
				sprintf(buf,"%s������%d��%s��Ʊ(ÿ��%d����ٸ��)�����%d����ٸ��\n", 
					currentuser.userid, -addto_num[i], stockname[i], stock_price[i], -total_money);
				millionairesrec(genbuf, buf, "��Ʊ����");
				total_sum -= total_money;
				sprintf(genbuf, "�۳������Ѻ����û���%d����ٸ��", (-1) * total_money);
				move(t_lines - 2, 0);
				clrtoeol();
				prints("%s", genbuf);
				pressanykey();
			}
			sleep(1);
			break;
		case 'c':
		case 'C':
			/*if (stop_buy()) {
				clear();
				move(7, 10);
				prints("\033[1;31m����ٸ������δ����\033[0m");
				pressanykey();
				break;
			}*/
			move(t_lines - 1, 0);
       		usercomplete("ת�ù�Ʊ��˭��", uident);
			if (uident[0] == '\0')
				return 0;
			if (!getuser(uident)) {
				move(t_lines - 2, 0);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				return 0;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				if (seek_in_file(DIR_MC "jijin", currentuser.userid));
				else if (!seek_in_file(DIR_MC "mingren", uident)) {
					clear();
					move(t_lines - 2, 0);
					prints
					    ("�Բ���֤��᲻������������ת�ù�Ʊ��");
					pressreturn();
					break;
				}
			}
			getdata(t_lines - 1, 0, "��ѡ����֧��Ʊ?[0]", genbuf, 7,
				DOECHO, YEA);
			getnum=atoi(genbuf);
			if(getnum<0||getnum>count-1)
				break; //�Ƿ�����
			else 
				i=getnum;
			getdata(t_lines - 1, 0, "��Ҫת�ö��ٹ�?[0]", genbuf, 7,
				DOECHO, YEA);
			stock_num[i] =
			    loadValue(currentuser.userid, stockname[i], 1000000);
			addto_num[i] = atoi(genbuf);
			if (addto_num[i] < 0){
				move(t_lines - 2, 0);
				prints("��ת�ø��ģ�����...");
				pressanykey();
				break;
			}
			if (addto_num[i] == 0){
				pressanykey();
				break;
			}
			if (stock_num[i] < addto_num[i]) {
				move(t_lines - 2, 0);
				prints
				    ("��û����ô���Ʊ��...���㷸�λ����ҷ���?");
				pressanykey();
				break;
			}
        
			sprintf(genbuf, "ȷ��ת�˸� %s %d %s��", 
				uident, addto_num[i], stockname[i]);
			if (askyn(genbuf, NA, NA) == YEA){
				saveValue(currentuser.userid, stockname[i],
					-addto_num[i], 1000000);
				saveValue(uident, stockname[i],
					addto_num[i], 1000000);
				sprintf(genbuf, "����ת����%d�ɹ�Ʊ",addto_num[i]);
	            		sprintf(title, "�������Ѹ�����%s��Ʊ����", stockname[i]);
				mail_buf(genbuf, uident, title);
				sprintf(genbuf, "%s���й�Ʊ����(ת��)", currentuser.userid);
				sprintf(buf,"%s��%sת����%d��%s��Ʊ(ÿ�ɼ�ֵ%d����ٸ��)", 
					currentuser.userid, uident, addto_num[i], stockname[i], stock_price[i]);
				millionairesrec(genbuf, buf, "��Ʊ����");
				move(t_lines - 2, 0);
				clrtoeol();
				prints("ת�óɹ�", genbuf);
				pressanykey();
			}
			sleep(1);
			//quit=1;
			break;

		case 'Q':
		case 'q':
			quit = 1;
			break;
		default:
			break;
		}
		//if (quit)
		//	return 0;
		limit_cpu();
	}
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	persenal_stock_info(stock_num, stock_price,
			    money, stockboard, stock_board);
	move(t_lines - 2, 0);
	clrtobot();
	if (total_sum > 0)
		sprintf(genbuf, "��ν��������û�%d����ٸ��", total_sum);
	else if (total_sum < 0)
		sprintf(genbuf, "��ν������㻨����%d����ٸ��", -total_sum);
	else
		sprintf(genbuf, "����ν�����û��ʹ�õ��ֽ�");
	prints("%s", genbuf);
	pressanykey();
	return 0;
}

static void /*��ʾmoney*/
money_show_stat(char *position)
{
	int money, credit;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
	clear();
	move(2, 0);
	prints("�����ϴ��� \033[1;31m%d\033[m ����ٸ�ң�", money);
	prints("��� \033[1;31m%d\033[m ����ٸ�ҡ���ǰλ�� \033[1;33m%s\033[m",
	       credit, position);
	move(3, 0);
	prints
	    ("\033[1m--------------------------------------------------------------------------------\033[m");
}

static void /*��ʾ��ǰλ��*/
nomoney_show_stat(char *position)
{
	clear();
	move(2, 0);
	prints
	    ("\033[1;32m��ӭ���ٱ���ٸ�������ģ���ǰλ����\033[0m \033[1;33m%s\033[0m",
	     position);
	move(3, 0);
	prints
	    ("\033[1m--------------------------------------------------------------------------------\033[m");
}

static int /*�ĳ�����*/
money_gamble()
{
	int ch;
	int quit = 0;
	char uident[IDLEN + 1];
	char buf[STRLEN];
	clear();
	while (!quit) {
		clear();
		money_show_stat("����ٸ�ĳ�����");
		move(6, 4);
		prints("����ٸ�ĳ���������𣬴�Ҿ��˰�");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]���� [2]777 [3]������ [4]���˿���� [5]����˹���� [6]������ [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_dice();
			break;
		case '2':
			money_777();
			break;
		case '3':
			//guess_number();
			russian_gun();
			break;
		case '4':
			p_gp();
			break;
		case '5':
			russian_gun();
			break;
		case '6':
			clear();
		    char name[20];
            whoTakeCharge2(3, name);
		    nomoney_show_stat("����ٸ�ĳ�������");
			whoTakeCharge(3, uident);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("����%s����ȵ�:�����Ĺ�������������ϰ�%s�����ٽ�Ǯ�����ˡ���",
				     name,uident);
				pressanykey();
				break;
			} else {
				move(6, 4);
				prints("��ѡ���������:");
				move(7, 6);
				prints
				    ("1. ����VIP��                  2. �ջ�VIP��");
				move(8, 6);
				prints
				    ("3. VIP�ͻ�                    4. �����뺯");
				move(9, 6);
				prints("5. ����ϴ��                   6. �˳�");
				ch = igetkey();
				switch (ch) {
				case '1':
					move(12, 4);
					usercomplete("��˭����VIP����", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("�����ʹ���ߴ���...");
						pressanykey();
						break;
					}
					if (seek_in_file
					    (DIR_MC "gamble_VIP", uident)) {
						prints
						    ("�ÿͻ��Ѿ�ӵ�жĳ�VIP����");
						pressanykey();
						break;
					}
					if (askyn("ȷ����", NA, NA) == YEA) {
						addtofile(DIR_MC "gamble_VIP",
							  uident);
						sprintf(genbuf,
							"%s ���㷢�ű���ٸ�ĳ�VIP��",
							currentuser.userid);
						mail_buf
						    ("�𾴵Ŀͻ��� ��ӭ�����ٱ���ٸ�ĳ�����ף����!",
						     uident, genbuf);
						move(14, 4);
						prints("������ɡ�");
						sprintf(buf, "��%s���Ŷĳ�VIP��",uident);
						sprintf(genbuf, "%s��ʹ�ĳ�����Ȩ��",currentuser.userid);
						millionairesrec(genbuf, buf, "BMYboss");
						pressanykey();
					}
					break;
				case '2':
					move(12, 4);
					usercomplete("�ջ�˭��VIP����", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("�����ʹ���ߴ���...");
						pressanykey();
						break;
					}
					if (!seek_in_file
					    (DIR_MC "gamble_VIP", uident)) {
						prints("�ÿͻ�û�жĳ�VIP����");
						pressanykey();
						break;
					}
					if (askyn("ȷ����", NA, NA) == YEA) {
						del_from_file(DIR_MC
							      "gamble_VIP",
							      uident);
						sprintf(genbuf,
							"%s �ջ�����ı���ٸ�ĳ�VIP��",
							currentuser.userid);
						mail_buf
						    ("���ûǮ�˻�VIP�����±��Ӱɣ�",
						     uident, genbuf);
						move(14, 4);
						prints("�����ջء�");
						sprintf(buf, "�ջ�%s�Ķĳ�VIP��",uident);
						sprintf(genbuf, "%s��ʹ�ĳ�����Ȩ��",currentuser.userid);
						millionairesrec(genbuf, buf, "BMYboss");
						pressanykey();
					}
					break;
				case '3':
					clear();
					move(1, 0);
					prints("Ŀǰӵ�жĳ�VIP���Ŀͻ���");
					listfilecontent(DIR_MC "gamble_VIP");
					pressanykey();
					break;
				case '4':
					move(12, 4);
					/*if (time(0) <
					    3600 +
					    loadValue(currentuser.userid,
						      "last_invitation",
						      2000000000)) {
						prints("���̫���������ðɣ�");
						pressanykey();
						break;
					}*/
					usercomplete("��˭�����뺯��", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("�����ʹ���ߴ���...");
						pressanykey();
						break;
					}
					if (loadValue(uident, "invitation", 1)) {
						prints("�Ѿ������ˡ�");
						pressanykey();
						break;
					}
					if (askyn("ȷ����", NA, NA) == YEA) {
						saveValue(uident, "invitation",
							  1, 1);
						saveValue(currentuser.userid,
							  "last_invitation",
							  -2000000000,
							  2000000000);
						saveValue(currentuser.userid,
							  "last_invitation",
							  time(0), 2000000000);
						sprintf(genbuf,
							"%s ���������˶ĳ����뺯",
							currentuser.userid);
						mail_buf
						    ("�����л�����20��󽱣����ǣ����и���Ļ���Ϊ�������������������ڱ���ٸ�ĳ�����˹���̶ģ�",
						     uident, genbuf);
						move(14, 4);
						prints("���뺯����ȥ�ˡ�");
						sprintf(buf, "��%s���Ŷĳ����뺯",uident);
						sprintf(genbuf, "%s��ʹ�ĳ�����Ȩ��",currentuser.userid);
						millionairesrec(genbuf, buf, "BMYboss");
						pressanykey();
					}
					break;
				case '5':
					move(12, 4);
					if (askyn
					    ("�����Ҫ����ϴ����", NA,
					     NA) == YEA) {
					/*	del_from_file(MC_BOSS_FILE,
							      "gambling");
						sprintf(genbuf,
							"%s ������ȥ����ٸ�ĳ�����ְ��",
							currentuser.userid);
						deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
						move(14, 4);
						prints
						    ("�ðɣ���Ȼ�����Ѿ���������ֻ��˵�ټ��ˣ�");
						quit = 1;
					*/
						sprintf(genbuf, "%s Ҫ��ȥ�ĳ�����ְ��",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
						pressanykey();
					}
					break;
				case 'q':
				case 'Q':
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static int/*�Ĳ�-- 777*/
money_777()
{
	int ch;
	int quit = 0;
	int bid;
	int money;
	int t1, t2, t3, winrate, r;
	char n[9] = "-R-B-6-7";
	char title[STRLEN], buf[256];
	
	clear();
	srandom(time(0));
	while (!quit) {
		if (utmpshm->mc.prize777 <= 0)
			utmpshm->mc.prize777 = 30000;
		bid = 0;
		clear();
		money_show_stat("����ٸ�ĳ�777");
		move(6, 4);
		prints("--R 1:2    -RR 1:3    RR- 1:3    -BB 1:5    BB- 1:5");
		move(7, 4);
		prints("RRR 1:10   BBB 1:20   666 1:40   677 1:60   --- 1:1");
		move(8, 4);
		prints
		    ("         777 1:80 ���л���Ӯ�õ�ǰ�ۻ������һ��         ");
		move(9, 4);
		prints("����ٸĿǰ�ۻ�������: %d����Ӯ��ô��ѹ 100 �����Ŷ��",
		       utmpshm->mc.prize777);
		r = random() % 40;
		if (r < 1)
			money_police();

		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1] ѹ30 [2] ѹ100 [Q]�뿪                                          \033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			bid = 30;
			break;
		case '2':
			bid = 100;
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		if (quit)
			break;
		if (bid == 0)
			continue;
		money =
		    loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		if (money < bid) {
			move(11, 4);
			prints("ûǮ�ͱ����...");
			pressanykey();
			continue;
		}
		saveValue(currentuser.userid, MONEY_NAME, -bid, MAX_MONEY_NUM);
		t1 = random() % 8;
		t2 = random() % 8;
		t3 = random() % 8;
		move(11, 20);
		prints("%c", n[t1]);
		refresh();
		sleep(1);
		move(11, 22);
		prints("%c", n[t2]);
		refresh();
		sleep(1);
		move(11, 24);
		prints("%c", n[t3]);
		refresh();
		sleep(1);
		winrate = calc777(t1, t2, t3);
		if (winrate <= 0) {
			utmpshm->mc.prize777 += bid * 80 / 100;
			if (utmpshm->mc.prize777 >= MAX_MONEY_NUM)
				utmpshm->mc.prize777 = MAX_MONEY_NUM;
			
			sprintf(title, "%s����Ĳ�(777)(��)", currentuser.userid);
			sprintf(buf, "%s��777 ����%d����ٸ��", currentuser.userid, bid);
			millionairesrec(title, buf, "�Ĳ�777");
					
			move(12, 4);
			prints
			    ("���ˣ���ע�ٷ�֮��ʮ�������ٸ�ۻ������츣���˵����츣�Լ���");
			limit_cpu();
			pressanykey();
			continue;
		}
		if (winrate > 0) {
			saveValue(currentuser.userid, MONEY_NAME, bid * winrate,
				  MAX_MONEY_NUM);
			move(12, 4);
			prints("��Ӯ�� %d Ԫ", bid * (winrate - 1));
			utmpshm->mc.prize777 -= bid * (winrate - 1);

			sprintf(title, "%s����Ĳ�(777)(Ӯ)", currentuser.userid);
			sprintf(buf, "%s��777 Ӯ��%d����ٸ��", currentuser.userid, bid * (winrate - 1));
			millionairesrec(title, buf, "�Ĳ�777");
		}
		if (winrate == 81 && bid == 100) {
			saveValue(currentuser.userid, MONEY_NAME,
				  utmpshm->mc.prize777 / 2, MAX_MONEY_NUM);
			utmpshm->mc.prize777 /= 2;
			move(13, 4);
			prints("��ϲ����ñ���ٸ�󽱣�");
			sprintf(title, "%s����Ĳ�(777)(Ӯ������)", currentuser.userid);
			sprintf(buf, "%s��777 Ӯ��%d����ٸ��", currentuser.userid, utmpshm->mc.prize777 / 2);
			millionairesrec(title, buf, "�Ĳ�777");
		}
		limit_cpu();
		pressanykey();
	}
	return 0;
}

static int/*�Ĳ�--777*/
calc777(int t1, int t2, int t3)
{
	if ((t1 % 2 == 0) && (t2 % 2 == 0) && (t3 % 2 == 0))
		return 2;
	if ((t1 % 2 == 0) && (t2 % 2 == 0) && (t3 == 1))
		return 3;
	if ((t1 % 2 == 0) && (t2 == 1) && (t3 == 1))
		return 4;
	if ((t1 == 1) && (t2 == 1) && (t3 % 2 == 0))
		return 4;
	if ((t1 % 2 == 0) && (t2 == 3) && (t3 == 3))
		return 6;
	if ((t1 == 3) && (t2 == 3) && (t3 % 2 == 0))
		return 6;
	if ((t1 == 1) && (t2 == 1) && (t3 == 1))
		return 11;
	if ((t1 == 3) && (t2 == 3) && (t3 == 3))
		return 21;
	if ((t1 == 5) && (t2 == 5) && (t3 == 5))
		return 41;
	if ((t1 == 5) && (t2 == 7) && (t3 == 7))
		return 61;
	if ((t1 == 7) && (t2 == 7) && (t3 == 7))
		return 81;
	return 0;
}

static int/*�Ĳ�--������*/
guess_number()
{
	int quit = 0;
	int ch, num, money;
	int a, b, c;
	int win;
	int count;
	char ans[5] = "";
	int bet[7] = { 0, 100, 50, 20, 5, 3, 1 };
	char title[STRLEN], buf[256];
	
	srandom(time(0));
	while (!quit) {
		clear();
		money_show_stat("����ٸ��������֮·...");
		move(4, 4);
		prints("\033[1;31m�����Խ�׬Ǯ��~~~\033[m");
		move(5, 4);
		//prints("��Сѹ 100 ����ٸ�ң�����999");
		prints("һ�� 100 ����ٸ��.");
		move(6, 4);
		prints("mAnB��ʾ��m�����ֲ¶���λ��Ҳ��,n�����ֲ¶Ե�λ�ò���");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��ע [Q]�뿪                                                 \033[m");
		if (random() % 40 < 1)
			money_police();
		ch = igetkey();
		switch (ch) {
		case '1':
			win = 0;
		
			getdata(8, 4, "��ѹ���ٱ���ٸ�ң�[100]", genbuf, 5,
			   DOECHO, YEA);
			   num = atoi(genbuf);
			   if (!genbuf[0])
			   num = 100;
			   if (num < 100) {
			   move(9, 4);
			   prints("��û��Ǯ������ô��Ǯ���ǲ������");
			   pressanykey();
			   break;
			   } 
			//num = 100;
			sprintf(genbuf,
				"��ѹ�� \033[1;31m%d\033[m ����ٸ�ң�ȷ��ô��",
				num);
			move(9, 4);
			if (askyn(genbuf, YEA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				if (money < num) {
					move(11, 4);
					prints("ȥȥȥ��û��ô��Ǯ��ʲô��         \n");
					pressanykey();
					break;
				}
				//if (num > 999)
					//num = 999;
				saveValue(currentuser.userid, MONEY_NAME, -num,
					  MAX_MONEY_NUM);
				do {
					itoa(random() % 10000, ans);
					for (a = 0; a < 3; a++)
						for (b = a + 1; b < 4; b++)
							if (ans[a] == ans[b])
								ans[0] = 0;
				} while (!ans[0]);
				for (count = 1; count < 7; count++) {
					do {
						move(10, 4);
						prints
						    ("�������ĸ����ظ�������");
						getdata(11, 4, "���[q - �˳�] �� ", genbuf, 5, DOECHO, YEA);
						if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
							utmpshm->mc.prize777 += num;
							if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
								utmpshm->mc.prize777 = MAX_MONEY_NUM;
							move(12, 4);
							prints("byebye!");
							pressanykey();
							quit = 1;
							return 0;
						}
						c = atoi(genbuf);
						itoa(c, genbuf);
						for (a = 0; a < 3; a++)
							for (b = a + 1; b < 4; b++)
								if (genbuf[a] == genbuf[b])
									genbuf[0] = 0;
						if (!genbuf[0]) {
							move(12, 4);
							prints ("��������������!!");
							pressanykey();
							move(12, 4);
							prints ("                ");
						}
					} while (!genbuf[0]);
					move(count + 13, 0);
					prints("  �� %2d �Σ� %s  ->  %dA %dB ",
					       count, genbuf, an(genbuf, ans),
					       bn(genbuf, ans));
					if (an(genbuf, ans) == 4)
						break;
					sleep(1);
				}

				move(12, 4);
				if (count > 6) {
					sprintf(genbuf,
						"�������ϣ���ȷ���� %s���´��ټ��Ͱ�!!",
						ans);
					sprintf(genbuf,
						"\033[1;31m����û�µ������� %d Ԫ��\033[m",
						num);
					//utmpshm->mc.prize777 += num;

					sprintf(title, "%s����Ĳ�(������)(��)", currentuser.userid);
					sprintf(buf, "%s�ڲ���������%d����ٸ��", currentuser.userid, num);
					millionairesrec(title, buf, "�Ĳ�������");
					
					if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
						utmpshm->mc.prize777 = MAX_MONEY_NUM;
				} else {
					int oldmoney = num;
					num *= bet[count];
					if (num - oldmoney > 0) {
						sprintf(genbuf,
							"��ϲ���ܹ����� %d �Σ���׬���� %d Ԫ",
							count, num);
						num += oldmoney;
						saveValue(currentuser.userid,
							  MONEY_NAME, num,
							  MAX_MONEY_NUM);
						win = 1;
						
						sprintf(title, "%s����Ĳ�(������)(Ӯ)", currentuser.userid);
						sprintf(buf, "%s�ڲ�����Ӯ��%d����ٸ��", currentuser.userid, num);
						millionairesrec(title, buf, "�Ĳ�������");
					
					} else if (num - oldmoney == 0) {
						sprintf(genbuf,
							"�������ܹ����� %d �Σ�û��ûӮ��",
							count);
						saveValue(currentuser.userid,
							  MONEY_NAME, num,
							  MAX_MONEY_NUM);
					} else {
						utmpshm->mc.prize777 +=
						    oldmoney;
						if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
							utmpshm->mc.prize777 = MAX_MONEY_NUM;

						sprintf(genbuf,
							"�������ܹ����� %d �Σ���Ǯ %d Ԫ��",
							count,
							oldmoney - money);
					}
				}
				prints("���: %s", genbuf);
				move(13, 4);
				pressanykey();
			}
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int
an(a, b)
char *a, *b;
{
	int i, k = 0;
	for (i = 0; i < 4; i++)
		if (*(a + i) == *(b + i))
			k++;
	return k;
}

static int
bn(a, b)
char *a, *b;
{
	int i, j, k = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (*(a + i) == *(b + j))
				k++;
	return (k - an(a, b));
}

static void
itoa(i, a)
int i;
char *a;
{
	int j, k, l = 1000;

	for (j = 3; j > 0; j--) {
		k = i - (i % l);
		i -= k;
		k = k / l + 48;
		a[3 - j] = k;
		l /= 10;
	}
	a[3] = i + 48;
	a[4] = 0;

}

static int/*����--�����ټ�*/
money_police()
{
	char ch;
	char buf[200], title[STRLEN];
	int money = 0, quit = 1, check_num;
	//int mode = 0, color;
	move(t_lines - 1, 0);
	check_num = 97 + random() % 26;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
	if (random() % 4 > 0) {
		sprintf
		    (buf,
		     "\033[1;41m ϵͳ�ټ� \033[1;46m �������ַ�:%c        \033[m",
		     check_num);
	} 
	//else if (random() % 3 == 0)
	else {
		check_num = 0;
		sprintf(buf,
			"\033[1;41m ϵͳ�ټ� \033[1;46m ���������ID(ע���Сд!):        \033[m");
	} 
	/*else {
		mode = 1;
		color = (random() % 7) + 31;
		snprintf(buf, 200,
			 "\033[1;41m ϵͳ�ټ� \033[1;46m \033[1;%dm������ɫ\033[1;37m:[\033[1;31m��\033[1;32m��\033[1;33m��\033[1;34m��\033[1;35m��\033[1;36m��\033[1;37m��]\033[m",
			 color);
		getdata(t_lines - 1, 4, buf, genbuf, 5, DOECHO, YEA);
		if (color == 31 && (!strcmp("��", genbuf) || !strcmp("red", genbuf)))
			quit = 0;
		if (color == 32 && (!strcmp("��", genbuf) || !strcmp("green", genbuf)))
			quit = 0;
		if (color == 33 && (!strcmp("��", genbuf) || !strcmp("yellow", genbuf)))
			quit = 0;
		if (color == 34 && (!strcmp("��", genbuf) || !strcmp("blue", genbuf)))
			quit = 0;
		if (color == 35 && (!strcmp("��", genbuf) || !strcmp("purple", genbuf)))
			quit = 0;
		if (color == 36 && (!strcmp("��", genbuf) || !strcmp("cyan", genbuf)))
			quit = 0;
		if (color == 37 && (!strcmp("��", genbuf) || !strcmp("white", genbuf)))
			quit = 0;
			quit = 0;
	}
	*/
	//if (mode == 0) {
		getdata(t_lines - 1, 4, buf, genbuf, 13, DOECHO, YEA);
		if (check_num > 0) {
			ch = genbuf[0];
			if (ch == check_num)
				quit = 0;
			else
				quit = 1;
		} else {
			if (strcmp(genbuf, currentuser.userid))
				quit = 1;
			else
				quit = 0;
		}
	//}
	if (quit) {
		move(t_lines - 2, 4);
		prints("ϵͳ��Ϊ�����������ɣ��������������ֽ�,��Ұ�~~~");
		saveValue("millionaires", MONEY_NAME, money, MAX_MONEY_NUM);
		sprintf(title, "%s��ϵͳ�ټ�", currentuser.userid);
		sprintf(buf, "ϵͳ�ټ�, %s�����������ֽ�%d����ٸ��", currentuser.userid, money);
		millionairesrec(title, buf, "ϵͳ�ټ�");
		pressanykey();
		Q_Goodbye();
	} else {
		saveValue(currentuser.userid, MONEY_NAME, money, MAX_MONEY_NUM);
		move(t_lines - 2, 4);
		sprintf(buf, "����Ǵ���������%s��,��������...",
			currentuser.userid);
		prints("%s", buf);
		pressanykey();

	}
	return 0;
}

static void/*���˹�Ʊϵͳ*/
persenal_stock_info(int stock_num[MAX_STOCK_NUM],
		    int stock_price[MAX_STOCK_NUM], int money,
		    char stockboard[STRLEN][MAX_STOCK_NUM], int stock_board[MAX_STOCK_NUM])
{
	int i, count;
	count = listfilecontent(MC_STOCK_BOARDS);
	clear();
	move(0, 4);
	prints("����ٸ������Ӫҵ...��������ĸ��ɳ�����,ÿ�ɹ�������1000000��");
	move(1, 4);
	sprintf(genbuf, "Ŀǰ��ı���ٸ�ҽ��Ϊ%d", money);
	prints("%s", genbuf);
	for (i = 0; i < count; i++) {
		move(3 + i, 0);
		/*sprintf(genbuf,
			"���:%2d Stock%c��Ǯ:%d\t������:%d\t����:%-10s ���й�Ʊ��:%d",i,
			65 + i, stock_price[i], stock_num[i], stockboard[i],
			bcache[stock_board[i]].stocknum);
		*/
		sprintf(genbuf,
			"���:%2d ��Ǯ:%-5d ������:%-7d ����:%-18s ���й�Ʊ��:%d",
 			i, stock_price[i], stock_num[i], stockboard[i], bcache[stock_board[i]].stocknum);
		if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i]))
			prints("\033[1;30m%s\033[m", genbuf);
		else
			prints("%s", genbuf);
	}
}
/*
static void//���˹�Ʊϵͳ
persenal_stock_info2(int stock_num[MAX_STOCK_NUM2],
		    int stock_price[MAX_STOCK_NUM2], int money,
		    char *stockboard[], int stock_board[MAX_STOCK_NUM2])
{
	int i;
	clear();
	move(0, 4);
	prints("����ٸ������Ӫҵ...��������ĸ��ɳ�����,ÿ�ɹ�������50,000��");
	move(1, 4);
	sprintf(genbuf, "Ŀǰ��ı���ٸ�ҽ��Ϊ%d", money);
	prints("%s", genbuf);
	for (i = 0; i < MAX_STOCK_NUM2; i++) {
		move(3 + i, 0);
		sprintf(genbuf,
			"���:%2d Stock%c��Ǯ:%d\t������:%d\t����:%-10s���й�Ʊ��:%d",i,
			65 + i, stock_price[i], stock_num[i], stockboard[i],
			bcache[stock_board[i]].stocknum);
		prints("%s", genbuf);
	}
}*/

/*-------------�����ɵĺؿ�ϵͳ-------macintosh 20051203------*/
/*static int
shop_card_show(char *card[][2], int group)
{
	int key, i, j, x = 0, y = 0;
	int global_x = 0, local_x = 0, limit = 0, base = 0;
	y = 1;
	clear();
	move(5, 4);
	prints("����ؿ����Ǳ������������ֺؿ���������ԭ��δ�ܱ������ߡ�\n");
	move(6, 4);
	prints("��ؿ������߶�����Ʒ���ڱ���������飬���뱾վ�����ܹ���ϵ��\n");
	move(7, 4);
	prints("��վ����ʱ����������Ը����������\n");
	move(9, 20);
	prints("����ٸ�ؿ��� \n");
	pressanykey();
	while (y) {
		clear();
		nomoney_show_stat("����ٸ�ؿ��̵�");

		if (y == 1) {
			for (i = 0; i < group; i++) {
				move(5 + i, 4);
				if (i == x)
					sprintf(genbuf,
						"\033[1;41m[+]> %s\033[0m",
						card[i][0]);
				else
					sprintf(genbuf,
						"\033[1;43m[+]  %s\033[0m",
						card[i][0]);
				prints("%s", genbuf);
			}
		} else if (y == 2) {
			i = 0;
			for (j = 0; j < group; j++) {
				if (6 + 2 + local_x > 22) {
					limit = 1;
					base = 8 + local_x - 22;
				} else
					limit = 0;
				if (j == global_x) {
					if (!limit) {
						move(5 + j + i, 4);
						sprintf(genbuf,
							"\033[1;44m[-] %s\033[0m",
							card[j][0]);
						prints("%s", genbuf);
					}
					for (i = 0; i < atoi(card[x][1]); i++) {
						if (!limit) {
							if (6 + j + i > 23)
								continue;
							else {
								move(6 + j + i,
								     8);
								if (i ==
								    local_x)
									sprintf
									    (genbuf,
									     "\033[1;41m>|--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								else
									sprintf
									    (genbuf,
									     "\033[1;42m |--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								prints("%s",
								       genbuf);
							}
						} else {
							// base = 8+local_x-22;
							// local_x = 15; base = 1;
							if ((i - base) > 4
							    && (i - base) <
							    24) {
								move(i - base,
								     8);
								if (i ==
								    local_x)
									sprintf
									    (genbuf,
									     "\033[1;41m>|--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								else
									sprintf
									    (genbuf,
									     "\033[1;42m |--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								prints("%s",
								       genbuf);
							} else
								continue;
						}
					}
				} else {
					if (!limit) {
						if ((5 + j + i) < 24
						    && (5 + j + i) > 4) {
							move(5 + j + i, 4);
							sprintf(genbuf,
								"\033[1;43m[+] %s\033[0m",
								card[j][0]);
							prints("%s", genbuf);
						}
					}
				}
			}
		}
		move(t_lines - 1, 4);
		prints
		    ("\033[1;45m��������������������һ�㣬�Ҽ����룬���¼�ѡ�� \033[0m");
		key = egetch();
		switch (key) {
		case KEY_LEFT:
		case 'q':
		case 'Q':
		case 'e':
		case 'E':
			y--;
			global_x = x;
			break;
		case KEY_RIGHT:
		case '\n':
		case '\r':
			if (y == 2) {
				buy_card(card[global_x][0], local_x + 1);
			}
			if (y < 2) {
				y++;
				local_x = 0;
			}
			global_x = x;
			break;
		case KEY_UP:
			if (y == 2) {
				local_x--;
				if (local_x < 0)
					local_x = atoi(card[x][1]) - 1;
			} else {
				x--;
				if (x < 0)
					x = group - 1;

			}
			break;
		case KEY_DOWN:
			if (y == 2) {
				local_x++;
				if (local_x >= atoi(card[x][1]))
					local_x = 0;
			} else {
				x++;
				if (x > group - 1)
					x = 0;

			}
			break;
		}
		limit_cpu();
		if (y == 0)
			break;
	}
	move(t_lines - 2, 5);
	prints("��ӭ������!");
	pressanykey();
	return 0;
}*/

/*----�����ɵĺؿ�ϵͳ(����ؿ�)-----macintosh 20051203-----*/
/*static int
buy_card(char *cardname, int cardnumber)
{
	char card_name[20], filepath[80], uident[IDLEN + 1];
	char note[3][STRLEN], tmpname[STRLEN];
	int money, i;
	bzero(card_name, sizeof (card_name));
	sprintf(card_name, "%s%d", cardname, cardnumber);
	sprintf(filepath, "0Announce/game/cardshop/%s/%d", cardname,
		cardnumber);
	ansimore2(filepath, 0, 0, 25);
	move(2, 0);
	prints("Preview....");
	move(8, 20);
	prints("Preview....");
	move(14, 40);
	prints("Preview....");
	//clear();
	move(t_lines - 2, 4);
	sprintf(genbuf, "��ȷ��Ҫ��ؿ�%sô?", card_name);
	if (askyn(genbuf, YEA, NA) != YEA)
		return 0;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	if (money < 1000) {
		move(t_lines - 2, 4);
		prints("���Ǯ������~~~");
		pressanykey();
		return 0;
	}

	saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
	move(0, 0);
	clrtobot();
	usercomplete("Ҫ���⿨Ƭ�͸�˭? ", uident);
	if (uident[0] == '\0') {
		move(t_lines - 2, 4);
		clrtobot();
		prints("��Ȼ��д��ַ������Ŀ�Ƭ��ʧ���ʼ�;��...");
		pressanykey();
		return 0;
	}

	if (!getuser(uident)) {
		move(t_lines - 2, 4);
		clrtobot();
		prints("û������˰�������Ŀ�Ƭ���ʵ�Ա˽����...");
		pressanykey();
		return 0;
	}
	move(0, 0);
	clear();
	prints("�л�Ҫ�ڿ�Ƭ��˵��");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(1 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}
	sprintf(tmpname, "bbstmpfs/tmp/card.%s.%d", currentuser.userid,
		uinfo.pid);
	copyfile(filepath, tmpname);
	if (i > 0) {
		int j;
		FILE *fp = fopen(tmpname, "a");
		fprintf(fp, "\n������ %s �ĸ���:\n", currentuser.userid);
		for (j = 0; j < i; j++)
			fprintf(fp, "%s", note[j]);
		fclose(fp);
	}

	move(t_lines - 2, 4);
	clrtobot();
	prints("��ĺؿ��Ѿ�����ȥ��");
	mail_file(tmpname, uident, "�Ҵӱ���ٸ�̵���������ĺؿ����㿴������ô?");
	unlink(tmpname);
	pressanykey();
	return 0;
}*/

//add by happybird for �ʻ��꣬�ؿ���
//��ʾ��ӭ����
//���Ҫ���ļ����ܻ������ģ�С��ʹ��
int show_welcome(char *filepath,int startline,int endline)
{
	FILE *fp;
	char buf[400];
	int linecount=0;

	fp=fopen(filepath,"r");
	if(!fp){
		move(startline,10);
		prints("��ӭ���ĵ���!");
		return 0;
	}
	linecount=0;
	while(!feof(fp)){
		if(fgets(buf,400,fp)){
			move(startline+linecount,0);
			prints("%s",buf);
			linecount++;
		}
		if(linecount >endline-startline) break;
	}
	fclose(fp);
	return 1;
}


#define PRESENT_DIR "0Announce/groups/GROUP_0/" MC_BOARD "/system/present"
//#define FLOWER_WELCOME	MY_BBS_HOME FLOWER_DIR "/welcome"
#define PATHLEN 1000

//��Ʒ�꣬�ʻ��ؿ�����һ��macintosh@bmy 20051204
static int 
shop_present(int order, char *kind, char *touserid)
{
	char ok_filename[PATHLEN];
	char ok_title[STRLEN];
	int price_per=0;
	char *ptr1,*ptr2;
	//char filepath[256];
	//void *buffer_me = NULL;
	char buf[STRLEN];
	int ma;

	sprintf(buf, "����ٸ��Ʒ��%s��̨", kind);
	nomoney_show_stat(buf);
	sprintf(buf, "%s%d%s", PRESENT_DIR, order, "/welcome");
	show_welcome(buf,6,20);
	pressanykey();

	DIR *dp;
//	struct dirent *dirp;
	char dirNameBuffer[10][PATHLEN], dirTitleBuffer[10][STRLEN];
	char fileNameBuffer[10][PATHLEN],  fileTitleBuffer[10][STRLEN];
	char dirpath[PATHLEN], filepath[PATHLEN], dir[PATHLEN], indexpath[PATHLEN], title[STRLEN];
	int numDir=0, numFile=0, dirIndex, cardIndex, m;
	int HIDE=0;
	FILE *fp;

	sprintf(buf, "����ٸ��Ʒ��%s��̨", kind);
	nomoney_show_stat(buf);
	move(4,4);
	sprintf(dir, "%s%d/", PRESENT_DIR, order);
	sprintf(indexpath, "%s.Names", dir);
	prints("����������������%s: ", kind);
	if ((dp = opendir(dir)) == NULL)
		return -1;

	fp=fopen(indexpath, "r");
	if(fp!=0) { 
		while(fgets(buf, STRLEN, fp)>0 && numDir<10) {
			if(!strncmp(buf, "Name=", 5)) {
				sprintf(title, "%s", buf+5);
				if(strstr(title + 38,"(BM: SYSOPS)") ||
					strstr(title + 38,"(BM: BMS)")||
					!strncmp(title, "<HIDE>",6))
					HIDE=1;
				else
					HIDE=0;
				title[38]=0;	
				fgets(buf, STRLEN, fp);
				if(!strncmp("Path=~/", buf, 6)) {
					if(HIDE) continue;
					snprintf(dirpath, PATHLEN,  "%s%s", dir, buf+7);
					for(m=0; m<strlen(dirpath); m++) if (dirpath[m]<27) dirpath[m]=0;
					if (!file_isdir(dirpath))
						continue;
					for(m=0; m<strlen(title); m++) if(title[m]==' ') title[m]=0;
					strncpy(dirNameBuffer[numDir], dirpath, PATHLEN);
					strncpy(dirTitleBuffer[numDir], title, STRLEN);
					move(6 + numDir, 8);
					prints("%d. %s", numDir, title);
					numDir++;
				}
			}
		}
		fclose(fp);
	}

/*	
	for (numDir = 0; (dirp = readdir(dp)) != NULL && numDir < 10; ) {
		snprintf(dirpath, 255,  "%s%s", dir, dirp->d_name);
		if (!file_isdir(dirpath) || dirp->d_name[0] == '.')
			continue;
		move(6 + numDir, 8);
		prints("%d. %s", numDir, dirp->d_name);
		strncpy(dirNameBuffer[numDir], dirp->d_name, 31);
		dirNameBuffer[numDir][31] = '\0';
		numDir++;
	}
*/
	while(1) {
		getdata(16, 4, "��ѡ������:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		dirIndex = atoi(buf);
		if (dirIndex >= 0 && dirIndex < numDir)
			break;
	}

	sprintf(buf, "����ٸ��Ʒ��%s��̨", kind);
	nomoney_show_stat(buf);
	move(4,4);
	snprintf(dirpath, PATHLEN, "%s", dirNameBuffer[dirIndex]);
	if ((dp = opendir(dirpath)) == NULL)
		return -1;
	//prints("����������������%s: ", kind);

	sprintf(indexpath, "%s/.Names", dirpath);
	fp=fopen(indexpath, "r");
	if(fp!=0) { 
		while(fgets(buf, STRLEN, fp)>0 && numFile<10) {
			if(!strncmp(buf, "Name=", 5)) {
				sprintf(title, "%s", buf+5);
				if(strstr(title + 38,"(BM: SYSOPS)") ||
					strstr(title + 38,"(BM: BMS)")||
					!strncmp(title, "<HIDE>",6))
					HIDE=1;
				else
					HIDE=0;
				title[38]=0;	
				fgets(buf, STRLEN, fp);
				if(!strncmp("Path=~/", buf, 6)) {
					if(HIDE) continue;
					snprintf(filepath, PATHLEN,  "%s/%s", dirpath, buf+7);
					for(m=0; m<strlen(filepath); m++) if (filepath[m]<27) filepath[m]=0;
					if (!file_isfile(filepath))
						continue;
					//for(m=0; m<strlen(title); m++) if(title[m]==' ') title[m]=0;
					strncpy(fileNameBuffer[numFile], filepath, PATHLEN);
					strncpy(fileTitleBuffer[numFile], title, STRLEN);
					move(6 + numFile, 8);
					prints("%d. %s", numFile, title);
					numFile++;
				}
			}
		}
		fclose(fp);
	}

	move(4,4);
	prints("���� %s ��%s���� %d ��: ", dirTitleBuffer[dirIndex], kind, numFile);
/*
	for (numFile = 0; (dirp = readdir(dp)) != NULL; ) {
		snprintf(filepath, PATHLEN, "%s/%s", dirpath, dirp->d_name);
		if(file_isfile(filepath) && dirp->d_name[0] != '.')
			numFile++;
	}
*/	
	move(17, 4);
	while(1) {
		getdata(18, 4, "��ѡ��ҪԤ���ı��[ENTER����]: ", 
			buf, 3, DOECHO, YEA);
		if(buf[0] == '\0')
			return 0;
		cardIndex = atoi(buf);
		if (cardIndex >= 0 && cardIndex <= numFile)
			break;
	}

	sprintf(buf, "%s��̨%s ��%sչʾ", kind, dirTitleBuffer[dirIndex], fileTitleBuffer[cardIndex]);
	nomoney_show_stat(buf);
	//show_welcome(fileNameBuffer[cardIndex], 5, 20);
	ansimore2(fileNameBuffer[cardIndex], 1, 5, 20);

	limit_cpu();

	strncpy(ok_filename, fileNameBuffer[cardIndex], PATHLEN);
	strncpy(ok_title, fileTitleBuffer[cardIndex], STRLEN);
	if(!ok_filename[0])  return 0;

	sprintf(buf, "��Ʒ��%s����̨", kind);
	money_show_stat(buf);
	//ok_title= õ�廨1(֦)   ��:100bmyb
	ptr1= strstr(ok_title,"��:");
	if(!ptr1){
		move(7,10);
		prints("My God! ����Ʒ��û�ж��ۣ��Ͽ�ȥ������Ʒ���ϰ��");
		pressanykey();
		return 0;
	}
	ptr1+=3;
	if(!ptr1){
		move(7,10);
		prints("My God! ����Ʒ��û�ж��ۣ��Ͽ�ȥ������Ʒ���ϰ��");
		pressanykey();
		return 0;
	}
	ptr2= strstr(ptr1,"bmyb");
	if(!ptr2){
		move(7,10);
		prints("My God! ����Ʒ���������⣬�Ͽ�ȥ������Ʒ���ϰ��");
		pressanykey();
		return 0;
	}
	*ptr2='\0';
	price_per = atoi(ptr1);
	*ptr2='b';
	if(price_per<0){
			move(7,10);
			prints("My God! ����Ʒ���������⣬�Ͽ�ȥ������Ʒ���ϰ��");
			pressanykey();
			return 0;
	}else if(price_per == 0){
			move(7,10);
			prints("�ٺ�! ����Ʒ������ͣ��Ժ�Ҫ���֧�ֱ����");
			pressanykey();
	}
	
	ptr1-=4;
	while(*ptr1==' ') ptr1--;
	*(ptr1+1) = '\0';

	move(8,10);
	sprintf(genbuf, "��ȷ��Ҫ��Ǯ����%s��",ok_title);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;		
	ma =
		buy_present(order, kind, ok_title, ok_filename, price_per, touserid);
	if (ma==9) return 9;
	return 1;
}


static int
buy_present(int order, char *kind, char *cardname, char *filepath, int price_per,char *touserid)
{
	int i;
	int inputNum=1;
	char uident[IDLEN + 1], note[3][STRLEN], tmpname[STRLEN];
	int price;
	char buf[200];
	char *ptr1,*ptr2;
	char unit[STRLEN];
	
	clear();
	ansimore2(filepath, 0, 0, 25);
	move(t_lines - 2, 0);
	sprintf(genbuf, "������Ҫ���������[%d]: ",inputNum);
	while(1) {
		inputNum = 1;
		getdata(15, 4, genbuf, buf, 8, DOECHO, YEA);
		if(buf[0] == '\0' || (inputNum = atoi(buf)) >= 1)
			break;
	}
	price = price_per*inputNum;
	//�Ӹ�����
	if (price < 0 || price > MAX_MONEY_NUM){
		move(t_lines - 2, 4);
		prints("���ڻ�������ǰԤԼ...");
		pressanykey();
		return 0;
	}
	
	//cardname= õ�廨1(֦)   ��:100bmyb
	ptr1= strstr(cardname,"(");
	ptr1++;
	if(!ptr1)
		sprintf(buf,"%s","��");
	else{
		ptr2=strstr(ptr1,")");
		if(!ptr2)
			sprintf(buf,"%s","��");
		else{
			*ptr2='\0';
			strncpy(buf, ptr1, STRLEN);
			}
		if (!strlen(buf))
			sprintf(buf,"%s","��");
		ptr1--;
		*ptr1='\0';
		}
	sprintf(unit,"%s",buf);
	sprintf(genbuf, "��ȷ��Ҫ����%d����ٸ�ҹ���%d%s%s��",price,inputNum,unit,cardname);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;	
	if (loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) < price) {
		move(t_lines - 2, 4);
		prints("���Ǯ������~~~");
		pressanykey();
		return 0;
	}
	saveValue(currentuser.userid, MONEY_NAME, -price, MAX_MONEY_NUM);
	saveValue("millionaires", MONEY_NAME, price, MAX_MONEY_NUM);
	clear();
	if(touserid && touserid[0]){
		strncpy(uident,touserid,IDLEN);
	}else{
		move(0, 0);
		sprintf(buf, "Ҫ��%s�͸�˭? ", kind);
		usercomplete(buf, uident);
		if (uident[0] == '\0') {
			move(t_lines - 2, 4);
			clrtobot();
			sprintf(buf, "��Ȼ��д��ַ�������%s��ʧ���ʼ�;��...", kind);
			prints(buf);
			pressanykey();
			return 0;
			}
		if (!getuser(uident)) {
			move(t_lines - 2, 4);
			clrtobot();
			sprintf(buf, "û������˰��������%s���ʵ�Ա˽����...", kind);
			prints(buf);
			pressanykey();
			return 0;
			}
		}
	move(2, 0);
	prints("����ʲô��Ҫ������[����д3���]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(3 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}
	sprintf(tmpname, "bbstmpfs/tmp/present.%s.%d", currentuser.userid,
		uinfo.pid);
	copyfile(filepath, tmpname);
	if (i > 0) {
		int j;
		FILE *fp = fopen(tmpname, "a");
		fprintf(fp, "\n������ %s �ĸ���:\n", currentuser.userid);
		for (j = 0; j < i; j++)
			fprintf(fp, "%s", note[j]);
		fclose(fp);
	}
	sprintf(buf,"����%d%s%s��ϲ����",inputNum,unit,cardname);
	if (mail_file(tmpname, uident, buf) >= 0) {
		move(8,0);
		sprintf(buf,"���%s�Ѿ�����ȥ��",kind);
		prints(buf);
		pressanykey();
		return 9; //for marry
	} else {
		move(8,0);
		prints("����ʧ�ܣ��Է����䳬��");
		pressanykey();
	}
	unlink(tmpname);
	return 0;
}


/* write by dsyan               */
/* 87/10/24                     */
/* �쳤�ؾ� Forever.twbbs.org   */

//char card[52], mycard[5], cpucard[5], sty[100], now;
char *card, *mycard, *cpucard, *sty;
int now;
static int
forq(a, b)
char *a, *b;
{
	char c = (*a) % 13;
	char d = (*b) % 13;
	if (!c)
		c = 13;
	if (!d)
		d = 13;
	if (c == 1)
		c = 14;
	if (d == 1)
		d = 14;
	if (c == d)
		return *a - *b;
	return c - d;
}

static void/*�Ĳ�--���*/
p_gp()
{
	char genbuf[200], hold[5];
	int quit = 0;
	int num, i, j, k, tmp, x, xx, doub, gw = 0, cont = 0, money = 0;
//      int game_times = 0;
	char ans[5] = "", ch = ' ';
	char c1[52], mycard1[5], cpucard1[5], sty1[100];
	char title[STRLEN], buf[256];
	card = c1;
	mycard = mycard1;
	cpucard = cpucard1;
	sty = sty1;
	srandom(time(0));
	num = 0;
	while (!quit) {
		clear();
		nomoney_show_stat("���˿����");
		move(4, 4);
		prints("\033[1;31m�����Խ�׬Ǯ��~~~\033[m");
		move(5, 4);
		prints("һ��ѹ 100 ����ٸ��");
		move(6, 4);
		prints("��С:");
		move(7, 4);
		prints
		    ("ͬ��˳����֦����«��ͬ����˳�ӣ����������ߣ����ߣ�����");
		move(8, 4);
		prints("����ӷ֣�");
		move(9, 4);
		prints("ͬ��˳  ������");
		move(10, 4);
		prints("����֦  ������");
		move(11, 4);
		prints("����«��������");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]��ע [Q]�뿪                                                \033[m");
		if (random() % 40 < 1)
			money_police();
		if (!cont)
			ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			money_show_stat("���˿����");
			if (!cont) {
				ans[0] = 0;
				move(6, 4);
				//if (askyn("��ȷ��Ҫѹ100����ٸ��ô?", YEA, NA) == NA)
					//break;
				getdata(8, 4, "��ѹ���ٱ���ٸ�ң�[100-9999]", genbuf, 5,
					DOECHO, YEA);
				num = atoi(genbuf);
				if (!genbuf[0])
				num = 999;
				if (num < 100) {
				   move(9, 4);
				   prints("��û��Ǯ������ô��Ǯ���ǲ������");
				   pressanykey();
				   break;
				} 
				money = loadValue(currentuser.userid, 
					MONEY_NAME, MAX_MONEY_NUM);
				if (money < num) {
					move(8, 4);
					prints("ȥȥȥ��û��ô��Ǯ��ʲô�ң�      \n");
					pressanykey();
					break;
				}
				//num = 100;
				saveValue(currentuser.userid, MONEY_NAME, -num,
						  MAX_MONEY_NUM);
			}
			clear();
			money_show_stat("���˿����");
			move(21, 0);
			if (cont > 0)
				prints
				    ("\033[33;1m (��)(��)�ı�ѡ��  (SPACE)�ı任��  (Enter)ȷ��\033[m");
			else
				prints
				    ("\033[33;1m (��)(��)�ı�ѡ��  (d)Double  (SPACE)�ı任��  (Enter)ȷ��\033[m");
			move(22, 0);
			prints("��ǰ��ע���: %d ����ٸ��", num);
			for (i = 0; i < 52; i++)
				card[i] = i;	/* 0~51 ..�ڽܿ��� 1~52 */

			for (i = 0; i < 1000; i++) {
				j = random() % 52;
				k = random() % 52;
				tmp = card[j];
				card[j] = card[k];
				card[k] = tmp;
			}

			now = doub = 0;
			for (i = 0; i < 5; i++) {
				mycard[i] = card[now++];
				hold[i] = 1;
			}
			qsort(mycard, 5, sizeof (char), (void *) forq);

			for (i = 0; i < 5; i++)
				show_card(0, mycard[i], i);

			x = xx = tmp = 0;
			while (tmp != '\r' && tmp != '\n' && tmp != 'k') {
				for (i = 0; i < 5; i++) {
					move(16, i * 4 + 2);
					outs(hold[i] < 0 ? "��" : "  ");
					move(17, i * 4 + 2);
					outs(hold[i] < 0 ? "��" : "  ");
				}
				move(8, xx * 4 + 2);
				outs("  ");
				move(8, x * 4 + 2);
				outs("��");
				move(t_lines - 1, 0);
				xx = x;

				tmp = egetch();
				switch (tmp) {
#ifdef GP_DEBUG
				case KEY_UP:
					getdata(21, 0, "���ƻ���> ", genbuf, 3,
						DOECHO, YEA);
					mycard[x] = atoi(genbuf);
					qsort(mycard, 5, sizeof (char), forq);
					for (i = 0; i < 5; i++)
						show_card(0, mycard[i], i);
					break;
#endif
				case KEY_LEFT:
				case 'l':
					x = x ? x - 1 : 4;
					break;
				case KEY_RIGHT:
				case 'r':
					x = (x == 4) ? 0 : x + 1;
					break;
				case ' ':
					hold[x] *= -1;
					break;
				case 'd':
					if (!cont && !doub
					    && loadValue(currentuser.userid,
							 MONEY_NAME,
							 MAX_MONEY_NUM) >=
					    num) {
						doub++;
						move(21, 0);
						clrtoeol();
						prints
						    ("\033[33;1m (��)(��)�ı�ѡ��  (SPACE)�ı任��  (Enter)ȷ��\033[m");
						saveValue(currentuser.userid,
							  MONEY_NAME, -num,
							  MAX_MONEY_NUM);
						num *= 2;
						move(22, 0);
						prints("��ǰ��ע��� %d ����ٸ��",
						       num);
						//show_money(bet, NULL, NA);
					}
					break;
				}
			}

			for (i = 0; i < 5; i++)
				if (hold[i] == 1)
					mycard[i] = card[now++];
			qsort(mycard, 5, sizeof (char), (void *) forq);
			for (i = 0; i < 5; i++)
				show_card(0, mycard[i], i);
			move(11, x * 4 + 2);
			outs("  ");
			money_cpu();
#ifdef GP_DEBUG
			for (x = 0; x < 5; x++) {
				getdata(21, 0, "���ƻ���> ", genbuf, 3, DOECHO,
					YEA);
				cpucard[x] = atoi(genbuf);
			}
			qsort(cpucard, 5, sizeof (char), forq);
			for (i = 0; i < 5; i++)
				show_card(1, cpucard[i], i);
#endif
			i = gp_win();

			if (i < 0) {
				saveValue(currentuser.userid, MONEY_NAME,
					  num * 2, MAX_MONEY_NUM);
				
				sprintf(title, "%s����Ĳ�(���)(Ӯ)", currentuser.userid);
				sprintf(buf, "%s�����Ӯ��%d����ٸ��", currentuser.userid, num);
				millionairesrec(title, buf, "�Ĳ����");
				
				sprintf(genbuf,
					"��!�ð��!!! ��׬ %d Ԫ...  :D", num);
				prints("%s", genbuf);
				if (cont > 0)
					sprintf(genbuf,
						"��ʤ %d ��, Ӯ�� %d Ԫ",
						cont + 1, num);
				else
					sprintf(genbuf, "Ӯ�� %d Ԫ", num);
				num = (num > 50000 ? 100000 : num * 2);
				gw = 1;
			} else if (i > 1000) {
				switch (i) {
				case 1001:
					doub = 15;
					break;
				case 1002:
					doub = 10;
					break;
				case 1003:
					doub = 5;
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME,
					  num * 2 * doub, MAX_MONEY_NUM);

				sprintf(title, "%s����Ĳ�(���)(Ӯ)", currentuser.userid);
				sprintf(buf, "%s�����Ӯ��%d����ٸ��", currentuser.userid, num * 2 * doub - num);
				millionairesrec(title, buf, "�Ĳ����");

				sprintf(genbuf, "��!�ð��!!!��׬ %d Ԫ...  :D",
					num * 2 * doub - num);
				prints("%s", genbuf);
				if (cont > 0)
					sprintf(genbuf,
						"��ʤ %d ��, Ӯ�� %d Ԫ",
						cont + 1, num * doub);
				else
					sprintf(genbuf, "Ӯ�� %d Ԫ",
						num * doub);
				num = (num > 5000 ? 10000 : num * 2 * doub);
				gw = 1;
				num = (num >= 10000 ? 10000 : num);
			} else {
				prints("����...:~~~");
				
				sprintf(title, "%s����Ĳ�(���)(��)", currentuser.userid);
				sprintf(buf, "%s���������%d����ٸ��", currentuser.userid, num);
				millionairesrec(title, buf, "�Ĳ����");

				if (cont > 1)
					sprintf(genbuf,
						"��ֹ %d ��ʤ, ���� %d Ԫ",
						cont, num);
				else
					sprintf(genbuf, "���� %d Ԫ", num);
				cont = 0;
				num = 0;
				pressanykey();
			}

			if (gw == 1) {
				gw = 0;
				getdata(21, 0, "��Ҫ�ѽ������ѹע�� (y/n)?",
					ans, 2, DOECHO, YEA);
				if (ans[0] == 'y' || ans[0] == 'Y') {
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);	/* added by soff */
					cont++;
				} else {
					cont = 0;
					num = 0;
				}
			}
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
}
static /*�Ĳ�--���--��ʾ�˿���*/ 
void show_card(isDealer, c, x)
int isDealer, c, x;
{
	int beginL;
	char *suit[4] = { "��", "��", "��", "��" };
	char *num[13] = { "��", "��", "��", "��", "��", "��", "��",
		"��", "��", "��", "10", "��", "��"
	};

	beginL = (isDealer) ? 4 : 12;
	move(beginL, x * 4);
	outs("�q�������r");
	move(beginL + 1, x * 4);
	prints("��%2s    ��", num[c % 13]);
	move(beginL + 2, x * 4);
	prints("��%2s    ��", suit[c / 13]);	/* ��������ڽܿ� */
#ifdef GP_DEBUG
	move(beginL + 3, x * 4);
	prints("��%2d    ��", c);	/* �е㲻ͬ�!! */
#else
	move(beginL + 3, x * 4);
	outs("��      ��");	/* �е㲻ͬ�!! */
#endif
	move(beginL + 4, x * 4);
	outs("��      ��");
	move(beginL + 5, x * 4);
	outs("��      ��");
	move(beginL + 6, x * 4);
	outs("�t�������s");
}

static/*�Ĳ�--���*/
void money_cpu()
{
	char hold[5];
	int i, j;
	char p[13], q[5], r[4];
	char a[5], b[5];

	for (i = 0; i < 5; i++) {
		cpucard[i] = card[now++];
		hold[i] = 0;
	}
	qsort(cpucard, 5, sizeof (char), (void *) forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);

	money_suoha_tran(a, b, cpucard);
	money_suoha_check(p, q, r, cpucard);

	for (i = 0; i < 13; i++)
		if (p[i] > 1)
			for (j = 0; j < 5; j++)
				if (i == cpucard[j] % 13)
					hold[j] = -1;

	for (i = 0; i < 5; i++) {
		if (a[i] == 13 || a[i] == 1)
			hold[i] = -1;
		move(8, i * 4 + 2);
		outs(hold[i] < 0 ? "��" : "  ");
		move(9, i * 4 + 2);
		outs(hold[i] < 0 ? "��" : "  ");
	}
	move(11, 25);
	prints("\033[44;37m���Ի���ǰ...\033[40m");
	pressanykey();
	move(11, 0);
	clrtoeol();

	for (i = 0; i < 5; i++)
		if (!hold[i])
			cpucard[i] = card[now++];
	qsort(cpucard, 5, sizeof (char), (void *) forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);
}

static/*�Ĳ�-���*/
int gp_win()
{
	int my, cpu, ret = 0;
	char myx, myy, cpux, cpuy;

	my = complex(mycard, &myx, &myy);
	cpu = complex(cpucard, &cpux, &cpuy);
	show_style(my, cpu);

	if (my != cpu)
		ret = my - cpu;
	else if (myx == 1 && cpux != 1)
		ret = -1;
	else if (myx != 1 && cpux == 1)
		ret = 1;
	else if (myx != cpux)
		ret = cpux - myx;
	else if (myy != cpuy)
		ret = cpuy - myy;

	if (ret < 0)
		switch (my) {
		case 1:
			ret = 1001;
			break;
		case 2:
			ret = 1002;
			break;
		case 3:
			ret = 1003;
			break;
		}

	return ret;
}

//ͬ��˳����֦������ͬ����˳�����������ߡ��ߡ�һֻ
static
    int
complex(cc, x, y)
char *cc, *x, *y;
{
	char p[13], q[5], r[4];
	char a[5], b[5], c[5], d[5];
	int i, j, k;

	money_suoha_tran(a, b, cc);
	money_suoha_check(p, q, r, cc);

	/* ͬ��˳ */
	if ((a[0] == a[1] - 1 && a[1] == a[2] - 1 && a[2] == a[3] - 1
	     && a[3] == a[4] - 1) && (b[0] == b[1] && b[1] == b[2]
				      && b[2] == b[3] && b[3] == b[4])) {
		*x = a[4];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 2 && a[1] == 3 && a[2] == 4 && a[3] == 5 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = a[3];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 10 && a[1] == 11 && a[2] == 12 && a[3] == 13 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = 1;
		*y = b[4];
		return 1;
	}
	/*��֦  */
	if (q[4] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 4)
				*x = i ? i : 13;
		return 2;
	}
	/* ��« */
	if (q[3] == 1 && q[2] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 3)
				*x = i ? i : 13;
		return 3;
	}
	/* ͬ�� */
	for (i = 0; i < 4; i++)
		if (r[i] == 5) {
			*x = i;
			return 4;
		}
	/* ˳�� */
	memcpy(c, a, 5);
	memcpy(d, b, 5);
	for (i = 0; i < 4; i++)
		for (j = i; j < 5; j++)
			if (c[i] > c[j]) {
				k = c[i];
				c[i] = c[j];
				c[j] = k;
				k = d[i];
				d[i] = d[j];
				d[j] = k;
			}
	if (10 == c[1] && c[1] == c[2] - 1 && c[2] == c[3] - 1
	    && c[3] == c[4] - 1 && c[0] == 1) {
		*x = 1;
		*y = d[0];
		return 5;
	}
	if (c[0] == c[1] - 1 && c[1] == c[2] - 1 && c[2] == c[3] - 1
	    && c[3] == c[4] - 1) {
		*x = c[4];
		*y = d[4];
		return 5;
	}
	/* ���� */
	if (q[3] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 3) {
				*x = i ? i : 13;
				return 6;
			}
	/* ���� */
	if (q[2] == 2) {
		for (*x = 0, i = 0; i < 13; i++)
			if (p[i] == 2) {
				if ((i > 1 ? i : i + 13) > (*x == 1 ? 14 : *x)) {
					*x = i ? i : 13;
					*y = 0;
					for (j = 0; j < 5; j++)
						if (a[j] == i && b[j] > *y)
							*y = b[j];
				}
			}
		return 7;
	}
	/* �� */
	if (q[2] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 2) {
				*x = i ? i : 13;
				*y = 0;
				for (j = 0; j < 5; j++)
					if (a[j] == i && b[j] > *y)
						*y = b[j];
				return 8;
			}
	/* һ�� */
	*x = 0;
	*y = 0;
	for (i = 0; i < 5; i++)
		if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1) {
			*x = a[i];
			*y = b[i];
		}
	return 9;
}

/* a �ǵ��� .. b �ǻ�ɫ */
static void/*�Ĳ�--���*/
money_suoha_tran(a, b, c)
char *a, *b, *c;
{
	int i;
	for (i = 0; i < 5; i++) {
		a[i] = c[i] % 13;
		if (!a[i])
			a[i] = 13;
	}

	for (i = 0; i < 5; i++)
		b[i] = c[i] / 13;
}

static void/*�Ĳ�--���*/
money_suoha_check(p, q, r, cc)
char *p, *q, *r, *cc;
{
	int i;

	for (i = 0; i < 13; i++)
		p[i] = 0;
	for (i = 0; i < 5; i++)
		q[i] = 0;
	for (i = 0; i < 4; i++)
		r[i] = 0;

	for (i = 0; i < 5; i++)
		p[cc[i] % 13]++;

	for (i = 0; i < 13; i++)
		q[(int) p[i]]++;

	for (i = 0; i < 5; i++)
		r[cc[i] / 13]++;
}

//ͬ��˳����֦������ͬ����˳�����������ߡ��ߡ�һֻ
static void/*�Ĳ�--���*/
show_style(my, cpu)
char my, cpu;
{
	char *style[9] = { "ͬ��˳", "��֦", "��«", "ͬ��", "˳��",
		"����", "����", "����", "һ��"
	};
	move(5, 26);
	prints("\033[41;37;1m%s\033[m", style[cpu - 1]);
	move(15, 26);
	prints("\033[41;37;1m%s\033[m", style[my - 1]);
	sprintf(sty, "�ҵ���\033[44;1m%s\033[m..���Ե���\033[44;1m%s\033[m",
		style[my - 1], style[cpu - 1]);
}

static void/*�Ĳ�--����*/
russian_gun()
{

	int i;
	int line;
	int first;
	char uident[IDLEN + 1];
	char title[STRLEN], buf[256];
	
	clear();
	money_show_stat("����ٸ�ĳ�����");
	////slowaction
	if (currentuser.stay < 86400) {
		move(7, 4);
		prints
		    ("С������ƴʲô���������ǼҴ�������\n�ԣ�����һ�����ٸ�����ǳ�");
		pressanykey();
		return;
	}
	//slowaction
	
	move(6, 4);
	if (!loadValue(currentuser.userid, "invitation", 1)) {
		prints
		    ("ֵ���������´���������Σ�˵����������û�����ֶķ������߰ɡ���");
		pressanykey();
	} else {
		saveValue(currentuser.userid, "invitation", -1, 1);
		whoTakeCharge(3, uident);
		prints
		    ("ֵ�ྭ������ݹ������뺯�������¿��˿�����˵������������������");
		pressanykey();
		clear();
		money_show_stat("����ٸ�ĳ�����");
		move(4, 4);
		prints
		    ("������һ�䲻������ң��ܾ������ÿ��¡��������ƺ���Ѫ�ȵ�ζ��...");
		move(6, 4);
		prints
		    ("%s��������ǰ��΢Ц�������������Լ�������е�������������",
		     uident);
		pressanykey();
		move(8, 4);
		prints("һ��������ǹ�ӵ�������...һ��������������������ǰ...");
		move(10, 4);
		if (askyn("��֪������˹���̶ĵĹ�����", NA, NA) == NA) {
			move(12, 4);
			prints
			    ("%s̾�˿�����˵���������ˣ�����������ô���¶���֪�������߰ɣ���",
			     uident);
			pressanykey();
			return;
		}
		move(12, 4);
		if (askyn
		    ("�ã��ǾͿ�ʼ�ɣ�ף����ˡ����ǿͣ���Ҫ������", NA,
		     NA) == YEA) {
			first = 1;
		} else {
			first = 0;
		}
		clear();
		money_show_stat("����ٸ�ĳ�����");
		set_safe_record();
		currentuser.dietime = currentuser.stay + 4444 * 60;
		substitute_record(PASSFILE,
				  &currentuser, sizeof (currentuser), usernum);
		for (i = 0, line = 6; i < 6; i += 2) {
			srandom(time(0));
			move(line++, 4);
			if (first) {
				prints
				    ("������������ǹ����׼�Լ���̫��Ѩ��ҧҧ���۶��˰��...");
			} else {
				prints
				    ("������������������ǹ����׼�Լ���̫��Ѩ���۶��˰��...");
			}
			pressanykey();
			if (random() % (6 - i)) {
				move(line++, 4);
				if (first) {
					prints
					    ("\033[1;33m���գ�\033[mһ��������㾪��δ��֮�෢���Լ�������...");
				} else {
					prints
					    ("\033[1;33m���գ�\033[mһ��������������Ӻ�������...");
				}
				move(line++, 4);
				if (5 - i == 1 && first) {
					if (first) {
						move(line++, 4);
						prints
						    ("�������Ӿ����Ĳ����ţ��ð����������%s��",
						     uident);
						move(line++, 4);
						prints
						    ("\033[1;31m�飡һ�����죬��������Ѫ������...\033[m");
						pressanykey();
						move(line++, 4);
						prints
						    ("%s���˲�����ð�̵���ǹ���ַŽ��˿ڴ��",
						     uident);
						break;
					} else {
						prints
						    ("�������ӵ������Ц�ţ���ǹ�ڶ�׼����...");
						prints
						    ("\033[1;31m�飡һ�����죬��ֻ������ʶ˲��ģ��...\033[m");
						pressanykey();
						Q_Goodbye();
					}
				}

				if (first) {
					prints("�����ֵ���������...");
				} else {
					prints
					    ("�����ֵ�����...�������\033[5;31m����\033[m��������...");
				}
				pressanykey();
				if (random() % (5 - i)) {
					move(line++, 4);
					if (first) {
						prints
						    ("\033[1;33m���գ�\033[mһ��������������Ӻ�������...");
					} else {
						prints
						    ("\033[1;33m���գ�\033[mһ��������㾪��δ��֮�෢���Լ�������...");
					}
				} else {
					move(line++, 4);
					if (first) {
						prints
						    ("\033[1;31m�飡һ�����죬��������Ѫ������...\033[m");
						break;
					} else {
						prints
						    ("\033[1;31m�飡һ�����죬��ֻ������ʶ˲��ģ��...\033[m");
						pressanykey();
						Q_Goodbye();
					}
				}
			} else {
				move(line++, 4);
				if (first) {
					prints
					    ("\033[1;31m�飡һ�����죬��ֻ������ʶ˲��ģ��...\033[m");
					pressanykey();
					Q_Goodbye();
				} else {
					prints
					    ("\033[1;31m�飡һ�����죬��������Ѫ������...\033[m");
					break;
				}
			}
		}
		move(line++, 4);
		set_safe_record();
		currentuser.dietime = 0;
		substitute_record(PASSFILE,
				  &currentuser, sizeof (currentuser), usernum);
		prints
		    ("һ�ж�������...��һ��Ҳ��Ը������ֲ��ĵط���������õ���200000 ����ٸ�ҡ�");
		saveValue(currentuser.userid, MONEY_NAME,200000,
			  MAX_MONEY_NUM);
		
		sprintf(title, "%s����Ĳ�(����)", currentuser.userid);
		sprintf(buf, "%s������Ӯ��%d����ٸ��", currentuser.userid, 200000);
		millionairesrec(title, buf, "�Ĳ�����");

		pressanykey();
	}
}

static void
policereport(char *str)
{
	FILE *se;
	char fname[STRLEN], title[STRLEN];
	
	sprintf(fname, "bbstmpfs/tmp/police.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", "����ƪ�����ɱ���ٸ�����Զ�����ϵͳ����\n\n");
		fprintf(se, "%s", str);
		fclose(se);
		sprintf(title,"[����]%s", str);
		postfile(fname, "Police", title, 2);
		unlink(fname);
	}
}


static int/*����*/
money_cop()
{
	int ch;
	int quit = 0;
	char uident[IDLEN + 1];
	char buf[200], title[STRLEN];
	int robTimes;
	int seized;
	int die = 0;
	int id;
	int escTime;
	int money=0;

	while (!quit) {
		clear();
		nomoney_show_stat("����ٸ����");
		move(8, 16);
		prints("������ά���ΰ���");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m ѡ�� \033[1;46m [1]���� [2]���� [3]ͨ���� [4]�̾��� [5]�𳤰칫�� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			nomoney_show_stat("����ٸ����Ӵ���");
			move(4, 4);
			prints("������������ٻ�͵�ԣ���������κη��������˵����������򾯷����档\n    ��ȷ�ٱ��н����̰������ܷ�");
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if(money<5000)
			{
				break;
				return 0;
			}
			move(6, 4);
			prints("�������������������þ��棡");
			move(7, 4);
			usercomplete("�ٱ�˭��", uident);
			move(8, 4);
			if (uident[0] == '\0')
				break;
			if (!getuser(uident)) {
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				prints("�˶����ˣ�����Ҳû�취...");
				//saveValue(currentuser.userid, MONEY_NAME,-2000, MAX_MONEY_NUM);
				pressreturn();
				break;
			}
			if (loadValue(uident, "freeTime", 2000000000) > 0) {
				prints("������Ѿ����������ˡ�");
				//saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			if (loadValue(uident, "rob", 50) == 0) {
				prints
				    ("���������ܰ��ְ����㲻Ҫ�̰�����Ŷ��");
				saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			if (seek_in_file(DIR_MC "criminals_list", uident)) {
				prints
				    ("�����Ѿ�������ͨ���ˣ�����������Ȼ�����ʾ��л��");
				saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			getdata(8, 4, "�������飺", genbuf, 40, DOECHO, YEA);
			if (genbuf[0] == '\0') 
				break;
			move(9, 4);
			if (askyn("\033[1;33m���򾯷��ṩ��������Ϣ��ʵ��\033[0m", NA, NA) == NA) 
				break;
			saveValue(currentuser.userid, MONEY_NAME, +2000, MAX_MONEY_NUM);
			strcpy(buf, uident);
			strcat(buf, "\t");
			strcat(buf, genbuf);
			addtofile(DIR_MC "criminals_list", buf);
			move(10, 4);
			prints
			    ("�����ǳ���л���ṩ�����������ǽ����������ư���");
			pressanykey();
			sprintf(buf, "ID: %s\n����: %s", uident, genbuf);
			sprintf(genbuf, "%s����",currentuser.userid);
			millionairesrec(genbuf, buf, "");
			break;
		case '2':
			clear();
			nomoney_show_stat("����ٸ����Ӵ���");
			move(4, 4);
			prints("̹�״ӿ����ܴ��ϡ�");
			move(5, 4);
			prints("���������Լ�������,������һ��Ĵ�����");
			move(7, 4);
			robTimes = loadValue(currentuser.userid, "rob", 50);
			if (robTimes == 0) {
				prints("���в�����û����������...");
				pressanykey();
				break;
			}
			if (time(0) <12*3600 + loadValue(currentuser.userid, "last_rob", 2000000000)) {
				prints("��ô������������");
				pressanykey();
				break;
			}
			sprintf(genbuf,
				"\033[1;31m�����ʦ������,��������㽫������%d��������Ҫ������?\033[0m",
				robTimes / 2 + 1);
			move(8, 4);
			if (askyn(genbuf, NA, NA) == YEA) {
				move(9, 4);
				if (loadValue(currentuser.userid, "freeTime", 2000000000) > 0) {
					prints
					    ("���Ѿ�������ˣ�������Ҳ�������ˡ�");
					pressanykey();
					Q_Goodbye();
				}
				prints("��������,�����ü����úø���ɣ�");
				saveValue(currentuser.userid, "freeTime",
					  time(0) + 86400 * (robTimes / 2 + 1), 2000000000);
				saveValue(currentuser.userid, "rob", -robTimes, 50);
				del_from_file(DIR_MC "criminals_list", currentuser.userid);
				pressanykey();
				Q_Goodbye();
			} else {
				move(9, 4);
				prints("����˳�һ���㲻��ʮ�塣����Ϊ֪�ɣ�");
				pressanykey();
			}
			break;
		case '3':
			clear();
			move(1, 0);
			prints("����ٸ����ǰͨ���ķ���������:");
			listfilecontent(DIR_MC "criminals_list");
			pressanykey();
			break;
		case '4':
			clear();
			nomoney_show_stat("�̾���");
			move(6, 4);
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if (seek_in_file(DIR_MC "mingren", currentuser.userid))
			{
				move(5, 4);
				prints("    \033[1;32m  ��Ҫ����\033[m");
				pressanykey();
				break;
			}
			if (!seek_in_file(DIR_MC "policemen", currentuser.userid)||money<5000) {
				prints
				    ("���ﲻ�ǹ�԰�����µ��Ӵ���ȥ,�𵽴��Ҵ���\n");
                		prints
				    ("����Ǯ��ȥץ�ˣ���������ҽҩ�Ѷ�û��\n");
				pressanykey();
				break;
			}
			usercomplete("����ж���Ŀ�그����������:", uident);
			move(7, 4);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			if(ifinprison(lookupuser.userid))
			{prints("���Ѿ��ڼ����ˣ���Ҫץ��ȥ���������");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				prints("�˶����ˣ�����Ҳû�취...");
				pressreturn();
				break;
			}
			if (loadValue(uident, "freeTime", 2000000000) > 0) {
				prints("������Ѿ����������ˡ�");
				pressanykey();
				break;
			}
			if (time(0) < 5*60 + loadValue(currentuser.userid, "last_catch", 2000000000)) {
				prints("ץ�˲�����ô������");
				pressanykey();
				break;
			}
			robTimes = loadValue(uident, "rob", 50);
			if (robTimes == 0) {
				prints("���������ܰ��ְ����᲻���Ǹ����?\n");
                		prints("������������ְȨ����������5000�ľ�����ʧ��");
                		saveValue(currentuser.userid, MONEY_NAME, -5000, MAX_MONEY_NUM);
                		saveValue(uident, MONEY_NAME, +5000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			escTime = loadValue(uident, "escTime", 2000000000);
			if (escTime > 0 && time(0) < escTime + 3600) {
				prints
				    ("�÷��������˸ո�����,һʱ�������»��Ҳ�����");
				pressanykey();
				break;
			}
			move(8, 4);
			if (askyn("׼��������?", NA, NA) == YEA) {
				saveValue(currentuser.userid, "last_catch", -2000000000, 2000000000);
				saveValue(currentuser.userid, "last_catch", time(0), 2000000000);
				move(10, 4);
				prints
				    ("\033[1;33m���������ṩ����Ϣ,�������ҵ���%s����ĵط���\033[0m",
				     uident);
				move(11, 4);
				seized = 0;
				srandom(time(0));
				if (askyn("\033[5;31mҪ���Ŷ���ô?\033[0m", NA, NA) == YEA) {
					move(12, 4);
					prints
					    ("\033[1;31m��γ���ǹ��һ�Ž����߿������˽�ȥ�������������죡��\033[0m");
					move(13, 4);
					if (random() % 10 == 0) {
						prints
						    ("\033[1;32m�������һ�ˣ������Ǵ򿪵ġ�����%s�ո��������ӡ�\033[0m",
						     uident);
						move(14, 4);
						prints
						    ("��ֻ�ð��ն�������õĻ��ᰡ��");
						saveValue(uident, "escTime", -2000000000, 2000000000);
						saveValue(uident, "escTime", time(0), 2000000000);
						pressanykey();
						sprintf(buf,"%s����",uident);
						policereport(buf);
						sprintf(title, "%s����ץ��", currentuser.userid);
						millionairesrec(title, buf, "����");
						break;
					} else {
						if (robTimes < 3 && random() % 10) {
							prints("\033[1;32m%sһ�������ʱ��ɵ��,�ԹԾ�����˫�֡�\033[0m",
							     uident);
                           				sprintf(genbuf,
									"����ٸ�����ڽ����ץ���ж���ץ��һ���º�%s�ķ�ͽ\n����͸¶ץ�����̷ǳ�˳��\n\n"
									 "����ϣ��������������Ϊ�䣬\n ��վ����߶����ﾯ��ְԱΪ����� ", uident);
                                   		deliverreport("[����]����ٸ�����ܻ�һ����ͽ",genbuf);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(14, 4);
							seized = 1;
						} else if (robTimes >= 3 && robTimes < 6 && random() % 5) {
							prints("\033[1;32m%sһ�������Ҫ�������ܣ����������ֿ죬һǹ������С�ȡ�\033[0m",
							     uident);
							sprintf(genbuf,
									"����ٸ�����ڽ����ץ���ж���ץ��һ���º�%s�ķ�ͽ\n����͸¶�������뾯���ǹս�и���\n\n"
									 "����ϣ��������Ӳ�Ҫ�ܲ���\n ������ɲ���Ҫ������ ", uident);
	                               		deliverreport("[����]����ٸ�����ܻ�һ����ͽ",genbuf);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(14, 4);
							seized = 1;
						} else if (robTimes >= 6 && robTimes < 8 && random() % 3) {
							prints("\033[1;32m%s�������˹���������������ǹ��ֻ�ú���Ť��һ��...\033[0m",
							     uident);
							pressanykey();
							move(14, 4);
							prints("\033[1;32m����һ���������������Ʒ���%s��������Ҳ�۵ù�Ǻ������ҧ��һ�ڡ�\033[0m",
							     uident);
							sprintf(genbuf,
								"����ٸ�����ڽ����ץ���ж���ץ��һ���º�%s�ķ�ͽ\n����͸¶�о�Ա��ǹս�и���\n\n"
								 "����ϣ��������Ӳ�Ҫ�ܲ���\n ������ɲ���Ҫ������ ", uident);
                         			      deliverreport("[����]����ٸ�����ܻ�һ����ͽ",genbuf);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(15, 4);
							seized = 1;
						} else if (robTimes >= 8 && random() % 2) {
							prints("\033[5;32mԭ��%sҲ��ǹ������ͬʱ��׼�˶Է���\033[0m",
							     uident);
							pressanykey();
							move(14, 4);
							prints("\033[1;35mǹ�������%sʹ�����ס��������Ѫֱ�����㰲Ȼ�������Ұ���\033[0m",
							     uident);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(15, 4);
							seized = 1;
						}
						if (seized) {
							prints("�㽫%sѺ���˾���,�������������%d��������������һ����",
							     uident, robTimes);
							saveValue(uident, "rob", -robTimes, 50);
							saveValue(uident, "freeTime", time(0) + 86400 * robTimes, 2000000000);
							sprintf(genbuf,
								"�㱻����ٸ����ץ�񣬲�����%d��ļ����",
								robTimes);
							mail_buf_slow(uident, "�㱻�������", genbuf, "BMY_FBI");
							del_from_file(DIR_MC "criminals_list", uident);
							sprintf(buf, "%s\t%d", uident, robTimes);
							addtofile(DIR_MC "imprison_list", buf);
							pressanykey();
							sprintf(buf,"ץ��%s�������%d��", uident, robTimes);
							policereport(buf);
							sprintf(title, "%s����ץ��", currentuser.userid);
							millionairesrec(title, buf, "����");
							break;
						} else {
							saveValue(uident, "escTime", -2000000000, 2000000000);
							saveValue(uident, "escTime", time(0), 2000000000);
						}
						if (random() % 20) {
							prints
							    ("\033[5;32mԭ��%sҲ��ǹ������ͬʱ��׼�˶Է���\033[0m",
							     uident);
							move(14, 4);
							if (askyn("\033[1;31m�Ƿ������ܣ�", NA, NA) == YEA) {
								move(15, 4);
								if (random() %3) {
									prints("��һ���������ӵ����ŷ����������ŷɹ���");
									move(16, 4);
									prints("%s�û������ˣ��㲻֪���Ǹð��ջ������ҡ�",
									     uident);
									pressanykey();
									sprintf(buf,"%s����", uident);
									policereport(buf);
									sprintf(title, "%s����ץ��", currentuser.userid);
									millionairesrec(title, buf, "����");
									break;
								} else {
									prints("�����ܣ������Ѿ��������ˡ���ֻ�����ؿ�һ����Ѫ���˳���...");
									move(16,4);
									//saveValue(currentuser.userid, MONEY_NAME, 50000, MAX_MONEY_NUM);
									prints("\033[1;31m��׳�������ˡ�\033[0m");
									die = 1;
								}
							}
							else {
								move(15, 4);
								prints("\033[1;31m��·�������ʤ���������ԥ�Ŀ�ǹ�ˣ�\033[0m");
								move(16, 4);
								if (random() % 3) {
									prints("\033[1;35mǹ�������%s������ͷ��������������\033[0m",
									     uident);
									move(17, 4);
									prints("��ݺݵ�����һ��%s��ʬ�壬ͬʱ�������ҽ������ˡ�",
									     uident);								
 									//saveValue(currentuser.userid, MONEY_NAME, 50000, MAX_MONEY_NUM);
									saveValue(uident, "rob", -robTimes/2, 50);
									lookupuser.dietime = lookupuser.stay + 999*60;
									substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
									sprintf(genbuf,
										"����ٸ�����ڽ����ץ���ж��л���һ���º�%s�ķ�ͽ\n����͸¶�����г�ǹ�ܲ���Ϊ\n\n"
										 "����ϣ��������������Ϊ�䣬\n ��վ����߶����ﾯ��ְԱΪ����� ", uident);
                                   				deliverreport("[����]����ٸ�������һ����ͽ",genbuf);
									mail_buf_slow(uident, "�㱻�������","���ڵֿ�����ץ���Ĺ����У���һǹ����ͷ���������ƶ����б�����","BMY_FBI");
									del_from_file(DIR_MC "criminals_list", uident);
									pressanykey();
									sprintf(buf,"����%s", uident);
									policereport(buf);
									sprintf(title, "%s����ץ��", currentuser.userid);
									millionairesrec(title, buf, "����");
									break;
								} else {
									prints("ǹ���������ֻ�����ؿ�һ����Ѫ���˳���...");
									move(17, 4);
									prints("\033[1;31m��׳�������ˡ�\033[0m");
									die = 1;
								}
							}
						} else {
							prints
							    ("\033[5;32mԭ��%s������ף�һ������������%sֻ���������׺���ͬ���ھ���\033[0m",
							     uident, uident);
							move(14, 4);
							prints
							    ("\033[1;31m��׳�������ˡ�\033[0m");
							die = 1;
							saveValue(uident, "rob", -robTimes / 2,50);
							lookupuser.dietime = lookupuser.stay + 999 * 60;
							substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
							sprintf(genbuf,
									"����ٸ�����ڽ����ץ���ж��л���һ���º�%s�ķ�ͽ\n����͸¶�о�Ա��ǹս���е���������\n\n"
									 "�����ʾһ��ȫ�����ȣ�\n  ", uident);
							deliverreport("[����]����ٸ�������һ����ͽ",genbuf);
							mail_buf_slow(uident,
								 "�㱻�������","���ڵֿ�����ץ���Ĺ����У��������ϵ����ף��뾯��ͬ���ھ���","BMY_FBI");		
							del_from_file(DIR_MC "criminals_list", uident);
						}
						if (die) {
							set_safe_record();
							saveValue(uident, "rob", -robTimes/2, 50);
							currentuser.dietime = currentuser.stay + 999 * 60;
							substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
							set_safe_record();
							sprintf(buf,
								"һ������Ա�ڽ����ץ���ж��в���ѳְ������ٸ�������Ӣ��������ߵľ��⣬"
							"\n�����Ľ��ϳ��ﷸ������û��͸¶��Ա����ʵ����");
							deliverreport("[����]����ٸ����һ������ѳְ", buf);
							pressanykey();
							sprintf(buf,"%s��ץ��%sʱӢ��������", currentuser.userid, uident);
							policereport(buf);
							sprintf(title, "%s����ץ��", currentuser.userid);
							millionairesrec(title, buf, "����");
							Q_Goodbye();
						}
					}
				} else {
					move(12, 4);
					prints
					    ("����������Ȳ�Ҫ��ݾ��ߵĺ�...");
					pressanykey();
				}
			}
			break;
		case '5':
			clear();
			nomoney_show_stat("�𳤰칫��");
			char name[20];
			whoTakeCharge2(8, name);
			whoTakeCharge(8, uident);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("����%s��ס����,˵��:����%s���ں�æ,ûʱ��Ӵ��㡣��",
				     name,uident);
				move(8, 4);
				if (!seek_in_file(DIR_MC "policemen", currentuser.userid) 
					&& !slowclubtest("Police",currentuser.userid)){
				if (askyn("��������뾯����", NA, NA) == YEA) {
						sprintf(genbuf, "%s Ҫ���뾯��", currentuser.userid);
						mail_buf(genbuf, "BMYpolice", genbuf);
						move(14, 4);
						prints("�õģ��һ�֪ͨ�𳤵ġ�");
					}}
				pressanykey();
				break;
			} else {
				move(6, 4);
				prints("��ѡ���������:");
				move(7, 6);
				prints
				    ("1. ������Ա                  2. ��ְ��Ա");
				move(8, 6);
				prints
				    ("3. ��Ա����                  4. �������");
				move(9, 6);
				prints("5. ��ְ                      6. �˳�");
				ch = igetkey();
				switch (ch) {
				case '1':
					move(12, 4);
					usercomplete("����˭Ϊ��Ա��", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("�����ʹ���ߴ���...");
						pressanykey();
						break;
					}
					if (seek_in_file
					    (DIR_MC "policemen", uident)) {
						prints("��ID�Ѿ��Ǿ�Ա�ˡ�");
						pressanykey();
						break;
					}
					if (askyn("ȷ����", NA, NA) == YEA) {
						addtofile(DIR_MC "policemen",
							  uident);
						sprintf(genbuf,
							"%s ������Ϊ����ٸ����Ա",
							currentuser.userid);
						mail_buf
						    ("����ϣ���㲻ηǿ����������������˽������������",
						     uident, genbuf);
						move(14, 4);
						prints("�����ɹ���");
						sprintf(genbuf, "%s��ʹ�������Ȩ��",currentuser.userid);
						sprintf(buf, "����%sΪ����Ա", uident);
						millionairesrec(genbuf, buf, "BMYpolice");
						pressanykey();
					}
					break;
				case '2':
					move(12, 4);
					usercomplete("��ְ��λ��Ա��", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("�����ʹ���ߴ���...");
						pressanykey();
						break;
					}
					if (!seek_in_file
					    (DIR_MC "policemen", uident)) {
						prints
						    ("��ID���Ǳ���ٸ����Ա��");
						pressanykey();
						break;
					}
					if (askyn("ȷ����", NA, NA) == YEA) {
						del_from_file(DIR_MC
							      "policemen",
							      uident);
						sprintf(genbuf,
							"%s �����ı���ٸ����Աְ��",
							currentuser.userid);
						mail_buf
						    ("��л��һֱ�����Ĺ�������ϣ������Ϊ�������Ϊά���ΰ���������",
						     uident, genbuf);
						move(14, 4);
						prints("��ְ�ɹ���");
						sprintf(genbuf, "%s��ʹ�������Ȩ��",currentuser.userid);
						sprintf(buf, "���%s�ľ���Ա���", uident);
						millionairesrec(genbuf, buf, "BMYpolice");
						pressanykey();
					}
					break;
				case '3':
					clear();
					move(1, 0);
					prints("Ŀǰ����ٸ����Ա������");
					listfilecontent(DIR_MC "policemen");
					pressanykey();
					break;
				case '4':
					clear();
					move(1, 0);
					prints("Ŀǰ����ٸ�������ﷸ������");
					move(2, 0);
					prints("�ﷸID\t�������");
					listfilecontent(DIR_MC "imprison_list");
					pressanykey();
					break;
				case '5':
					move(12, 4);
					if (askyn
					    ("�����Ҫ��ְ��", NA, NA) == YEA) {
					/*	del_from_file(MC_BOSS_FILE, "police");
						sprintf(genbuf,
							"%s ������ȥ����ٸ������ְ��",
							currentuser.userid);
						deliverreport(genbuf,
							      "����ٸ�������Ķ���һֱ�����Ĺ�����ʾ��л��ף�Ժ�˳����");
						move(14, 4);
						prints
						    ("�ðɣ���Ȼ�����Ѿ�������Ҳֻ����׼��");
						quit = 1;
						pressanykey();
					*/
						sprintf(genbuf, "%s Ҫ��ȥ����ٸ������ְ��",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
						pressanykey();
					}
					break;
				}
		case '6':
				break;
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}


static int //slowaction 
/*���Ȩ��*/
Allclubtest(char *id)
{  if (slowclubtest("Beggar",id))
        return 1;
    else if (slowclubtest("Rober",id))
         return 1;
        else if (slowclubtest("Police",id))
             return 1;
			  else if (slowclubtest("killer",id))
				   return 1;
              else  return 0;
}

static int  //slowaction
slowclubtest(char *board,char *id)
{
	char buf[256];
	sprintf(buf, "boards/%s/club_users", board);
	return seek_in_file(buf, id);
}

//��Ʊ����
static int 
stop_buy()
{
	FILE *f_fp;
	char fname[125];
	sprintf(fname,"%s/stopbuy", DIR_MC);
	f_fp=fopen(fname,"r");
	if(f_fp!=NULL)
	{
		fclose(f_fp);
		return 1;
	}
		return 0;
}
/*int
mail_buf_slow(char *userid, char *title, char *content, char *sender)
{
        FILE *fp;
        char buf[256], dir[256];
        struct fileheader header;
        int t;
	int now;
        bzero(&header, sizeof (header));
        fh_setowner(&header, sender, 0);
        sprintf(buf, "mail/%c/%s/", mytoupper(userid[0]), userid);
	if (!file_isdir(buf))
		return -1;
	now = time(NULL);
        t = trycreatefile(buf, "M.%d.A", now, 100);
        if (t < 0)
                return -1;
        header.filetime = t;
        strsncpy(header.title, title, sizeof (header.title));
        fp = fopen(buf, "w");
        if (fp == 0)
                return -2;
	fprintf(fp, "%s", content);
        fclose(fp);
        setmailfile(dir, userid, ".DIR");
        append_record(dir, &header, sizeof (header));
        return 0;
}
///slowaction to help bm
*/

//���
static int 
money_marry()
{
	int n, ch, quit = 0;
	size_t filesize=0;
	void *buffer = NULL;
	struct MC_Marry *marryMem,mm;
	char note[3][STRLEN];
	char buf[STRLEN];
	int i,j,k;
	int flag = 1;
	int freshflag = 1;

	if (!file_exist(MC_MARRY_RECORDS)){
		int fd;
		void *ptr = NULL;
		filesize = sizeof(struct MC_Marry);
		bzero(&mm, filesize);
		ptr = &mm;
		mkdir(DIR_MC_MARRY, 0770);
		if ((fd = open(MC_MARRY_RECORDS, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
			return -1;
		write(fd, ptr, filesize);
		close(fd);
		}
	n = get_num_records(MC_MARRY_RECORDS, sizeof(struct MC_Marry));
	if (n < 0)
		return 0;
/*	if(n<100){
		n=100;		//һ������һ����
		truncate(MC_MARRY_RECORDS,100*sizeof(struct MC_Marry));
	}
*/
	filesize = sizeof(struct MC_Marry) * n;
	//������Ϣ
	marryMem = loadData(MC_MARRY_RECORDS, buffer, filesize);
	if (marryMem == (void *) -1)
                return -1;
	//������ֻ���״̬�仯
	marry_refresh(marryMem,n);
	//�鿴�Ƿ����������
	for(j=0; j<n; j++){
		if(marryMem[j].enable == 0) continue;
		if(marryMem[j].status != MAR_COURT) continue;
		if(!strcmp(marryMem[j].bride, currentuser.userid)){
			break;
		}
	}
	//������ܶ�������
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride, currentuser.userid)){
			if(marryMem[i].status == MAR_MARRIED
					|| marryMem[i].status == MAR_MARRYING){
				marryMem[j].status =  MAR_COURT_FAIL;
				marryMem[j].enable = 0;
				sprintf(genbuf, "%s �Ѿ������˱��˵����\n",marryMem[j].bride);
				strcat(genbuf,"\n����ģ������ȵĸ�󡲻����������ȵĹ��ﻹ���е���~~");
				sprintf(buf, "�Բ���%s ���ܽ����������", marryMem[j].bride);
				mail_buf(genbuf, marryMem[j].bridegroom, buf);
				j=n;
			}
		}
	}

	if(j<n){
		money_show_stat("����ٸ����");
		move(5, 4);
		flag = 1;
		sprintf(buf, "�Ҹ����˶������Ƿ���� \033[1;33m%s\033[m ����飿",marryMem[j].bridegroom);
		if (askyn(buf, NA, NA) == NA) {
			move(6, 4);
			prints("�У������Ǹ���~~");
			flag = 0;
			marryMem[j].enable = 0;
		}else{
			move(6, 4);
			prints("*^^*�����ڵȵ���һ��~~");
			marryMem[j].enable = 1;
			marryMem[j].marry_t = time(NULL) + 24*60*60;		//����һ������
			marryMem[j].status = MAR_MARRYING;					
			flag = 1;
		}
					
		move(7, 4);
		prints("��Ҫ����˵Щʲô��[����д3���]");
		bzero(note, sizeof (note));
		for (i = 0; i < 3; i++) {
			getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
			if (note[i][0] == '\0')
				break;
		}
		sprintf(genbuf, "%s %s���������\n",marryMem[j].bride, flag?"����":"�ܾ�");
		if (i > 0) {
			sprintf(buf, "\033[1;33m%s\033[m%s��˵:\n", marryMem[j].bride, flag?"�ߴ��":"����");
			strcat(genbuf,buf);
			for (k = 0; k < i; k++){
				strcat(genbuf,note[k]);
				strcat(genbuf,"\n");
			}
		}
		if(flag)
			strcat(genbuf,"\n��վ��ɵ��������ȥ����ٸ����׼�������!");
		else strcat(genbuf,"\n����ģ������ȵĸ�󡲻����������ȵĹ��ﻹ���е���~~");

		sprintf(buf, "[%s]%s %s���������",flag?"��ϲ":"֪ͨ", marryMem[j].bride, flag?"����":"�ܾ�");
		mail_buf(genbuf, marryMem[j].bridegroom, buf);
		if (flag){
			if (i > 0) {
			sprintf(genbuf, "\033[1;33m%s\033[m�ߴ���˵:\n", marryMem[j].bride);
				for (k = 0; k < i; k++){
					strcat(genbuf,note[k]);
					strcat(genbuf,"\n");
				}
			}
			sprintf(buf,"[����]%s������%s�����",marryMem[j].bride,marryMem[j].bridegroom);
			if (note[0][0] == '\0')
				deliverreport(buf,"\n");
			else
	 			deliverreport(buf, genbuf);
		}
		move(13, 4);
		prints("�����Ѿ�����֪ͨ�˶Է�");
		pressanykey();
		}


	while (!quit) {
		clear();
		money_show_stat("����ٸ����");
		if(freshflag){
			show_welcome(MC_MAEEY_SET,4,22);
			freshflag =0;
		}
		//move(6, 4);
		//prints("��ӭ���߽�������Χ��");
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [1]�μӻ��� [2]��� [3]׼������ [4]��� [5]�ǼǱ� [6]��������� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			freshflag = 1;
			marry_attend(marryMem, n);
			break;
		case '2':
			freshflag = 1;
			marry_court(marryMem, n);
			break;
		case '3':
			freshflag = 1;
			marry_perpare(marryMem, n);
			break;
		case '4':
			freshflag = 1;
			marry_divorce();
			break;
		case '5':
			freshflag = 1;
			marry_recordlist(marryMem, n);
			break;
		case '6':
			freshflag = 1;
			marry_admin(marryMem, n);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	saveData(marryMem, filesize);
	return 0;
}

/*add by macintosh@BMY 2006.10*/
static int 
marry_admin(struct MC_Marry *marryMem, int n)
{
	int offset, ch, quit = 0;
	int count, count2, count3, no=0;
	char uident[IDLEN + 1], uident2[IDLEN + 1], buf[2048], title[STRLEN], ans[8];
	char jhdate[30], lhdate[30], lhz[2048];
	size_t filesize;
	//struct MC_Marry *marryMem;
	struct MC_Marry *mm;
	void *buffer = NULL;
	time_t now_t = time(NULL);
	
	nomoney_show_stat("����ٸ��������칫��");
	whoTakeCharge2(10, uident2);
	whoTakeCharge(10, uident);

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
		&& !seek_in_file(MC_MARRYADMIN_FILE, currentuser.userid) 
		&& strcmp(currentuser.userid, uident)) {
		move(6, 4);
		prints
		  ("����%s��ס����,˵��:��������������æ�Ŵ��齫��ûʱ��Ӵ�!��", uident2);
		pressanykey();
		return 0;
	}
	
	while (!quit) {
		nomoney_show_stat("����ٸ��������칫��");
		move(t_lines - 2, 0);
		prints("\033[1;44m ѡ \033[1;46m [1]��ѯ����״�� [2]������� [3]���ù�����Ա [4]�������֪ͨ��             \n"
			   "\033[1;44m �� \033[1;46m [5]ǿ�ƽ����Լ [Q]�뿪                                               ");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			move(6, 4);
			usercomplete("��˭�������", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			marry_query_records(uident);
			break;
		case '2':
			clear();
			move(4, 4);
			prints("\033[1;31;5m����IDʱ��ע���Сд\033[m");
			getdata(6, 4, "������Ů��ID: ", uident, 13, DOECHO, YEA);
			getdata(7, 4, "�������з�ID: ", uident2, 13, DOECHO, YEA);
			/*
			move(6, 4);
			usercomplete("������Ů��ID: ", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			move(7, 4);
			usercomplete("�������з�ID: ", uident2);
			if (uident2[0] == '\0')
				break;
			if (!searchuser(uident2)) {
				move(8, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			*/
			if (askyn("ȷ����", NA, NA) == YEA){
				if (!file_exist(MC_MARRY_RECORDS_ALL)){	
					clear();
					move(9, 4);
					prints("û���κμ�¼!");		
					pressanykey();
					break;
				}
				n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
				if (n <= 0)
					break;
				filesize = sizeof(struct MC_Marry) * n;
				marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
				if (marryMem == (void *) -1)
					break;
				count = 0;
				for(offset = 0; ;offset++){
					if (offset >= n || offset < 0) 
						break;
					mm = &marryMem[offset];
					if(mm->enable==0) 
						continue;
					if(!mm->bride[0] || !mm->bridegroom[0]) 
						continue;
					if(strcmp(mm->bride, uident))
						continue;
					if(strcmp(mm->bridegroom, uident2))
						continue;
					if(mm->status==MAR_MARRIED){
						sprintf(buf, "���ʱ��Ϊ%s��ȷ����", 
							get_simple_date_str(&mm->marry_t));
						if (askyn(buf, NA, NA) == YEA){	
							mm->status=MAR_DIVORCE;
							mm->divorce_t=time(NULL);
							count++;
							sprintf(jhdate, "%s", get_simple_date_str(&mm->marry_t));
							jhdate[10]=0;
							sprintf(lhdate, "%s", get_simple_date_str(&mm->divorce_t));
							lhdate[10]=0;
							no=offset;
						}
					}
				}
				move(12, 4);
				if (count>0){
					saveData(marryMem, filesize);
					sprintf(title, "%s��%s����ַ�", uident, uident2);
					sprintf(buf,"������Ȼ���̻�������칫������%s��ε��⣬"
						"����%s��Ů������%s���з����İ����Ѿ��ߵ���ͷ��"
						"��ѯ˫������󣬴��̻�������칫�Ҿ�����׼"
						"������飬Ը���˽������˳����\n",
						currentuser.userid, uident, uident2);
					deliverreport(title, buf);
					sprintf(title, "[����]%s��%s���", uident, uident2);
					sprintf(lhz,
							"[0m               [47m                                                [40m \n"
							"               [47m  [41m[1;32m�����������������������[47m  [40m \n"
							"               [47m  [41m��                                        ��[47m  [40m \n"
							"               [47m  [41m��               [37m�� �� ֤                 [32m��[47m  [40m \n"
							"               [47m  [41m��                                        ��[47m  [40m \n"
							"               [47m  [41m��                    [34m[[37m����[34m]�� [37m%5.5d [34m��   [32m��[47m  [40m \n"
							"               [47m  [41m��   [37m��֤��                               [32m��[47m  [40m \n"
							"               [47m  [41m��   [4;37m%-12.12s[0;1;41m��Ů��[4m%-12.12s[0;1;41m���У� [32m��[47m  [40m \n"
							"               [47m  [41m��   [37m������ڣ�[4m%s[0;41m                 [1;32m��[47m  [40m \n"
							"               [47m  [41m��      [37m������飬�������ϱ���ٸ����  [32m��[47m  [40m \n"
							"               [47m  [41m��   [37m�������Ĺ涨��׼��Ǽǣ�������֤�� [32m��[47m  [40m \n"
							"               [47m  [41m��          [37m��֤���� ���̻�������칫�� [32m��[47m  [40m \n"
							"               [47m  [41m��          [37m��֤���� %s           [32m��[47m  [40m \n"
							"               [47m  [41m��                                        ��[47m  [40m \n"
							"               [47m  [41m�����������������������[47m  [40m \n"
							"               [47m                                                [40m \n"
							"                                                                [m\n",
							no, uident, uident2, jhdate, lhdate);
					deliverreport(title, lhz);
					sprintf(title,"��������칫������%s�������ҵ��",
						currentuser.userid);
					millionairesrec(title, buf, "Marriage");
					sprintf(buf,"���̻�������칫��ͬ������%s�����Ҫ��Ը���������˳����\n", uident);
					mail_buf_slow(uident2, "���̻�������칫��ͬ���������Ҫ��", buf,"XJTU-XANET");
					sprintf(buf,"���̻�������칫��ͬ������%s�����Ҫ��Ը���������˳����\n", uident2);
					mail_buf_slow(uident, "���̻�������칫��ͬ���������Ҫ��", buf,"XJTU-XANET");
					prints("��ɲ���!");
					
					//�ٴ�ȷ���ǲ����Ѿ���飬�����Ƿ���ѽ������ɾ��
					n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
					if (n <= 0)
						break;
					filesize = sizeof(struct MC_Marry) * n;
					marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
					if (marryMem == (void *) -1)
						break;
					count2 = 0;
					count3 = 0;
					for(offset = 0; ;offset++){
						if (offset >= n || offset < 0) 
							break;
						mm = &marryMem[offset];
						if(mm->enable==0) 
							continue;
						if(!mm->bride[0] || !mm->bridegroom[0]) 
							continue;
						if(!strcmp(mm->bride, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bridegroom, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bride, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
						if(!strcmp(mm->bridegroom, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
					}
					
					if (count2==0){
						if (seek_in_file(MC_MARRIED_LIST, uident))
							del_from_file(MC_MARRIED_LIST, uident);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident))
							addtofile(MC_MARRIED_LIST, uident);
					}
					if (count3==0){
						if (seek_in_file(MC_MARRIED_LIST, uident2))
							del_from_file(MC_MARRIED_LIST, uident2);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident2))
							addtofile(MC_MARRIED_LIST, uident2);
					}
				} else 
					prints("û���ҵ��κ���ؼ�¼!");
				pressreturn();
			}
			break;

		case '3':
			clear();
			if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)) {
				move(6, 4);
				prints("�ܹܲſ��Բ���Ӵ");
				pressanykey();
				break;
			}

			while (1) {
				clear();
				prints("�趨��������칫����Ա\n");
				count = listfilecontent(MC_MARRYADMIN_FILE);
				if (count)
					getdata(1, 0, "(A)���� (D)ɾ�� (E)�뿪 [E]: ", ans, 7, DOECHO, YEA);
				else
					getdata(1, 0, "(A)����  (E)�뿪[E]: ", ans, 7, DOECHO, YEA);
				if (*ans == 'A' || *ans == 'a') {
					move(1, 0);
					usercomplete("������Ա: ", uident);
					if (*uident != '\0') {
						if (seek_in_file(MC_MARRYADMIN_FILE, uident)) {
							move(2, 0);
							prints("�����ID �Ѿ�����!");
							pressreturn();
							break;
						}
						move(4, 0);
						if (askyn("���Ҫ���ô?", NA, NA) == YEA){
							addtofile(MC_MARRYADMIN_FILE, uident);
							sprintf(title, "%s��ʹ����Ȩ��(����)", currentuser.userid);
							sprintf(buf, "���%sΪ����������Ա", uident);
							millionairesrec(title, buf, "Marriage");
							//deliverreport(titlebuf, repbuf);
							//mail_buf(repbuf, uident, titlebuf);
						}
					}
				} else if ((*ans == 'D' || *ans == 'd') && count) {
					move(1, 0);
					namecomplete("ɾ����Ա: ", uident);
					move(1, 0);
					clrtoeol();
					if (uident[0] != '\0') {
						if (!seek_in_file(MC_MARRYADMIN_FILE, uident)) {
							move(2, 0);
							prints("�����ID ������!");
							pressreturn();
							break;
						}
						move(4, 0);
						if (askyn("���Ҫɾ��ô?", NA, NA)==YEA){
							del_from_file(MC_MARRYADMIN_FILE, uident);
							sprintf(title, "%s��ʹ����Ȩ��(����)", currentuser.userid);
							sprintf(buf, "ȡ��%s�Ļ�������ְ��", uident);
							millionairesrec(title, buf, "Marriage");
							//deliverreport(titlebuf, repbuf);
							//mail_buf(repbuf, uident, titlebuf);
						}
					}
				}  else
					break;
			}
			clear();
			break;	

		case '4':
			clear();
			move(6, 4);
			usercomplete("���������ŷ�ID: ", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			move(7, 4);
			usercomplete("�����������ID: ", uident2);
			if (uident2[0] == '\0')
				break;
			if (!searchuser(uident2)) {
				move(8, 4);
				prints("�����ʹ���ߴ���...");
				pressreturn();
				break;
			}
			
			if (askyn("ȷ����", NA, NA) == YEA){
				sprintf(lhdate, "%s", get_simple_date_str(&now_t));
				lhdate[10]=0;
				sprintf(buf,"�𾴵�%s��\n"
					"����������������������ԭ��%s"
					"����������һ������������"
					"�������Ĺ涨�����ʹ�Ա�"
					"֪ͨ����֮���𾭹�6�ռ���Ϊ�ʹ"
					"���������Ķ�����ٸ���̹�������"
					"��ع涨������6���������𸴣��涨���"
					"�Ʋ��ָ��ʧ�⳥����ԭ����ϵ��"
					"���ڵ�7�գ����ڼ���˳�ӣ���˴�����"
					"�������ڣ���30��Ϊ�ޣ��������йع涨"
					"�о���\n\n%80.80s\n%80.80s\n",
					uident, uident2, " ����ٸ���̻�����������", lhdate);
				mail_buf_slow(uident, "���֪ͨ��", buf, "Marriage");
				
				sprintf(title, "%s��%s������һ����ͥ����", uident2, uident);
				sprintf(buf, 
					"����ٸ�����칫������%s�������"
					"����%s���������֪ͨ�顣", uident2, uident);
				deliverreport(title, buf);
				
				sprintf(title,"��������칫������%s�������֪ͨ��",
					currentuser.userid);
				millionairesrec(title, buf, "Marriage");
				prints("��ɲ���!");
				pressanykey();
			}
			break;

		case '5':
			clear();
			showAt(2, 4, "\033[1;31m�˹�������! \033[m", 0);
			showAt(4, 4, "\033[1;32m����IDʱ��ע���Сд\033[m", 0);
			getdata(6, 4, "������Ů��ID: ", uident, 13, DOECHO, YEA);
			getdata(7, 4, "�������з�ID: ", uident2, 13, DOECHO, YEA);
			if (askyn("ȷ����", NA, NA) == YEA){
				/*if (!file_exist(MC_MARRY_RECORDS_ALL)){	
					clear();
					move(9, 4);
					prints("û���κμ�¼!");		
					pressanykey();
					break;
				}
				n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
				if (n <= 0)
					break;
				filesize = sizeof(struct MC_Marry) * n;
				marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
				if (marryMem == (void *) -1)
					break;
				*/
				count = 0;
				for(offset = 0; ;offset++){
					if (offset >= n || offset < 0) 
						break;
					mm = &marryMem[offset];
					if(mm->enable==0) 
						continue;
					if(!mm->bride[0] || !mm->bridegroom[0]) 
						continue;
					if(strcmp(mm->bride, uident))
						continue;
					if(strcmp(mm->bridegroom, uident2))
						continue;
					sprintf(buf, "����ʱ��Ϊ%s��ȷ����", 
						get_simple_date_str(&mm->court_t));
					if (askyn(buf, NA, NA) == YEA){	
						mm->enable=0;
						mm->status = MAR_COURT_FAIL;
						count++;
					}
					
				}
				move(12, 4);
				if (count>0){
					saveData(marryMem, filesize);
					sprintf(title,"��������칫������%sǿ�ƽ����Լ",
						currentuser.userid);
					sprintf(buf,"ǿ�ƽ��%s��%s�Ļ�Լ",
						uident, uident2);
					millionairesrec(title, buf, "Marriage");
					prints("��ɲ���!");
					
					//�ٴ�ȷ���ǲ����Ѿ���飬�����Ƿ���ѽ������ɾ��
					n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
					if (n <= 0)
						break;
					filesize = sizeof(struct MC_Marry) * n;
					marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
					if (marryMem == (void *) -1)
						break;
					count2 = 0;
					count3 = 0;
					for(offset = 0; ;offset++){
						if (offset >= n || offset < 0) 
							break;
						mm = &marryMem[offset];
						if(mm->enable==0) 
							continue;
						if(!mm->bride[0] || !mm->bridegroom[0]) 
							continue;
						if(!strcmp(mm->bride, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bridegroom, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bride, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
						if(!strcmp(mm->bridegroom, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
					}
					
					if (count2==0){
						if (seek_in_file(MC_MARRIED_LIST, uident))
							del_from_file(MC_MARRIED_LIST, uident);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident))
							addtofile(MC_MARRIED_LIST, uident);
					}
					if (count3==0){
						if (seek_in_file(MC_MARRIED_LIST, uident2))
							del_from_file(MC_MARRIED_LIST, uident2);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident2))
							addtofile(MC_MARRIED_LIST, uident2);
					}
				} else 
					prints("û���ҵ��κ���ؼ�¼!");
				pressreturn();
			}
			break;

		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int 
marry_recordlist(struct MC_Marry *marryMem, int n)
{
	int ch, quit = 0;
	while (!quit) {
		nomoney_show_stat("����ٸ���õ�����");
		move(8, 16);
		prints(" ���ģ��ѻ�ģ�����...ȫ��������أ�����");
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [1]���µǼǱ� [2]״����¼�� [3]���˲�ѯ [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			marry_active_records(marryMem, n);
			break;
		case '2':
			marry_all_records();
			break;
		case '3':
			marry_query_records(currentuser.userid);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

//��ѯ����״��
/*add by macintosh@BMY 2006.10*/
static int 
marry_query_records(char *id)
{
	int i,j;
	char buf[STRLEN];
	int offset;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];
	struct MC_Marry *marryMem;
	void *buffer = NULL;
	size_t filesize;
	int n;

	if (!file_exist(MC_MARRY_RECORDS_ALL)){	
		clear();
		move(6, 4);
		prints("û���κμ�¼!");		
		pressanykey();
		return 0;
	}
	n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
	if (n <= 0)
		return 0;
	filesize = sizeof(struct MC_Marry) * n;
	//������Ϣ
	marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	money_show_stat("����ٸ���õ�����");
	move(5, 0);
	prints("                       \033[1;31m���˻��������ѯ��� (%s)\033[m         ", id);
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","���","����","����","����","��/��/����ʱ��","����","״̬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i���ڿ���ҳ��
		now_t = time(NULL); 
		for(j=0; j<10; j++){
				move(8 + j , 0); 
				clrtoeol(); 
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//ÿ����ʾ���10
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				//continue;
				break;
			}
			mm = &marryMem[offset];
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			if(strcmp(mm->bride, id) 
				&& strcmp(mm->bridegroom, id))
				continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					strcpy(timestr,get_simple_date_str(&mm->court_t));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
					break;
				case MAR_DIVORCE:
					strcpy(timestr,get_simple_date_str(&mm->divorce_t));
					break;
				default:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
			}
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m", 
				offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
				(mm->status ==MAR_MARRYING)?32:37,marry_status[mm->status]);
			prints("%s", buf);
			}
		if ((offset >= n ) && (count <= 0)){
			move(9, 0);
			prints("û���ҵ��κ���ؼ�¼!");
			pressreturn();
			return 0;
		} else {
			getdata(19, 4, "[B]ǰҳ [C]��ҳ [Q]�˳�: [C]", buf, 2, DOECHO, YEA);
			if (toupper(buf[0]) == 'Q')
				return 0;
			if (toupper(buf[0]) == 'B')
				i = (i == 0) ? (i-1) : (i-2);
			else
				if (offset < n)
					i = (i == pages -1) ? (i-1) : i;
				else i--;
		}
	}
	return 1;
}
//״����¼��
//������飬��飬���ʧ��
static int 
marry_all_records()
{
	int i,j;
	char buf[STRLEN];
	int offset;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];
	struct MC_Marry *marryMem;
	void *buffer = NULL;
	size_t filesize;
	int n;

	if (!file_exist(MC_MARRY_RECORDS_ALL)){	
		clear();
		move(6, 4);
		prints("�ף���ôû�м�¼���ѵû���δ�йػ��»����");		
		pressanykey();
		return 0;
	}
	n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
	if (n <= 0)
		return 0;
	filesize = sizeof(struct MC_Marry) * n;
	//������Ϣ
	marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	money_show_stat("����ٸ���õ�����");
	move(5, 4);
	prints("                             \033[1;31m���û���״����¼��\033[m         ");
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","���","����","����","����","��/��/����ʱ��","����","״̬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i���ڿ���ҳ��
		now_t = time(NULL); 
		for(j=0; j<10; j++){
				move(8 + j , 0); 
				clrtoeol(); 
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//ÿ����ʾ���10
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				//continue;
				break;
			}
			mm = &marryMem[offset];
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					strcpy(timestr,get_simple_date_str(&mm->court_t));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
					break;
				case MAR_DIVORCE:
					strcpy(timestr,get_simple_date_str(&mm->divorce_t));
					break;
				default:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
			}
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m", 
				offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
				(mm->status ==MAR_MARRYING)?32:37,marry_status[mm->status]);
			prints("%s", buf);
			//offset++;
		}
		getdata(19, 4, "[B]ǰҳ [C]��ҳ [Q]�˳�: [C]", buf, 2, DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i-1) : (i-2);
		else
			if (offset < n)
				i = (i == pages -1) ? (i-1) : i;
			else i--;
	}
	return 1;
}

//�����ǼǱ�
//���µǼǱ�
static int 
marry_active_records(struct MC_Marry *marryMem, int n)
{
	int i,j;
	char buf[STRLEN];
	int offset=0;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];

	money_show_stat("����ٸ���õ�����");
	move(5, 4);
	prints("                             \033[1;31m���û��µǼǱ�\033[m         ");
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","���","����","����","����","���/����ʱ��","����","״̬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i���ڿ���ҳ��
		now_t = time(NULL);
		count = 0;
		for(j=0;j<10;j++) {
				move(8 + j , 0);
				clrtoeol();
		}
		for(j = 0; count < 10; j++) {	//ÿ����ʾ���10
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				break;
				//continue;
			}
			mm = &marryMem[offset];
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					strcpy(timestr,get_simple_date_str(&mm->court_t));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
					break;
				case MAR_DIVORCE:
					strcpy(timestr,get_simple_date_str(&mm->divorce_t));
					break;
				default:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
			}
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m", 
				offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
				(mm->status ==MAR_MARRYING)?32:37,
				(mm->status ==MAR_MARRYING)?((mm->marry_t > time(NULL))?"�ﱸ��":"������"):(marry_status[mm->status]));
			prints("%s", buf);
			//offset++;
		}
		getdata(19, 4, "[B]ǰҳ [C]��ҳ [Q]�˳�: [C]", buf, 2, DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i-1) : (i-2);
		else
			if (offset < n)
				i = (i == pages -1) ? (i-1) : i;
			else i--;
	}
	return 1;
}

//���������������������
static int 
marry_refresh(struct MC_Marry *marryMem, int n)
{
	int i;
	char buf[400];
	char filetmp[STRLEN];
	char invpath[STRLEN];
	char setpath[STRLEN];
	char visitpath[STRLEN];
	FILE *fp, *fp2;
	struct MC_Marry *mm;
	time_t now_t= time(NULL);

	for(i=0;i<n;i++){
		if(marryMem[i].status==MAR_COURT && (marryMem[i].enable==0||(now_t-marryMem[i].court_t)>7*24*60*60)){
			//���ʧ��
			mm = &marryMem[i];
			mm->status = MAR_COURT_FAIL;
			mm->enable = 0;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//ת���¼
		}
		/*else if(marryMem[i].status==MAR_MARRIED){
			//�ѽ�飬ת���¼�����������Ϊ�˴˴��޸ģ�һ�㲻�����
			mm = &marryMem[i];
			mm->status = MAR_MARRIED;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//ת���¼
			mm->enable = 0;	//�����active��������
			if (!seek_in_file(MC_MARRIED_LIST, mm->bride))
				addtofile(MC_MARRIED_LIST, mm->bride);
			if (!seek_in_file(MC_MARRIED_LIST, mm->bridegroom))
				addtofile(MC_MARRIED_LIST, mm->bridegroom);
		}
		*/
		else if(marryMem[i].status==MAR_MARRYING 
					&& marryMem[i].unused[0]!='d' 
					&& !(marryMem[i].marry_t > now_t)){
			mm = &marryMem[i];
			mm->unused[0]='d';
			sprintf(filetmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
				currentuser.userid, getpid());
			fp = fopen(filetmp,"w");
			if(!fp) continue;
			fprintf(fp,"     \033[1;31m%s\033[m��\033[1;32m%s\033[m�Ļ�����ʽ��ʼ����ӭ��ҹ���\n\n"
					"     �����ǹ�ͬף�����ǰɣ�\n\n",mm->bride,mm->bridegroom);
			fclose(fp);			
			sprintf(buf,"[����]%s��%s�Ļ�����ʽ��ʼ��",mm->bride,mm->bridegroom);
			postfile(filetmp, MC_BOARD, buf , 1);
		}else if(marryMem[i].status==MAR_MARRYING && now_t-marryMem[i].marry_t >4*60*60){
			//����4Сʱ�����
			mm = &marryMem[i];
			mm->status = MAR_MARRIED;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//ת���¼
			mm->enable = 0;	//�����active��������
			if (!seek_in_file(MC_MARRIED_LIST, mm->bride))
				addtofile(MC_MARRIED_LIST, mm->bride);
			if (!seek_in_file(MC_MARRIED_LIST, mm->bridegroom))
				addtofile(MC_MARRIED_LIST, mm->bridegroom);
			sprintf(invpath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
			sprintf(setpath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			sprintf(visitpath,"%s/M.%d.A",DIR_MC_MARRY, mm->visitfile);
			sprintf(filetmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
				currentuser.userid, getpid());
			fp = fopen(filetmp,"w");
			if(!fp) continue;
			fprintf(fp,"     \033[1;31m%s\033[m��\033[1;32m%s\033[m�Ļ��񵽴˽�������л��ҵĹ��٣�"
					"�����ǹ�ͬף�������Ҹ����۵Ļ�����\n\n",mm->bride,mm->bridegroom);
			fprintf(fp,"    \033[1;36m��������λ���������¼��ͳ��\033[m\n\n");
			fprintf(fp,"����ʱ��: %s\n",get_date_str(&mm->marry_t));
			fprintf(fp,"�������: \033[1;31m%d\033[m ����ٸ��\n",mm->giftmoney);
			fprintf(fp,"�����˴�: \033[1;31m%d\033[m\n",mm->visitcount);
			fp2= fopen(visitpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fprintf(fp,"\n\033[1m���: \033[m\n");
			fp2= fopen(invpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fprintf(fp,"\n\n\033[1m���񲼾�: \033[m\n");
			fp2= fopen(setpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fclose(fp);			
			sprintf(buf,"[��ϲ]%s��%s����ѳ�",mm->bride,mm->bridegroom);
			postfile(filetmp, MC_BOARD, buf , 1);
		}
	}
	return 1;
}

static int 
marry_givemoney(struct MC_Marry *mm)
{
	char uident[IDLEN + 1];
//	void *buffer = NULL;
	int i;
	char note[3][STRLEN];
	char buf[STRLEN];
	time_t now_t = time(NULL);
	int num;

	move(4,4);
	if(mm->marry_t > now_t){
			prints("����δ��ʼ,���Ժ�����");
			pressanykey();
			return 0;
	}
	else prints("��������У�����ĺ�ʱ��");

	if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
		move(7 ,4);
		prints("���������Լ�����Ǯ�Ͳ���ͨ�������˰�...");
		pressanykey();
		return 0;
	}

	move(5,4);
	if(now_t%2==1){	//���������һ��Ļ�������
		strncpy(uident,mm->bride,IDLEN);
		prints("����͵�����\033[1;31m%s\033[m������",uident);
	}else{
		strncpy(uident,mm->bridegroom,IDLEN);
		prints("����͵�����\033[1;32m%s\033[m������",uident);
	}

	getdata(6, 4, "ת�ʶ��ٱ���ٸ�ң�[100000]", buf, 10, DOECHO, YEA);
	num = atoi(buf);
	if (buf[0]=='\0') 
		num=100000;
	if (num<100000) {
		move(7, 4);
		prints("�˼��»��ϲ�أ���ô��Ǯ��Ҳ����˼�ó��֣�С�����ߺ�:(");
		pressanykey();
		return 0;
		}
	if (num>MAX_MONEY_NUM) 
		num=MAX_MONEY_NUM;
	move(7, 4);
	snprintf(buf, STRLEN - 1, "ȷ��ת�� %d ����ٸ����", num);
	if (askyn(buf, NA, NA) == NA)
       	return 0;
	if (loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) < num) {
		move(8, 4);
		prints("��������������...���˰���ڴ�...��������Ȼû��Ǯ��");
		pressanykey();
		return 0;
	}

	move(7, 4);
	prints("�л�Ҫ˵��[����д3���]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}

	saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);
	saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
	mm->giftmoney += num;
	sleep(1);

	sprintf(genbuf, "\033[1;32m%s\033[m�������� \033[1;31m%d\033[m ����ٸ�� ������\n\n",currentuser.userid,num);
	if (i > 0) {
		int j;
		sprintf(buf, "����:\n");
		strcat(genbuf,buf);
		for (j = 0; j < i; j++){
			strcat(genbuf,note[j]);
			strcat(genbuf,"\n");
		}
	}

	sprintf(buf, "[�����»�]%s�����ͺ�������", currentuser.userid);
	mail_buf(genbuf,uident, buf);
	sprintf(buf, "[�����»�]%s����%s��%s�»��ϲ", currentuser.userid, mm->bride, mm->bridegroom);
	sprintf(genbuf, "��\033[1;31m���\033[mһ��\n\n��ף�����������ϲ������úϣ���������:)");
	deliverreport(buf, genbuf);
	
	sprintf(buf, "%s�μ�%s��%s�Ļ���(�ͺ��)", currentuser.userid, mm->bride, mm->bridegroom);
	sprintf(genbuf, "%s��%s�ͺ�� (%d����ٸ��)",  currentuser.userid, uident, num);
	millionairesrec(buf, genbuf, "�μӻ���");

	move(14 ,4);
	prints("������ʹ");
	pressanykey();
	return 0;
}

//��������ڱ�
char *get_simple_date_str(time_t *tt)
{
    struct tm *tm;
	static char timestr[200];
	if(tt==0) return "------";
	tm = localtime(tt);
	sprintf(timestr,"%02d/%02d/%02d %02d:%02d",
		tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min);
	//prints(timestr);
	return timestr;
}

//�μӻ���
static int 
marry_attend(struct MC_Marry *marryMem, int n)
{
	int ch, quit = 0;
	int i,j;
	char buf[STRLEN];
	int offset;
	int pages;
	int count;
	struct MC_Marry *mm;
	int index;
	time_t now_t;
	int freshflag=1;
	char uident[IDLEN + 1];
	char visitfile[STRLEN];
	char filepath[STRLEN];
	time_t t;

	money_show_stat("����ٸ����");
	move(5, 4);
	prints("                             \033[1;31m���û���ǼǱ�\033[m         ");
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","���","����","����","����","����ʱ��","����","״̬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i���ڿ���ҳ��
		now_t = time(NULL);
		for(j=0;j<10;j++) {
				move(8 + j , 0);
				clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//ÿ����ʾ���10֧��Ʊ
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				break;
			}
			mm = &marryMem[offset];
			if(mm->status!=MAR_MARRYING) continue;
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			count++;
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m", 
				offset,mm->subject,mm->bride,mm->bridegroom,get_simple_date_str(&mm->marry_t),mm->visitcount,
				(mm->marry_t > now_t)?37:32,(mm->marry_t > now_t)?"�ﱸ��":"������");
			prints("%s", buf);
			//offset++;
		}
		getdata(19, 4, "[B]ǰҳ [C]��ҳ [S]ѡ�� [Q]�˳�: [C]", buf, 2, DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'S')
			break;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i-1) : (i-2);
		else
			i = (i == pages -1) ? (i-1) : i;
	}

	while(1) {
		getdata(t_lines-5, 4, "��ѡ����Ҫ�μӵĻ�����[ENTER����]:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		index = atoi(buf);
		if (index >= 0 && index < n && marryMem[index].status == MAR_MARRYING)
			break;
	}
	mm = &marryMem[index];
	mm->visitcount++;	//���ü�¼
	now_t = time(NULL);
	strncpy(visitfile,DIR_MC_MARRY,STRLEN-1);	
	if(mm->visitfile==0){
		t = trycreatefile(visitfile, "M.%d.A", now_t, 100);
		if (t < 0)
			return -1;
		mm->visitfile = t;
	}else sprintf(visitfile,"%s/M.%d.A",DIR_MC_MARRY, mm->visitfile);
	if(!seek_in_file(visitfile, currentuser.userid))
		addtofile(visitfile, currentuser.userid);

	while (!quit) {
		money_show_stat("����ٸ����");
		if(freshflag){
			sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			show_welcome(filepath,4,22);
			freshflag =0;
		}
		move(4, 10);
		now_t = time(NULL);
		if(mm->marry_t > now_t)
			prints("����δ��ʼ");
		else prints("���������...");
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [1]����� [2]���ʻ� [3]�ͺؿ� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			freshflag = 1;
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				move(5, 4);
				prints("    \033[1;32m  ����Ӱ�������˵����ȥ����ͷ�� \033[m");
				pressanykey();
				break;
			}	
			marry_givemoney(mm);
			break;
		case '2':
			freshflag = 1;
			move(5,4);
			if(mm->marry_t > now_t){
				prints("����δ��ʼ,���Ժ�����");
				pressanykey();
				break;
			}
			if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
				move(7 ,4);
				prints("���������Լ����Ͷ����Ͳ�����ô�鷳�˰�...");
				pressanykey();
				break;
			}
			if(now_t%2==1){	//���������һ��Ļ�������
				strncpy(uident,mm->bride,IDLEN);
				prints("�ʻ����͵�����\033[1;31m%s\033[m������",uident);
			}else{
				strncpy(uident,mm->bridegroom,IDLEN);
				prints("�ʻ����͵�����\033[1;32m%s\033[m������",uident);
			}
			pressanykey();
			if (shop_present(1, "�ʻ�",uident) == 9) {
				sprintf(buf, "[�����»�]%s����%s��%s�»��ϲ", currentuser.userid, mm->bride, mm->bridegroom);
				sprintf(genbuf, "��\033[1;31m�ʻ�\033[mһ��\n\n��ף�����������ϲ������úϣ���������:)");
				deliverreport(buf, genbuf);
			}
			break;
		case '3':
			freshflag = 1;
			move(5,4);
			if(mm->marry_t > now_t){
				prints("����δ��ʼ,���Ժ�����");
				pressanykey();
				break;
			}
			if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
				move(7 ,4);
				prints("���������Լ����Ͷ����Ͳ�����ô�鷳�˰�...");
				pressanykey();
				break;
			}
			if(now_t%2==1){	//���������һ��Ļ�������
				strncpy(uident,mm->bride,IDLEN);
				prints("�ؿ����͵�����\033[1;31m%s\033[m������",uident);
			}else{
				strncpy(uident,mm->bridegroom,IDLEN);
				prints("�ؿ����͵�����\033[1;32m%s\033[m������",uident);
			}
			pressanykey();
			if(shop_present(2, "�ؿ�",uident) == 9) {
				sprintf(buf, "[�����»�]%s����%s��%s�»��ϲ", currentuser.userid, mm->bride, mm->bridegroom);
				sprintf(genbuf, "��\033[1;32m�ؿ�\033[mһ��\n\n��ף�����������ϲ������úϣ���������:)");
				deliverreport(buf, genbuf);
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int PutMarryRecord(struct MC_Marry *marryMem, int n, struct MC_Marry *new_mm)
{
	int i, slot = -1;

	for(i = 0; i < n; i++) {
		if(marryMem[i].enable == 0 && slot == -1)	//�ŵ���һ����λ
			slot = i;
	}
	if(slot >= 0) {
		memcpy(&marryMem[slot], new_mm, sizeof(struct MC_Marry));
	}else{
		append_record(MC_MARRY_RECORDS, new_mm, sizeof(struct MC_Marry));
	}
	return slot;
}

//���
static int 
marry_court(struct MC_Marry *marryMem, int n)
{
	char note[3][STRLEN];
	char buf[STRLEN];
	struct MC_Marry mm;
	int i;
	char uident[IDLEN+2];

	money_show_stat("���õǼǴ�");
	if (seek_in_file(MC_MARRIED_LIST, currentuser.userid)){
		move(5, 4);
		prints("���Ѿ�����˰���С�ĸ����ػ��");
		pressanykey();
		return 0;
	}
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride,currentuser.userid) || !strcmp(marryMem[i].bridegroom,currentuser.userid) ){
			if( marryMem[i].status == MAR_COURT ){
				move(5, 4);
				prints("ι���������Ż��أ���ô��רһ����mm��ô������");
				pressanykey();
				return 0;
			}
			/*
			else if(marryMem[i].status == MAR_MARRIED){
				move(5, 4);
				prints("���Ѿ�����˰���С�ĸ����ػ��");
				pressanykey();
				return 0;
			}
			*/
			else if(marryMem[i].status == MAR_MARRYING){
				move(5, 4);
				prints("��û����������ھ����أ���Ҫ��飬�Դ�û���հ�~~");
				pressanykey();
				return 0;
			}
		}
	}

	move(5,4);
	prints("������ͬ��Ϸ����վ���ᳫ��飬�����ؿ��ǣ���");
	move(6,4);
	if (askyn("���¶�����Ҫ�������", NA, NA) == NA) {
		move(7, 4);
		prints("���������ٵȵȰ�....");
		pressanykey();
		return 0;
	}

	money_show_stat("���õǼǴ�");
	move(5, 4);
	usercomplete("��λmm��ô�Ҹ���", uident);
	if (uident[0] == '\0')
		return 0;
	if(!getuser(uident)) {
		move(6, 4);
		prints("û����ô��mm��....");
              pressanykey();
              return 0;
	}
	if(!strcmp(uident, currentuser.userid)){
		move(6, 4);
		prints("ι�����Ѱɣ�������Ҳ�������Լ���鰡��");
		pressanykey();
		return 0;
	}
	if (seek_in_file(MC_MARRIED_LIST, uident)){
		move(6, 4);
		prints("�˼��Ѿ������ѽ���������߲��õģ�");
		pressanykey();
		return 0;
	}
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride,uident) || !strcmp(marryMem[i].bridegroom,uident) ){
			if( marryMem[i].status == MAR_COURT && !strcmp(marryMem[i].bridegroom,uident)){
				move(5, 4);
				prints("�����İɣ��˼��Ѿ�����������....");
				pressanykey();
				return 0;
			}
			/*
			else if(marryMem[i].status == MAR_MARRIED){
				move(5, 4);
				prints("�˼��Ѿ������ѽ���������߲��õģ�");
				pressanykey();
				return 0;
			}
			*/
			else if(marryMem[i].status == MAR_MARRYING){
				move(5, 4);
				prints("��û����˼�������أ���ʲô�Ұ�~~");
				pressanykey();
				return 0;
			}
		}
	}


	move(7, 4);
	prints("�л����mm˵��[����д3���]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}

	sprintf(genbuf, "         \033[1;31m���\033[m\n\n");
	if (i > 0) {
		int j;
		sprintf(buf, "\033[1;33m%s\033[m�����˵:\n", currentuser.userid);
		strcat(genbuf,buf);
		for (j = 0; j < i; j++){
			strcat(genbuf,note[j]);
			strcat(genbuf,"\n");
		}
	}
	strcat(genbuf,"\n�������: �޸����ɣ��޸�����~~");
	
	move(11, 4);
	sprintf(buf,"���ɲ���С�£�Ҫ�����Ŷ�����¶�������%s�������?",uident);
	if (askyn(buf, YEA, NA) == NA) {
		move(12, 4);
		prints("�����ɺ޽�Ҫ��ͷ����ô��û�������~~");
		pressanykey();
		return 0;
	}

	memset(&mm,0,sizeof(struct MC_Marry));
	mm.enable = 1;
	strcpy(mm.bride ,uident);
	strcpy(mm.bridegroom, currentuser.userid);
	mm.status = MAR_COURT;
	mm.giftmoney = 0;
	mm.attendmen = 0;
	mm.court_t = time(NULL);
	mm.marry_t = 0;
	mm.divorce_t = 0;
	strcpy(mm.subject, "׷����ĳ���");
	mm.setfile = 0;
	mm.invitationfile = 0;

	PutMarryRecord(marryMem, n, &mm);

	sprintf(buf,"[����]%s��%s��������",currentuser.userid,uident);
 	deliverreport(buf, genbuf);
	sprintf(buf, "[��ϲ]%s������������", currentuser.userid);
	mail_buf(genbuf, uident, buf);
	move(13, 4);
	prints("��ϲ��������Ũ���������͵�%s���У��ȴ�����Ϣ��~~",uident);
	pressanykey();
	return 0;
}


char *get_date_str(time_t *tt)
{
    struct tm *tm;
	static char timestr[200];
	tm = localtime(tt);
	sprintf(timestr,"%04d��\033[1;33m%02d\033[m��\033[1;33m%02d\033[m�� \033[1;33m%02d\033[mʱ:\033[1;33m%02d\033[m��",
		tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min);
	//prints(timestr);
	return timestr;
}

static int 
marry_selectday(struct MC_Marry *mm)
{
	int ch, quit = 0;
	time_t now_t = time(NULL);
	//mm->marry_t = now_t;
	if(mm->marry_t < now_t){
		move(5,4);
		prints("�����Ѿ���ʼ...");
		return 0;
	}
	while (!quit) {
		money_show_stat("����ٸ����");
		now_t = time(NULL);
		//������һ���ڣ�10���Ӻ����
		if(mm->marry_t ==0) mm->marry_t = now_t + 600;
		if(mm->marry_t - now_t <600) mm->marry_t = now_t + 600;
		if(mm->marry_t - now_t >365*30*24*60*60 ) mm->marry_t = now_t + 365*30*24*60*60;
		move(6, 4);
		prints("��������:  " );
		prints(get_date_str(&mm->marry_t));
		move(10, 4);
		prints("��������: ab[+-��] cd[+-��] ef[+-ʱ] gh[+-]�� [Q]����");
	
		ch = igetkey();
		switch (ch) {
		case 'a':
		case 'A':	//��
			mm->marry_t += 30*24*60*60;
			break;
		case 'b':
		case 'B':
			mm->marry_t -= 30*24*60*60;
			break;
		case 'c':	//��
		case 'C':
			mm->marry_t += 24*60*60;
			break;
		case 'd':
		case 'D':
			mm->marry_t -= 24*60*60;
			break;
		case 'e':	//ʱ
		case 'E':
			mm->marry_t += 60*60;
			break;
		case 'f':
		case 'F':
			mm->marry_t -= 60*60;
			break;
		case 'g':
		case 'G':
			mm->marry_t += 60;
			break;
		case 'h':
		case 'H':
			mm->marry_t -= 60;
			break;		
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int 
marry_editinvitation(struct MC_Marry *mm)
{
	FILE *oldfp,*newfp;
	char buf[400];
	time_t t;
	char filepath[STRLEN];
	char attach_path[STRLEN];
	char edittmp[STRLEN];
	time_t now_t= time(NULL);

	strncpy(filepath,DIR_MC_MARRY,STRLEN-1);
	if(mm->invitationfile == 0){
		t = trycreatefile(filepath, "M.%d.A", now_t, 100);
		if (t < 0)
			return -1;
		mm->invitationfile = t;
		oldfp = fopen(MC_MAEEY_INVITATION,"r");	//��ʼʹ��Ĭ���ļ�
		if(oldfp){
			newfp = fopen(filepath,"w");
			if(newfp){
				while(!feof(oldfp)){
					if(fgets(buf,sizeof(buf),oldfp) == NULL) 
						break;
					char *s;
					int i;
					while (1) {
						s = strstr(buf, "$bridegroom");
						if (s == 0)
							break;
						for (i = 0; i < 11; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bridegroom); i++)
							s[i] = mm->bridegroom[i];
						}
					while (1) {
						s = strstr(buf, "$bride");
						if (s == 0)
							break;
						for (i = 0; i < 6; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bride); i++)
							s[i] = mm->bride[i];
						}
					while (1) {
						s = strstr(buf, "$marrytime");
						if (s == 0)
							break;
						for (i = 0; i < strlen(get_date_str(&mm->marry_t)); i++)
							s[i] =get_date_str(&mm->marry_t)[i];
						}
					fprintf(newfp,"%s",buf);
				}
				fclose(oldfp);
			}
			fclose(newfp);
		}
	}else 
	sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
	
	if (dashl(filepath) || !dashf(filepath))
				return -1;
	sprintf(edittmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
		currentuser.userid, getpid());
	copyfile_attach(filepath, edittmp);
	if (vedit(edittmp, 0, YEA) < 0) {
		unlink(edittmp);
		clear();
		do_delay(-1);	/* by ylsdd */
		return -1;
	}
	snprintf(attach_path, sizeof (attach_path),
		 PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);
	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, edittmp,
					 currentuser.userid);
	unlink(edittmp);
	return 1;
}


static int 
marry_editset(struct MC_Marry *mm)
{
	FILE *oldfp,*newfp;
	char buf[400];
	time_t t;
	char filepath[STRLEN];
	char attach_path[STRLEN];
	char edittmp[STRLEN];
	time_t now_t= time(NULL);

	strncpy(filepath,DIR_MC_MARRY,STRLEN-1);
	if(mm->setfile == 0){
		t = trycreatefile(filepath, "M.%d.A", now_t, 100);
		if (t < 0)
			return -1;
		mm->setfile = t;
		oldfp = fopen(MC_MAEEY_SET,"r");	//��ʼʹ��Ĭ���ļ�
		if(oldfp){
			newfp = fopen(filepath,"w");
			if(newfp){
				while(!feof(oldfp)){
					if(fgets(buf,sizeof(buf),oldfp) == NULL) 
						break;
					char *s;
					int i;
					while (1) {
						s = strstr(buf, "$bridegroom");
						if (s == 0)
							break;
						for (i = 0; i < 11; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bridegroom); i++)
							s[i] = mm->bridegroom[i];
						}
					while (1) {
						s = strstr(buf, "$bride");
						if (s == 0)
							break;
						for (i = 0; i < 6; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bride); i++)
							s[i] = mm->bride[i];
						}
					while (1) {
						s = strstr(buf, "$marrytime");
						if (s == 0)
							break;
						for (i = 0; i < strlen(get_date_str(&mm->marry_t)); i++)
							s[i] = get_date_str(&mm->marry_t)[i];
						}
					fprintf(newfp,"%s",buf);
					}/*by macintosh 20051203*/
				fclose(oldfp);
			}
			fclose(newfp);
		}
	}else 
	sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
	
	if (dashl(filepath) || !dashf(filepath))
				return -1;
	sprintf(edittmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
		currentuser.userid, getpid());
	copyfile_attach(filepath, edittmp);
	if (vedit(edittmp, 0, YEA) < 0) {
		unlink(edittmp);
		clear();
		do_delay(-1);	/* by ylsdd */
		return -1;
	}
	snprintf(attach_path, sizeof (attach_path),
		 PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);
	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, edittmp,
					 currentuser.userid);
	unlink(edittmp);

	return 1;
}


//׼������
static int 
marry_perpare(struct MC_Marry *marryMem, int n)
{
	int ch, quit = 0;
	int i;
	struct MC_Marry *mm;
	char buf[STRLEN];
	char filepath[STRLEN];
	char title[STRLEN];
	char uident[IDLEN+2];
	int freshflag = 1;

	clear();
	for(i=0; i<n; i++){
		if(marryMem[i].enable == 0) continue;
		if(marryMem[i].status != MAR_MARRYING) continue;
		if(!strcmp(marryMem[i].bride, currentuser.userid) || !strcmp(marryMem[i].bridegroom, currentuser.userid)){
			mm = &marryMem[i];
			break;
		}
	}
	if(i>=n){
		prints("����û�����Ļ���Ǽǰ����������������Ӧ����");
		pressanykey();
		return 0;
	}
	while (!quit) {
		money_show_stat("����ٸ����");
		if(freshflag){
			//sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			//show_welcome(filepath,4,22);
			freshflag =0;
		}
		move(5, 4);
		prints(mm->subject);
		move(6, 4);
		prints("����:\033[1;31m%s\033[m ����:\033[1;32m%s\033[m ",mm->bride, mm->bridegroom);
		move(7, 4);
		prints("û�뵽��λ���ô�����ף�æ����ͷת��\n    ������������Ҹ�����ٺ٣������Ǹ�����~~");
		move(t_lines - 1, 0);
		prints("\033[1;44m ѡ�� \033[1;46m [1]ѡ���� [2]д��� [3]������ [4]�������� [5]�������� [6]���ý��� [Q]�뿪\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			freshflag = 1;
			marry_selectday(mm);
			break;
		case '2':
			freshflag = 1;
			marry_editinvitation(mm);
			break;
		case '3':
			if (HAS_PERM(PERM_DENYMAIL)) {
				move(5, 4);
				prints("������ֹ����");
				pressanykey();
				break;
			}
			freshflag = 1;
			clear();
			move(5, 4);
			if (askyn("Ҫ���������к�����", YEA, NA) == NA) {		
				move(6, 4);
				usercomplete("��������λ��", uident);
				if (uident[0] == '\0')
					break;
				if(!searchuser(uident)) {
					move(7, 4);
	                		prints("���ּǴ��˰�...");
					pressanykey();
	                		break;
					}
				if(!strcmp(uident,currentuser.userid)){
					move(10, 4);
					prints("ι������˰ɣ����Լ������");
					pressanykey();
					break;
				}
				sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
				sprintf(title,"%s̨��,%s��%s�Ļ������",uident,mm->bride,mm->bridegroom);
				mail_file(filepath,uident,title);
			}else {
				sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
				for (i = 0; i  < uinfo.fnum; i++) {
					move(6, 4);
					clrtoeol();
					getuserid(uident, uinfo.friend[i]);
					if (!getuser(uident)) {
						prints("%s���ʹ���ߴ����Ǵ����.\n",uident);
						pressanykey();
						continue;
					} else if (!(lookupuser.userlevel & PERM_READMAIL)) {
						prints("�޷����Ÿ� [1m%s[m\n", lookupuser.userid);
						pressanykey();
						continue;
 					} else if (!strcmp(uident, currentuser.userid)) {
						prints("�Լ��Ͳ�Ҫ���Լ�������\n");
						pressanykey();
						continue;
					}
					sprintf(title,"%s̨��,%s��%s�Ļ������",uident,mm->bride,mm->bridegroom);
					mail_file(filepath,uident,title);
				}	
			}
			move(11, 4);
			prints("����ѷ���");
			pressanykey();
			break;
		case '4':		//���湫��
			freshflag = 1;
			move(9, 4);
			if(mm->invitationfile == 0){
				prints("��ûд�������");
				pressanykey();
				break;
			}
			sprintf(buf,"��ȷ��Ҫ�������������̡�����");
			if (askyn(buf, YEA, NA) == NA) {
				move(10, 4);
				prints("���ţ����Ҫ�ٸĸ�~~");	
				pressanykey();
				break;
			}
			sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
			sprintf(title,"[���]�������صڲμ�%s��%s�Ľ�����",mm->bride,mm->bridegroom);
			move(12, 4);
			if (mm->enable<3){
				postfile(filepath, MC_BOARD, title ,1);
				mm->enable++;
				prints("���Ļ����ѹ������£���ϲ��~~");
			}else 
				prints("����Ҳ��Ҫ�ܷ�ѽ�����ξͺ���~~");
			pressanykey();
			break;
		case '5':
			freshflag = 1;
			buf[0] = 0;
			getdata(9, 0, "�������������[���28����]: ", buf, 56, DOECHO, NA);
			if(buf[0]){
				strncpy(mm->subject,buf,58);
			}
			break;
		case '6':
			freshflag = 1;
			marry_editset(mm);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}//end of while...
	return 0;
}

//���
static int 
marry_divorce()
{
	clear();
	move(10, 4);
	//prints("��ѽ�����Ӷ���ô���˻���ʲô�벻���ģ����ȥ�úù����Ӱ�~~");
	prints("������ɣ������Ը�����㳡�طѣ���������");
	move(12, 4);
	prints("������飬������̻�������칫����ϵ���");
	pressanykey();
	return 0;
}

//������
static int 
money_deny()
{
	char uident[STRLEN];
	char ans[8];
	char msgbuf[256];
	int count;

	while (1) {
		clear();
		prints("�趨������\n");
		count = listfilecontent(MC_DENY_FILE);
		if (count)
			getdata(1, 0, "(A)���� (D)ɾ�� (C)�ı� or (E)�뿪 [E]: ", ans,
				7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)���� or (E)�뿪 [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("��˭���������: ", uident);
			if (*uident != '\0') 
				if (mc_addtodeny(uident, msgbuf, 0 ) == 1) 
					mc_denynotice(1, uident, msgbuf);
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("�ı�˭�ķ��ʱ���˵��: ", uident);
			if (*uident != '\0') 
				if (mc_addtodeny(uident, msgbuf, 1) == 1) 
					mc_denynotice(3, uident, msgbuf);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("�Ӻ�������ɾ��˭: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') 
				if (del_from_file(MC_DENY_FILE, uident)) 
					mc_denynotice(2, uident, msgbuf);
		} else
			break;
	}
	clear();
	return 1;
}
	


static int
mc_addtodeny(char *uident, char *msg, int ischange)
{
	char buf[50], strtosave[256];
	char buf2[50];
	int day;
	time_t nowtime;
	char ans[8];
	int seek;

	seek = seek_in_file(MC_DENY_FILE, uident);
	if ((ischange && !seek) || (!ischange && seek)) {
		move(2, 0);
		prints("�����ID����!");
		pressreturn();
		return -1;
	}
	buf[0] = 0;
	move(2, 0);
	prints("�������%s", uident);
	while (strlen(buf) < 4)
		getdata(3, 0, "����˵��(��������): ", buf, 40, DOECHO, YEA);

	do {
		getdata(4, 0, "��������(0-�ֶ����): ", buf2, 4, DOECHO, YEA);
		day = atoi(buf2);
	} while (day < 0);

	nowtime = time(NULL);
	if (day) {
		struct tm *tmtime;
		time_t undenytime = nowtime + day * 24 * 60 * 60;
		tmtime = gmtime(&undenytime);
		sprintf(strtosave, "%-12s %-40s %2d��%2d�ս� \x1b[%ldm", uident,
			buf, tmtime->tm_mon + 1, tmtime->tm_mday,
			(long int) undenytime);
		sprintf(msg,
			"�ݴ������ŷ����˽���͸¶��%s ��Ϊ"
			" \033[1m%s\033[m ԭ���ܹ� %s ��ֹ���������Ϸ"
			" %d �죬ϣ�����д�����ʿ����Ϊ�䣬"
			"��ͬ������г���̣�",
			uident, buf, currentuser.userid, day);
	} else {
		sprintf(strtosave, "%-12s %-35s �ֶ����", uident, buf);
		sprintf(msg, "�ݴ������ŷ����˽���͸¶��%s ��Ϊ"
			" \033[1m%s \033[mԭ���ܹ� %s ���ý�ֹ���������Ϸ��"
			"ϣ�����д�����ʿ����Ϊ�䣬��ͬ������г���̣�",
			uident, buf, currentuser.userid);
	}
	if (ischange)
		getdata(5, 0, "���Ҫ�ı�ô?[Y/N]: ", ans, 7, DOECHO, YEA);
	else
		getdata(5, 0, "���Ҫ��ô?[Y/N]: ", ans, 7, DOECHO, YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
	if (ischange)
		del_from_file(MC_DENY_FILE, uident);
	return addtofile(MC_DENY_FILE, strtosave);
}


static int
mc_denynotice(int action, char *user, char *msgbuf)
{
	char repbuf[STRLEN];
	char repuser[IDLEN + 1];
	strcpy(repuser, user);
	switch (action) {
	case 1:
		sprintf(repbuf,
			"[����]%s��������̺�����", repuser);
		deliverreport(repbuf, msgbuf);
		sprintf(repbuf,
			"%s��%s������̺�����",
			user, currentuser.userid);
		mail_buf(msgbuf, user, repbuf);
		millionairesrec(repbuf, msgbuf,"");
		break;
	case 3:
		sprintf(repbuf,
			"%s�ı�%s���̺�������ʱ���˵��", 
			currentuser.userid, user);
		millionairesrec(repbuf, msgbuf,"");
		mail_buf(msgbuf, user, repbuf);
		break;
	case 2:
		sprintf(repbuf,
			"�ָ� %s ���������Ϸ��Ȩ��",
			repuser);
		snprintf(msgbuf, 256, "%s %s\n"
			 "���������ܹܹ�����лл��\n", 
			 currentuser.userid, repbuf);
		deliverreport(repbuf, msgbuf);
		millionairesrec(repbuf, msgbuf,"");
		mail_buf(msgbuf, user, repbuf);
		break;
	}
	return 0;
}

static int
mc_autoundeny()
{
	char *ptr, buf[STRLEN];
	int undenytime;
	if (!seek_in_file(MC_DENY_FILE, currentuser.userid))
		return 0;
	readstrvalue(MC_DENY_FILE, currentuser.userid, buf, STRLEN);
	ptr=strchr(buf, 0x1b);
	if (ptr)
		memmove(buf, ptr+2, sizeof(buf));
	else return 0;
	undenytime=atoi(buf);
	if (undenytime > time(0))
		return 0;
	if (del_from_file(MC_DENY_FILE, currentuser.userid)) {
		sprintf(buf,
			"�ָ� %s ���������Ϸ��Ȩ��",
			currentuser.userid);
		//deliverreport(buf, "���������ܹܹ�����лл��\n");
		millionairesrec(buf, "ϵͳ�Զ����\n","");
		mail_buf("���������ܹܹ�����лл��\n", currentuser.userid, buf);
	}
	return 1;
}

static int
addstockboard(char *sbname, char *fname)
{
	int i;
	int seek;
	
	if ((i = getbnum(sbname)) == 0){
		move(3, 0);
		prints("���󣬲����ڵİ���");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	seek = seek_in_file(fname, sbname);
	if (seek) {
		move(3, 0);
		prints("����İ����Ѿ�����!");
		pressreturn();
		return 0;
	}
	move(3, 0);
	if (askyn("���Ҫ�����", NA, YEA) == NA) {
		pressanykey();
		return 0;
	}
	return addtofile(fname, sbname);

}

static int
delstockboard(char *sbname, char *fname)
{
	int i, seek;
	if ((i = getbnum(sbname)) == 0){
		move(3, 0);
		prints("���󣬲����ڵİ���");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	seek = seek_in_file(fname, sbname);
	if (!seek) {
		move(3, 0);
		prints("����İ��治���б���!");
		pressreturn();
		return 0;
	}
	move(3, 0);
	if (askyn("���Ҫɾ����", NA, NA)==NA){
		pressanykey();
		return 0;
	}
	return del_from_file(fname, sbname);
}

static int
stockboards()
{
	char uident[STRLEN];
	char ans[8], repbuf[200], buf[200], titlebuf[STRLEN], bname[STRLEN],  bpath[STRLEN];
	int count, ch2;
	struct stat st;
	FILE *f_fp;

	nomoney_show_stat("֤�����ϯ�칫��");
	whoTakeCharge2(6, buf);
	whoTakeCharge(6, uident);
	if (strcmp(currentuser.userid, uident)) {
		move(6, 4);
		prints
	  	  ("����%s��ס����,˵��:����ϯ%s���ں�æ,ûʱ��Ӵ��㡣��", buf,uident);
		pressanykey();
		return 0;
	} else {
		move(6, 4);
		prints("��ѡ���������:");
		move(7, 6);
		prints("1. �趨���а�������          2. ��ͣ/�ָ�ĳ�ɽ���");
		move(8, 6);
		prints("3. ��ͣȫ������              4. �ָ�ȫ������");
		move(9, 6);
		prints("5. ��ְ                      6. �˳�");
		ch2 = igetkey();
		switch (ch2) {
		case '1':
			ansimore(MC_STOCK_BOARDS, YEA);
			while (1) {
				clear();
				prints("�趨���а�������\n");
				count = listfilecontent(MC_STOCK_BOARDS);
				if (count)
					getdata(1, 0, "(A)���� (D)ɾ�� (E)�뿪 [E]: ",
						ans, 7, DOECHO, YEA);
				else
					getdata(1, 0, "(A)����  (E)�뿪 [E]: ", ans, 7,
						DOECHO, YEA);
				if (*ans == 'A' || *ans == 'a') {
					move(1, 0);
					make_blist();
					namecomplete("���Ӱ���: ", bname);
					setbpath(bpath, bname);
					if ((*bname == '\0') || (stat(bpath, &st) == -1)) {
						move(2, 0);
						prints("����ȷ��������.\n");
						pressreturn();
						break;
					}
					if (!(st.st_mode & S_IFDIR)) {
						move(2, 0);
						prints("����ȷ��������.\n");
						pressreturn();
						break;
					}
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {						
						if (addstockboard(bname, MC_STOCK_BOARDS)) {
							sprintf(repbuf, "[����]%s������", bname);
							sprintf(buf,
								"�����������룬����֤���ͨ����"
								"��׼%s�������У�����Ӫ��һ���£�"
								"��������ע�⡣"
								"���������в����������İ��棬"
								"��ӭ������������������С�\n",
								bname);
							deliverreport(repbuf, buf);
							sprintf(titlebuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
							sprintf(repbuf, "������а���: %s��", bname);
							millionairesrec(titlebuf, repbuf, "");					
						}
					}
				} else if ((*ans == 'D' || *ans == 'd') && count) {
					move(1, 0);
					namecomplete("ɾ������: ", bname);
					move(1, 0);
					clrtoeol();
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {						
						if (delstockboard(bname, MC_STOCK_BOARDS)) {
							getdata(6, 0, "ȡ��ԭ��", buf, 50, DOECHO, YEA);
							/*move(7, 0);
							if (askyn("ȷ����", NA, NA) == NA) {
								addtofile(MC_STOCK_BOARDS, bname);
								pressanykey();
								break;
							}*/
							sprintf(repbuf, "ԭ��%s", buf);
							sprintf(titlebuf, "[����]%s������", bname);
							deliverreport(titlebuf, repbuf);
							sprintf(titlebuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
							sprintf(repbuf, "ȡ�����а���: %s��\n\nȡ��ԭ��%s\n", bname, buf);
							millionairesrec(titlebuf, repbuf, "");					
						}
					}
				} else
				break;
			}
			break;

		case '2':
			//ansimore(MC_STOCK_STOPBUY, YEA);
			while (1) {
				clear();
				prints("����ͣ���׵İ�������\n");
				count = listfilecontent(MC_STOCK_STOPBUY);
				if (count)
					getdata(1, 0, "(A)���� (D)ɾ�� (E)�뿪 [E]: ",
						ans, 7, DOECHO, YEA);
				else
					getdata(1, 0, "(A)����  (E)�뿪 [E]: ", ans, 7,
						DOECHO, YEA);
				if (*ans == 'A' || *ans == 'a') {
					move(1, 0);
					make_blist();
					namecomplete("��ͣ�İ潻��: ", bname);
					setbpath(bpath, bname);
					if ((*bname == '\0') || (stat(bpath, &st) == -1)) {
						move(2, 0);
						prints("����ȷ��������.\n");
						pressreturn();
						break;
					}
					if (!(st.st_mode & S_IFDIR)) {
						move(2, 0);
						prints("����ȷ��������.\n");
						pressreturn();
						break;
					}
					if (!seek_in_file(MC_STOCK_BOARDS, bname)){
						move(2, 0);
						prints("��ѡ��İ���û������\n");
						pressreturn();
						break;
					}
						
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
						if (addstockboard(bname, MC_STOCK_STOPBUY)) {
							getdata(6, 0, "��ͣԭ��", buf, 50, DOECHO, YEA);
							move(7, 0);
							if (askyn("ȷ����", NA, NA) == NA) {
								pressanykey();
								break;
							}
							sprintf(repbuf, "��ͣԭ��%s", buf);
							sprintf(titlebuf, "[����]%s���Ʊͣ��", bname);
							deliverreport(titlebuf, repbuf);
							sprintf(titlebuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
							sprintf(repbuf, "��ͣ%s���Ʊ����\n\nԭ��%s\n", bname, buf);
							millionairesrec(titlebuf, repbuf, "");					
						}
					}
				} else if ((*ans == 'D' || *ans == 'd') && count) {
					move(1, 0);
					namecomplete("Ҫ�ָ����׵İ���: ", bname);
					move(1, 0);
					clrtoeol();
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
						if (delstockboard(bname, MC_STOCK_STOPBUY)) {
							getdata(6, 0, "�ָ�ԭ��", buf, 50, DOECHO, YEA);
							/*move(7, 0);
							if (askyn("ȷ����", NA, NA) == NA) {
								pressanykey();
								break;
							}*/
							sprintf(repbuf, "�ָ�ԭ��%s", buf);
							sprintf(titlebuf, "[����]%s���Ʊ����", bname);
							deliverreport(titlebuf, repbuf);
							sprintf(titlebuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
							sprintf(repbuf, "�ָ�%s���Ʊ����\n\nԭ��%s\n", bname, buf);
							millionairesrec(titlebuf, repbuf, "");					
						}
					}
				} else
				break;
			}
			break;

		case '3':
		//	utmpshm->ave_score = 0;
			sprintf(buf,"%s/stopbuy",DIR_MC);
			if (file_exist(buf)){	
				clear();
				move(6, 4);
				prints("�Ѿ�ͣ��");		
				pressreturn();
				break;
			}
			
			f_fp=fopen(buf,"w");
			if(f_fp!=NULL){
				fclose(f_fp);
				//sprintf(repbuf, "ԭ��%s", buf);
				//sprintf(titlebuf, "[����]����ٸ����ͣ��");
				deliverreport("[����]����ٸ����ͣ��", "");
				sprintf(titlebuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
				//sprintf(repbuf, "��ͣȫ������\n\nԭ��%s\n", bname, buf);
				millionairesrec(titlebuf, "��ͣȫ������", "");	

				clear();
				move(6, 4);
				prints("�����ɹ�!");
				pressanykey();
			}else{
				clear();
				move(6, 4);
				prints("��������");
				pressreturn();
			}
			break;

		case '4':
			sprintf(buf,"%s/stopbuy",DIR_MC);
			if (!file_exist(buf)){	
				clear();
				move(6, 4);
				prints("û��ͣ�̰�");		
				pressreturn();
				break;
			}
			remove(buf);
			deliverreport("[����]����ٸ�������¿���", "");
			sprintf(titlebuf, "%s��ʹ���й���Ȩ��", currentuser.userid);
			millionairesrec(titlebuf, "�ָ�ȫ������", "");	
				
			clear();
			move(6, 4);
			prints("�����ɹ�!");
			pressanykey();
			break;

		case '5':
			move(12, 4);
			if (askyn("�����Ҫ��ְ��", NA,NA) == YEA) {
			sprintf(genbuf, "%s Ҫ��ȥ֤�����ϯְ��",
				currentuser.userid);
			mail_buf(genbuf, "millionaires", genbuf);
			move(14, 4);
			prints("�ðɣ��Ѿ����Ÿ�֪�ܹ���");
			pressanykey();
			}
			break;
		}
	}
	clear();
	return FULLUPDATE;
}

/* ��ƱƱ�ۼ������ by macintosh 2006.12.28 */
/* 2007.10.26�޸�*/

struct ticket_info {
	char CheCi[6];
	char ShiFa[11];
	char ZhongDao[11];
	//������Ʊ����Ϣ
	int LiCheng;
	int LiCheng2;
	//ͨƱǰһ�ε����
	char PiaoZhong;
	//ѧ�к�����
	char XiBie;
	//��Ӳ
	char JiaKuai;
	//�տ��ؿ�
	char KongTiao;
	//�յ�
	char WoPu;
	//û�У�������
	char DongChe;
	//������һ������������
	char DaoDi;
	//ͨƱ��������
	float ShangFu;
	//�ϸ�
	float ZaiFu;
	//�ٸ�
} myTicket;

struct TrainInfo{
	char CheCi[6];
	char KongTiao;
	float ShangFu;
	float ZaiFu;
};

//Ĭ�Ͽ����ؿ��пյ����տ��޿յ������½��г����⳵��
//���½��г�����վ�������Σ���������
struct TrainInfo XianTrain[]= {
	{"T193", 2, 0.3, 0.0},{"T194", 2, 0.3, 0.0},{"T191", 2, 0.3, 0.0},{"T192", 2, 0.3, 0.0},
	{"T197", 2, 0.3, 0.0},{"T198", 2, 0.3, 0.0},
	{"K5", 2, 0.4, 0.0},	{"K6", 2, 0.4, 0.0},
	{"K165", 2, 0.4, 0.0},{"K166", 2, 0.4, 0.0},
	{"K173", 0, 0.0, 0.0},{"K174", 0, 0.0, 0.0},{"K171", 0, 0.0, 0.0},{"K172", 0, 0.0, 0.0},
	{"K241", 2, 0.4, 0.0},{"K242", 2, 0.4, 0.0},{"K243", 2, 0.4, 0.0},{"K244", 2, 0.4, 0.0},	
	{"K245", 2, 0.4, 0.0},{"K246", 2, 0.4, 0.0},{"K247", 2, 0.4, 0.0},{"K248", 2, 0.4, 0.0},
	{"K317", 2, 0.4, 0.0},{"K318", 2, 0.4, 0.0},{"K315", 2, 0.4, 0.0},{"K316", 2, 0.4, 0.0},
	{"K361", 2, 0.4, 0.0},{"K362", 2, 0.4, 0.0},{"K360", 2, 0.4, 0.0},{"K359", 2, 0.4, 0.0},
	{"K385", 2, 0.4, 0.0},{"K386", 2, 0.4, 0.0},{"K387", 2, 0.4, 0.0},{"K388", 2, 0.4, 0.0},
	{"K419", 2, 0.4, 0.0},{"K420", 2, 0.4, 0.0},{"K417", 2, 0.4, 0.0},{"K418", 2, 0.4, 0.0},
	{"K447", 2, 0.4, 0.0},{"K448", 2, 0.4, 0.0},{"K446", 2, 0.4, 0.0},{"K445", 2, 0.4, 0.0},
	{"K467", 2, 0.3, 0.0},{"K468", 2, 0.3, 0.0},{"K466", 2, 0.3, 0.0},{"K465", 2, 0.3, 0.0},
	{"K543", 2, 0.4, 0.0},{"K544", 2, 0.4, 0.0},{"K542", 2, 0.4, 0.0},{"K541", 2, 0.4, 0.0},
	{"K595", 2, 0.4, 0.0},{"K596", 2, 0.4, 0.0},{"K594", 2, 0.4, 0.0},{"K593", 2, 0.4, 0.0},
	{"K617", 2, 0.4, 0.0},{"K618", 2, 0.4, 0.0},
	{"K621", 2, 0.3, 0.0},{"K622", 2, 0.3, 0.0},{"K623", 2, 0.3, 0.0},{"K624", 2, 0.3, 0.0},
	{"1131", 2, 0.5, 0.0},{"1132", 2, 0.5, 0.0},{"1130", 2, 0.5, 0.0},{"1129", 2, 0.5, 0.0},
	{"1158", 2, 0.5, 0.0},{"1159", 2, 0.5, 0.0},{"1157", 2, 0.5, 0.0},{"1160", 2, 0.5, 0.0},
	{"1353", 2, 0.5, 0.0},{"1354", 2, 0.5, 0.0},{"1352", 2, 0.5, 0.0},{"1351", 2, 0.5, 0.0},
	{"1363", 2, 0.3, 0.0},{"1364", 2, 0.3, 0.0},
	{"1433", 2, 0.3, 0.0},{"1434", 2, 0.3, 0.0},{"1432", 2, 0.3, 0.0},{"1431", 2, 0.3, 0.0},
	{"N373", 0, 0.0, 0.0},{"N374", 0, 0.0, 0.0},
	{"N375", 2, 0.5, -0.15},	{"N376", 2, 0.5, -0.15},	{"N376", 2, 0.5, -0.15},	{"N378", 2, 0.5, -0.15},	
	{"N359", 2, 0.4, 0.0},{"N360", 2, 0.4, 0.0},{"N357", 2, 0.4, 0.0},{"N358", 2, 0.4, 0.0},
	{"4901", 2, 0.4, 0.0},{"4902", 2, 0.4, 0.0},{"4903", 2, 0.4, 0.0},{"4904", 2, 0.4, 0.0},
	{"4909", 2, 0.4, 0.0},{"4910", 2, 0.4, 0.0},{"4908", 2, 0.4, 0.0},{"4907", 2, 0.4, 0.0},
	{"4911", 2, 0.3, 0.0},{"4912", 2, 0.3, 0.0},
	{"4915", 2, 0.5, -0.3},{"4916", 2, 0.5, -0.3},{"4917", 2, 0.5, -0.3},{"4918", 2, 0.5, -0.3},
	
	{"A351", 1, 0.0, 0.0},{"A352", 1, 0.0, 0.0},
};

//��������
static float 
Round(float num)
{
	num = (float)(int) (num + 0.5);
	return num;
}


//����Ӳϯ����Ʊ
static float 
calc_basic_price(int LiCheng, int flag)
{	
	int mininum, distance = 0, order = 0, i, j;
	float rate=0, basic_price=0;
	const float BASIC = 0.05861;

	if (LiCheng <= 0)
		return 0;

	//�������
	switch (flag){
		case 2:
			mininum = 100;//�ӿ�Ʊ
			break;
		case 3:
			mininum = 400;//����Ʊ
			break;
		default:
			mininum = 0;//20
			break;
	}
	if (LiCheng <= mininum)
		distance = mininum;
	else
		//����μ���������
		for (i = 4600, j = 100; j > 0; j -= 10){
			if (LiCheng > i){
				order = (LiCheng - i) / j;
				if ((LiCheng - i) % j == 0)
					order--;
				distance = i + order * j + j/2;
				break;
			}else
				i = i-(j/10-1)*100;
			//�������<=200�����
			if (j == 20)
				i = 0;
		}

	//Ʊ�۵�Զ�ݼ�����
	const int qd[7] = {0, 200, 500, 1000, 1500, 2500, 99999};
	
	for (i = 0; i < 6; i++){
		if (distance > qd[i]){
			rate = BASIC * (1 - 0.1 * i);
			basic_price += rate * (min(qd[i+1], distance) - qd[i]);
		} else
			break;
	}
	return basic_price;
}


static float 
show_ticket()
{
	float JiBenPiao, BaoXian, KePiao, KuaiPiao, KongPiao, WoPiao, QuanJia, JiJin, CheZhan, KePiao2;
	int YouXiaoQi=2, i;
	float jk1=0, jk2=0;
	char printbuf[1024], ZTKN[5], printbuf2[128];

	if (myTicket.DongChe > 0){
		switch (myTicket.DongChe){
			case 3:
				QuanJia = 0.2805 * myTicket.LiCheng * 1.1 * 0.75;
				break;
			case 2:
				QuanJia = 0.3366 * myTicket.LiCheng * 1.1;
				break;
			case 1:
			default:
				QuanJia = 0.2805 * myTicket.LiCheng  * 1.1;
				break;
		}
		QuanJia = ceil(QuanJia);
			
		sprintf(printbuf, "\033[1m%s%s\033[0m�������г�Ʊ����Ϣ�����%d���",
			myTicket.CheCi,  myTicket.CheCi[0]?"��":"",
			myTicket.LiCheng);
		showAt(5, 6, printbuf, 0);
		showAt(9, 6, "ע��: ������Ʊ�۽����ο�������Ʊ�۲μ���վ���档", 0);

		sprintf(printbuf, "�� %.2f Ԫ\t\t\t  ������%s����",
			QuanJia,
			(myTicket.DongChe == 2)?"һ":"��");
		showAt(16, 6, printbuf, 0);
		
		if (myTicket.DongChe == 3)
			showAt(16, 24, "(ѧ)", 0);
		showAt(18, 6, "�� г ��", 0);

		return QuanJia;
	}
		
	JiBenPiao = calc_basic_price(myTicket.LiCheng, 0);
	
	//��Ʊ����
	//ͨƱ
	if (myTicket.DaoDi > 0){
		JiBenPiao = calc_basic_price(myTicket.LiCheng2, 0);
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //ֱ�ӽ�λ��0.1Ԫ
		BaoXian = BaoXian/10;
		//��ϯ
		if (myTicket.XiBie == 1)
			KePiao2 = Round(JiBenPiao * 2 + BaoXian);
		else 
			KePiao2 = Round(JiBenPiao + BaoXian);
		if (KePiao2 < 1)
			KePiao2 = 1; 
		KePiao2 = Round(KePiao2 * (1 + myTicket.ShangFu)) -Round(JiBenPiao + BaoXian);
		//ȫ��
		JiBenPiao = calc_basic_price(myTicket.LiCheng, 0);
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //ֱ�ӽ�λ��0.1Ԫ
		BaoXian = BaoXian/10;
		KePiao = Round(JiBenPiao + BaoXian);
		KePiao += KePiao2;
	}
	else{
		KePiao = JiBenPiao;
		//���շѼ���
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //ֱ�ӽ�λ��0.1Ԫ
		BaoXian = BaoXian/10;
		if (myTicket.XiBie == 1)
			KePiao *= 2;
		KePiao += BaoXian;
		KePiao = Round(KePiao);
		if (KePiao < 1)
			KePiao = 1; 
		KePiao *= (1 + myTicket.ShangFu);//�ϸ�
		KePiao = Round(KePiao);
	}
	
	//�ӿ�Ʊ����
	KuaiPiao = 0;
	//����ǰ�벿��(��ͨƱ��ȫ��)
	JiBenPiao = calc_basic_price(myTicket.LiCheng2, 2);
	if (myTicket.JiaKuai > 0)
		KuaiPiao = 0.2 * JiBenPiao;
	KuaiPiao = Round(KuaiPiao);
	KuaiPiao *= (1 + myTicket.ShangFu);//�ϸ�
	KuaiPiao = Round(KuaiPiao);
	if (myTicket.JiaKuai > 1)
		KuaiPiao *= 2;
	
	//ͨƱ����ǰ����Ӧ���׵ȼ��ļӿ�Ʊjk1
	if (myTicket.DaoDi > 0){
		if (myTicket.DaoDi == 2){
			if (myTicket.JiaKuai < 1)//��Ǯ���տ��г��տ쵽��
				jk1 = 0;
			else{
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
			}
		}
		if (myTicket.DaoDi == 3){
			if (myTicket.JiaKuai < 1)//��Ǯ���տ��г��ؿ쵽��
				jk1 = 0;
			else if (myTicket.JiaKuai < 2){//��Ǯ���տ��г��ؿ쵽��
					jk1 = 0.2 * JiBenPiao;
					jk1 = Round(jk1);
			}else{
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
				jk1 *= 2;
			}
		}			
	}

	//ͨƱȫ�̵��׼ӿ�Ʊjk2	
	if (myTicket.DaoDi > 1){
		JiBenPiao = calc_basic_price(myTicket.LiCheng, 2);
		jk2 = 0.2 * JiBenPiao;
		jk2 = Round(jk2);
	}
	if (myTicket.DaoDi > 2)
		jk2 *= 2;
	
	KuaiPiao = KuaiPiao - jk1 + jk2;
	

	//�յ�Ʊ����
	KongPiao = 0;
	if (myTicket.KongTiao > 0){
		JiBenPiao = calc_basic_price(myTicket.LiCheng2, 4);
		KongPiao = 0.25 * JiBenPiao;
		KongPiao = Round(KongPiao);
		if (KongPiao < 1)
			KongPiao = 1; //�յ�Ʊ����1Ԫ��1Ԫ��
		KongPiao *= (1 + myTicket.ShangFu);//�ϸ�
		KongPiao = Round(KongPiao);
	}
	
	//����Ʊ����
	WoPiao = 0;	
	JiBenPiao = calc_basic_price(myTicket.LiCheng2, 3);
	if (myTicket.XiBie == 0){//Ӳ��
		if (myTicket.WoPu == 1)
			WoPiao = 1.1 * JiBenPiao;
		else if (myTicket.WoPu == 2)
			WoPiao = 1.2 * JiBenPiao;
		else if (myTicket.WoPu == 3)
			WoPiao = 1.3 * JiBenPiao;
	} else {//����
		if (myTicket.WoPu == 1)
			WoPiao = 1.75 * JiBenPiao;
		else if (myTicket.WoPu > 1)
			WoPiao = 1.95 * JiBenPiao;
	} 
	WoPiao = Round(WoPiao);
	WoPiao *= (1 + myTicket.ShangFu);//�ϸ�
	WoPiao = Round(WoPiao);

	//���ϸ�
	KePiao = Round((1 + myTicket.ZaiFu) * KePiao);
	KuaiPiao = Round((1 + myTicket.ZaiFu) * KuaiPiao);
	KongPiao = Round((1 + myTicket.ZaiFu) * KongPiao);
	WoPiao = Round((1 + myTicket.ZaiFu) * WoPiao);
	
	//ѧ��Ʊ
	if (myTicket.PiaoZhong == 1 && myTicket.XiBie == 0){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
	}

	//С��Ʊ
	if (myTicket.PiaoZhong == 2){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
	}	

	//��Ʊ
	if (myTicket.PiaoZhong == 3){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
		WoPiao *= 0.5;
	}

	//С������ʹ������
	if (myTicket.PiaoZhong == 4){
		KePiao = 0;
		KuaiPiao = 0;
		KongPiao *= 0.5;
	}	

	//���̶�Ʊ��
	if (WoPiao > 0)
		WoPiao += 10;

	//��Ʊ��Ϣ����չ����
	if (KePiao + KuaiPiao + KongPiao + WoPiao > 5)
		JiJin = 1;
	else 
		JiJin = 0.5;

	//��վ�յ���
	if (myTicket.LiCheng >= 200)
		CheZhan = 1;	
	else
		CheZhan = 0;	
	//��ϯ���տյ���
	if (myTicket.XiBie)
		CheZhan = 0;	

	if (myTicket.LiCheng <= 0)
		KePiao = KuaiPiao = KongPiao = WoPiao = 0;

	QuanJia = KePiao + KuaiPiao + KongPiao + WoPiao + JiJin + CheZhan;

	sprintf(printbuf, "��ƱƱ�ۣ�\t%.2f Ԫ", KePiao);
	showAt(7, 6, printbuf, 0);
	sprintf(printbuf, "����ƱƱ�ۣ�\t%.2f Ԫ", WoPiao);
	showAt(7, 44, printbuf, 0);
	sprintf(printbuf, "\033[1;30m�����˺����գ�\t%.2f Ԫ", BaoXian);
	showAt(8, 6, printbuf, 0);
	sprintf(printbuf, "���̶�Ʊ�ѣ�\t%.2f Ԫ\033[m", (WoPiao > 0) ? 10.0 : 0.0);
	showAt(8, 44, printbuf, 0);
	sprintf(printbuf, "�ӿ�ƱƱ�ۣ�\t%.2f Ԫ", KuaiPiao);
	showAt(9, 6, printbuf, 0);
	sprintf(printbuf, "�յ�ƱƱ�ۣ�\t%.2f Ԫ", KongPiao);
	showAt(9, 44, printbuf, 0);
	sprintf(printbuf, "��Ʊ��Ϣ������\t%.2f Ԫ", JiJin);
	showAt(10, 6, printbuf, 0);
	sprintf(printbuf, "��վ�յ��ѣ�\t%.2f Ԫ\033[m", CheZhan);
	showAt(10, 44, printbuf, 0);
	
	switch (myTicket.CheCi[0]){
		case 'Z':
			strcpy(ZTKN, "ֱ��");
			break;
		case 'K':
		case 'N':
			strcpy(ZTKN, "����");
			break;
		case 'T':
		default:
			strcpy(ZTKN, "�ؿ�");
			break;
	}
	sprintf(printbuf, "�� %.2f Ԫ\t\t\t  %s%s%s%s%s",
		QuanJia,
		(myTicket.KongTiao == 2)?"��":"",
		(myTicket.KongTiao > 0)?"�յ�":"",
		(myTicket.XiBie)?"����":"Ӳ��",
		(myTicket.JiaKuai>0)?((myTicket.JiaKuai>1)?ZTKN:"�տ�"):"�տ�",
		(myTicket.WoPu>0)?"��":"");
	showAt(16, 6, printbuf, 0);

	if (myTicket.DaoDi > 0){
		sprintf(printbuf, "(%s����վ)",
			(myTicket.DaoDi > 1)?((myTicket.DaoDi > 2)?"�ؿ�":"�տ�"):"�տ�");
		showAt(16, 60, printbuf, 0);
		showAt(17, 42, "������վ", 0);
	}
		
	if (myTicket.DaoDi > 0)
		sprintf(printbuf2, "����תǰ%d����", myTicket.LiCheng2);
	else
		printbuf2[0] = 0;
	sprintf(printbuf, "\033[1m%s%s\033[0m%s�г�Ʊ����Ϣ�����%d����%s��",
		myTicket.CheCi,  myTicket.CheCi[0]?"��":"",
		(myTicket.JiaKuai>0)?((myTicket.JiaKuai>1)?ZTKN:"�տ�"):"��ͨ",
		myTicket.LiCheng,
		printbuf2);
	showAt(5, 6, printbuf, 0);

	YouXiaoQi = 2;
	i = 500;
	while (myTicket.LiCheng > i){
		YouXiaoQi ++;
		i += 1000;
	}
	sprintf(printbuf, "�� %d ���ڵ���Ч", YouXiaoQi);
	showAt(18, 6, printbuf, 0);

	switch (myTicket.PiaoZhong){
		case 1:
			showAt(16, 24, "(ѧ)", 0);
			break;
		case 2:
		case 4:
			showAt(16, 24, "(��)", 0);
			break;
		case 3:
			showAt(16, 24, "(��)", 0);
			break;
		default:
			break;
	}
		
	if (myTicket.ShangFu > 0 && myTicket.ShangFu < 0.5 && myTicket.KongTiao == 2)
		showAt(17, 24, "(��)", 0);

	return QuanJia;
}
/*
static int
calc_ticket_price()
{
	int ch, quit = 0, temp, i;
	char buf[STRLEN], ZTK=0;
	float tempf;

	bzero(&myTicket, sizeof (struct ticket_info));
	clear();
	while (!quit) {
		nomoney_show_stat("��ƱƱ�ۼ���");
		show_ticket();
		showAt(t_lines - 2, 0, "\033[1;44m �� \033[1;46m [0]���� [1]��� [2]Ʊ�� [3]ϯ�� [4]�ȼ� [5]�յ� [6]�ϸ��� [7]���� [8]�ٸ���          \033[m", 0);
		if (myTicket.DaoDi == 0)
			showAt(t_lines - 1, 0, "\033[1;44m �� \033[1;46m [A]ͨƱ [D]������Ʊ�� [H]���� [Q]�뿪                                                          \033[m", 0);
		else
			showAt(t_lines - 1, 0, "\033[1;44m �� \033[1;46m [A]ͨƱ [B]��תǰ��� [H]���� [Q]�뿪                                             \033[m", 0);
		
		ch = igetkey();
		switch (ch) {
		case '0':
			getdata(t_lines-3, 0, "�����복��: ", buf, 5, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			bzero(&myTicket, sizeof (struct ticket_info));
			sprintf(myTicket.CheCi, "%s", buf);
			if (isalpha(buf[0])){
				ZTK = toupper(buf[0]);
				myTicket.CheCi[0] = ZTK;
			}else
				ZTK = 0;
			temp = atoi(buf);
			
			if (ZTK=='Z' || ZTK=='T' || ZTK=='K' || ZTK=='N'){
				myTicket.JiaKuai = 2;
				myTicket.KongTiao = 2;
				myTicket.ShangFu = 0.5;
			}else if (ZTK=='D')
				myTicket.DongChe = 1;
			else if (temp < 6000)
				myTicket.JiaKuai = 1;
			else
				myTicket.JiaKuai = 0;

			i = 0; 
			while (XianTrain[i].CheCi[0] != '\0'){
				if (!strcmp(XianTrain[i].CheCi, myTicket.CheCi)){
					myTicket.KongTiao = XianTrain[i].KongTiao;
					myTicket.ShangFu = XianTrain[i].ShangFu;
					myTicket.ZaiFu = XianTrain[i].ZaiFu;
					break;
				}
				i++;
			}
			break;
			
		case '1':
			getdata(t_lines-3, 0, "���������: ", buf, 5, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.LiCheng = (temp > 0) ? temp : 0;
			myTicket.LiCheng2 = myTicket.LiCheng;
			break;

		case '2':
			showAt(t_lines-4, 0, "0.ȫ�� 1.ѧ��Ʊ 2.С��Ʊ 3.�˲о���Ʊ 4.С������ʹ������", 0);
			getdata(t_lines-3, 0, "������Ʊ��: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.PiaoZhong = (temp > 0 && temp < 5) ? temp : 0;
			break;

		case '3':
			showAt(t_lines-4, 0, "0.Ӳϯ   1.��ϯ", 0);
			getdata(t_lines-3, 0, "������ϯ��: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.XiBie = (temp > 0 && temp < 2) ? temp : 0;
			break;

		case '4':
			showAt(t_lines-4, 0, "0.�տ�   1.�տ�   2.����/�ؿ�/ֱ��", 0);
			getdata(t_lines-3, 0, "������ȼ�: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.JiaKuai = (temp > 0 && temp < 3) ? temp : 0;
			break;		

		case '5':
			showAt(t_lines-4, 0, "0.�޿յ�   1.��ͨ�пյ�   2.�����пյ�", 0);
			getdata(t_lines-3, 0, "������ȼ�: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.KongTiao = (temp > 0 && temp < 3) ? temp : 0;
			if (myTicket.KongTiao == 1)
				myTicket.ShangFu = 0.0;
			else if (myTicket.KongTiao == 2)
				myTicket.ShangFu = 0.5;
			break;	

		case '6':
			getdata(t_lines-3, 0, "�������ϸ���: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			tempf = atof(buf);
			if (tempf <= 0)
				tempf = 0;
			else if (tempf >= 2.99)
				tempf = 2.99;
			myTicket.ShangFu = tempf;
			break;

		case '7':
			if (myTicket.XiBie == 0)
				showAt(t_lines-4, 0, "0.ȡ��   1.����  2.����  3.����", 0);
			else
				showAt(t_lines-4, 0, "0.ȡ��   1.����  2.����", 0);
			getdata(t_lines-3, 0, "��������������: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.WoPu= (temp > 0 && temp < 4) ? temp : 0;
			break;
		
		case '8':
			getdata(t_lines-3, 0, "�������ٸ���(��Ϊ��ֵ): ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			tempf = atof(buf);
			if (tempf < -0.5)
				tempf = -0.5;
			else if (tempf > 0.3)
				tempf = 0.3;
			myTicket.ZaiFu = tempf;
			break;
		
		
		case 'A':
		case 'a':
			if (myTicket.DongChe > 0){
				myTicket.DaoDi = 0;
				showAt(t_lines-4, 0, "\033[1;5;31m�������г���Ʊ��Զֻ�����������г��յ�վ��\033[m", 0);
				pressreturn();
				break;
			}	 //�����鲻��ͨƱ
			showAt(t_lines-4, 0, "0.ȡ��   1.�տ͵���  2.�տ쵽��  3.�ؿ쵽��", 0);
			getdata(t_lines-3, 0, "������ͨƱ����: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.DaoDi= (temp > 0 && temp < 4) ? temp : 0;
			break;

		case 'B':
		case 'b':
			if (myTicket.DaoDi == 0)
				break;
			getdata(t_lines-3, 0, "��������תǰ�����: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.LiCheng2 = (temp > 0) ? temp : 0;
			if (myTicket.LiCheng2 > myTicket.LiCheng)
				myTicket.LiCheng2 = myTicket.LiCheng; 
			break;

		case 'd':
		case 'D':
			showAt(t_lines-4, 0, "0.ȡ��   1.������   2.һ����   3.������ѧ��Ʊ", 0);
			getdata(t_lines-3, 0, "������: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.DongChe = (temp > 0 && temp < 4) ? temp : 0;
			if (myTicket.DongChe > 0)
				myTicket.DaoDi = 0; //�����鲻��ͨƱ
			break;

		case 'h':
		case 'H':
			clear();
			showAt(5, 4, "ֱ��ؿ졢�����г�Ĭ��Ϊ�¿յ��г��������޸��밴5��", 0);
			showAt(7, 4, "�¿յ��г��ϸ���Ϊ0.5��һ�������ۿ۷ֱ�Ϊ0.4��0.3�������޸��밴6��", 0);
			showAt(9, 4, "\033[1m�г����������뵽\033[32m"MC_BOARD"\033[37m����ѯ!\033[m", 0);
			showAt(11, 4, "\033[1m����·Ʊ�۷��������뵽\033[32mtraffic\033[37m����ѯ!\033[m", 0);
			showAt(13, 4, "\033[1;32m��л����ʹ��! ��ӭ���´�����!\033[m", 1);
			break;
			
		case 'q':
		case 'Q':
			quit = 1;
	    	 	break;	
		}
	}
	return 0;
}
*/
/* ��� by macintosh  */

static int
loadContributions(char *cname, char *user)
{
	char value[20];
	char path[256];
	sprintf(path, DIR_CONTRIBUTIONS"%s", cname);
	if (readstrvalue(path, user, value, 20) != 0)
		return 0;
	else
		return limitValue(atoi(value), sizeof(int));
}  //��ȡ����������ֵ

static int
saveContributions(char *cname, char *user, int valueToAdd)
{
	int valueInt;
	char value[20], path[256];
	
	sprintf(path, DIR_CONTRIBUTIONS"%s", cname);	
	if (readstrvalue(path, user, value, 20) != 0)
		valueInt = 0;
	else
		valueInt = limitValue(atoi(value), MAX_CTRBT_NUM);
	valueInt += valueToAdd;
	valueInt = limitValue(valueInt, MAX_CTRBT_NUM);
	snprintf(value, 20, "%d", valueInt);
	savestrvalue(path, user, value);
	return 0;
}  //��������ֵ


static void
doContributions(struct MC_Jijin *clist)
{
	int money, i=0, num=0, num2, total_num, old_num ;
	float transfer_rate;
	char title[80], buf[512];

	clear();
	sprintf(buf, "No. %-12.12s  %16.16s  %s", "����ID", "��������", "�ۼƾ��");
	showAt(5, 2, buf, 0);
	while (clist[i].userid[0]!= 0){
		sprintf(buf, "ctr_%s", clist[i].userid);
		old_num = loadValue(currentuser.userid, buf, MAX_CTRBT_NUM);
		sprintf(buf, "%2d  %-12.12s  %17.17s  %d", i+1, clist[i].userid, clist[i].name, old_num);
		showAt(7+i, 2, buf, 0);
		i++;
	}
	sprintf(title, "��ѡ�������[1-%d]: ", i);
	getdata(t_lines-6, 2, title, buf, 3, DOECHO, YEA);
	if (buf[0] == '\0' || buf[0] == '\n')
		return;
	num = atoi(buf);
	if (num > i || num < 1){
		showAt(t_lines-4, 2, "���Ǻ����پ��...", 1);
		return;
	}
	num --;

	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	getdata(t_lines-5, 2, "�������ֽ���[����1000]: ", buf, 10, DOECHO, YEA);
	num2 = atoi(buf);
	if (num2 < 1000){
		showAt(t_lines-4, 2, "1000��û�а�...", 1);	
		return;
	}
	if (num2 > money || num2 <= 0){
		showAt(t_lines-4, 2, "�Բ���, ���ֽ����", 1);	
		return;
	}
	transfer_rate = utmpshm->mc.transfer_rate / 10000.0;
	sprintf(buf,
		" ������ %.2f���������ȡ 100000 ����ٸ�ң�����1��1��ȡ����",
		transfer_rate * 100);
	showAt(t_lines-4, 2, buf, 0);
	move(t_lines-3, 2);
	sprintf(buf, "ȷ���� %s ����%s����%d ����ٸ����", clist[num].name, clist[num].userid, num2);
	
	if (askyn(buf, YEA, NA) == YEA) {
		if (num2 * transfer_rate >= 100000) {
			total_num = num2 + 100000;
		} else {
			total_num = num2 * (1.0 + transfer_rate);
		}
		if (total_num - num2 < 1)
			total_num +=1;
		if (money < total_num) {
			move(t_lines-2, 4);
			prints("�����ֽ𲻹����������ѹ��� %d ����ٸ��", total_num);
			pressanykey();
			return;
		}
		saveValue(currentuser.userid, MONEY_NAME, -total_num, MAX_MONEY_NUM);
		saveValue(clist[num].userid, MONEY_NAME, num2, MAX_MONEY_NUM);
		sprintf(title, "[֪ͨ] %s ��%s������", currentuser.userid, clist[num].name);
		sprintf(buf,
			"%s ͨ������ٸ���칫���������� %d ����ٸ�ң�����ա�",
			currentuser.userid, num2);
		mail_buf(buf, clist[num].userid, title);

		sprintf(buf, "ctr_%s", clist[num].userid);
		saveValue(currentuser.userid, buf, num2, MAX_CTRBT_NUM);
		saveContributions(clist[num].userid, currentuser.userid, num2);

		sprintf(title, "[����] %s����%s���յ����", clist[num].name, clist[num].userid);
		sprintf(buf,"��л%s��%s����Ĵ���֧�֣�����ٸ���̴���ȫ����˰�������ʾ��л��", currentuser.userid, clist[num].name);
		deliverreport(title, buf);
		
		sprintf(genbuf, "%s���о��", currentuser.userid);
		sprintf(buf,"%s����%s����%s�� %d����ٸ��", currentuser.userid, clist[num].name, clist[num].userid, num2);
		millionairesrec(genbuf, buf, "���");
		showAt(t_lines-2, 4, "���ɹ�����л��Ա���ٸ���̵�֧�֡�", 1);
	}
	return;
}

static int 
money_contributions()
{
	int ch, money, money2, quit = 0, count = 0;
	void *buffer = NULL;
	size_t filesize;
	char title[STRLEN], buf[256];

	struct MC_Jijin clist1[]= {
		{"millionaires", "���̻���"},
		{"BMYbeg", "ؤ�����"},
		{"BMYRober", "�ڰ����"},
		{"BMYpolice", "�������"},
		{"BMYKillersky", "ɱ�ֻ���"},
		{"", ""}
	};
	struct MC_Jijin *clist2;

	while (!quit) {
		nomoney_show_stat("���̾��칫��");
		showAt(6, 4, "�׳�һ�ݰ���", 0);
		showAt(t_lines - 1, 0, 
			"\033[1;44m ѡ�� \033[1;46m [1]���ɻ��� [2]������ [3]���Ƽ����а� [4]����ȫ���Ʋ� [Q]�뿪             \033[m", 0);
		ch = igetkey();
		switch (ch) {
		case '1':
			doContributions(clist1);
			break;

		case '2':
			count = get_num_records(MC_JIJIN_CTRL_FILE, sizeof(struct MC_Jijin));
			filesize = sizeof(struct MC_Jijin) * count;
			clist2 = loadData(MC_JIJIN_CTRL_FILE, buffer, filesize);
			if (clist2 == (void *) -1)
				break;
			doContributions(clist2);
			break;
			
		case '3':	
			clear();
			showAt(4, 4, "\033[1;32m����\033[m", 1);
			break;
			
		case '4':
			showAt(5, 0, 
				"[1;32m��ȷ��Ǯ���ڿڴ����֣������������ģ�׼�����ƺ쳾�Ĵ�Կ��׿�ȫ��[m\n"
				"[1;32m�����ɽ������ô��[m\n"
				"[1;32m����ȫ���Ʋ�������millionaires��Ϊ��ȷѣ��ʽ����ڽ���ϣ��Сѧ[m\n"
				"[1;32m��Ԯ�����̲����ߣ��Լ�����̹ɣ���ǡ��ޱ��ǵȹ�ƶ��[m\n"
				"[1;31mע�⣺����ֻ������Ȳ������ף���[m\n"
				"[1;33mǮ�������ܵģ�û��Ǯȴ�����ܣ���˼�����а���[m\n"
				, 0);
			move(12, 0);
			if (askyn("ȷ������ȫ���Ʋ���? ", NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				money2 = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
				
				if (money + money2 == 0){
					showAt(15, 0, "ûǮ�Ͳ�������������~", 1);
					break;
				}
				
				saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
				saveValue("millionaires", MONEY_NAME, money, MAX_MONEY_NUM);
				saveValue(currentuser.userid, "ctr_millionaires", money, MAX_CTRBT_NUM);
				saveValue(currentuser.userid, CREDIT_NAME, -money2, MAX_MONEY_NUM);
				saveValue("millionaires", CREDIT_NAME, money2, MAX_MONEY_NUM);
				saveValue(currentuser.userid, "ctr_millionaires", money2, MAX_CTRBT_NUM);

				sprintf(title, "%s����ȫ���Ʋ�", currentuser.userid);
				sprintf(buf, "%s����ȫ���Ʋ�:\n�ֽ�%d����ٸ��\n���%d����ٸ��", currentuser.userid, money, money2);
				millionairesrec(title, buf, "���");

				sprintf(title, "[����] ���̻����յ�����%s�ľ��", currentuser.userid);
				sprintf(buf,"��л%s�����ٸ���̾�����ȫ���Ʋ������������ʾ���ߵľ��⣡\n"
						"��ףԸ��������˳����", currentuser.userid);
				deliverreport(title, buf);
		
				showAt(15, 0, "���!", 1);
			}
			break;
			
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}


static int 
money_office()
{
	int ch, quit = 0;
	char uident[IDLEN + 1];
	
	while (!quit) {
		nomoney_show_stat("����ٸ���̹�������");
		showAt(6, 4, "���̹������Ļ�ӭ�㣡", 0);
		showAt(t_lines - 1, 0, 
			"\033[1;44m ѡ�� \033[1;46m [1]���칫�� [2]�ŷð칫�� [3]���� [4]������ [5]�ܹܰ칫�� [Q]�뿪       \033[m", 0);
		ch = igetkey();
		switch (ch) {
		case '1':
			money_contributions();
			break;

		case '2':
			if (!HAS_PERM(PERM_POST))
				break;
			move(6, 4);
			if (askyn("ȷ��Ҫ������? ", NA, NA) == YEA) 
				m_send("millionaires");
			break;
			
		case '3':	
			showAt(6, 4, "����̽ͷ̽�������������ӣ�������������������Ǿ͵���AK-47��\n"
						"Ƥ���˰ɣ�С�ĵ羯����", 1);
			break;

		case '4':
			money_postoffice();
			break;
			
		case '5':
			nomoney_show_stat("�����ܹܰ칫��");
			whoTakeCharge2(11, uident);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("ֵ������%s��ס���㣬˵��:�����������ڿ��ᣬ�����Ĵ�תת�ɡ���",
				     uident);
				pressanykey();
				break;
			}
			break;
		
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

