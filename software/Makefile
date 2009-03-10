include Makefile.Base
apache_v:=1.3.27
apache_ssl_v:=1.48
fastcgi_v:=2.2.12
fcgi_v:=2.2.2
newtv:=0.50.33
newtplevel:=1
ghthashv=0.5.0
apache2_v:=2.0.43
fastcgicvs_v:=SNAP-0212222223

apache:=apache_$(apache_v)
apache_s:=$(apache).tar.gz
apache_ssl_s:=apache_$(apache_v)+ssl_$(apache_ssl_v).tar.gz
apache2:=httpd-$(apache2_v)
apache2_s:=$(apache2).tar.gz
fastcgi:=mod_fastcgi-$(fastcgi_v)
fastcgi_s:=$(fastcgi).tar.gz
fastcgicvs:=mod_fastcgi-$(fastcgicvs_v)
fastcgicvs_s:=$(fastcgicvs).tar.gz
fcgi:=fcgi-$(fcgi_v)
fcgi_s:=$(fcgi).tar.gz
VPATH:=$(apache)/src
CPU:=pentium

.PHONY:installhttpd installfcgidev

httpsd:$(apache_s) $(apache_ssl_s) $(fastcgi_s) mod_redurl.c mod_conn.c
	rm -fr $(apache);tar xzvf $(apache_s);tar xzvf $(fastcgi_s);mv $(fastcgi) $(apache)/src/modules/fastcgi 
	cd $(apache);tar xzvf ../$(apache_ssl_s);echo Y|./FixPatch; patch -p1 < ../apachelimit.patch;OPTIM="-O2 -funroll-loops -ffast-math -malign-double -mcpu=$(CPU) -march=$(CPU) -fomit-frame-pointer -fno-exceptions" ./configure \
	--disable-module=autoindex \
	--disable-module=auth \
	--disable-module=imap \
	--disable-module=include \
	--enable-module=rewrite \
	--disable-module=userdir \
	--add-module=../mod_redurl.c --add-module=../mod_conn.c \
	--activate-module=src/modules/fastcgi/libfastcgi.a \
	--with-layout=RedHat;make;make install
installhttpd:httpsd
	-mv /usr/sbin/httpd /usr/sbin/httpd.old
	mv $(apache)/src/httpsd /usr/sbin/httpd
	mv $(apache)/src/modules/ssl/gcache /usr/sbin
#	cat ssl.conf >> /etc/httpd/conf/httpd.conf
installfcgidev:$(fcgi_s)
	rm -fr $(fcgi);tar xzvf $<;cd $(fcgi);./configure --prefix=/usr;make;make install
installnewt:newt-$(newtv)-$(newtplevel).src.rpm
#	rpm2cpio $^|cpio -i
	tar xzvf newt-$(newtv).tar.gz
	patch -p0< newt-ytht.patch
	tar -cvpzf newt-$(newtv).tar.gz newt-$(newtv)
	rpm -tb newt-$(newtv).tar.gz
	rpm -Uvh /usr/src/redhat/RPMS/i386/newt-*.rpm
installghthash:libghthash-$(ghthashv).tar.gz
	tar xzvf $^;cd $(^:.tar.gz=);./configure;$(MAKE);$(MAKE) install

installhttpd2:$(apache2_s) $(fastcgicvs_s)
	rm -fr $(apache2);tar xzvf $(apache2_s)
	cd $(apache2);NOTEST_CFLAGS="-O2 -funroll-loops -ffast-math -malign-double -mcpu=$(CPU) -march=$(CPU) -fomit-frame-pointer -fno-exceptions" ./configure \
	--disable-autoindex --disable-imap --disable-include \
	--disable-asis --disable-actions --disable-status \
	--disable-userdir --disable-negotiation \
	--enable-rewrite --with-mpm=worker;$(MAKE);$(MAKE) install
	rm -fr $(fastcgicvs);tar xzvf $(fastcgicvs_s)
	cd $(fastcgicvs);cp Makefile.AP2 Makefile;$(MAKE);$(MAKE) install
#	cp httpd2.conf /usr/local/apache2/conf/httpd.conf
