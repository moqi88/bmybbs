#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs/bmyMainPic";
my $cgibin = "http://202.117.1.8/cgi-bin/bbs";
my $loginadd = "http://202.117.1.8/picmgr.htm";
my $remote_ip = $req -> remote_addr ();
print $req -> header ({-charset=>gb2312});
unless ($req -> cookie('id'))
{
    print "��½��ʱ�������µ�½!<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    die;
}#�����û��cookie
my $id = $req -> cookie('id');
$id =~ /([a-z]*)/g;
my $username = $1;
$id =~ /([0-9]*)/g;
my $checknum = $1;
unless (-e "/tmp/$username.se")
{
    print "��½��ʱ�������µ�½<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
    die;
}#�����û��session�ļ�
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
unless (($last_ip == $remote_ip) && ($checknum == $randnum) && (($nowtime - $acttime) < 600))
{
    print "��½��ʱ�������µ�½<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    close (SE);
    die;
}#�����û�г�ʱ
seek (SE,length ($randnum) + length ($last_ip) + 2,0) || die;
print SE time;#�������ʱ��
close (SE);
opendir (DIR,"$htmpath/used") || die;
my @used = readdir (DIR);
closedir (DIR);
if ($req -> param ())
{
    if ($req -> param ("ADD"))
    {
	my $added = 0;
	for (my $i = 2;$i < @used;$i ++)
	{
	    if ($req -> param ($used[$i]))
	    {
		$toadd[$added ++] = $used[$i];
	    }
	}
	open (PIC,"+<$bbshome/logpics") || die;
	my $picnum = <PIC>;
	chomp ($picnum);
	$picnum =~ s/\D//g;#��������ʽȥ��������Ϣ
	seek (PIC,0,2);
	for (my $i = 0;$i < $added;$i ++)
	{
	    rename ($htmpath."/used/".$toadd[$i],$htmpath."/using/".$toadd[$i]) || die;
	    print PIC $toadd[$i],"\n" || die;
	    print "$toadd[$i]�����","<br>";
	}
	seek (PIC,0,0);
	print PIC "total:",$picnum + $added,"\n" || die;
	close (PIC);
	print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
    }
    else
    {
	my $deled = 0;
	for (my $i = 2;$i < @used;$i ++)
	{
	    if ($req -> param ($used[$i]))
	    {
		$todel[$deled ++] = $used[$i];
	    }
	}
	for (my $i = 0;$i < $deled;$i ++)
	{
	    unlink ($htmpath."/used/".$todel[$i]) || die;
		unlink ($bbshome."/loglinks/".$todel[$i]) || die;
	    print "$todel[$i]��ɾ��","<br>";
	}
	print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
    }
}
else
{
    print "û�в���<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
}
