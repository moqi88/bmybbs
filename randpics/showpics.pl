#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs";
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
open (PIC,$bbshome."/logpics") || die "���󣡴��ļ��б�ʧ��!";
my $picnum = <PIC>;
chomp ($picnum);
$picnum =~ s/\D//g;#��������ʽȥ��������Ϣ
for (my $i = 0;$i < $picnum;$i ++)
{
    chomp ($pics[$i] = <PIC>);
}#���ݵõ����ļ������ļ��б��������
close (PIC);
opendir (DIR,"$htmpath/bmyMainPic/uploaded") || die;
my @nonused = readdir (DIR);
closedir (DIR);#�õ�uploaded/�µ��ļ���
opendir (DIR,"$htmpath/bmyMainPic/used") || die;
my @used = readdir (DIR);
closedir (DIR);#�õ�used/�µ��ļ���
open (LINK,"$bbshome/loglink") || die;
seek (LINK,46,0) || die;
my $board = <LINK>;
chomp ($board);
close (LINK);
print $req -> start_html (),
    $req -> h2 ("Welcome to XJTU bbs BMY!"),
    $req -> a ({-href=>"$cgibin/upload.pl"},"�ϴ�"),
    "|",
    $req -> a ({-href=>"$cgibin/logout.pl"},"�˳�"),
    $req -> hr (),
    $req -> start_multipart_form ("POST","$cgibin/using.pl","gb2312"),
    $req -> th ("Now using pictures"),
    $req -> table ({-border=>'1'}),
    $req -> Tr (),
    $req -> td (["Thumbnail","File name","Link","Select"]);
for (my $i = 0;$i < $picnum;$i ++)
{
	if(open (PICLINK,$bbshome."/loglinks/".$pics[$i])){
		chomp ($piclink = <PICLINK>);
		close (PICLINK);
    	print $req -> Tr (),
    	$req -> td ([$req -> img ({-src=>("/bmyMainPic/using/".$pics[$i]),-width=>'200',-height=>'100'}),$pics[$i],$piclink,$req -> checkbox ({-name=>$i,-value=>"ON",-label=>""})]);
	}
	else{
		print $req -> Tr (),
		$req -> td ([$req -> img ({-src=>("/bmyMainPic/using/".$pics[$i]),-width=>'200',-height=>'100'}),$pics[$i],"",$req -> checkbox ({-name=>$i,-value=>"ON",-label=>""})]);
	}
}#�ѵ�ǰʹ�õĽ�վ������ʾ����
print $req -> end_table, 
    $req -> submit ({-label=>'delete'}),
    $req -> endform;
print $req -> p,
    $req -> start_multipart_form ("POST","$cgibin/uploaded.pl","gb2312"),
    $req -> th ("Not used yet pictures"),
    $req -> table ({-border=>'1'}),
    $req -> Tr (),
    $req -> td (["Thumbnail","File name","Board","File","Select"]);
for (my $i = 2;$i < @nonused;$i ++)
{
#	$req -> start_multipart_form ("POST","$cgibin/uploaded.pl","gb2312"),
    print $req -> Tr (),
    $req -> td ([$req -> img ({-src=>("/bmyMainPic/uploaded/".$nonused[$i]),-width=>'200',-height=>'100'}),$nonused[$i], $req -> textfield ({-name=>"Board".$i}), $req -> textfield ({-name=>"File".$i}),$req -> checkbox ({-name=>$nonused[$i],-value=>"ON",-label=>""})]);
#	$req -> endform;
}#�����ϴ���ûʹ�õĽ�վ������ʾ����
print $req -> end_table,
    $req -> submit ({-name=>'ADD',-label=>'add'}),
    $req -> submit ({-name=>'DEL',-label=>'del'}),
    $req -> endform;
print $req -> p,
    $req -> start_multipart_form ("POST","$cgibin/used.pl","gb2312"),
    $req -> th ("Already used pictures"),
    $req ->table ({-border=>'1'}),
    $req -> Tr (),
    $req -> td (["Thumbnail","File name","Select"]);
for (my $i = 2;$i < @used;$i ++)
{
    print $req -> Tr (),
    $req -> td ([$req -> img ({-src=>("/bmyMainPic/used/".$used[$i]),-width=>'200',-height=>'100'}),$used[$i],$req -> checkbox ({-name=>$used[$i],-value=>"ON",-label=>""})]);
}#���Ѿ�ʹ�ù��Ļ����ϴ���ɾ���Ľ�վ������ʾ����
print $req -> end_table,
    $req -> submit ({-name=>'ADD',-label=>'add'}),
    $req -> submit ({-name=>'DEL',-label=>'del'}),
    $req ->endform,
    $req -> hr (),
    $req -> start_multipart_form ("POST","$cgibin/link.pl","gb2312"),
    $req -> th ("The linked board of the login picture is <font color=red>$board</font>"),
    $req -> textfield ({-name=>'BOARD',-maxlength=>'16'}),
    $req -> submit ({-label=>'modify'}),
    $req -> endform,
    $req -> end_html;
