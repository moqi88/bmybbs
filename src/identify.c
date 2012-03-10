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
// 用户绑定操作的入口
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
        prints("您已经成功完成了验证手续!\n");
        prints("欢迎使用本站服务!");
        pressreturn();
        return 0;
    }

    setactivefile(genbuf,cuser->userid,ACTIVE_FILE);
    //如果文件存在，择曾经获取过验证码
    if (access(genbuf, 0)==0) {
        clear();
        move(2, 0);
        prints("您已经进行过获取验证码操作，但未进行验证.\n");
        prints("现在继续验证操作请输入 'Y' ，更换验证方式或重新获取验证码请输入 'N' \n");
        getdata(4, 0, "Y/N? [Y] >> ", style, 2, DOECHO, NULL, true);
        if (*style!='N' && *style!='n') {
            continue_active(cuser);
            return 1;
        }
    }
    clear();
    move(2, 0);
    prints("您还没有进行实名制认证，您在本站将没有发文权限。\n");
    prints("您的其余权利将不受影响。为了正常使用本站服务，我们建议您现在进行认证\n\n");
    prints("[1] ac.cn信箱认证\n");
    prints("[2] 手机号码认证\n");
    prints("[3] 上传身份证件认证(暂未开通)\n");
    prints("[4] 我不想认证\n");

    getdata(10, 0, "请选择认证方式 >> ",  style, 2, DOECHO, NULL, true);
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

//曾经获取过验证码的用户继续验证进程
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
    //读取曾经获取的验证码
    response=get_active_code(cuser->userid, code, value, &style);
    if (response==FILE_NOT_FOUND) {
        clear();
        move(2, 0);
        prints("未找到曾经验证的记录，请重新获取验证码!\n");
        pressreturn();
        return 0;
    }
    prints("您的验证方式:     \t%s\n", style_to_str(style));
    prints("您的验证码发送至:\t%s\n", value);
    prints("请输入您的验证码，放弃验证请直接回车:\n");
    getdata(16 ,0, ">> ", incode,CODELEN+1,DOECHO,NULL ,true);
    while (strcmp(code, incode)!=0) {
        if (!incode[0]) {
            return 0;
        }
        getdata(16 ,0, "验证码错误，请重新输入 >> ", incode,CODELEN+1,DOECHO,NULL ,true);
    }
   if (query_record_num(value, style)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("您已经验证过三个id，无法再用于验证了!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
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
        prints("  验证成功!");
	 //本校学生
	 str_to_lowercase(value);
	 if (style==MAIL_ACTIVE && !strcmp(strstr(value, "@")+1, "mails.gucas.ac.cn")) {
	 	cuser->userlevel |= PERM_DEFAULT;
		move(5, 0);
		prints("您已经完成注册程序。欢迎使用本站的服务!");
	 }
	 else if (!HAS_PERM(cuser, PERM_LOGINOK)){
	 	move(5, 0);
		prints("您还没有填写注册单。请填写注册单，完成注册手续");
		x_fillform();
		return 1;
	 }
        pressreturn();
        return 1;
    }
    clear();
    move(3, 0);
    prints("  验证失败!");
    pressreturn();
    return 0;
}

//邮件验证
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
    prints("现在将进行信箱验证，请准备好ac.cn域名的电子信箱 ...\n");
    prints("系统将向您的信箱发送一封信件，每个信箱可以验证3个ID。\n\n");
    prints("请输入信箱地址:");
    getdata(6, 0, ">> ", mbox, VALUELEN, DOECHO, NULL, true);
    //统一变换大大写避免大小写的一致性问题
    str_to_lowercase(mbox);
    while (invalid_mail(mbox)) {
        clear();
        move(2, 0);
        prints("您的信箱地址不属于ac.cn域或为非法地址 ...\n");
        getdata(4, 0, "是否重新输入?(Y/N) [Y]  >> ", an, 2, DOECHO, NULL, true);
        if (*an != 'n' && *an != 'N') {
            move(5, 0);
            prints("请输入信箱地址:");
            getdata(6, 0, ">> ", mbox, VALUELEN, DOECHO, NULL, true);
            str_to_lowercase(mbox);
        } else {
            return 0;
        }
    }
    if (!strcmp(strstr(mbox, "@")+1, "mails.gucas.ac.cn")) {
		move(7, 0);
		prints("您的身份是本校学生或校友，请输入学号验证:");
		getdata(8, 0, ">> ", stunum, VALUELEN, DOECHO, NULL, true);
		if (!valid_stunum(mbox, stunum)) {
			clear();
			move(3, 0);
			prints("您输入的学号和信箱不对应! 请检查输入");
			pressreturn();
			return 0;
		}
		strcpy(act_data.email, mbox);
		strcpy(act_data.stdnum,stunum);
		get_official_data(&act_data);
    	}
    //第一次验证信箱数目约束
    if (query_record_num(mbox, MAIL_ACTIVE)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("您的信箱已经验证过三个id，无法再用于验证了!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    //生成随机码
    gencode(code);
    //生成的随机码写入用户的home目录
    set_active_code(cuser->userid, code, mbox, MAIL_ACTIVE);

    //发送验证信函
    send_active_mail(mbox, code, cuser->userid, session);
    //记录至数据库

    strcpy(act_data.email, mbox);
	
    int ret = write_active(&act_data);
    if (ret!=WRITE_SUCCESS && ret!=UPDATE_SUCCESS) {
        clear();
        move(3, 0);
        prints("数据库保存错误，请联系SYSOP!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
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
    prints("认证信函已经寄出，内含验证码，请查收!\n");
    pressreturn();
    continue_active(cuser);
    return 0;
}

//手机号验证
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
    prints("现在将进行手机验证 ...\n");
    prints("系统将向您的手机发送一条短信，每个手机号可以验证3个ID。\n\n");
    prints("请输入手机号码:");
    getdata(6, 0, ">> ", phone, VALUELEN, DOECHO, NULL, true);

    //第一次验证信箱数目约束
    if (query_record_num(phone, PHONE_ACTIVE)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("您的手机已经验证过三个id，无法再用于验证了!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    //生成随机码
    gencode(code);
    //生成的随机码写入用户的home目录
    set_active_code(cuser->userid, code, phone, PHONE_ACTIVE);

    //发送验证信函
    send_active_msg(phone, code, cuser->userid);
    //记录至数据库

    strcpy(act_data.phone, phone);
	
    int ret = write_active(&act_data);
    if (ret!=WRITE_SUCCESS && ret!=UPDATE_SUCCESS) {
        clear();
        move(3, 0);
        prints("数据库保存错误，请联系SYSOP!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
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
    prints("认证短信已经发出，内含验证码，请查收!\n");
    pressreturn();
    continue_active(cuser);

    return 0;
}

//上传证件验证
int active_manual(struct userec* cuser, session_t* session)
{
    clear();
    move(2, 0);
    prints("此功能还在开发中!");
    pressreturn();
    return 1;
}

*/



//验证信息管理的入口
int x_active_manager()
{
    char an[2];
    char style[2];
    char userid[IDLEN+2];
    char value[VALUELEN];
    if (!HAS_PERM(getCurrentUser(), PERM_ACCOUNTS) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        clear();
        move(2, 0);
        prints("你没有管理权限!");
        return 0;
    }
    clear();
    move(2, 0);
    prints("[1] 不进行认证而强行激活某用户\n");
    prints("[2] 查询实名认证记录\n");
    prints("[3] 修改实名认证记录(保持已认证状态)\n");
    prints("[4] 删除实名认证记录(释放信箱)\n");
    prints("[5] 查询某记录下绑定的id\n");
    prints("[6] 离开");

    getdata(10 ,0, ">> ", an,2,DOECHO,NULL ,true);


    else if (!strcmp(an, "1")) {
        clear();
        move(2, 0);
        prints("输入要激活的id: ");
        usercomplete(" ", userid);
        force_comfirm(userid);
        return 1;
    } else if (!strcmp(an, "2")) {
        clear();
        move(1, 0);
        prints("输入要查询的id: ");
        usercomplete(" ", userid);
        query_active(userid);
        return 1;
    } else if (!strcmp(an, "3")) {
        clear();
        move(1, 0);
        prints("输入要修改的id: ");
        usercomplete(" ", userid);
        update_active(userid);
        return 1;
    } else if (!strcmp(an, "4")) {
        clear();
        move(1, 0);
        prints("输入要解除认证的id: ");
	getdata(3, 0, ">> ", userid, VALUELEN, DOECHO, NULL, true);
        delete_active(userid);
        return 1;
    } else if (!strcmp(an, "5")) {
        clear();
        move(1, 0);
	 prints("输入要查询的%s:\n", style_to_str(atoi(1)));
	 getdata(3, 0, ">> ", value, VALUELEN, DOECHO, NULL, true);
        query_value(value, atoi(1));
        return 1;
    } 
    return 0;
}

//查询某id的验证信息
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
        prints("用户名   :\t%s\n", act_data.userid);
        prints("姓名     :\t%s\n", act_data.name);
        prints("%s信箱     :\t%s\n", act_data.status==1?"\033[31m":"\033[37m", act_data.email);
        prints("%s电话     :\t%s\n", act_data.status==2?"\033[31m":"\033[37m", act_data.phone);
        prints("%s身份证号 :\t%s\n", act_data.status==3?"\033[31m":"\033[37m", act_data.idnum);
        prints("学号     :\t%s\n", act_data.stdnum);
        prints("培养单位:\t%s\n", act_data.dept);
        prints("认证时间 :\t%s\n", act_data.status<1?"":act_data.uptime);
        prints("认证类型 :\t%s\n", active_style_str[act_data.status]);
        if (act_data.status==IDCARD_ACTIVE) {
            //显示图片地址
        }
        if (act_data.status>0 && act_data.status<4) {
            get_active_value(value, &act_data);
            prints("\n----------------------------------------------------------\n\n");
            prints("同认证记录下的其他ID:\n");
            sprintf(sqlbuf,"SELECT userid,status FROM %s WHERE %s='%s';" , USERREG_TABLE, active_style_str[act_data.status], value);
            mysql_real_query(s, sqlbuf, strlen(sqlbuf));
            res = mysql_store_result(s);
            //列出同记录下的其他id
            for (i=0; i<mysql_num_rows(res); ++i) {
                row = mysql_fetch_row(res);
                prints("%-12s\t%s\n", row[0], style_to_str(atoi(row[1])));
            }
        }
    } else {
        move(5, 0);
        prints("未找到用户 %s 的认证与绑定信息!", userid);
    }
    mysql_close(s);
    pressreturn();
    return 1;		
}

//强制激活某用户
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
        prints("此用户已经激活!\n");
        pressreturn();
        return 0;
    }
    move(5, 0);
    prints("确定操作?\n");
    getdata(8, 0, "Y/N [N] >> ", an, 2, DOECHO, NULL, true);
    if (*an == 'Y' || *an == 'y') {
        strcpy(act_data.userid, userid);
	 strcpy(act_data.operator, currentuser->userid);
	 act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser->lasthost);
	 write_active(&act_data);
	 cuser->userlevel |= PERM_DEFAULT;
	 //版面记录没有写
        pressreturn();
        return 1;
    }
    return 0;
}

//删除激活记录
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
        prints("此用户并未激活!\n");
        //anyway，仍然去掉激活记录
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, getCurrentUser()->userid);
        write_active(&act_data);
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    */
    clrtobot();
    move(5, 0);
    prints("确定取消认证记录?\n");
    getdata(6, 0, "Y/N [N] >> ", an, 2, DOECHO, NULL, true);
    if (*an == 'Y' || *an == 'y') {
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, currentuser->userid);
        write_active(&act_data);
        //cuser->flags &= ~ACTIVATED_FLAG;
        //setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
        //unlink(genbuf);
	 //版面记录没有写
        pressreturn();
        return 1;
    }
    return 0;
}

//更新激活信息记录
//一般情况下应该删除激活记录再要求用户重新手工激活
//而不是使用本功能
int update_active(char* userid)
{
    struct userec* cuser;
    struct active_data act_data;
    char an[2];
    int response;

    getuser(userid, &cuser);
    response=read_active(userid, &act_data);

    clrtobot();
    getfield(5, "", "真实姓名", act_data.name);
    getfield(6, "", "工作单位", act_data.dept);
    getfield(7, "", "Email", act_data.email);
    getfield(8, "", "手机号", act_data.phone);
    getfield(9, "", "身份证件号码", act_data.idnum);
    getfield(10, "", "学号", act_data.stdnum);
    str_to_lowercase(act_data.email);
    prints("确定操作?\n");
    getdata(12, 0, "Y/N [N] >> ", an, STRLEN, DOECHO, NULL, YEA);
    if (*an == 'Y' || *an == 'y') {
        //记录操作站务的id
        act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser->lasthost);
	 strcpy(act_data.operator, currentuser->userid);
	 //版面记录没有写
	 write_active(&act_data);
        pressreturn();
        return 1;
    }
    return 0;
}


//查询某记录下绑定的id
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
    prints("同认证记录下的ID:\n");
    sprintf(sqlbuf,"SELECT userid,status FROM %s WHERE lower(%s)='%s';" , USERREG_TABLE, active_style_str[style], value);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    //列出同记录下的其他id
    for (i=0; i<mysql_num_rows(res); ++i) {
        row = mysql_fetch_row(res);
        prints("%-12s\t%s\n", row[0], style_to_str(atoi(row[1])));
    }
    mysql_close(s);
    pressreturn();
    return 1;
}
    




