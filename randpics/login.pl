#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $username = $req -> param ("UN");
my $password = $req -> param ("PW");
my $passfile = "/home/bbs/.PASSWDS";
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs";
my $cgibin = "http://202.117.1.8/cgi-bin/bbs";
my $loginadd = "http://202.117.1.8/picmgr.htm";
my $lock_sh = 1;
my $lock_un = 8;
my $randnum = int (rand () * 1000000000000);
while ($randnum <= 100000000000)
{
    $randnum = int (rand () * 1000000000000);
}
$username = lc ($username);
my $cookie = $username.$randnum;
print "Set-Cookie:id=$cookie\n";
print $req -> header ({-charset=>gb2312});
if (!$username || $username =~ m/[^a-z]/ || $password =~ m/\s/)
{
    print "����!IDֻ�ܰ�����ĸ�����벻�ܰ����ո�!";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    die;
}#��������ʽ�������ID���������
open (AD,"$bbshome/artdesign") || die;
chomp (my $allow = <AD>);
while (lc ($allow) ne $username)
{
    chomp ($allow = <AD>);
    unless ($allow)
    {
	print "û�е�½Ȩ��,����ϵ����վ��";
	close (AD);
	die;
    }
}#�ж���û��������Ȩ��
close (AD);
open (PW,$passfile) || die "���󣡴��ļ�$passfileʧ��!";
until (flock (PW,$lock_sh))
{
    sleep 1;
}
read (PW,$id,14);
until (($lastchar = chop ($id)) ne "\0")
{
    ;
}
$id .= $lastchar;
while (lc ($id) ne $username)
{
    seek (PW,438,1);
    read (PW,$id,14);
    unless ($id)
    {
	print "�޴�ID,�����µ�½";
	print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
	close (PW);
	die;
    }
    until (($lastchar = chop ($id)) ne "\0")
    {
	;
    }
    $id .= $lastchar;
}#����ID
seek (PW,32,1);
read (PW,$passwd,14);
until (($lastchar = chop ($passwd)) ne "\0")
{
    ;
}
$passwd .= $lastchar;
until (flock (PW,$lock_un))
{
    sleep 1;
}#��������
close (PW);
my $salt = substr ($passwd,0,2);
my $chkpw = crypt ($password,$salt);
if ($chkpw eq $passwd)#�ж������Ƿ���ȷ
{
    open (SE,">/tmp/$username.se") || die "�����޷�����session�ļ�!";
    my $acttime = time;
    my $remote_ip = $req -> remote_addr ();
    print SE $remote_ip,"\n";
    print SE $randnum,"\n";
    print SE $acttime,"\n";
    close (SE);
    print "��½�ɹ�";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">"; 
}
else
{
    print "��½ʧ�ܣ�����ID�����룬������ϵ����վ����<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
}
