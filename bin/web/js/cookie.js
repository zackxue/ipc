var Cookies = {};
/**//**
 * 设置Cookies
 */
Cookies.set = function(name, value){
     var argv = arguments;
     var argc = arguments.length;
     var expires = (argc > 2) ? argv[2] : null;
     var path = (argc > 3) ? argv[3] : '/';
     var domain = (argc > 4) ? argv[4] : null;
     var secure = (argc > 5) ? argv[5] : false;
     document.cookie = name + "=" + escape (value) +
       ((expires == null) ? "" : ("; expires=" + expires.toGMTString())) +
       ((path == null) ? "" : ("; path=" + path)) +
       ((domain == null) ? "" : ("; domain=" + domain)) +
       ((secure == false) ? "" : "; secure");

};
/**//**
 * 读取Cookies
 */
Cookies.get = function(name){
    var arg = name;
    var alen = arg.length;
    var clen = document.cookie.length;
    var i = 0;
    var j = 0;
    while(i < clen){
        j = i + alen;
        if (document.cookie.substring(i, j) == arg)
            return Cookies.getCookieVal(j + 1);
        i = document.cookie.indexOf(" ", i) + 1;
        if(i == 0)
            break;
    }
    return null;
};
/**//**
 * 清除Cookies
 */
Cookies.clear = function(name) {
  if(Cookies.get(name)){
    var expdate = new Date(); 
    expdate.setTime(expdate.getTime() - (86400 * 1000 * 1)); 
    Cookies.set(name, "", expdate); 
  }
};

Cookies.getCookieVal = function(offset){
   var endstr = document.cookie.indexOf(";", offset);
   if(endstr == -1){
       endstr = document.cookie.length;
   }
   if(document.cookie.substring(offset,endstr) == 'null'){
	   return '';
   }else{
		return unescape(document.cookie.substring(offset,endstr));
   }
   
};

 function setCookie30Days(name,value)
{
    var Days = 30;
    var exp  = new Date();
    exp.setTime(exp.getTime() + Days*24*60*60*1000);
    document.cookie = name + "="+ escape (value) + ";expires=" + exp.toGMTString();
}

function login_set(usr,pwd,auto_sync_time)
{
    //pwd = !pwd ? 'null' : pwd;
    Cookies.set("sync_time", auto_sync_time);
	Cookies.set("usr", usr);
	Cookies.set("pwd", pwd);
	
}
