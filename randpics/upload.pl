#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $upfilecount = 1;
my $maxupload = 5;
my $ext = ".jpg";
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
print $req -> start_html (),
    $req -> h2 ("Welcome to XJTU bbs BMY!"),
    $req -> a ({-href=>"$cgibin/showpics.pl"},"����"),
    "|",
    $req -> a ({-href=>"$cgibin/logout.pl"},"�˳�"),
    $req -> hr (),
    $req -> start_multipart_form ("POST","","gb2312");
for (my $i = 1;$i <= $maxupload;$i ++)
{
    print $req -> filefield ({-name=>"FILE$i"}),
    $req -> br (),
}
print $req -> submit ({-label=>'upload'}),
    $req -> endform,
    $req -> hr ();#�����ϴ��ļ��ı�
if ($req -> param ())
{
    while ($upfilecount <= $maxupload)
    {
	my $file = $req -> param ("FILE$upfilecount");
	if ($file)
	{
	    $filename = $file;
	    $filename =~ s/^.*(\\|\/)//; #��������ʽȥ��·�������õ��ļ���
	    if ((-e "$htmpath/uploaded/$filename") || (-e "$htmpath/using/$filename") || (-e "$htmpath/used/$filename"))
	    {
		$message .= "�ļ�$filename�Ѿ����ڣ������������������ϴ�<br>\n";
		$upfilecount ++;
		next;
	    }
	    my $extname = lc (substr ($filename,length ($filename) - 4,4));
	    unless ($extname ne $ext)
	    {
		open (OUTFILE, ">$htmpath/uploaded/$filename");
		while (read ($file,my $buffer,1024))
		{
		    print OUTFILE $buffer;
		}
		close (OUTFILE);
		$message .= "$filename�ѳɹ��ϴ�<br>\n";
	    }
	    else
	    {
		$message .= "$filename�ϴ�ʧ��,ֻ����jpg�ļ�<br>\n";
	    }
	}
	$upfilecount ++;
    }
}
unless ($message)
{
    $message .= "��ѡ��Ҫ�ϴ����ļ�";
}
print $message; #�������ϴ���Ϣ
print $req -> end_html;
