#!/usr/bin/perl
use CGI;
$basedir = "/home/apache/htdocs/bmy/"; #�ϴ����ļ���ŵ�ַ
print "Content-type: text/html\n\n";
my $req = new CGI;
my $username = $req->param("USERNAME");
my $chkpasswd = $req->param("PASSWD");
unless (open (PW,"/home/apache/htdocs/bmy/BMY_MAINPIC/$username.pw")){
	print "�����û�$username������";
	die;
	}
chomp ($passwd = <PW>);
close (PW);
if ($chkpasswd eq $passwd){
	my $file = $req->param("FILE");
	if ($file ne "") {
		$fileName = $file;
		$fileName =~ s/^.*(\\|\/)//; #��������ʽȥ�����õ�·�������õ��ļ���
		my $newmain = $fileName;
		$filenotgood = "no";
		if ($fileName ne "cai.jpg"){
			$filenotgood = "yes";
			}
		}
	chomp (my $used = `date +%F-%R`);
	system "mv /home/apache/htdocs/bmy/cai.jpg /home/apache/htdocs/bmy/BMY_MAINPIC/used/$used.jpg";
	if ($filenotgood ne "yes") { #��ο�ʼ�ϴ�
		open (OUTFILE, ">$basedir/$fileName");
		binmode(OUTFILE); #���ȫ�ö����Ʒ�ʽ�������Ϳ��Է����ϴ��������ļ��ˡ������ı��ļ�Ҳ�����ܸ���
		while (my $bytesread = read($file, my $buffer, 1024)) {
			print OUTFILE $buffer;
            		}
            	close (OUTFILE);
            	$message.="cai.jpg �ѳɹ��ϴ�,ԭ�ļ�����Ϊ$used.jpg<br>\n";
        	}
        else{
            $message.="�ϴ�ʧ�ܣ�����ļ����ĳ�cai.jpg�����ϴ�<br>\n";
	}
}
else{
$message.="���벻��ȷ������ϵ����վ�� <br>\n";
}
print $message; #�������ϴ���Ϣ
