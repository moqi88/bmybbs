#!/usr/bin/python
import sys,cgi,os,string

usertmpdir="/home/bbsattach/user/"

def er(a):
	print "ճ��ʧ��, �ļ�̫��? �ļ�������? �ļ����ְ����������ַ�? û�е�¼?<a href=/upload.htm>����</a>"
	print `a`
	sys.exit(1)

#���������
print "Content-type: text/html; charset=GB2312\n\n\n"
print "<link rel=stylesheet type=text/css href='/bbs.css'>"
if not os.environ.has_key("CONTENT_LENGTH") or string.atoi(os.environ["CONTENT_LENGTH"]) > 5000000:
	er(1)
filesize=string.atoi(os.environ["CONTENT_LENGTH"])
if not os.environ.has_key("PATH_INFO") or len(os.environ["PATH_INFO"])!=34:
	er(2)
sessionid=os.environ["PATH_INFO"][4:]
utmpnum=os.environ["PATH_INFO"][1:4]
fromhost=os.environ["REMOTE_ADDR"]
import re
if not re.match("^\w{3,3}$",utmpnum) or not re.match("^\w{30}$",sessionid):
	er(4)
import commands
userid=commands.getoutput("/usr/local/bin/getuser %s %s %s" % (utmpnum,sessionid,fromhost))
if userid=="test" or userid=="":
	er(5)
form=cgi.FieldStorage()
if not form.has_key("userfile"):
	er(6)
fileitem=form["userfile"]
if fileitem.file:
	filename=string.split(string.replace(fileitem.filename,'\\','/'),'/')[-1]
	if string.strip(filename)=='' or len(filename) > 30:
		er(7)
	for i in filename:
		if i in '$"\',~`:&#@()[]{};><|/\\%!?*\n\r^':
			er(8)
	filename=string.replace(filename,".php3",".html")
	if os.path.exists(usertmpdir+userid):
		nowsize=os.path.getsize(usertmpdir+userid)
	else:
		nowsize=0
	if nowsize+filesize < 5000000 :
		wf=open(usertmpdir+userid , "a")
		import uu
		uu.encode(fileitem.file,wf,filename,0644)
		wf.close()
		print "�ļ�"+filename+"ճ���ɹ�!<a href=/" SMAGIC "%s/bbsupload>����������ճ��</a>" % os.environ["PATH_INFO"][1:]
	else:
		print "��ճ��̫���ļ���"
		sys.exit(1)
