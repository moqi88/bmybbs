function openlog()
{
	open('bbslform','','left=255,top=190,width=130,height=100');
}
function eva(board, file) {
var s;
	s=" [<a href=eva?B="+board +"&F="+file+"&star=";
	document.writeln("喜欢这个文章么? 这个文章"+s+"1>不错</a>]"+s+"3>很好</a>]"+s+"5>强烈推荐!</a>]");
}
function docform(a, b){
	document.writeln("<table border=0 cellspacing=0 cellpading=0><tr><td><form name=docform1 action="+a+"><a href="+a+"?B="+b+"&S=1>第一页</a> <a href="+a+"?B="+b+"&S=0>最后一页</a> <input type=hidden name=B value="+b+"><input type=submit value=转到>第<input type=text name=start size=4>篇</form></td><td><form name=docform2 action="+a+"><input type=submit value=转到><input type=text name=B size=7>讨论区</form></td></tr></table>");
}
function share(a,title,board,file){
    var url;
    switch(a){
        case 'sina':
            url = "http://v.t.sina.com.cn/share/share.php?appkey=3235716756&title=%23BMYBBS%E8%AF%9D%E9%A2%98%E5%88%86%E4%BA%AB%23"+title+"&url=http://bbs.xjtu.edu.cn/BMY/con?B="+board+"%26F="+file;
            break;
        case 'renren':
            url = "http://share.renren.com/share/buttonshare.do?link=http%3A%2F%2Fbbs.xjtu.edu.cn%2FBMY%2Fcon%3FB%3D"+board+"%26F%3D"+file;
            break;
        case 'tencent':
            url = "http://share.v.t.qq.com/index.php?c=share&a=index&title=%23BMYBBS%E8%AF%9D%E9%A2%98%E5%88%86%E4%BA%AB%23"+title+"&url=http%3A%2F%2Fbbs.xjtu.edu.cn%2FBMY%2Fcon%3FB%3D"+board+"%26F%3D"+file+"&appkey=801082141&pic=&assname";
            break;
        default:
            break;
    }
    var popup=window.open(url);
    popup.focus();
}
