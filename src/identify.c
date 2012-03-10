#include "bbs.h"
#include "identify.h"

int x_active_user(void);
int active_mail(struct userec* cuser, session_t* session);
int active_phone(struct userec* cuser, session_t* session);
int active_manual(struct userec* cuser, session_t* session);

int continue_active(struct userec* cuser);

int x_active_manager();
int active_manual_confirm();
int query_active(char* userid);
int force_comfirm(char* userid);
int delete_active(char* userid);
int update_active(char* userid);
int query_value(char* value, int style);
int update_email(char* value);


static const char *active_style_str[] = {"", "email", "phone", "idnum", NULL};

/*
// �û��󶨲��������
int x_active_user(void)
{
    char style[4];
    char genbuf[STRLEN];
    struct userec* cuser;
    struct session_t* session;
    cuser=getCurrentUser();
    session=getSession();
    if (cuser->flags & ACTIVATED_FLAG) {
        clear();
        move(2, 0);
        prints("���Ѿ��ɹ��������֤����!\n");
        prints("��ӭʹ�ñ�վ����!");
        pressreturn();
        return 0;
    }

    setactivefile(genbuf,cuser->userid,ACTIVE_FILE);
    //����ļ����ڣ���������ȡ����֤��
    if (access(genbuf, 0)==0) {
        clear();
        move(2, 0);
        prints("���Ѿ����й���ȡ��֤���������δ������֤.\n");
        prints("���ڼ�����֤���������� 'Y' ��������֤��ʽ�����»�ȡ��֤�������� 'N' \n");
        getdata(4, 0, "Y/N? [Y] >> ", style, 2, DOECHO, NULL, true);
        if (*style!='N' && *style!='n') {
            continue_active(cuser);
            return 1;
        }
    }
    clear();
    move(2, 0);
    prints("����û�н���ʵ������֤�����ڱ�վ��û�з���Ȩ�ޡ�\n");
    prints("��������Ȩ��������Ӱ�졣Ϊ������ʹ�ñ�վ�������ǽ��������ڽ�����֤\n\n");
    prints("[1] ac.cn������֤\n");
    prints("[2] �ֻ�������֤\n");
    prints("[3] �ϴ����֤����֤(��δ��ͨ)\n");
    prints("[4] �Ҳ�����֤\n");

    getdata(10, 0, "��ѡ����֤��ʽ >> ",  style, 2, DOECHO, NULL, true);
    if (!strcmp(style, "1")) {
        active_mail(cuser, session);
        return 1;
    } else if (!strcmp(style, "2")) {
        active_phone(cuser, session);
        return 1;
    } else if (!strcmp(style, "3")) {
        active_manual(cuser, session);
        return 1;
    }

    return 0;
}

//������ȡ����֤����û�������֤����
int continue_active(struct userec* cuser)
{
    char value[VALUELEN];
    char code[CODELEN+1];
    char incode[CODELEN+1];
    char genbuf[STRLEN];
    int style;
    int response;
    struct active_data act_data;

    move(13, 0);
    clrtobot();
    //��ȡ������ȡ����֤��
    response=get_active_code(cuser->userid, code, value, &style);
    if (response==FILE_NOT_FOUND) {
        clear();
        move(2, 0);
        prints("δ�ҵ�������֤�ļ�¼�������»�ȡ��֤��!\n");
        pressreturn();
        return 0;
    }
    prints("������֤��ʽ:     \t%s\n", style_to_str(style));
    prints("������֤�뷢����:\t%s\n", value);
    prints("������������֤�룬������֤��ֱ�ӻس�:\n");
    getdata(16 ,0, ">> ", incode,CODELEN+1,DOECHO,NULL ,true);
    while (strcmp(code, incode)!=0) {
        if (!incode[0]) {
            return 0;
        }
        getdata(16 ,0, "��֤��������������� >> ", incode,CODELEN+1,DOECHO,NULL ,true);
    }
   if (query_record_num(value, style)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("���Ѿ���֤������id���޷���������֤��!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    read_active(cuser->userid, &act_data);
    act_data.status=style;
    strcpy(act_data.ip, cuser->lasthost);
    response=write_active(&act_data);

    if (response==WRITE_SUCCESS || response==UPDATE_SUCCESS) {
        cuser->flags |= ACTIVATED_FLAG;
        clear();
        move(3, 0);
        prints("  ��֤�ɹ�!");
	 //��Уѧ��
	 str_to_lowercase(value);
	 if (style==MAIL_ACTIVE && !strcmp(strstr(value, "@")+1, "mails.gucas.ac.cn")) {
	 	cuser->userlevel |= PERM_DEFAULT;
		move(5, 0);
		prints("���Ѿ����ע����򡣻�ӭʹ�ñ�վ�ķ���!");
	 }
	 else if (!HAS_PERM(cuser, PERM_LOGINOK)){
	 	move(5, 0);
		prints("����û����дע�ᵥ������дע�ᵥ�����ע������");
		x_fillform();
		return 1;
	 }
        pressreturn();
        return 1;
    }
    clear();
    move(3, 0);
    prints("  ��֤ʧ��!");
    pressreturn();
    return 0;
}

//�ʼ���֤
int active_mail(struct userec * cuser, session_t* session)
{
    char mbox[VALUELEN];
    char stunum[VALUELEN];
    char an[2];
    char code[CODELEN+1];
    //char incode[CODELEN+1];
    int response;
    char genbuf[STRLEN];
    struct active_data act_data;

    memset(&act_data, 0, sizeof(struct active_data));
    strcpy(act_data.userid, cuser->userid);
    strcpy(act_data.ip, cuser->lasthost);
    strcpy(act_data.operator, cuser->userid);
    act_data.status=0;
	
    clear();
    move(2, 0);
    prints("���ڽ�����������֤����׼����ac.cn�����ĵ������� ...\n");
    prints("ϵͳ�����������䷢��һ���ż���ÿ�����������֤3��ID��\n\n");
    prints("�����������ַ:");
    getdata(6, 0, ">> ", mbox, VALUELEN, DOECHO, NULL, true);
    //ͳһ�任���д�����Сд��һ��������
    str_to_lowercase(mbox);
    while (invalid_mail(mbox)) {
        clear();
        move(2, 0);
        prints("���������ַ������ac.cn���Ϊ�Ƿ���ַ ...\n");
        getdata(4, 0, "�Ƿ���������?(Y/N) [Y]  >> ", an, 2, DOECHO, NULL, true);
        if (*an != 'n' && *an != 'N') {
            move(5, 0);
            prints("�����������ַ:");
            getdata(6, 0, ">> ", mbox, VALUELEN, DOECHO, NULL, true);
            str_to_lowercase(mbox);
        } else {
            return 0;
        }
    }
    if (!strcmp(strstr(mbox, "@")+1, "mails.gucas.ac.cn")) {
		move(7, 0);
		prints("��������Ǳ�Уѧ����У�ѣ�������ѧ����֤:");
		getdata(8, 0, ">> ", stunum, VALUELEN, DOECHO, NULL, true);
		if (!valid_stunum(mbox, stunum)) {
			clear();
			move(3, 0);
			prints("�������ѧ�ź����䲻��Ӧ! ��������");
			pressreturn();
			return 0;
		}
		strcpy(act_data.email, mbox);
		strcpy(act_data.stdnum,stunum);
		get_official_data(&act_data);
    	}
    //��һ����֤������ĿԼ��
    if (query_record_num(mbox, MAIL_ACTIVE)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("���������Ѿ���֤������id���޷���������֤��!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    //���������
    gencode(code);
    //���ɵ������д���û���homeĿ¼
    set_active_code(cuser->userid, code, mbox, MAIL_ACTIVE);

    //������֤�ź�
    send_active_mail(mbox, code, cuser->userid, session);
    //��¼�����ݿ�

    strcpy(act_data.email, mbox);
	
    int ret = write_active(&act_data);
    if (ret!=WRITE_SUCCESS && ret!=UPDATE_SUCCESS) {
        clear();
        move(3, 0);
        prints("���ݿⱣ���������ϵSYSOP!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        unlink(genbuf);
        pressreturn();
        return 0;
    }
#endif
#if 0
    move(8, 0);
    prints("%s, write ret: %d", code, ret);
#endif
#ifdef KYXK
    move(10, 0);
    prints("��֤�ź��Ѿ��ĳ����ں���֤�룬�����!\n");
    pressreturn();
    continue_active(cuser);
    return 0;
}

//�ֻ�����֤
int active_phone(struct userec* cuser, session_t* session)
{
    char phone[VALUELEN];
    char an[2];
    char code[CODELEN+1];
    int response;
    char genbuf[STRLEN];
    struct active_data act_data;

    memset(&act_data, 0, sizeof(struct active_data));
    strcpy(act_data.userid, cuser->userid);
    strcpy(act_data.ip, cuser->lasthost);
    strcpy(act_data.operator, cuser->userid);
    act_data.status=0;
	
    clear();
    move(2, 0);
    prints("���ڽ������ֻ���֤ ...\n");
    prints("ϵͳ���������ֻ�����һ�����ţ�ÿ���ֻ��ſ�����֤3��ID��\n\n");
    prints("�������ֻ�����:");
    getdata(6, 0, ">> ", phone, VALUELEN, DOECHO, NULL, true);

    //��һ����֤������ĿԼ��
    if (query_record_num(phone, PHONE_ACTIVE)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("�����ֻ��Ѿ���֤������id���޷���������֤��!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    //���������
    gencode(code);
    //���ɵ������д���û���homeĿ¼
    set_active_code(cuser->userid, code, phone, PHONE_ACTIVE);

    //������֤�ź�
    send_active_msg(phone, code, cuser->userid);
    //��¼�����ݿ�

    strcpy(act_data.phone, phone);
	
    int ret = write_active(&act_data);
    if (ret!=WRITE_SUCCESS && ret!=UPDATE_SUCCESS) {
        clear();
        move(3, 0);
        prints("���ݿⱣ���������ϵSYSOP!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        unlink(genbuf);
        pressreturn();
        return 0;
    }
#endif
#if 0
    move(8, 0);
    prints("%s, write ret: %d", code, ret);
#endif
#ifdef KYXK
    move(10, 0);
    prints("��֤�����Ѿ��������ں���֤�룬�����!\n");
    pressreturn();
    continue_active(cuser);

    return 0;
}

//�ϴ�֤����֤
int active_manual(struct userec* cuser, session_t* session)
{
    clear();
    move(2, 0);
    prints("�˹��ܻ��ڿ�����!");
    pressreturn();
    return 1;
}

*/



//��֤��Ϣ��������
int x_active_manager()
{
    char an[2];
    char style[2];
    char userid[IDLEN+2];
    char value[VALUELEN];
    if (!HAS_PERM(getCurrentUser(), PERM_ACCOUNTS) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        clear();
        move(2, 0);
        prints("��û�й���Ȩ��!");
        return 0;
    }
    clear();
    move(2, 0);
    prints("[1] ��������֤��ǿ�м���ĳ�û�\n");
    prints("[2] ��ѯʵ����֤��¼\n");
    prints("[3] �޸�ʵ����֤��¼(��������֤״̬)\n");
    prints("[4] ɾ��ʵ����֤��¼(�ͷ�����)\n");
    prints("[5] ��ѯĳ��¼�°󶨵�id\n");
    prints("[6] �뿪");

    getdata(10 ,0, ">> ", an,2,DOECHO,NULL ,true);


    else if (!strcmp(an, "1")) {
        clear();
        move(2, 0);
        prints("����Ҫ�����id: ");
        usercomplete(" ", userid);
        force_comfirm(userid);
        return 1;
    } else if (!strcmp(an, "2")) {
        clear();
        move(1, 0);
        prints("����Ҫ��ѯ��id: ");
        usercomplete(" ", userid);
        query_active(userid);
        return 1;
    } else if (!strcmp(an, "3")) {
        clear();
        move(1, 0);
        prints("����Ҫ�޸ĵ�id: ");
        usercomplete(" ", userid);
        update_active(userid);
        return 1;
    } else if (!strcmp(an, "4")) {
        clear();
        move(1, 0);
        prints("����Ҫ�����֤��id: ");
	getdata(3, 0, ">> ", userid, VALUELEN, DOECHO, NULL, true);
        delete_active(userid);
        return 1;
    } else if (!strcmp(an, "5")) {
        clear();
        move(1, 0);
	 prints("����Ҫ��ѯ��%s:\n", style_to_str(atoi(1)));
	 getdata(3, 0, ">> ", value, VALUELEN, DOECHO, NULL, true);
        query_value(value, atoi(1));
        return 1;
    } 
    return 0;
}

//��ѯĳid����֤��Ϣ
int query_active(char* userid)
{
    char sqlbuf[512];
    struct active_data act_data;
    char value[VALUELEN];
    int i;
	
    MYSQL *s = NULL;
    MYSQL_RES *res;
    MYSQL_ROW row;
    s = mysql_init(s);
    if (!my_connect_mysql(s)) {
        return 0;
    }

    i=read_active(userid, &act_data);
    if (i>0) {
        str_to_lowercase(act_data.email);
	 //if (act_data.status==MAIL_ACTIVE && !strcmp(strstr(act_data.email, "@")+1, "mails.gucas.ac.cn")) {
		char gname[VALUELEN];
		char gdept[VALUELEN];
		u2g(act_data.name, strlen(act_data.name), gname, VALUELEN);
		u2g(act_data.dept, strlen(act_data.dept), gdept, VALUELEN);
		strcpy(act_data.name, gname);
		strcpy(act_data.dept, gdept);
       // }
        clrtobot();
        move(3, 0);
        prints("�û���   :\t%s\n", act_data.userid);
        prints("����     :\t%s\n", act_data.name);
        prints("%s����     :\t%s\n", act_data.status==1?"\033[31m":"\033[37m", act_data.email);
        prints("%s�绰     :\t%s\n", act_data.status==2?"\033[31m":"\033[37m", act_data.phone);
        prints("%s���֤�� :\t%s\n", act_data.status==3?"\033[31m":"\033[37m", act_data.idnum);
        prints("ѧ��     :\t%s\n", act_data.stdnum);
        prints("������λ:\t%s\n", act_data.dept);
        prints("��֤ʱ�� :\t%s\n", act_data.status<1?"":act_data.uptime);
        prints("��֤���� :\t%s\n", active_style_str[act_data.status]);
        if (act_data.status==IDCARD_ACTIVE) {
            //��ʾͼƬ��ַ
        }
        if (act_data.status>0 && act_data.status<4) {
            get_active_value(value, &act_data);
            prints("\n----------------------------------------------------------\n\n");
            prints("ͬ��֤��¼�µ�����ID:\n");
            sprintf(sqlbuf,"SELECT userid,status FROM %s WHERE %s='%s';" , USERREG_TABLE, active_style_str[act_data.status], value);
            mysql_real_query(s, sqlbuf, strlen(sqlbuf));
            res = mysql_store_result(s);
            //�г�ͬ��¼�µ�����id
            for (i=0; i<mysql_num_rows(res); ++i) {
                row = mysql_fetch_row(res);
                prints("%-12s\t%s\n", row[0], style_to_str(atoi(row[1])));
            }
        }
    } else {
        move(5, 0);
        prints("δ�ҵ��û� %s ����֤�����Ϣ!", userid);
    }
    mysql_close(s);
    pressreturn();
    return 1;		
}

//ǿ�Ƽ���ĳ�û�
int force_comfirm(char* userid)
{
    struct userec* cuser;
    struct active_data act_data;
    char an[2];
    int response;

    response=getuser(userid, &cuser);
    memset(&act_data, 0, sizeof(struct active_data));

    if (cuser->userlevel& PERM_LOGINOK) {
        clrtobot();
        move(5, 0);
        prints("���û��Ѿ�����!\n");
        pressreturn();
        return 0;
    }
    move(5, 0);
    prints("ȷ������?\n");
    getdata(8, 0, "Y/N [N] >> ", an, 2, DOECHO, NULL, true);
    if (*an == 'Y' || *an == 'y') {
        strcpy(act_data.userid, userid);
	 strcpy(act_data.operator, currentuser->userid);
	 act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser->lasthost);
	 write_active(&act_data);
	 cuser->userlevel |= PERM_DEFAULT;
	 //�����¼û��д
        pressreturn();
        return 1;
    }
    return 0;
}

//ɾ�������¼
int delete_active(char* userid)
{
    struct userec* cuser;
    char an[2];
    char genbuf[STRLEN];
    struct active_data act_data;

    getuser(userid, &cuser);
    read_active(userid, &act_data);
   // s = mysql_init(s);
   /*
    if (!(cuser->flags & ACTIVATED_FLAG)) {
        clrtobot();
        move(5, 0);
        prints("���û���δ����!\n");
        //anyway����Ȼȥ�������¼
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, getCurrentUser()->userid);
        write_active(&act_data);
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    */
    clrtobot();
    move(5, 0);
    prints("ȷ��ȡ����֤��¼?\n");
    getdata(6, 0, "Y/N [N] >> ", an, 2, DOECHO, NULL, true);
    if (*an == 'Y' || *an == 'y') {
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, currentuser->userid);
        write_active(&act_data);
        //cuser->flags &= ~ACTIVATED_FLAG;
        //setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //ɾ���û�Ŀ¼�µ���֤���ļ�
        //unlink(genbuf);
	 //�����¼û��д
        pressreturn();
        return 1;
    }
    return 0;
}

//���¼�����Ϣ��¼
//һ�������Ӧ��ɾ�������¼��Ҫ���û������ֹ�����
//������ʹ�ñ�����
int update_active(char* userid)
{
    struct userec* cuser;
    struct active_data act_data;
    char an[2];
    int response;

    getuser(userid, &cuser);
    response=read_active(userid, &act_data);

    clrtobot();
    getfield(5, "", "��ʵ����", act_data.name);
    getfield(6, "", "������λ", act_data.dept);
    getfield(7, "", "Email", act_data.email);
    getfield(8, "", "�ֻ���", act_data.phone);
    getfield(9, "", "���֤������", act_data.idnum);
    getfield(10, "", "ѧ��", act_data.stdnum);
    str_to_lowercase(act_data.email);
    prints("ȷ������?\n");
    getdata(12, 0, "Y/N [N] >> ", an, STRLEN, DOECHO, NULL, YEA);
    if (*an == 'Y' || *an == 'y') {
        //��¼����վ���id
        act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser->lasthost);
	 strcpy(act_data.operator, currentuser->userid);
	 //�����¼û��д
	 write_active(&act_data);
        pressreturn();
        return 1;
    }
    return 0;
}


//��ѯĳ��¼�°󶨵�id
int query_value(char* value, int style)
{
    char sqlbuf[512];
    int i;

    MYSQL *s = NULL;
    MYSQL_RES *res;
    MYSQL_ROW row;

    s = mysql_init(s);
    if (!my_connect_mysql(s)) {
        return 0;
    }

    clrtobot();
    move(5, 0);
    str_to_lowercase(value);
    prints("ͬ��֤��¼�µ�ID:\n");
    sprintf(sqlbuf,"SELECT userid,status FROM %s WHERE lower(%s)='%s';" , USERREG_TABLE, active_style_str[style], value);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    //�г�ͬ��¼�µ�����id
    for (i=0; i<mysql_num_rows(res); ++i) {
        row = mysql_fetch_row(res);
        prints("%-12s\t%s\n", row[0], style_to_str(atoi(row[1])));
    }
    mysql_close(s);
    pressreturn();
    return 1;
}
    




