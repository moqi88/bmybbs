#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs/bmyMainPic";
my $cgibin = "http://202.117.1.8/cgi-bin/bbs";
my $loginadd = "http://202.117.1.8/picmgr.htm";
my $remote_ip = $req -> remote_addr ();
print $req -> header ({-charset=>'gb2312'});
if ($req -> cookie('id'))#�����û��cookie
{
    my $id = $req -> cookie('id');
    $id =~ /([a-z]*)/g;
    my $username = $1;
    $id =~ /([0-9]*)/g;
    my $checknum = $1;
    unless (-e "/tmp/$username.se")#�����û��session�ļ�
    {
	print "��½��ʱ�������µ�½<br>";
	print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
	die;
    }
    unless (open (SE,"+</tmp/$username.se"))
    {
	print "���ļ�ʧ��<br>";
	print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
	die;
    }
    chomp (my $last_ip = <SE>);
    chomp (my $randnum = <SE>);
    chomp (my $acttime = <SE>);
    my $nowtime = time;
    if (($last_ip == $remote_ip) && ($checknum == $randnum) && (($nowtime - $acttime) < 600))
    {
	seek (SE,length ($randnum) + length ($last_ip) + 2,0) || die;
	print SE time;#���û�г�ʱ��������ʱ��
	close (SE);
	if ($req -> param ())
	{
	    open (PIC,$bbshome."/logpics") || die "����!���ļ��б�ʧ��!";
	    chomp (my $picnum = <PIC>);
	    $picnum =~ s/\D//g;
	    my $del = 0;#��¼ɾ���ļ�������±�
	    my $new = 0;#��¼�����ļ�������±�
	    for (my $i = 0;$i < $picnum;$i ++)
	    {
		chomp ($pics[$i] = <PIC>);
		if ($req -> param ($i))
		{
		    $delpics[$del ++] = $pics[$i];
		}
		else
		{
		    $newpics[$new ++] = $pics[$i];
		}
	    }#��ɾ���ļ��ͱ����ļ��ֱ��������������
	    close (PIC);
	    open (PIC,">$bbshome/logpics") || die $!;
	    print PIC "total:".$new,"\n";
	    for ($i = 0;$i < $new;$i ++)
	    {
		print PIC $newpics[$i],"\n";
	    }#�ѱ����ļ��б�д���ļ�����
	    close (PIC);
	    for ($i = 0;$i < $del;$i ++)
	    {
		rename ($htmpath."/using/".$delpics[$i],$htmpath."/used/".$delpics[$i]) || die $!;
	    }#��ɾ���ļ��Ƶ�used�ļ���
	    print "ɾ���ɹ�<br>";
	    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
	}
	else
	{
	    print "û�в�������<br>";
	    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
	}    
    }#����½�Ƿ��Ѿ���ʱ
    else
    {
	close (SE);
	print "��½��ʱ�������µ�½<br>";
	print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    }
}
else
{
    print "��½��ʱ�������µ�½<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
}
