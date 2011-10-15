function showloginpics(pics){
    var list=pics.split(";;");

	for(i=0;i<list.length;++i){
		var pic=list[i].split(";");
		$('<a href="'+ pic[1] +'" target="_blank"><img id="pic' + i + '" src="' + pic[0] + '"/></a>').appendTo('div#container');
	}
	if(list.length>1){	
	$('img#pic0').parent().addClass('show');
	
	$('#container a').css({opacity: 0.0});
	
	$('#container a.show').css({opacity: 1.0});
	
	setInterval('gallery()', 5000);}
}

function gallery() {
	var current = ($('#container a.show')? $('#container a.show') : $('#container a:first'));
	
	//var next = ((current.next().length) ? ((current.next())? $('#container a:first') : current.next()) : $('#container a:first'));
	var next = ((current.next().length)? current.next() : $('#container a:first'));
	next.css({opacity: 0.0})
	.addClass('show')
	.animate({opacity:1.0}, 1500);
	
	current.animate({opacity: 0.0}, 1500)
	.removeClass('show');
}

