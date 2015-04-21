var lang_cookie="juanipcam_lang";
var lang_area="";
var support_lang=new Array("en","zh-cn","zh-tw");

function setCookie(c_name,value,exdays)
{
var exdate=new Date();
exdate.setDate(exdate.getDate() + exdays);
var c_value=escape(value) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());
document.cookie=c_name + "=" + c_value;
}

function getCookie(c_name)
{
var i,x,y,ARRcookies=document.cookie.split(";");
for (i=0;i<ARRcookies.length;i++)
{
  x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));
  y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);
  x=x.replace(/^\s+|\s+$/g,"");
  if (x==c_name)
    {
    return unescape(y);
    }
  }
}
	
function getDefaultLanguage()
{
	var type=navigator.appName;
	var i_lang;
	var slang;
	var sarea="";
	//load from cookie first
	i_lang=getCookie(lang_cookie);
	if(i_lang == null || i_lang == ""){
		if(type=="Netscape"){
		i_lang = navigator.language;
		} else {
		i_lang = navigator.userLanguage;
		}
		//alert("navigator:"+type+", language code:"+i_lang);
		if(i_lang == null || i_lang== ""){ i_lang = "en";}// maybe there is a browser witch don't support internationalization
		i_lang=i_lang.toLowerCase();
		slang=i_lang.substring(0,2);
		if(i_lang=="zh" || i_lang=="zh-cn")	{slang = "zh-cn";}
		else if(i_lang=="zh-hk" || i_lang=="zh-sg" || i_lang=="zh-tw"){slang = "zh-tw";}
		//alert("get language : "+slang);
		if(support_lang.join().indexOf(slang)<0){ slang="en";}// if we don't support this language
		// set cookie
		setCookie(lang_cookie,slang,10);//expire after 10 days
	} else {	
		slang=i_lang;
		if(support_lang.join().indexOf(slang)<0){ slang="en";}// if we don't support this language
		//alert("get language from cookie : "+slang);
	}
	var sz_language="js/"+slang+".js";	
	return sz_language;
}

function change_page_string()
{
document.getElementById("record").innerHTML=langstr.record;
document.getElementById("playback").innerHTML=langstr.playback;
document.getElementById("logo-config").getElementsByTagName("span")[0].innerHTML=langstr.preview;
document.getElementById("logo-config").getElementsByTagName("span")[1].innerHTML=langstr.setup;
document.getElementById("consoler-title").innerHTML=langstr.console;
document.getElementById("zoom").innerHTML=langstr.zoom;
document.getElementById("fullscreen").innerHTML=langstr.full_screen;
document.getElementById("stream-change").innerHTML=langstr.stream_change;
if (stream_state == 1){
	$('.window-preview').children('.title-window')[0].innerHTML = langstr.main_stream;
}else if (stream_state == 1){
	$('.window-preview').children('.title-window')[0].innerHTML = langstr.sub_stream;
}
//window.location.reload(false);	
}

function change_language(sel_lang)
{
	var xmlhttp;
	// make a async http request
	if (window.XMLHttpRequest)
	{// code for IE7+, Firefox, Chrome, Opera, Safari
		xmlhttp=new XMLHttpRequest();
	}
	else
	{// code for IE6, IE5
		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}
	// set onready callback
	xmlhttp.onreadystatechange=function()
	{
		if (xmlhttp.readyState==4 && xmlhttp.status==200)
		{
			var old_lang_elem=document.getElementById("js_lang");
			var parent_elem=old_lang_elem.parentNode;
			var new_lang_elem=document.createElement("script");
			new_lang_elem.text=xmlhttp.responseText;
			new_lang_elem.setAttribute("id", "js_lang");
			parent_elem.replaceChild(new_lang_elem,old_lang_elem);
			
			eval(xmlhttp.responseText);// execute the js
			change_page_string();
		}
	}
	var lang_path="js/"+sel_lang+".js";
	xmlhttp.open("GET",lang_path,true);// async
	xmlhttp.send();
}

function setLanguage(lang)
{
	var cur_lang=getCookie(lang_cookie);
	if(cur_lang == lang){	// same with current
		return;
	}
	if(lang == "lang"){
		lang=support_lang[document.getElementById("sel_lang").selectedIndex-1];
		document.getElementById("sel_lang").selectedIndex=0;
	}
	if(support_lang.join().indexOf(lang) < -1){// not support
		lang="en";
	}
	//alert("current set lang:"+lang);
	setCookie(lang_cookie,lang,10);
	//change_language(lang); // use for change language without update
	self.location.reload();
}
