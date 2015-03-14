#summary From BMY Code to Linux Programming by ~interma@bmy

站外链接：http://olds.blogcn.com/wp-content/uploads/46/4644/2007/11/bc2lp.html

## 引子 ##

通过bmy的代码示例，讲解一些linux环境下programming的高级操作，发到linux版。

形式大概是连载，每期一个主题，每周放出来2，3篇左右，现在感觉能弄10期左右。里边尽量说一些浅显有趣的，篇幅基本不超过telnet下的一屏。

也算是给bmy积累点文档，同时活跃一下版上的技术气氛，恩。

如果大家还算喜欢，我就着手准备了，呵呵。

## 序 ##

既然大家如此给面子，就从今天开始了。第0期就不讲技术了，先顺便扯几句。

首先标题改成BC2LP了，节省空间，希望大家以后记得这个标题～我这里说的Linux Programming指的是Linux环境下的Progrmming，具体讲的时候可能就无所不包了，类如网络，架构，算法等可能都有涉及，但是都是和bmy开发相关的。同时我说的高级操作也不是那种晦涩难懂的，只是大家平时编程时很少用到的（受到项目类型和规模所限）。我讲的时候对细节就不细究了，争取做到浅入浅出，写的也不会太长，让大家都能看懂，能知道"哦，有这么一回事儿啊"就行了，权当给大家消遣了～

我具体的参考资料就是apue，unp，ulk之流了，大家可以去书上查阅细节；和bmy相关的则是这几年的经验和一些文档积累了。由于水平有限，肯定有错误之处，大家就都多指正喽～

bmy的代码在这里（_IronBlood.BMY 注：原文的代码链接已失效，代码可以访问[这里](http://code.google.com/p/bmybbs/source/browse/)_），主要代码在src目录(telnet)和nju09目录(www)中。

大概就是这么多了，正好一屏左右，顺便预告一下第一期的内容：套接字选项。

## 套接字选项(part1) ##
大家socket程序平时写的很多，但是socket option用的应该就不太多了，这里我分几期介绍一下bmy中socket option的使用。

首先介绍一下src/bbsd.c文件，它是bbsd程序的主要文件，从名字就能知道bbsd程序是bbs在telnet下daemon进程。它负责接入每个客户端的请求，然后fork，再exec一个bbs进程来对这个用户进行服务。整个bmy的telnet下边就是一个bbsd监听请求，多个 bbs负责为客户服务的并发服务器结构。

让我们看看190:src/bbsd.c（表示src/bbsd.c文件的第190行）：
```c

n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
val = 1;
setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (val));

// some code ...

if ((bind(n, (struct sockaddr *) &sin, sizeof (sin)) < 0) || (listen(n, QLEN) < 0))
exit(1);
```

其中n就是listen socket，它被加上了一个SO\_REUSEADDR选项，SO\_REUSEADDR可能是所有套接字选项最常用的了，那它到底是干什么用的呢？让我们考虑一下如下这种情况：

#bbsd已经运行了一段时间，若干个bbs进程在为大家进行服务。
#由于某种原因，bbsd down了，这导致了新的用户无法接入。
#interma自然在这个时候会出现，重启bbsd进程。但是他发现如果没有设置SO\_REUSEADDR选项的话，重启bbsd的时候会bind出错，提示无法绑定23端口，因为这个端口正在使用中(正在服务的bbs进程还连接着这个端口)！

那我们该如何防止这种情况发生呢？不用我说了吧？设置SO\_REUSEADDR选项就行了，它的常用功能就是端口复用（还有其它3个主要功能，详见unp）。

这期就到此为止了，欢迎大家下期继续听我扯套接字选项。

## 套接字选项(part2) ##

接着上期，让我们看看bmy中还用了那些套接字选项。

(1) SO\_LINGER

194:src/bbsd.c

```c

ld.l_onoff = ld.l_linger = 0;
setsockopt(n, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof (ld));
```

SO\_LINGER可以设定在tcp中close操作的具体行为，但是当ld.l\_onoff设为0的时候该选项关闭，保持默认行为，因此上边2行不要也行。具体SO\_LINGER可以如何改变close的行为，大家查阅unp即可。那为啥还要写出这2行？不写出来，我给你讲啥啊！

(2) SO\_SNDBUF 它和SO\_RCVBUF从字面上就能看出来功能：改变发送/接收缓冲区的大小。

71:yftpd/commands.c

```c

on = 65536;
setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void *) &on, sizeof (on));
```

由此可以看出，发送缓冲区的大小被改为了65536 byte，那么默认大小是多少呢？自己'sysctl -a | grep net.ipv4.tcp\_wmem'一下就知道喽。一般都是最小4096，默认16384。那为啥要改变它们的大小呢？为了控制（这里是增大）数据传输的吞吐率。 btw:yftpd目录下边是一个简易ftp的c实现，感兴趣的可以去瞅瞅代码，特别是XXX课程设计的时候～

好了，bmy中的套接字选项就扯到这里了，下一期：mmap

## mmap ##
这回要讲的是mmap，传说中的存储映射I/O了，具体的使用方法APUE中说的很清楚了，这里不再详述。这个东西在bmy中使用的非常广泛，大家先自己去看1段代码：

154:ythtlib/fileop.c

```c

int mmapfile(char *filename, struct mmapfile *pmf)
{
//...
pmf->ptr = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
//...
}
```

bmy为了使用方便自己弄了一个mmapfile()包装了一下mmap，相信语法细节大家也都能看懂，关键是为啥要这么用，看下边的2个Q&A

Q：那么bmy把它用在什么地方呢？
A：大多是要读入然后显示某个文件（如www下10大的显示，10大是一个定期生成的文件）的时候。

Q：那为啥要用mmap，而不直接读入文件呢？
A：为了速度！mmap不像read/write需要在user buffer和kernel buffer之间copy data，它直接调用kernel中的do\_mmap()，do\_mmap()直接attach一块地址到用户程序中，大家可以查阅LKD了解细节。一句话："爱慕爱慕AP，I/O中的战斗机！"

最后大家要注意的就是mmap使用不当的话，会导致很严重的安全问题，想了解详情的欢迎去骚扰pg mm。

下期内容：signal。

## signal ##
信号处理是linux编程必不可缺的一部分，但我们平时最多可能就处理一下^D之类，这里要讲一下bmy如何利用signal来增强自己的功能，以讯息发送为例。

从第1讲中我们知道bmy telnet下是一个多进程并发服务器结构，每个用户都有一个bbs进程为其服务。

考虑以下场景，interma给pg发fb信息，如何保证这次骚扰具有实时性呢？pg所在的bbs进程如何知道现在有一条msg过来了呢？signal这个时候就发挥作用了！看看bmy讯息的实现原理： sender在receiver的目录下建一个file（message），存放讯息内容，然后kill一下这个receiver process，这样receiver就知道有信息来了，同时显示一下message文件的内容就可以了。

接下来还有2个重要问题，(1) 具体使用的哪个信号？(2) 如何知道receiver的pid？看看bmy的代码吧：

790:src/sendmsg.c
```c

kill(uin->pid, SIGUSR2);
```

一目了然了，使用了SIGUSR2信号，它和SIGUSR1一起作为用户自定义信号。 pid放在了struct user\_info（include/struct.h）中，这个结构的实例数组会放到bbs的shm（共享内存，以后会讲到）中，保证了读取的效率。

趁热打铁，下期内容：talk,chat,以及message的实现机理。

## talk,chat,message ##
本周事情多，还要开讲座，这期就稍微水一点了，以下部分是我摘抄的。

talk, chat, 以及 message, 这三种方式看起来很类似, 可是它们的作法完全不同.

talk 是以在两个 user process 间开 socket 的方式完成的. chat 的作法则是, BBS 系统会跑一个 chat deamon, user process 以和 chat deamon 沟通的方式达成和其它的 user process 交换信息. 至于message, 上期中已经讲到，因为通常 message 只是一句话, 为了一句话就开个socket实在不划算, 因此, 它是以 file 来完成的. Sender 端在 Receiver 的目录下建立一个file, 里头存放着这次 message 的内容, 并且以 kill() 这个 system call 来通知Receiver process. Receiver process 收到后检视自己的目录下存放有 message 的这个file. 于是 Receiver便收到了来自 Sender 端的 message.

message大家用的多了，talk和chat功能在谈天说地区中，希望大家试用一下，会有更深刻的体会。

下期：还没想好，-_-bb，函数指针或者ipc吧。_

## 函数指针 ##
在任何一个用C语言编写的大型软件中，函数指针的使用都是必不可少的，bmy在这里也不例外，先看代码：

1118:src/mail.c
```c

struct one_key mail_comms[] = {
{'d', mail_del, "删除信件"},
{'D', mail_del_range, "区段删除"},
{Ctrl('P'), M_send, "发送信件"},
{'E', edit_post, "编辑信件"},
//...
{'\0', NULL, ""}
};
```

从这段代码我们已经能看到一些端倪，再看看one\_key结构的定义：

123:include/struct.h

```c

struct one_key {
int key;
int (*fptr) (); // 我们可爱的函数指针～
char func[33];
};
```

到此一切都真相大白了，mail\_comms数组是一个命令列表，每个条目的第2个成员都挂载了一个处理函数。这种设计方式算是一种惯用法了，无论是代码的清晰性还是可扩展性都是单纯使用switch语言所无法比拟的，可以用SICP中的一句话说明："利用数据构造抽象"。

下期内容:ipc

## ipc ##
ipc有3种，bmy只用到了其中2者:shm（共享内存）和msq（消息队列），先看代码：

188:include/config.h
```c

#define BCACHE_SHMKEY 7813
#define UCACHE_SHMKEY   7912
#define ENDLINE1_SHMKEY 5006
#define STAT_SHMKEY     5100
// ...
#define BBSLOG_MSQKEY 3333
//...
```
其中展示了bmy使用的所有shm和msq，具体使用细节，大家搜索各个key就可以了，同时可以参考UNP v2。

bmy代码中msq用的不是很多，主要用它(BBSLOG\_MSQKEY)来实现bbslogd(日志进程)和bbs进程之间的通讯；而shm则进行了大量的使用，如下进行详细的介绍：对于bmy，保障灌水的流畅性(速度)是很重要的，因此很多东西都放到我们巨海的16G内存中了，通过减少I/O操作来提高速度。但是内存又是非常宝贵的稀缺资源（16G也不能乱用），一些bbs进程之间可以共享的内容没必要每个进程存一份，因此shm这个时候就派上用场了，它将共享的内容在内存中只保留一份，再将共享的地址空间attach给各个bbs进程来实现共享。大家从shmkey的名字就能对它们的用途略知一二，例如ENDLINE1\_SHMKEY（底线），UCACHE\_SHMKEY（用户信息）等等。但是shm的大量使用也带来一些麻烦，由于ipc都是独立于process而存在(可以通过ipc\*命令来控制)，一旦shm中内容的格式发生变化，就要清除这些shm，然后重新进行init，因此有几次bbs代码更新需要停机维护的原因就是这个（最近的一次是自定义锁屏）。

下期内容：文件锁

## 文件锁 ##
更准确的说法应该是记录锁(Record Locking)，这里为了直观，使用文件锁这一名字。 bmy中每个bbs进程服务于一个用户，但是一旦这些进程同时操作同一个文件（如用户帐号文件.PASSWD）的时候就不可避免的要加入互斥机制，不同于thread，多个进程间无法用mutex进行互斥（其实可以用pthread\_mutex，但是很麻烦），因此本期的文件锁就要出场了:

243:src/record.c

```c

#ifdef LINUX
ldata.l_type = F_WRLCK;
ldata.l_whence = 0;
ldata.l_len = size;
ldata.l_start = size * (id - 1);
if (fcntl(fd, F_SETLKW, &ldata) == -1) {
errlog("reclock error %d", errno);
return -1;
}
#else
flock(fd, LOCK_EX);
```

从中可以看出在linux系统（bmy）中我们用fcntl加文件锁，这里配合F\_SETLKW和F\_WRLCK产生一个阻塞式读写锁（对于文件最常用的锁），具体细节可以参考APUE。而在其他系统中我们使用的flock，这里只是简单的加上了LOCK\_EX（互斥锁）。 fcntl源自SVR2，进入了POSIX.1规范，flock源自BSD（linux下是否能用，man一下就知道了），且对于文件，互斥锁性能低于读写锁，因此推荐使用fcntl。

下期内容：bmy存储结构

## bmy存储结构 ##
从本期开始要水一些（pg可以拍我），对bmy的体系结构进行一些介绍，首先是存储结构。

首先要明确的是bmy没有数据库，所有需要持久化的信息都要用文件来存储，每个帖子/邮件都是一个小文件，例如web下的链接：

> .../con?B=ArtDesign&F=M.1195036844.A

其中的M.1195036844.A就是这篇帖子的文件名。

我们只考虑版面，大概有400个版，每个版有10000篇帖子，那么整个bbs就有：400\*1w = 400万帖子，这还没考虑邮件，精华区等其他，因此得知bmy的小文件数总共应该在1000万以上。由于有这么多小文件，ext文件系统管理起来已经力不从心了，因此bmy采用了ReiserFS，它是基于平衡树的文件系统结构，可以大大提高文件的访问效率。

只用ReiserFS还不足以满足效率需求，一些关键的数据结构bmy还利用目录对它们进行了索引，例如版面的存储方法：

> {bbshome}/boards/{boardname}/{filename}

每个版面都是一个目录。

但是涉及到查询（例如同作者）的时候还是要遍历1万个文件，因此为了提高查询效率，又加入了一个索引文件.DIR，它保存在每个版面的目录下，其中存放这个版面所有文件的元数据（例如作者，标题，是否m，是否@等），我们的C-g就是通过在这个文件中查找来实现的（无法全文检索的原因就在此）。

下期内容：服务器模型

## 服务器模型 ##
本期也是科普时间，继续水～。

在第一讲中已经对telnet下的模型进行了简单介绍：bbsd作为daemon进程然后对于每个客户spawn(fork+exec)一个bbs进程来服务，典型的并发服务器结构，没有使用进程池，优点就是代码简单稳定性好，缺点对资源的消耗很大。

接下来要对web下的服务器模型进行介绍了：web下的代码都是cgi模式，使用apache+fastcgi模块(可以大幅提高cgi的效率)来提供服务。既然采用apache，就不能不提到apache的MPM(Multi-Processing Modules)模块，它对web下的性能有很大影响，下边开始讲故事：

最开始bmy的MPM用的是prefork模式（顾名思义，预先fork一个进程池来满足服务），默认池中有256个进程，后来随着bmy的用户增多，web下并发请求也越来越大，256的并发数已经无法满足需求了，于是web下的速度越来越慢（这时大概是2006年，大家应该都有印象），对于每个请求，需要10s左右才能完成，速度接近龟爬。。。程序组于是不能再忍了！便对MPM进行调优，换用worker模式（进线程混合模式，速度快，节省资源）并增大并发数，于是web下的速度有很大提升，但是出现了不定期假死问题，搞得我们很郁闷！在这个需要英雄的时刻，interma同学做了一个关键的决定：缠着lanboy同学买了一个更nb的服务器（2007年3月到位，就是现在这个 3G\*4cpu,16Gmem的机器），然后将MPM模型改回prefork(稳定性更好，感谢蚂蚁)，然后并发数改成768，在新机器的强大支援下，世界恢复了平静，假死再没有出现，又回到了你好我也好的时代。

故事讲完了，下期内容：代码结构

## bmy代码结构 ##
介绍一下bmy代码结构，希望能对linux下的项目有启发作用。PS:今天想吃螃蟹了。。。

bmy的核心代码由4个部分组成（更具体的参考cvs的Doc模块下的System\_Maintenance/bmy程序结构说明文档.txt）：

**include目录：全局头文件** ythtlib目录和libythtbbs目录：全局共享库，分2个层次
**src目录：telnet下所有功能代码** nju09目录：web下所有功能代码

下边随便扯2点：

代码本来采用autoconf+makefile组织，由于configure.in改起来很麻烦，于是interma邪恶的把这个文件废弃了，不再考虑可移植性，直接上去修改makefile了，期待将来高人把这个弄好，同时把bmy做到真正的一键构建（由于interma的胡乱修改，现在安装超级麻烦）。

小工具cproto的使用，这算是一个小tip了，它可以从c/c++代码中抽取函数声明出来，省去了直接写的麻烦，例如：

73:src/Makefile
```

proto proto.h:
echo '#include "bbs.h"' > proto.h
cproto $(CFILE) $(DEFINES) -DBACK_DELETE_RANG -I \
$(BASEPATH)/include -I $(BASEPATH)/ythtlib -I \
$(BASEPATH)/libythtbbs >> proto.h
```

生成了src目录下源码中的所有函数声明，导出为proto.h文件，满足了我们这些懒人的需要。

下期内容：算法

## 算法 ##
bmy的算法很多很多，这里只取一瓢，介绍一下文章未读标志。大家已经知道，bmy会记住每个用户在每个版的未读情况，我们先看看弱智方法需要多少空间： 40000(用户数)×400(版面数)×10000(每个版面的文章数)×1bit=20TB 瓜了吧？如果这么弄，买硬盘的钱都不够了，现在我说其实只用了120M就够保存所有的未读标记了，我可不是侯总，这也不是八心八箭的破盘价，我们来看看bmy的实现吧（这部分nju的Haishion写过了，我也就不班门弄斧了，全部摘抄～）：

> 发信人: Haishion.bbs@bbs.nju.edu.cn, 原信区: BBSDev, 标题: 剖析文章未读标志, 发信站: 南京大学小百合站 (Mon Jul 22 21:06:55 2002), 站内信件

> 你说：系统怎么知道我读过哪些文章，没读过哪些文章？我来解释下吧:)

> 未读标识是这么控制的。代码主要在boards.c里面。最后几个函数brd\_xxx。每个用户的未读标志存放在自己用户目录的.boardrc里面。.boardrc的格式是多个record，每个record先来一个BRC\_STRLEN长度的字符串，存放版名。然后是一个char类型的数字，记录接下来有几个time\_t类型的值。（记录的最大值BRC\_MAXNUM缺省为60）接着就是多个time\_t类型的值。

> 大家知道bbs每篇文章都是以时间为文件名，比如M.994224383.A就是说这篇文章是 994224383时写的。（对于一切系统或者数据库系统的时间来说，都是以时间戳记录的，只是显示出来做了转化），所以如果你看过M.994224383.A这篇文章，系统把这篇文章的文件名994224383提取出来，把他插入到time\_t类型的列表中，并且把char类型加1。 .boardrc由大到小排列（使查找速度加快），如果超过BRC\_MAXNUM，则替换掉时间最小的那篇文章。

> 程序判断未读是这样的，int brc\_unread(filename)传入filename，把filename提取出的994224383跟60个列表里面的所有值比较，注意列表是按大到小排列的，如果filename时间不等正在判断的值，比较下一个，如果大于正在判断的值，立刻返回未读，如果等于，立刻返回已读。如果都小于，则返回已读。（认真想想:)）

> 这里顺便可以解答几个疑问：

> (1) 为什么有的板块改名了，那个版的文章都变成未读了。因为板块改名，不会去每个用户目录下去修改.boardrc，把相应的板块名字改为修改后的板块。所以你去看新板块的时候，.boardrc里面没有记录，系统就认为你均未看过这个版。

> (2) 既然最大的值只记录60个，如果我看了61篇文章，会出现什么效果？这个问题问得好，我也不知道。

> 吓，我怎么会不知道哈，我们来做一个实验，找一个版块你从未看过的。我们打算把102和160篇文章都看了，这就是59篇了，这时100篇以前的文章你还都是未看过的，然后再看一下101篇，哈，前面100篇都变成已经读了，是不是有点不合理呀:)

> (3) 为什么有时我上站发现我看过的文章又变成未读的了？因为你没有正常下线。文章未读标志只有在离开板块时才brc\_update()写入.boardrc。

> (4) 我登陆2个帐号？为什么没有同步？因为brd\_update()只在离开板块时调用，所以你2个帐号在同一板块看了不同的文章， brd\_update() 不仅写入 .boardrc ，同时还重新载入 .boardrc ，当然，这一切都需要在已读未读标志改变以后。所以如果第一个窗口已经看了多好版，结果第二个窗口显示还是没看过，那么可以在第二个窗口里采用以下的简单办法操作：进入任意版，阅读任意文章（当然最简单的就是 c 一下清掉未读），退出该版，退回分类讨论区菜单，重新进入讨论区列表，然后就可以发现第二个窗口的未读标记跟第一个窗口已经同步了。

看懂了吧，再sto一下源作者的智慧，里边的要点就是"未读的局部性"，cache的原理。

下期：没了。。。

## 休息时刻 ##
到上期为止，原计划的内容就全讲完了，这12期有很多扯谈和灌水的内容，肯定也有很多错误的的东西，不过大家还是很捧场，这里多谢大家了。希望这一系列文章能给大家以消遣，更希望大家也能对bmy代码产生兴趣，加入程序组（其实无所谓），一起来维护她，改进她，从而给我们带来一个更好的bmy，广告时间到此为止。。。

以后可能还会不定期出几期，但是由于我很懒也可能不出了，但是我希望这个系列将来会有高人来续上，就当貂尾续狗（马）了～

```perl

#!/usr/bin/perl
until (!($interma->wake_up() || $nber->come()))
{
sleep 1;
}
$bc2lp->continue();
```