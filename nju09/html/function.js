function openlog()
{
	open('bbslform','','left=255,top=190,width=130,height=100');
}
function eva(board, file) {
var s;
	s=" [<a href=eva?B="+board +"&F="+file+"&star=";
	document.writeln("ϲ���������ô? �������"+s+"1>����</a>]"+s+"3>�ܺ�</a>]"+s+"5>ǿ���Ƽ�!</a>]");
}
function docform(a, b){
	document.writeln("<table border=0 cellspacing=0 cellpading=0><tr><td><form name=docform1 action="+a+"><a href="+a+"?B="+b+"&S=1>��һҳ</a> <a href="+a+"?B="+b+"&S=0>���һҳ</a> <input type=hidden name=B value="+b+"><input type=submit value=ת��>��<input type=text name=start size=4>ƪ</form></td><td><form name=docform2 action="+a+"><input type=submit value=ת��><input type=text name=B size=7>������</form></td></tr></table>");
}
