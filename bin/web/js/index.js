 var ipc_saved_usr = new Array();
 
var usr;
var pwd;
var dvr_url;
var g_ip;
var g_port;
var disp_delaytime_ms = 1000;
var hide_delaytime_ms = 7000;
 
 window.onload = function(){
	$('#userName').val('');
    //分析cookie值，显示上次的登陆信息
    //alert(document.cookie);
	
	var login = document.getElementById("submit").innerHTML;

	var str = Cookies.get("login");
	if(str != null)
	{
		var strs = str.split(",");
		for(var i = 0; i < strs.length/2; i++)
		{
			ipc_saved_usr.push(new Array(strs[i*2 + 0], strs[i*2 + 1]));
		}
		$('#userName').val(strs[0]);
		$('#password').val(strs[1]);
	}   
    var tmp_ip=document.location.hostname;
	g_ip=document.location.hostname;
	var tmp_port=document.location.port;
	if(tmp_port == "")	//80
	{
		tmp_port = 80;
	}
	else
	{
		var tmp_index = document.location.host.indexOf(":");
		tmp_ip = document.location.host.substring(0, tmp_index);
	}
	dvr_url = "http://" + tmp_ip +  ":" + tmp_port;
	
	 //写入点击事件
    $("#submit").click(function(){
		g_usr = $("#userName").val();
		g_pwd = $("#password").val();
		//var url = 'http'+document.location.hostname+':'+document.location.port; //'http://192.168.2.220:80';   //测试用地址
		/*var xml = loadXMLString(toXMLString("<juan ver=\"\" squ=\"\" dir=\"0\"><rpermission usr=\"" + usr + "\" pwd=\"" + pwd + "\" ><config base=\"\" /><playback base=\"\" /></rpermission></juan>"));
		//alert(xml);
		$.ajax({ 
		type:"GET",
		url:"/cgi-bin/gw.cgi",  //地址有问题
		processData: false, 
		cache: false,
		data: "xml=" + xml, 
		async:true,
		success: function(data, textStatus){
			
		
		//alert(ipc_saved_usr.join());
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(123);	
		}
	});*/

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/user_list.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd, 
		async:true,
//		dataType: 'get',
//		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus, xmlhttp){
			//alert("recv:" + data);
			//alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				setTimeout("showInfo(langstr.login_fail)",disp_delaytime_ms);
				setTimeout("hideInfo()",hide_delaytime_ms);
				//alert(langstr.login_fail);	
			}
			else
			{
				if( $("saveCookie").checked){  
					setCookie("userName",$("userName").value,24,"/");
					setCookie("password",$("password").value,24,"/");
				}    
				//alert("登陆成功,欢迎你," + userNameValue + "!");
				login_set(g_usr,g_pwd, $('#timeAlign').is(':checked'));
				self.location.replace("view.html");
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error:" + textStatus);
			//alert(langstr.login_fail);	
			setTimeout("showInfo(langstr.login_fail)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		}
	});	
		if(g_usr!=''){
			if($('#saveCookie').is(':checked')){
				save_usr(g_usr,g_pwd);
			}else{
				del_usr(g_usr);
			}
			setCookie30Days("login", ipc_saved_usr.join());		    
		}else{
			Cookies.clear('login');
		}		
	})
}

/*if($('#saveCookie :selected')){
				var usr = $('#userName').val();
				var pwd = $('#password').val();
			}
			
			//var userNameValue = $("userName").value ? $('#userName').val() : $("userName").value;//取前台用户名的值
			//var passwordValue = $("password").value ? $('#password').val() : $("password").value; //取前台密码的值
			//服务器验证（模拟） 这里是对用户的一个验证．在项目中，你读取数据库的代码写在这 
			var isUserA = userNameValue == "java" && passwordValue =="123456";
			var isMatched = isUserA;
			if(isMatched){ //如果存在这个用户,就把用户名和密码存进cookie
				if( $("saveCookie").checked){  
					setCookie("userName",$("userName").value,24,"/");
					setCookie("password",$("password").value,24,"/");
				}    
				//alert("登陆成功,欢迎你," + userNameValue + "!");
				self.location.replace("login.html");
			}
			//else alert("用户名或密码错误，请重新输入！");*/
			
function save_usr(_usr, _pwd)
{
	//save
	var modified = false;		
	for(var i = 0; i < ipc_saved_usr.length; i++)
	{
		if(ipc_saved_usr[i][0] == _usr) //modify
		{
			ipc_saved_usr[i][1] = _pwd;
			modified = true;
			break;
		}
	}
	if(modified == false) //add
	{
		ipc_saved_usr.push(new Array(_usr, _pwd));
	}
}

function del_usr(_usr)
{
	//delete
	for(var i = 0; i < ipc_saved_usr.length; i++)
	{
		if(ipc_saved_usr[i][0] == _usr)
		{
			ipc_saved_usr.splice(i, 1);
		}else{
      Cookies.clear(_usr);
    }
	}
}
//显示信息
function showInfo(para, callback){
	if(!$('#txtLoadinInfo').length){
		$('#message_here').append('<span id="txtLoadinInfo">' + para +'</span>');
	}else
	{
		$('#txtLoadinInfo')[0].innerText = para;
	};
	$('#txtLoadinInfo').stop(false, true).css({
		opacity:0
	}).animate({
		opacity:1
	},250, function(){
		if ($.isFunction(callback)) {
			callback();
		};
	});
};

//隐藏信息
function hideInfo(para){
	window.setTimeout(function(){
		$('#txtLoadinInfo').stop(false, true).animate({
			opacity:0
		}, 250, function(){
			$(this).remove();
		});
	}, para);
};

