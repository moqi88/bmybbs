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
