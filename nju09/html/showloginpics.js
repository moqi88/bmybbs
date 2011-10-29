function showloginpics(pics){
    var list=pics.split(";;");

	for(i=0;i<list.length;++i){
		var pic=list[i].split(";");
		$('<a href="'+ pic[1] +'" target="_blank"><img id="pic' + i + '" src="' + pic[0] + '"/></a>').appendTo('div#container');
	}
	if(list.length>1){	
		/*$('img#pic0').parent().addClass('show');
		
		$('#container a').css({opacity: 0.0});
		
		$('#container a.show').css({opacity: 1.0});
		
		setInterval('gallery()', 5000);*/
		
		$('div#container').coinslider({width:770,height:400,navigation:true,delay:5500,hoverPause: true});
		$('div#container').dropShadow('top');
	}
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

(function($){

  var dropShadowZindex = 1;  //z-index counter

  $.fn.dropShadow = function(options)
  {
    // Default options
    var opt = $.extend({
      left: 4,
      top: 4,
      blur: 2,
      opacity: .5,
      color: "black",
      swap: false
      }, options);
    var jShadows = $([]);  //empty jQuery collection
    
    // Loop through original elements
    this.not(".dropShadow").each(function()
    {
      var jthis = $(this);
      var shadows = [];
      var blur = (opt.blur <= 0) ? 0 : opt.blur;
      var opacity = (blur == 0) ? opt.opacity : opt.opacity / (blur * 8);
      var zOriginal = (opt.swap) ? dropShadowZindex : dropShadowZindex + 1;
      var zShadow = (opt.swap) ? dropShadowZindex + 1 : dropShadowZindex;
      
      // Create ID for shadow
      var shadowId;
      if (this.id) {
        shadowId = this.id + "_dropShadow";
      }
      else {
        shadowId = "ds" + (1 + Math.floor(9999 * Math.random()));
      }

      // Modify original element
      $.data(this, "shadowId", shadowId); //store id in expando
      $.data(this, "shadowOptions", options); //store options in expando
      jthis
        .attr("shadowId", shadowId)
        .css("zIndex", zOriginal);
      if (jthis.css("position") != "absolute") {
        jthis.css({
          position: "relative",
          zoom: 1 //for IE layout
        });
      }

      // Create first shadow layer
      bgColor = jthis.css("backgroundColor");
      if (bgColor == "rgba(0, 0, 0, 0)") bgColor = "transparent";  //Safari
      if (bgColor != "transparent" || jthis.css("backgroundImage") != "none" 
          || this.nodeName == "SELECT" 
          || this.nodeName == "INPUT"
          || this.nodeName == "TEXTAREA") {   
        shadows[0] = $("<div></div>")
          .css("background", opt.color);                
      }
      else {
        shadows[0] = jthis
          .clone()
          .removeAttr("id")
          .removeAttr("name")
          .removeAttr("shadowId")
          .css("color", opt.color);
      }
      shadows[0]
        .addClass("dropShadow")
        .css({
          height: jthis.outerHeight(),
          left: blur,
          opacity: opacity,
          position: "absolute",
          top: blur,
          width: jthis.outerWidth(),
          zIndex: zShadow
        });
        
      // Create other shadow layers
      var layers = (8 * blur) + 1;
      for (i = 1; i < layers; i++) {
        shadows[i] = shadows[0].clone();
      }

      // Position layers
      var i = 1;      
      var j = blur;
      while (j > 0) {
        shadows[i].css({left: j * 2, top: 0});           //top
        shadows[i + 1].css({left: j * 4, top: j * 2});   //right
        shadows[i + 2].css({left: j * 2, top: j * 4});   //bottom
        shadows[i + 3].css({left: 0, top: j * 2});       //left
        shadows[i + 4].css({left: j * 3, top: j});       //top-right
        shadows[i + 5].css({left: j * 3, top: j * 3});   //bottom-right
        shadows[i + 6].css({left: j, top: j * 3});       //bottom-left
        shadows[i + 7].css({left: j, top: j});           //top-left
        i += 8;
        j--;
      }

      // Create container
      var divShadow = $("<div></div>")
        .attr("id", shadowId) 
        .addClass("dropShadow")
        .css({
          left: jthis.position().left + opt.left - blur,
          marginTop: jthis.css("marginTop"),
          marginRight: jthis.css("marginRight"),
          marginBottom: jthis.css("marginBottom"),
          marginLeft: jthis.css("marginLeft"),
          position: "absolute",
          top: jthis.position().top + opt.top - blur,
          zIndex: zShadow
        });

      // Add layers to container  
      for (i = 0; i < layers; i++) {
        divShadow.append(shadows[i]);
      }
      
      // Add container to DOM
      jthis.after(divShadow);

      // Add shadow to return set
      jShadows = jShadows.add(divShadow);

      // Re-align shadow on window resize
      $(window).resize(function()
      {
        try {
          divShadow.css({
            left: jthis.position().left + opt.left - blur,
            top: jthis.position().top + opt.top - blur
          });
        }
        catch(e){}
      });
      
      // Increment z-index counter
      dropShadowZindex += 2;

    });  //end each
    
    return this.pushStack(jShadows);
  };


  $.fn.redrawShadow = function()
  {
    // Remove existing shadows
    this.removeShadow();
    
    // Draw new shadows
    return this.each(function()
    {
      var shadowOptions = $.data(this, "shadowOptions");
      $(this).dropShadow(shadowOptions);
    });
  };


  $.fn.removeShadow = function()
  {
    return this.each(function()
    {
      var shadowId = $(this).shadowId();
      $("div#" + shadowId).remove();
    });
  };


  $.fn.shadowId = function()
  {
    return $.data(this[0], "shadowId");
  };


  $(function()  
  {
    // Suppress printing of shadows
    var noPrint = "<style type='text/css' media='print'>";
    noPrint += ".dropShadow{visibility:hidden;}</style>";
    $("head").append(noPrint);
  });

})(jQuery);

(function($) {

	var params 		= new Array;
	var order		= new Array;
	var images		= new Array;
	var links		= new Array;
	var linksTarget = new Array;
	var titles		= new Array;
	var interval	= new Array;
	var imagePos	= new Array;
	var appInterval = new Array;	
	var squarePos	= new Array;	
	var reverse		= new Array;
	
	$.fn.coinslider= $.fn.CoinSlider = function(options){
		
		init = function(el){
				
			order[el.id] 		= new Array();	// order of square appereance
			images[el.id]		= new Array();
			links[el.id]		= new Array();
			linksTarget[el.id]	= new Array();
			titles[el.id]		= new Array();
			imagePos[el.id]		= 0;
			squarePos[el.id]	= 0;
			reverse[el.id]		= 1;						
				
			params[el.id] = $.extend({}, $.fn.coinslider.defaults, options);
						
			// create images, links and titles arrays
			$.each($('#'+el.id+' img'), function(i,item){
				images[el.id][i] 		= $(item).attr('src');
				links[el.id][i] 		= $(item).parent().is('a') ? $(item).parent().attr('href') : '';
				linksTarget[el.id][i] 	= $(item).parent().is('a') ? $(item).parent().attr('target') : '';
				titles[el.id][i] 		= $(item).next().is('span') ? $(item).next().html() : '';
				$(item).hide();
				$(item).next().hide();
			});			
			

			// set panel
			$(el).css({
				'background-image':'url('+images[el.id][0]+')',
				'width': params[el.id].width,
				'height': params[el.id].height,
				'position': 'relative',
				'background-position': 'top left'
			}).wrap("<div class='coin-slider' id='coin-slider-"+el.id+"' />");	
			
				
			// create title bar
			$('#'+el.id).append("<div class='cs-title' id='cs-title-"+el.id+"' style='position: absolute; bottom:0; left: 0; z-index: 1000;'></div>");
						
			$.setFields(el);
			
			if(params[el.id].navigation)
				$.setNavigation(el);
			
			$.transition(el,0);
			$.transitionCall(el);
				
		}
		
		// squares positions
		$.setFields = function(el){
			
			tWidth = sWidth = parseInt(params[el.id].width/params[el.id].spw);
			tHeight = sHeight = parseInt(params[el.id].height/params[el.id].sph);
			
			counter = sLeft = sTop = 0;
			tgapx = gapx = params[el.id].width - params[el.id].spw*sWidth;
			tgapy = gapy = params[el.id].height - params[el.id].sph*sHeight;
			
			for(i=1;i <= params[el.id].sph;i++){
				gapx = tgapx;
				
					if(gapy > 0){
						gapy--;
						sHeight = tHeight+1;
					} else {
						sHeight = tHeight;
					}
				
				for(j=1; j <= params[el.id].spw; j++){	

					if(gapx > 0){
						gapx--;
						sWidth = tWidth+1;
					} else {
						sWidth = tWidth;
					}

					order[el.id][counter] = i+''+j;
					counter++;
					
					if(params[el.id].links)
						$('#'+el.id).append("<a href='"+links[el.id][0]+"' class='cs-"+el.id+"' id='cs-"+el.id+i+j+"' style='width:"+sWidth+"px; height:"+sHeight+"px; float: left; position: absolute;'></a>");
					else
						$('#'+el.id).append("<div class='cs-"+el.id+"' id='cs-"+el.id+i+j+"' style='width:"+sWidth+"px; height:"+sHeight+"px; float: left; position: absolute;'></div>");
								
					// positioning squares
					$("#cs-"+el.id+i+j).css({ 
						'background-position': -sLeft +'px '+(-sTop+'px'),
						'left' : sLeft ,
						'top': sTop
					});
				
					sLeft += sWidth;
				}

				sTop += sHeight;
				sLeft = 0;					
					
			}
			
			
			$('.cs-'+el.id).mouseover(function(){
				$('#cs-navigation-'+el.id).show();
			});
		
			$('.cs-'+el.id).mouseout(function(){
				$('#cs-navigation-'+el.id).hide();
			});	
			
			$('#cs-title-'+el.id).mouseover(function(){
				$('#cs-navigation-'+el.id).show();
			});
		
			$('#cs-title-'+el.id).mouseout(function(){
				$('#cs-navigation-'+el.id).hide();
			});	
			
			if(params[el.id].hoverPause){	
				$('.cs-'+el.id).mouseover(function(){
					params[el.id].pause = true;
				});
			
				$('.cs-'+el.id).mouseout(function(){
					params[el.id].pause = false;
				});	
				
				$('#cs-title-'+el.id).mouseover(function(){
					params[el.id].pause = true;
				});
			
				$('#cs-title-'+el.id).mouseout(function(){
					params[el.id].pause = false;
				});	
			}
					
			
		};
				
		
		$.transitionCall = function(el){
		
			clearInterval(interval[el.id]);	
			delay = params[el.id].delay + params[el.id].spw*params[el.id].sph*params[el.id].sDelay;
			interval[el.id] = setInterval(function() { $.transition(el)  }, delay);
			
		}
		
		// transitions
		$.transition = function(el,direction){
			
			if(params[el.id].pause == true) return;
			
			$.effect(el);
			
			squarePos[el.id] = 0;
			appInterval[el.id] = setInterval(function() { $.appereance(el,order[el.id][squarePos[el.id]])  },params[el.id].sDelay);
					
			$(el).css({ 'background-image': 'url('+images[el.id][imagePos[el.id]]+')' });
			
			if(typeof(direction) == "undefined")
				imagePos[el.id]++;
			else
				if(direction == 'prev')
					imagePos[el.id]--;
				else
					imagePos[el.id] = direction;
		
			if  (imagePos[el.id] == images[el.id].length) {
				imagePos[el.id] = 0;
			}
			
			if (imagePos[el.id] == -1){
				imagePos[el.id] = images[el.id].length-1;
			}
	
			$('.cs-button-'+el.id).removeClass('cs-active');
			$('#cs-button-'+el.id+"-"+(imagePos[el.id]+1)).addClass('cs-active');
			
			if(titles[el.id][imagePos[el.id]]){
				$('#cs-title-'+el.id).css({ 'opacity' : 0 }).animate({ 'opacity' : params[el.id].opacity }, params[el.id].titleSpeed);
				$('#cs-title-'+el.id).html(titles[el.id][imagePos[el.id]]);
			} else {
				$('#cs-title-'+el.id).css('opacity',0);
			}				
				
		};
		
		$.appereance = function(el,sid){

			$('.cs-'+el.id).attr('href',links[el.id][imagePos[el.id]]).attr('target',linksTarget[el.id][imagePos[el.id]]);

			if (squarePos[el.id] == params[el.id].spw*params[el.id].sph) {
				clearInterval(appInterval[el.id]);
				return;
			}

			$('#cs-'+el.id+sid).css({ opacity: 0, 'background-image': 'url('+images[el.id][imagePos[el.id]]+')' });
			$('#cs-'+el.id+sid).animate({ opacity: 1 }, 300);
			squarePos[el.id]++;
			
		};
		
		// navigation
		$.setNavigation = function(el){
			// create prev and next 
			/*
			$(el).append("<div id='cs-navigation-"+el.id+"'></div>");
			$('#cs-navigation-'+el.id).hide();
			
			$('#cs-navigation-'+el.id).append("<a href='#' id='cs-prev-"+el.id+"' class='cs-prev'>prev</a>");
			$('#cs-navigation-'+el.id).append("<a href='#' id='cs-next-"+el.id+"' class='cs-next'>next</a>");
			$('#cs-prev-'+el.id).css({
				'position' 	: 'absolute',
				'top'		: params[el.id].height/2 - 15,
				'left'		: 0,
				'z-index' 	: 1001,
				'line-height': '30px',
				'opacity'	: params[el.id].opacity
			}).click( function(e){
				e.preventDefault();
				$.transition(el,'prev');
				$.transitionCall(el);		
			}).mouseover( function(){ $('#cs-navigation-'+el.id).show() });
	
			$('#cs-next-'+el.id).css({
				'position' 	: 'absolute',
				'top'		: params[el.id].height/2 - 15,
				'right'		: 0,
				'z-index' 	: 1001,
				'line-height': '30px',
				'opacity'	: params[el.id].opacity
			}).click( function(e){
				e.preventDefault();
				$.transition(el);
				$.transitionCall(el);
			}).mouseover( function(){ $('#cs-navigation-'+el.id).show() });
		    */
			// image buttons
			$("<div id='cs-buttons-"+el.id+"' class='cs-buttons'></div>").appendTo($('#coin-slider-'+el.id));

			
			for(k=1;k<images[el.id].length+1;k++){
				$('#cs-buttons-'+el.id).append("<a href='#' class='cs-button-"+el.id+"' id='cs-button-"+el.id+"-"+k+"'>"+k+"</a>");
			}
			
			$.each($('.cs-button-'+el.id), function(i,item){
				$(item).click( function(e){
					$('.cs-button-'+el.id).removeClass('cs-active');
					$(this).addClass('cs-active');
					e.preventDefault();
					$.transition(el,i);
					$.transitionCall(el);				
				})
			});	
			
			$('#cs-navigation-'+el.id+' a').mouseout(function(){
				$('#cs-navigation-'+el.id).hide();
				params[el.id].pause = false;
			});						

			$("#cs-buttons-"+el.id).css({
				'left'			: '50%',
				'margin-left' 	: -images[el.id].length*15/2-5,
				'position'		: 'relative'
				
			});
			
				
		}




		// effects
		$.effect = function(el){
			
			effA = ['random','swirl','rain','straight'];
			if(params[el.id].effect == '')
				eff = effA[Math.floor(Math.random()*(effA.length))];
			else
				eff = params[el.id].effect;

			order[el.id] = new Array();

			if(eff == 'random'){
				counter = 0;
				  for(i=1;i <= params[el.id].sph;i++){
				  	for(j=1; j <= params[el.id].spw; j++){	
				  		order[el.id][counter] = i+''+j;
						counter++;
				  	}
				  }	
				$.random(order[el.id]);
			}
			
			if(eff == 'rain')	{
				$.rain(el);
			}
			
			if(eff == 'swirl')
				$.swirl(el);
				
			if(eff == 'straight')
				$.straight(el);
				
			reverse[el.id] *= -1;
			if(reverse[el.id] > 0){
				order[el.id].reverse();
			}

		}

			
		// shuffle array function
		$.random = function(arr) {
						
		  var i = arr.length;
		  if ( i == 0 ) return false;
		  while ( --i ) {
		     var j = Math.floor( Math.random() * ( i + 1 ) );
		     var tempi = arr[i];
		     var tempj = arr[j];
		     arr[i] = tempj;
		     arr[j] = tempi;
		   }
		}	
		
		//swirl effect by milos popovic
		$.swirl = function(el){

			var n = params[el.id].sph;
			var m = params[el.id].spw;

			var x = 1;
			var y = 1;
			var going = 0;
			var num = 0;
			var c = 0;
			
			var dowhile = true;
						
			while(dowhile) {
				
				num = (going==0 || going==2) ? m : n;
				
				for (i=1;i<=num;i++){
					
					order[el.id][c] = x+''+y;
					c++;

					if(i!=num){
						switch(going){
							case 0 : y++; break;
							case 1 : x++; break;
							case 2 : y--; break;
							case 3 : x--; break;
						
						}
					}
				}
				
				going = (going+1)%4;

				switch(going){
					case 0 : m--; y++; break;
					case 1 : n--; x++; break;
					case 2 : m--; y--; break;
					case 3 : n--; x--; break;		
				}
				
				check = $.max(n,m) - $.min(n,m);			
				if(m<=check && n<=check)
					dowhile = false;
									
			}
		}

		// rain effect
		$.rain = function(el){
			var n = params[el.id].sph;
			var m = params[el.id].spw;

			var c = 0;
			var to = to2 = from = 1;
			var dowhile = true;


			while(dowhile){
				
				for(i=from;i<=to;i++){
					order[el.id][c] = i+''+parseInt(to2-i+1);
					c++;
				}
				
				to2++;
				
				if(to < n && to2 < m && n<m){
					to++;	
				}
				
				if(to < n && n>=m){
					to++;	
				}
				
				if(to2 > m){
					from++;
				}
				
				if(from > to) dowhile= false;
				
			}			

		}

		// straight effect
		$.straight = function(el){
			counter = 0;
			for(i=1;i <= params[el.id].sph;i++){
				for(j=1; j <= params[el.id].spw; j++){	
					order[el.id][counter] = i+''+j;
					counter++;
				}
				
			}
		}

		$.min = function(n,m){
			if (n>m) return m;
			else return n;
		}
		
		$.max = function(n,m){
			if (n<m) return m;
			else return n;
		}		
	
	this.each (
		function(){ init(this); }
	);
	

	};
	
	
	// default values
	$.fn.coinslider.defaults = {	
		width: 565, // width of slider panel
		height: 290, // height of slider panel
		spw: 7, // squares per width
		sph: 5, // squares per height
		delay: 3000, // delay between images in ms
		sDelay: 30, // delay beetwen squares in ms
		opacity: 0.7, // opacity of title and navigation
		titleSpeed: 500, // speed of title appereance in ms
		effect: '', // random, swirl, rain, straight
		navigation: true, // prev next and buttons
		links : true, // show images as links 
		hoverPause: true // pause on hover		
	};	
	
})(jQuery);
