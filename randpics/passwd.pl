#!/usr/bin/perl
use CGI;
print "Content-type: text/html\n\n";
my $req = new CGI;
my $username = $req->param("UN");
my $filename = "/home/bbs/$username.pw";
if (-e $filename){
	unless (open (PW,"/home/bbs/$username.pw")){
		print "���󣡴��ļ�$username.pwʧ��";
		die;
		}
	chomp (my $oldpw = <PW>);
	my $chkpasswd = $req->param("OP");
	if ($chkpasswd eq $oldpw){
		close (PW);
		my $newpw = $req->param("NP1");
		my $newpw2 = $req->param("NP2");
		unless ($newpw eq $newpw2){
			print "�������벻ƥ��";
			die;
			}
		unless (open (PW,">/home/bbs/$username.pw")){
			print "����д���ļ�$username.pwʧ��";
			die;
			}
		print PW "$newpw\n";
		close (PW);
		print "�����޸ĳɹ�";
		}
	else{
		print "���������";
		}
	}
else{
	print "�����û�$username������";
	die;
	}
