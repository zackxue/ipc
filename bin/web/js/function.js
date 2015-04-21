//initial
var rtsp;
var brower_type="ie";
var bFullScreen = false;
var g_usr;
var g_pwd;
var dvr_url;
var g_ip;
var g_port;

//video

var dvr_ajax;
//image
var slider_sha;
var slider_hue;
var slider_con;
var slider_lum;
var slider_sat;
var sharpen=0;
var hue=0;
var saturation=0;
var brightness=0;
var contrast=0;
var disp_delaytime_ms = 2000;
var hide_delaytime_ms = 5000;
var g_model = "";
//isp
var slider_str;
var slider_AEcom;
var slider_denoise_strength;
var strength=0;
var ae_compensation=0;
var denoise_strength=0;

function flash_video(_video_id, _stream_name)
{

	var obj = document.getElementById(_video_id);
	if(obj && obj.ConnectRTMP)
	{
		obj.SetPlayerNum(1);
		obj.SetPoster(0, dvr_url+"/snapshot?chn=1&u=" + g_usr + "&p=" + g_pwd + "&q=0&d=1&rand=" + Math.random());
		obj.ConnectRTMP(0, "rtmp://"+g_ip + ":" + g_port,_stream_name);
		
		obj.SetBufferTime(0.05);
	}
	else
	{
		setTimeout("flash_video(\"" + _video_id + "\", \"" + _stream_name + "\")", 1000);
	}

}

$(document).ready(function(){
	var tmp_ip=document.location.hostname;
	g_ip=document.location.hostname;
	var tmp_port=document.location.port;
	renewtime();
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
	g_usr = Cookies.get("usr");
	g_pwd = Cookies.get("pwd");
	var bEnable_sync_time = Cookies.get("sync_time");
	if(bEnable_sync_time == 'true'){
		sync_pc_time();
	}
	load1();
});

var __ipcam_display_content = false;
function flv_view(streamtype)
{
	if(__ipcam_display_content == false)
	{
//		__ipcam_display_content = true;
		var chn = streamtype;
		var str = "<object id=\"JaViewer_view\" type=\"application/x-shockwave-flash\" data=\"JaViewer.swf\" style=\"outline: medium none;\" height=\"100%;\" width=\"100%\">";
		str += "<param name=\"movie\" value=\"JaViewer.swf\">";
		str += "<param name=\"allowFullScreen\" value=\"true\">";
		str += "<param name=\"allowScriptAccess\" value=\"always\">";
		str += "</object>";
		document.getElementById("ipcam_display").innerHTML = str;
		var player = document.getElementById("ipcam_display");
		player.innerHTML = str;
	}
	if(streamtype == 0){
		setTimeout("flash_video(\"JaViewer_view\", \"720p.264\")", 2000);
	}else if(streamtype == 1){
		setTimeout("flash_video(\"JaViewer_view\", \"360p.264\")", 2000);
	}else{
		setTimeout("flash_video(\"JaViewer_view\", \"720p.264\")", 2000);
	}
//	}
}

var DHiMPlayer,DHiMPlayer_image,DHiMPlayer_isp;
function connect()
{
	if(__ipcam_display_content == false)
	{
		__ipcam_display_content = true;
		var str = '<object id="DHiMPlayer" name="TestR" classid="clsid:F1649976-BD20-4689-BB65-E5A64EF61C7E" height = "1px;" width = "1px;" ></object>';
		document.getElementById("ipcam_display").innerHTML = str;
		DHiMPlayer = document.getElementById("DHiMPlayer");
		DHiMPlayer.attachEvent("InitInstance",function Init(ID,sInfo){});
		DHiMPlayer.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer.FullScreen(bFullScreen = !bFullScreen);});
		DHiMPlayer.attachEvent("ConRefuse",function OnConRefuse(res){
			var RefuseMsg = langstr.connect_refuse;
			alert(RefuseMsg);
		});	
	}
	
	if (DHiMPlayer.Version == "undefined")
	{
		load_attract();
	}
	else if(DHiMPlayer.Version >= "1, 0, 1, 3")
	{
		$('.mask_ipcam').css('display','none');
		$(DHiMPlayer).width('100%').height('100%');
		while(DHiMPlayer.ReadyFlag != true);
		DHiMPlayer.OpenByIP(g_ip,g_port);
		DHiMPlayer.UserLogon(g_usr,g_pwd);
		DHiMPlayer.OpenChannel(0,0);
	}
	else
	{
		load_attract();
	}
}
function load_attract(){
	var btn = false;
    var timer = setInterval(function(){
			if(!btn){
				$("#manual_download").css("background-image","url(images/stream-change-hover.png)");
				$('.blink').css('color','#E6AF14');			
				btn = true;
			}else{
				$("#manual_download").css("background-image","url(images/stream-change.png)");
				$('.blink').css('color','#666');
				btn = false;
				}
		},500)
	
	}
function openChannel()
{
//	DHiMPlayer.OpenChannelEx(0);
}

function check_os_and_browser()
{
	var ua = navigator.userAgent;
	var ret = new Object();
	if(/MSIE/i.test(ua) == true)
	{
		ret.os = "windows";
		if(/MSIE 9/i.test( ua ) == true)
		{
			ret.browser = "ie9";
		}
		else if(/MSIE 8/i.test( ua ) == true)
		{
			ret.browser = "ie8";
		}
		else if(/MSIE 7/i.test( ua ) == true)
		{
			ret.browser = "ie7";
		}
		else if(/MSIE 6/i.test( ua ) == true)
		{
			ret.browser = "ie6";
		}
		else
		{
			ret.browser = "ie";
		}
	}
	else if(/Chrome/i.test( ua ) == true)
	{
		ret.browser = "chrome";
		if(/Macintosh/i.test( ua ) == true)
		{
			ret.os = "mac";
		}
		else if(/Windows/i.test( ua ) == true)
		{
			ret.os = "windows";
		}
		else if(/Android/i.test( ua ) == true)
		{
			ret.os = "android";
		}
		else if(/Linux/i.test( ua ) == true)
		{
			ret.os = "linux";
		}
		else
		{
			ret.os = "";
		}	
	}
	else if(/Safari/i.test( ua ) == true)
	{
		ret.browser = "safari";
		if(/Macintosh/i.test( ua ) == true)
		{
			ret.os = "mac";
		}
		else if(/iPhone/i.test( ua ) == true)
		{
			ret.os = "iphone";
		}
		else if(/iPad/i.test( ua ) == true)
		{
			ret.os = "ipad";
		}
		else if(/Windows/i.test( ua ) == true)
		{
			ret.os = "windows";
		}
		else if(/Android/i.test( ua ) == true)
		{
			ret.os = "android";
		}
		else if(/Linux/i.test( ua ) == true)
		{
			ret.os = "linux";
		}
		else
		{
			ret.os = "";
		}
	}
	else if(/Firefox/i.test( ua ) == true)
	{
		ret.browser = "firefox";
		if(/Macintosh/i.test( ua ) == true)
		{
			ret.os = "mac";
		}
		else if(/Windows/i.test( ua ) == true)
		{
			ret.os = "windows";
		}
		else if(/Linux/i.test( ua ) == true)
		{
			ret.os = "linux";
		}
		else
		{
			ret.os = "";
		}
	}
	else
	{
		ret.browser = "";
		ret.os = "";
	}
	
//	if(ret.os == "" || ret.browser == "")
//	{
//		alert(ua);	
//	}
//	alert("os="+ret.os+",browser="+ret.browser);
	return ret;
}

function load1()
{
	g_port=document.location.port;

	if(g_port=="")	//80
	{ 
		g_port=80;
	}
/*	else
	{
		var i=document.location.host.indexOf(":");
		ip=document.location.host.substring(0,i);
	}
*/
/*
	if (navigator.appName.indexOf("Microsoft Internet Explorer") != -1)
	{
		connect()
		openChannel();
	}//end ie
	else if(navigator.appName.indexOf("Netscape") != -1)//chrome,firefox
	{
		flv_view(0);
	}
*/
	var os_browser = check_os_and_browser();
	if(os_browser.browser == "ie6" || os_browser.browser == "ie7" || os_browser.browser == "ie8" || os_browser.browser == "ie9")
	{
		$('.mask_ipcam').css('display','block');
		connect()
	//	openChannel();
	}
	else if(os_browser.os == "iphone" || os_browser.os == "ipad")
	{
		flv_view(0);
	}
	else if(os_browser.os == "android")
	{
		flv_view(0);
	}
	else
	{
		flv_view(0);
	}
}

function streamchange(streamtype)
{
	if (navigator.appName.indexOf("Microsoft Internet Explorer") != -1)
	{
		//DHiMPlayer.CloseChannelEx(streamtype?0:1);
		//DHiMPlayer.OpenChannelEx(streamtype);
		//location.reload();
		if(DHiMPlayer){
				DHiMPlayer.OpenByIP(g_ip,g_port);
				DHiMPlayer.UserLogon(g_usr,g_pwd);
				DHiMPlayer.OpenChannel(0,stream_state);
			}
	}
	else if (navigator.appName.indexOf("Netscape") != -1)
	{
		flv_view(streamtype);
	}
	else{
		alert("browser not support!");
	}
}

function snap()
{
//	DHiMPlayer.Snapshot();
}
function records()
{
//	if(DHiMPlayer.GetLocalRecStatus(0)==true)
//	{
//		DHiMPlayer.EnableLocalRec(0,false);
//	}
//	else
//	{
//		DHiMPlayer.EnableLocalRec(0,true);
//	}
}
function playback()
{
//	DHiMPlayer.PlayBack();
}
function fullscreen_func()
{
	var os_browser = check_os_and_browser();
	if(os_browser.browser == "ie6" || os_browser.browser == "ie7" || os_browser.browser == "ie8" || os_browser.browser == "ie9")
	{
		var DHiMPlayer=document.getElementById("Test");
		DHiMPlayer.fullscreen(bFullScreen = !bFullScreen);
	}
	else
	{
		var obj = document.getElementById("JaViewer_view");
		obj.fullStage();
	}
}

//消除object函数
function purify(obj)
{
}

//video function
function video_data2ui(dvr_data, sensor_type)
{
	var max_frate = 30;
	/*switch(dvr_data.juan.conf.vin0.shutter)
	{
	case "50hz":
		max_frate = 25;
		break;
	case "60hz":
	default:
		max_frate = 30;
		break;
	}*/
	
	for(var i = 0; i < $("#juan_envload\\#video\\@m_shutter")[0].options.length; i++)
	{
		if($("#juan_envload\\#video\\@m_shutter")[0].options[i].value == dvr_data.juan.conf.vin0.shutter)
		{
			$("#juan_envload\\#video\\@m_shutter")[0].selectedIndex = i;
		}
	}
	if(sensor_type == "ar0130"){
		$("#juan_envload\\#video\\@m_resol option").eq(2).html('1280x960');
	}else{
		$("#juan_envload\\#video\\@m_resol option").eq(2).html('1280x720');
	}
	reset_frame_rate_select($("#juan_envload\\#video\\@m_frate")[0], max_frate);
	reset_frame_rate_select($("#juan_envload\\#video\\@s_frate")[0], max_frate);
	if(g_model == "N18A"){
	reset_frame_rate_select($("#juan_envload\\#video\\@ss_frate")[0], max_frate);
	}

//	for(var i = $("#juan_envload\\#video\\@m_frate")[0].options.length - 1; i >= 0; i--)
//	{
//		$("#juan_envload\\#video\\@m_frate")[0].options.remove(i);
//	}
//	for(var i = 0; i < max_frate; i++)
//	{
//		var oOption = document.createElement("OPTION");
//		oOption.text = i + 1 + "fps";
//		oOption.value = i + 1;
//		$("#juan_envload\\#video\\@m_frate")[0].add(oOption);
//	}
//
//	for(var i = $("#juan_envload\\#video\\@s_frate")[0].options.length - 1; i >= 0; i--)
//	{
//		$("#juan_envload\\#video\\@s_frate")[0].options.remove(i);
//	}
//	for(var i = 0; i < max_frate; i++)
//	{
//		var oOption = document.createElement("OPTION");
//		oOption.text = i + 1 + "fps";
//		oOption.value = i + 1;
//		$("#juan_envload\\#video\\@s_frate")[0].add(oOption);
//	}
//
//	for(var i = $("#juan_envload\\#video\\@ss_frate")[0].options.length - 1; i >= 0; i--)
//	{
//		$("#juan_envload\\#video\\@ss_frate")[0].options.remove(i);
//	}
//	for(var i = 0; i < max_frate; i++)
//	{
//		var oOption = document.createElement("OPTION");
//		oOption.text = i + 1 + "fps";
//		oOption.value = i + 1;
//		$("#juan_envload\\#video\\@ss_frate")[0].add(oOption);
//	}

	
	$("#juan_envload\\#video\\@m_bitrate")[0].value = dvr_data.juan.conf.vin0.encode_h2640.stream0.bps;
	$("#juan_envload\\#video\\@s_bitrate")[0].value = dvr_data.juan.conf.vin0.encode_h2641.stream0.bps;
	if(g_model == "N18A"){
		$("#juan_envload\\#video\\@ss_bitrate")[0].value = dvr_data.juan.conf.vin0.encode_h2642.stream0.bps;      //码流
	}
	if(max_frate >= dvr_data.juan.conf.vin0.encode_h2640.stream0.fps){
		$("#juan_envload\\#video\\@m_frate")[0].value = dvr_data.juan.conf.vin0.encode_h2640.stream0.fps;
	}else{
		$("#juan_envload\\#video\\@m_frate")[0].value = max_frate;
	}
	if(max_frate >= dvr_data.juan.conf.vin0.encode_h2641.stream0.fps){
		$("#juan_envload\\#video\\@s_frate")[0].value = dvr_data.juan.conf.vin0.encode_h2641.stream0.fps;
	}else{
		$("#juan_envload\\#video\\@s_frate")[0].value = max_frate;
		}
	if(g_model == "N18A"){
		if(max_frate >= dvr_data.juan.conf.vin0.encode_h2642.stream0.fps){
			$("#juan_envload\\#video\\@ss_frate")[0].value = dvr_data.juan.conf.vin0.encode_h2642.stream0.fps;		//帧率
		}else{
			$("#juan_envload\\#video\\@ss_frate")[0].value = max_frate;
		}
	}
	if(dvr_data.juan.conf.vin0.encode_h2640.stream0.mode == "cbr")											//码流可变
	{
		$("#juan_envload\\#video\\@m_brfmt")[0].checked = true;
	}
	else
	{
		$("#juan_envload\\#video\\@m_brfmt1")[0].checked = true;
	}
	if(dvr_data.juan.conf.vin0.encode_h2641.stream0.mode == "cbr")
	{
		$("#juan_envload\\#video\\@s_brfmt")[0].checked = true;
	}
	else
	{
		$("#juan_envload\\#video\\@s_brfmt1")[0].checked = true;
	}
	/*if(dvr_data.juan.conf.vin0.encode_h2642.stream0.mode == "cbr")
	{
		$("#juan_envload\\#video\\@ss_brfmt")[0].checked = true;
	}
	else
	{
		$("#juan_envload\\#video\\@ss_brfmt1")[0].checked = true;
	}*/
	m_bps_type();
	s_bps_type();
	ss_bps_type();
	switch (dvr_data.juan.conf.vin0.encode_h2640.stream0.size)												//分辨犿	
	{
		case "180p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 0;break;
		case "360p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 1;break;
		case "720p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 2;break;
		default: break;
	}
	switch (dvr_data.juan.conf.vin0.encode_h2641.stream0.size)																						
	{
		case "180p": $("#juan_envload\\#video\\@s_resol")[0].selectedIndex = 0;break;
		case "360p": $("#juan_envload\\#video\\@s_resol")[0].selectedIndex = 1;break;
		//case "720p": $("#juan_envload\\#video\\@s_resol")[0].selectedIndex = 2;break;
		default: break;
	}
	/*switch (dvr_data.juan.conf.vin0.encode_h2642.stream0.size)																						
	{
		case "180p": $("#juan_envload\\#video\\@ss_resol")[0].selectedIndex = 0;break;
		case "360p": $("#juan_envload\\#video\\@ss_resol")[0].selectedIndex = 1;break;
		case "720p": $("#juan_envload\\#video\\@ss_resol")[0].selectedIndex = 2;break;
		default: break;
	}*/

	switch (dvr_data.juan.conf.vin0.encode_h2640.stream0.quality)											//编码质量
	{
		case "highest": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 0;break;
		case "high": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 1;break;
		case "medium": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 2;break;   
		case "low": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 3;break;
		case "lowest": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 4;break;
		default: break;
	}
	switch (dvr_data.juan.conf.vin0.encode_h2641.stream0.quality)
	{
		case "highest": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 0;break;
		case "high": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 1;break;
		case "medium": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 2;break;
		case "low": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 3;break;
		case "lowest": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 4;break;
		default: break;
	}	
	if(dvr_data.juan.conf.vin0.encode_h2642)
	{
		switch (dvr_data.juan.conf.vin0.encode_h2642.stream0.quality)
		{
			case "highest": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 0;break;
			case "high": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 1;break;
			case "medium": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 2;break;
			case "low": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 3;break;
			case "lowest": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 4;break;
			default: break;
		}
	}
}
function video_preload_content()
{
	devinfo_load_content(false);
}
function video_load_content(model, sensor_type)
{
	if(model == "N18C"){
		document.getElementById("substream2").style.display="none"; 
	}else{
		document.getElementById("substream2").style.display=""; 
	}
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<vin0 shutter="" capture="" hue="" contrast="" brightness="" saturation="" sharpen="">';
	xmlstr += '<md width="" height="" sensitivity="" data="" />';
	xmlstr += '<encode_h2640>';
	xmlstr += '<stream0 name="" profile="" size="" mode="" on_demand="" fps="" gop="" ain_bind="" quality="" bps="" />';
	xmlstr += '</encode_h2640>';
	xmlstr += '<encode_h2641>';
	xmlstr += '<stream0 name="" profile="" size="" mode="" on_demand="" fps="" gop="" ain_bind="" quality="" bps="" />';
	xmlstr += '</encode_h2641>';
	if(model == "N18A"){
		xmlstr += '<encode_h2642>';
		xmlstr += '<stream0 name="" profile="" size="" mode="" on_demand="" fps="" gop="" ain_bind="" quality="" bps="" />';
		xmlstr += '</encode_h2642>';
	}
	xmlstr += '<encode_jpeg0 name="" quality="" size="" />';
	xmlstr += '</vin0>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j",
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false);
			video_data2ui(dvr_data, sensor_type);			
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function bitrate_change(id,min_,max_)
{
	var inbox=document.getElementById(id);
	var str=inbox.value;
	if(isNaN(str)==true)
	{
		alert("Not a Number,Please retry");
		inbox.value=max_;
	}
	else{
		if(Number(str)>max_)
		{
			alert("Input number is too big,Please retry");
			inbox.value=max_;
		}
		else if(Number(str)<min_)
		{
			alert("Input number is too small,Please retry");
			inbox.value=min_;
		}
	}
}
function video_save_content()
{
	showInfo(langstr.save_setup);
	var m_bit_s, m_quality;
	var s_bit_s, s_quality;
	var ss_bit_s, ss_quality;
	var m_resol = document.getElementById('juan_envload#video@m_resol');
	var s_resol = document.getElementById('juan_envload#video@s_resol');
	var ss_resol = document.getElementById('juan_envload#video@ss_resol');
	switch ($("#juan_envload\\#video\\@m_qual")[0].selectedIndex)							//编码质量
	{
		case 0: m_quality = "highest";break;
		case 1: m_quality = "high";break;
		case 2: m_quality = "medium";break;
		case 3: m_quality = "low";break;
		case 4: m_quality = "lowest";break;
		default: break;
	}
	switch ($("#juan_envload\\#video\\@s_qual")[0].selectedIndex)							
	{
		case 0: s_quality = "highest";break;
		case 1: s_quality = "high";break;
		case 2: s_quality = "medium";break;
		case 3: s_quality = "low";break;
		case 4: s_quality = "lowest";break;
		default: break;
	}
	switch ($("#juan_envload\\#video\\@ss_qual")[0].selectedIndex)							
	{
		case 0: ss_quality = "highest";break;
		case 1: ss_quality = "high";break;
		case 2: ss_quality = "medium";break;
		case 3: ss_quality = "low";break;
		case 4: ss_quality = "lowest";break;
		default: break;
	}
	
	if ($("#juan_envload\\#video\\@m_brfmt1")[0].checked == true)									//码流可变
	{
		m_bit_s = "vbr";
	}
	else
	{
		m_bit_s = "cbr";
	}
	if ($("#juan_envload\\#video\\@s_brfmt1")[0].checked == true)									//码流可变
	{
		s_bit_s = "vbr";
	}
	else
	{
		s_bit_s = "cbr";
	}
	if ($("#juan_envload\\#video\\@ss_brfmt1")[0].checked == true)									//码流可变
	{
		ss_bit_s = "vbr";
	}
	else
	{
		ss_bit_s = "cbr";
	}

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<vin0 shutter="' + $("#juan_envload\\#video\\@m_shutter")[0].value + '">';
//	xmlstr += '<md width="" height="" sensitivity="" data="" />';
	xmlstr += '<encode_h2640>';
	xmlstr += '<stream0 size="' + m_resol.options[m_resol.selectedIndex].value + '" mode="' + m_bit_s + '" fps="' + $("#juan_envload\\#video\\@m_frate")[0].value + '" quality="' + m_quality + '" bps="' + $("#juan_envload\\#video\\@m_bitrate")[0].value + '" />';
	xmlstr += '</encode_h2640>';
	xmlstr += '<encode_h2641>';
	xmlstr += '<stream0 size="' + s_resol.options[s_resol.selectedIndex].value + '" mode="' + s_bit_s + '" fps="' + $("#juan_envload\\#video\\@s_frate")[0].value + '" quality="' + s_quality + '" bps="' + $("#juan_envload\\#video\\@s_bitrate")[0].value + '" />';
	xmlstr += '</encode_h2641>';
	if(g_model == "N18A"){
	xmlstr += '<encode_h2642>';
	xmlstr += '<stream0 size="' + ss_resol.options[ss_resol.selectedIndex].value + '" mode="' + ss_bit_s + '" fps="' + $("#juan_envload\\#video\\@ss_frate")[0].value + '" quality="' + ss_quality + '" bps="' + $("#juan_envload\\#video\\@ss_bitrate")[0].value + '" />';
	xmlstr += '</encode_h2642>';
	}
	xmlstr += '</vin0>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j",
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
	},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function reset_frame_rate_select(_select, _max_frate)
{
	var frame_rate = _select.selectedIndex + 1;
	for(var i = _select.options.length - 1; i >= 0; i--)
	{
		_select.options.remove(i);
	}
	for(var i = 0; i < _max_frate; i++)
	{
		var oOption = document.createElement("OPTION");
		oOption.text = i + 1 + "fps";
		oOption.value = i + 1;
		_select.add(oOption);
	}
	if(frame_rate < _max_frate)
	{
		_select.selectedIndex = frame_rate - 1;
	}
	else
	{
		_select.selectedIndex = _max_frate - 1;
	}
}
function on_shutter_change()
{
	var shutter = 30;
	/*switch($("#juan_envload\\#video\\@m_shutter")[0].options[$("#juan_envload\\#video\\@m_shutter")[0].selectedIndex].value)
	{
	case "50hz":
		shutter = 25;
		break;
	case "60hz":
	default:
		shutter = 30;
		break;
	}*/
	reset_frame_rate_select($("#juan_envload\\#video\\@m_frate")[0], shutter);
	reset_frame_rate_select($("#juan_envload\\#video\\@s_frate")[0], shutter);
	if(g_model == "N18A"){
	reset_frame_rate_select($("#juan_envload\\#video\\@ss_frate")[0], shutter);
	}
}
function m_bps_type()
{
	var obj=document.getElementById('juan_envload#video@m_brfmt');
	var obj2=document.getElementById('juan_envload#video@m_brfmt1');
	var inbox=document.getElementById('juan_envload#video@m_bitrate');
	if(obj.checked==false || obj2.checked==true)
	{
//		inbox.value=0;
		inbox.disabled=true;
	}
	else
	{
		inbox.disabled=false;
	}
}
function s_bps_type()
{
	var obj=document.getElementById('juan_envload#video@s_brfmt');
	var obj2=document.getElementById('juan_envload#video@s_brfmt1');
	var inbox=document.getElementById('juan_envload#video@s_bitrate');
	if(obj.checked==false || obj2.checked==true)
	{
//		inbox.value=0;
		inbox.disabled=true;
	}
	else
	{
		inbox.disabled=false;
	}
}
function ss_bps_type()
{
	var obj=document.getElementById('juan_envload#video@ss_brfmt');
	var obj2=document.getElementById('juan_envload#video@ss_brfmt1');
	var inbox=document.getElementById('juan_envload#video@ss_bitrate');
	if(obj.checked==false || obj2.checked==true)
	{
//		inbox.value=0;
		inbox.disabled=true;
	}
	else
	{
		inbox.disabled=false;
	}
}

//image
function image_data2ui(dvr_data)
{
	var con,bri,sat,hue,sha;
	//var num = Array(5), maxmum = Array(5);
	var num = Array(5), maxmum = Array(5);
	con = dvr_data.juan.conf.isp.image_attr.contrast.split("/");
	bri = dvr_data.juan.conf.isp.image_attr.brightness.split("/");
	sat = dvr_data.juan.conf.isp.image_attr.saturation.split("/");
	hue = dvr_data.juan.conf.isp.image_attr.hue.split("/");
	sha = dvr_data.juan.conf.isp.image_attr.sharpen.split("/");
	num[0] = parseInt(con[0]); maxmum[0] = parseInt(con[1]);
	num[1] = parseInt(bri[0]); maxmum[1] = parseInt(bri[1]);
	num[2] = parseInt(sat[0]); maxmum[2] = parseInt(sat[1]);
	num[3] = parseInt(hue[0]); maxmum[3] = parseInt(hue[1]);
	num[4] = parseInt(sha[0]); maxmum[4] = parseInt(sha[1]);
	slider_con.n_maxValue = maxmum[0];
	slider_con.n_pix2value = slider_con.n_pathLength / (slider_con.n_maxValue - slider_con.n_minValue);
	slider_lum.n_maxValue = maxmum[1];
	slider_lum.n_pix2value = slider_lum.n_pathLength / (slider_lum.n_maxValue - slider_lum.n_minValue);
	slider_sat.n_maxValue = maxmum[2];
	slider_sat.n_pix2value = slider_sat.n_pathLength / (slider_sat.n_maxValue - slider_sat.n_minValue);
	slider_hue.n_maxValue = maxmum[3];
	slider_hue.n_pix2value = slider_hue.n_pathLength / (slider_hue.n_maxValue - slider_hue.n_minValue);
	slider_sha.n_maxValue = maxmum[4];
	slider_sha.n_pix2value = slider_sha.n_pathLength / (slider_sha.n_maxValue - slider_sha.n_minValue);
	/*$('#juan_envload#color@con')[0].value = num[0];*/
	$("#juan_envload\\#color\\@con")[0].value = num[0];
	$("#juan_envload\\#color\\@lum")[0].value = num[1];
	$("#juan_envload\\#color\\@sat")[0].value = num[2];
	$("#juan_envload\\#color\\@hue")[0].value = num[3];
	$("#juan_envload\\#color\\@sha")[0].value = num[4];
	slider_con.f_setValue(num[0],0);
	slider_lum.f_setValue(num[1],0);
	slider_sat.f_setValue(num[2],0);
	slider_hue.f_setValue(num[3],0);
	slider_sha.f_setValue(num[4],0);
}

var __display_board_content = false;
function showpreview()
{
	if(g_port=="")	//80
	{
		g_port=80;
	}
/*	else
	{
		var i=document.location.host.indexOf(":");
		ip=document.location.host.substring(0,i);
	}
*/
	var os_browser = check_os_and_browser();
	if(os_browser.browser == "ie6" || os_browser.browser == "ie7" || os_browser.browser == "ie8" || os_browser.browser == "ie9")
	{
		if(__display_board_content == false)
		{
			__display_board_content = true;	
			var str = '<object id="DHiMPlayer_image" name="TestR" classid="clsid:F1649976-BD20-4689-BB65-E5A64EF61C7E" height = "380" width = "600"></object>';
		document.getElementById('display_board').innerHTML = str;
		DHiMPlayer_image = document.getElementById("DHiMPlayer_image");
		DHiMPlayer_image.attachEvent("InitInstance",function Init(ID,sInfo){});
		DHiMPlayer_image.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer_image.FullScreen(bFullScreen = !bFullScreen);});
	}
		if(DHiMPlayer_isp){
			DHiMPlayer_isp.close();
		}
				while(DHiMPlayer_image.ReadyFlag != true);
				DHiMPlayer_image.OpenByIP(g_ip,g_port);
				DHiMPlayer_image.UserLogon(g_usr,g_pwd);
				DHiMPlayer_image.OpenChannel(0,1);
	}
	else
	{

			var td1 = document.getElementById('preview_board');
			var str = "<object id=\"JaViewer_preview\" type=\"application/x-shockwave-flash\" data=\"JaViewer.swf\" style=\"outline: medium none;\" height=\"447px;\" width=\"" + (td1.offsetWidth) + "\">";
			str += "<param name=\"movie\" value=\"JaViewer.swf\">";
			str += "<param name=\"allowFullScreen\" value=\"true\">";
			str += "<param name=\"allowScriptAccess\" value=\"always\">";
			str += "</object>";
			var player = document.getElementById("display_board");
			player.innerHTML = str;
	
	}
		setTimeout("flash_video(\"JaViewer_preview\", \"360p.264\")", 1000);

}
function image_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<image_attr hue="" contrast="" brightness="" saturation="" sharpen="" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
	//alert(xmlstr);
	showpreview();
	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
			//alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);		
			image_data2ui(dvr_data);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function image_save_content()
{
	showInfo(langstr.save_setup);
	var con = $("#juan_envload\\#color\\@con")[0].value + "/" + slider_con.n_maxValue,
		lum = $("#juan_envload\\#color\\@lum")[0].value + "/" + slider_lum.n_maxValue,
		sat = $("#juan_envload\\#color\\@sat")[0].value + "/" + slider_sat.n_maxValue;
		hue = $("#juan_envload\\#color\\@hue")[0].value + "/" + slider_hue.n_maxValue;
		sha = $("#juan_envload\\#color\\@sha")[0].value + "/" + slider_sha.n_maxValue;
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<image_attr contrast="' + con + '" brightness="' + lum + '" hue="' + hue + '" saturation="' + sat + '" sharpen="' + sha + '">';
	xmlstr += '</image_attr>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
//			setTimeout("showInfo('已保嫿')",3000);
			showInfo(langstr.save_refresh);
			setTimeout("hideInfo()",disp_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function image_preview_content(id)
{
	showInfo(langstr.update_preview);
	var xmlstr;
if(id == 'sl0slider' ){
	var con = $("#juan_envload\\#color\\@con")[0].value + "/" + slider_con.n_maxValue;
	xmlstr = '';
		xmlstr += '<juan ver="1.0" seq="0">';
		xmlstr += '<conf type="write" user="admin" password="">';
		xmlstr += '<isp>';
		xmlstr += '<image_attr contrast="' + con + '">';
		xmlstr += '</image_attr>';
		xmlstr += '</isp>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';
}else if(id == 'sl1slider'){
	var lum = $("#juan_envload\\#color\\@lum")[0].value + "/" + slider_lum.n_maxValue;
	xmlstr = '';
		xmlstr += '<juan ver="1.0" seq="0">';
		xmlstr += '<conf type="write" user="admin" password="">';
		xmlstr += '<isp>';
		xmlstr += '<image_attr brightness="' + lum + '">';
		xmlstr += '</image_attr>';
		xmlstr += '</isp>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';
}else if(id == 'sl2slider'){
	var sat = $("#juan_envload\\#color\\@sat")[0].value + "/" + slider_sat.n_maxValue;
	xmlstr = '';
		xmlstr += '<juan ver="1.0" seq="0">';
		xmlstr += '<conf type="write" user="admin" password="">';
		xmlstr += '<isp>';
		xmlstr += '<image_attr saturation="' + sat + '">';
		xmlstr += '</image_attr>';
		xmlstr += '</isp>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';
 }else if(id == 'sl3slider'){
	var hue = $("#juan_envload\\#color\\@hue")[0].value + "/" + slider_hue.n_maxValue;
	xmlstr = '';
		xmlstr += '<juan ver="1.0" seq="0">';
		xmlstr += '<conf type="write" user="admin" password="">';
		xmlstr += '<isp>';
		xmlstr += '<image_attr hue="' + hue + '">';
		xmlstr += '</image_attr>';
		xmlstr += '</isp>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';
}else if(id == 'sl4slider'){
	var sha = $("#juan_envload\\#color\\@sha")[0].value + "/" + slider_sha.n_maxValue;
	//alert(sha);
	var xmlstr = '';
		xmlstr += '<juan ver="1.0" seq="0">';
		xmlstr += '<conf type="write" user="admin" password="">';
		xmlstr += '<isp>';
		xmlstr += '<image_attr sharpen="' + sha + '">';
		xmlstr += '</image_attr>';
		xmlstr += '</isp>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';
}else if(id == 'sl5slider' ){
	var str = $("#juan_envload\\#color\\@str")[0].value + "/" + slider_str.n_maxValue;
	//alert(str);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<wide_dynamic_range strength="'+ str +'" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'sl6slider' ){
	var AEcom = $("#juan_envload\\#color\\@AEcom")[0].value + "/" + slider_AEcom.n_maxValue;
	//alert(AEcom);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<exposure ae_compensation="' + AEcom + '"/>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'sl7slider' ){
	var denoise_strength = $("#juan_envload\\#color\\@denoise_strength")[0].value + "/" + slider_denoise_strength.n_maxValue;
	//alert(d2);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<denoise denoise_strength="' + denoise_strength + '">';
	xmlstr += '</denoise>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';	
}else{
	var xmlstr ='';
	xmlstr += 'error';
	alert(xmlstr);
}
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			showInfo(langstr.save_preview);
			setTimeout("hideInfo()",3000);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
//isp
function isp_data2ui(dvr_data)
{
	var str,AEcom,denoise_strength;
	var num = Array(3), maxmum = Array(3);
	str = dvr_data.juan.conf.isp.wide_dynamic_range.strength.split("/");
	AEcom = dvr_data.juan.conf.isp.exposure.ae_compensation.split("/");
	denoise_strength = dvr_data.juan.conf.isp.denoise.denoise_strength.split("/");
	num[0] = parseInt(str[0]); maxmum[0] = parseInt(str[1]);
	num[1] = parseInt(AEcom[0]); maxmum[1] = parseInt(AEcom[1]);
	num[2] = parseInt(denoise_strength[0]); maxmum[2] = parseInt(denoise_strength[1]);
	slider_str.n_maxValue = maxmum[0];
	slider_str.n_pix2value = slider_str.n_pathLength / (slider_str.n_maxValue - slider_str.n_minValue);
	slider_AEcom.n_maxValue = maxmum[1];
	slider_AEcom.n_pix2value = slider_AEcom.n_pathLength / (slider_AEcom.n_maxValue - slider_AEcom.n_minValue);
	slider_denoise_strength.n_maxValue = maxmum[2];
	slider_denoise_strength.n_pix2value = slider_denoise_strength.n_pathLength / (slider_denoise_strength.n_maxValue - slider_denoise_strength.n_minValue);
	$("#juan_envload\\#color\\@str")[0].value = num[0];
	$("#juan_envload\\#color\\@AEcom")[0].value = num[1];
	$("#juan_envload\\#color\\@denoise_strength")[0].value = num[2];
	slider_str.f_setValue(num[0],0);
	slider_AEcom.f_setValue(num[1],0);
	slider_denoise_strength.f_setValue(num[2],0);

	//alert(dvr_data.juan.conf.isp.scene.mode);
	switch(dvr_data.juan.conf.isp.scene.mode)
	{
		case "auto": $("#juan_envload\\#isp\\@scene_mode")[0].selectedIndex = 0;break;
		case "indoor": $("#juan_envload\\#isp\\@scene_mode")[0].selectedIndex = 1;break;
		case "outdoor": $("#juan_envload\\#isp\\@scene_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.white_balance.mode)
	{
		case "auto": $("#juan_envload\\#isp\\@white_balance_mode")[0].selectedIndex = 0;break;
		case "indoor": $("#juan_envload\\#isp\\@white_balance_mode")[0].selectedIndex = 1;break;
		case "outdoor": $("#juan_envload\\#isp\\@white_balance_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.day_night_mode.ircut_control_mode)
	{
		case "hardware": $("#juan_envload\\#isp\\@ircut_control_mode")[0].selectedIndex = 0;break;
		case "software": $("#juan_envload\\#isp\\@ircut_control_mode")[0].selectedIndex = 1;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.day_night_mode.ircut_mode)
	{
		case "auto": $("#juan_envload\\#isp\\@ircut_mode")[0].selectedIndex = 0;break;
		case "daylight": $("#juan_envload\\#isp\\@ircut_mode")[0].selectedIndex = 1;break;
		case "night": $("#juan_envload\\#isp\\@ircut_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	//alert(dvr_data.juan.conf.isp.wide_dynamic_range.enable);
	switch(dvr_data.juan.conf.isp.wide_dynamic_range.enable)
	{
		case "yes": $("#juan_envload\\#isp\\@wide_dynamic_range_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@wide_dynamic_range_enable")[0].selectedIndex = 1;break;
		default:break;
	}
	//alert(dvr_data.juan.conf.isp.exposure.mode);
	switch(dvr_data.juan.conf.isp.exposure.mode)
	{
		case "auto": $("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex = 0;break;
		case "bright": $("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex = 1;break;
		case "dark": $("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	
	switch(dvr_data.juan.conf.isp.denoise.denoise_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@denoise_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@denoise_enable")[0].selectedIndex = 1;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.advance.anti_fog_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@anti_fog_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@anti_fog_enable")[0].selectedIndex = 1;break;
		default:break;
	}
	/*switch(dvr_data.juan.conf.isp.advance.lowlight_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@lowlight_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@lowlight_enable")[0].selectedIndex = 1;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.advance.gamma)
	{
		case "default": $("#juan_envload\\#isp\\@gamma")[0].selectedIndex = 0;break;
		case "normal": $("#juan_envload\\#isp\\@gamma")[0].selectedIndex = 1;break;
		case "high": $("#juan_envload\\#isp\\@gamma")[0].selectedIndex = 2;break;
		default:break;
	}*/
	switch(dvr_data.juan.conf.isp.advance.defect_pixel_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@defect_pixel_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@defect_pixel_enable")[0].selectedIndex = 1;break;
		default:break;
	}
}

var __display_board1_content = false;
function showpreview_isp()
{
	if(g_port=="")	//80
	{
		g_port=80;
	}
//	else
//	{
//		var i=document.location.host.indexOf(":");
//		ip=document.location.host.substring(0,i);
//	}

	var os_browser3 = check_os_and_browser();
	if(os_browser3.browser == "ie6" || os_browser3.browser == "ie7" || os_browser3.browser == "ie8" || os_browser3.browser == "ie9")
	{
		if(__display_board1_content == false)
		{
			__display_board1_content = true;	
		var str1 = '<object id="DHiMPlayer_isp" name="TestR" classid="clsid:F1649976-BD20-4689-BB65-E5A64EF61C7E" height = "380" width = "600"></object>';
		document.getElementById('display_board1').innerHTML = str1;
		DHiMPlayer_isp = document.getElementById("DHiMPlayer_isp");
		DHiMPlayer_isp.attachEvent("InitInstance",function Init(ID,sInfo){});
		DHiMPlayer_isp.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer_isp.FullScreen(bFullScreen = !bFullScreen);});
	}
		if(DHiMPlayer_image){
			DHiMPlayer_image.close();
		}
				while(DHiMPlayer_isp.ReadyFlag != true);
				DHiMPlayer_isp.OpenByIP(g_ip,g_port);
				DHiMPlayer_isp.UserLogon(g_usr,g_pwd);
				DHiMPlayer_isp.OpenChannel(0,1);
	}
	else
	{
//		if(__display_board_content == false)
//		{
//			__display_board_content = true;
			var td2 = document.getElementById('preview_board1');
			var str1 = "<object id=\"JaViewer_preview\" type=\"application/x-shockwave-flash\" data=\"JaViewer.swf\" style=\"outline: medium none;\" height=\"447px;\" width=\"" + (td2.offsetWidth) + "\">";
			str1 += "<param name=\"movie\" value=\"JaViewer.swf\">";
			str1 += "<param name=\"allowFullScreen\" value=\"true\">";
			str1 += "<param name=\"allowScriptAccess\" value=\"always\">";
			str1 += "</object>";
			var player = document.getElementById("display_board1");
			player.innerHTML = str1;
	
	}
		setTimeout("flash_video(\"JaViewer_preview\", \"360p.264\")", 1000);

}

function isp_load_content()
{
	var xmlstr = '<juan ver="1.0" seq="0">';
		xmlstr += '<conf type="read" user="admin" password="">';
		xmlstr += '<isp>';
		xmlstr += '<image_attr hue="" contrast="" brightness="" saturation="" sharpen="" flip="" mirror="" />';
		xmlstr += '<scene mode="" />';
		xmlstr += '<white_balance mode="" />';
		xmlstr += '<day_night_mode ircut_control_mode="" ircut_mode="" />';
		xmlstr += '<wide_dynamic_range enable="" strength="" />';
		xmlstr += '<exposure  mode="" ae_compensation="" />';
		xmlstr += '<denoise denoise_enable="" denoise_strength="" />';
		xmlstr += '<advance anti_fog_enable="" lowlight_enable="" gamma="" defect_pixel_enable="" />';
		xmlstr += '</isp>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';	
//	alert(xmlstr);
	showpreview_isp();
	$.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		timeout:3000,

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
			//alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);		
			isp_data2ui(dvr_data);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(errorThrown);
		}
	});	
}
function isp_save_content()
{
	showInfo(langstr.save_setup);
	var scene_mode,white_balance_mode,ircut_control_mode,ircut_mode,wide_dynamic_range_enable,exposure_mode,denoise_enable,anti_fog_enable,lowlight_enable,gamma,defect_pixel_enable;
	switch($("#juan_envload\\#isp\\@scene_mode")[0].selectedIndex)
	{
		case 0: scene_mode = "auto";break;
		case 1: scene_mode = "indoor";break;
		case 2: scene_mode = "outdoor";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@white_balance_mode")[0].selectedIndex)
	{
		case 0: white_balance_mode = "auto";break;
		case 1: white_balance_mode = "indoor";break;
		case 2: white_balance_mode = "outdoor";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@ircut_control_mode")[0].selectedIndex)
	{
		case 0: ircut_control_mode = "hardware";break;
		case 1: ircut_control_mode = "software";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@ircut_mode")[0].selectedIndex)
	{
		case 0: ircut_mode = "auto";break;
		case 1: ircut_mode = "daylight";break;
		case 2: ircut_mode = "night";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@wide_dynamic_range_enable")[0].selectedIndex)
	{
		case 0: wide_dynamic_range_enable = "yes";break;
		case 1: wide_dynamic_range_enable = "no";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex)
	{
		case 0: exposure_mode = "auto";break;
		case 1: exposure_mode = "bright";break;
		case 2: exposure_mode = "dark";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@denoise_enable")[0].selectedIndex)
	{
		case 0: denoise_enable = "yes";break;
		case 1: denoise_enable = "no";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@anti_fog_enable")[0].selectedIndex)
	{
		case 0: anti_fog_enable = "yes";break;
		case 1: anti_fog_enable = "no";break;
		default:break;
	}
	/*switch($("#juan_envload\\#isp\\@lowlight_enable")[0].selectedIndex)
	{
		case 0: lowlight_enable = "yes";break;
		case 1: lowlight_enable = "no";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@gamma")[0].selectedIndex)
	{
		case 0: gamma = "default";break;
		case 1: gamma = "normal";break;
		case 2: gamma = "high";break;
		default:break;
	}*/
	switch($("#juan_envload\\#isp\\@defect_pixel_enable")[0].selectedIndex)
	{
		case 0: defect_pixel_enable = "yes";break;
		case 1: defect_pixel_enable = "no";break;
		default:break;
	}
	var str = $("#juan_envload\\#color\\@str")[0].value + "/" + slider_str.n_maxValue,
		AEcom = $("#juan_envload\\#color\\@AEcom")[0].value + "/" + slider_AEcom.n_maxValue,
		denoise_strength = $("#juan_envload\\#color\\@denoise_strength")[0].value + "/" + slider_denoise_strength.n_maxValue;
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<scene mode="' + scene_mode+ '"/>';
	xmlstr += '<white_balance mode="' + white_balance_mode + '"/>';
	xmlstr += '<day_night_mode ircut_control_mode="' + ircut_control_mode + '" ircut_mode="' + ircut_mode + '" />';
	xmlstr += '<wide_dynamic_range enable="' + wide_dynamic_range_enable + '" strength="' + str + '" />';
	xmlstr += '<exposure mode="' + exposure_mode + '" ae_compensation="' + AEcom + '" />';
	xmlstr += '<denoise denoise_enable="' + denoise_enable + '" denoise_strength="' + denoise_strength + '" />';
	xmlstr += '<advance anti_fog_enable="' + anti_fog_enable + '" lowlight_enable="' + lowlight_enable + '" defect_pixel_enable="' + defect_pixel_enable + '" />';
	//xmlstr += '<advance anti_fog_enable="' + anti_fog_enable + '" lowlight_enable="' + lowlight_enable + '" gamma="' + gamma + '" defect_pixel_enable="' + defect_pixel_enable + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
//			setTimeout("showInfo('已保嫿')",3000);
			showInfo(langstr.save_refresh);
			setTimeout("hideInfo()",disp_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function isp_preview_content(id,value)
{
	showInfo(langstr.update_preview);
	//alert(id);
if(id == 'juan_envload#isp@scene_mode' ){
	//alert(value);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<scene mode="' + value + '"/>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'juan_envload#isp@white_balance_mode' ){
	//alert(value);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<white_balance mode="' + value + '"/>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'juan_envload#isp@ircut_control_mode' ){
	//alert(value);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<day_night_mode ircut_control_mode="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'juan_envload#isp@ircut_mode' ){
	//alert(ircut_mode);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<day_night_mode ircut_mode="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'juan_envload#isp@wide_dynamic_range_enable' ){
	//alert(wide_dynamic_range_enable);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<wide_dynamic_range enable="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'sl5slider' ){
	var str = $("#juan_envload\\#color\\@str")[0].value + "/" + slider_str.n_maxValue;
	//alert(str);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<wide_dynamic_range strength="'+ str +'" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'juan_envload#isp@exposure_mode' ){
	//alert(exposure_mode);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<exposure mode="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}
else if(id == 'sl6slider' ){
	var AEcom = $("#juan_envload\\#color\\@AEcom")[0].value + "/" + slider_AEcom.n_maxValue;
	//alert(AEcom);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<exposure ae_compensation="' + AEcom + '"/>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}else if(id == 'juan_envload#isp@denoise_enable' ){
	//alert(denoise_3d_enable);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<denoise denoise_enable="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
}
else if(id == 'sl7slider' ){
	var denoise_strength = $("#juan_envload\\#color\\@denoise_strength")[0].value + "/" + slider_denoise_strength.n_maxValue;
	//alert(d2);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<denoise denoise_strength="' + denoise_strength + '">';
	xmlstr += '</denoise>';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';	
}
else if(id == 'juan_envload#isp@anti_fog_enable' ){
	//alert(anti_fog_enable);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<advance anti_fog_enable="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';	
}else if(id == 'juan_envload#isp@lowlight_enbale' ){
	//alert(lowlight_enbale);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<advance" lowlight_enable="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';	
}else if(id == 'juan_envload#isp@gamma' ){;
	//alert(gamma);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<advance" gamma="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';	
}else if(id == 'juan_envload#isp@defect_pixel_enable' ){
	//alert(defect_pixel_enable);
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<isp>';
	xmlstr += '<advance defect_pixel_enable="' + value + '" />';
	xmlstr += '</isp>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';	
}else{
	var xmlstr ='';
	xmlstr += 'error';
	alert(xmlstr);
	}
//alert(xmlstr);
	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			showInfo(langstr.save_preview);
			setTimeout("hideInfo()",3000);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}


//remote
function remote_data2ui(dvr_data)
{
	$("#juan_envload\\#ddns\\@url")[0].value = dvr_data.juan.conf.network.ddns.url;
	$("#juan_envload\\#ddns\\@usr")[0].value = dvr_data.juan.conf.network.ddns.username;
	$("#juan_envload\\#ddns\\@pwd")[0].value = dvr_data.juan.conf.network.ddns.password;
	$("#juan_envload\\#pppoe\\@usr")[0].value = dvr_data.juan.conf.network.pppoe.username;
	$("#juan_envload\\#pppoe\\@pwd")[0].value = dvr_data.juan.conf.network.pppoe.password;
	switch (dvr_data.juan.conf.network.ddns.enable)
	{
		case "yes": $("#juan_envload\\#network\\@ddns_1")[0].checked = 1;break;
		case "no": $("#juan_envload\\#network\\@ddns_0")[0].checked = 1;break;
		default: break;
	}
	switch (dvr_data.juan.conf.network.pppoe.enable)
	{
		case "no": $("#juan_envload\\#network\\@pppoe_0")[0].checked = 1;break;
		case "yes": $("#juan_envload\\#network\\@pppoe_1")[0].checked = 1;break;
		default: break;
	}
	remote_change();
	switch (dvr_data.juan.conf.network.ddns.provider + "")
	{
		case "dyndns": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 0;break;
		case "noip": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 1;break;
		case "3322": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 2;break;
		case "changeip": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 3;break;
		case "popdvr": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 4;break;
		case "skybest": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 5;break;
		case "dvrtop": $("#juan_envload\\#ddns\\@provider")[0].selectedIndex = 6;break;
		default: break;
	}
}

function remote_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<network>';
	xmlstr += '<pppoe enable="" username="" password="" />';
	xmlstr += '<ddns enable="" provider="" url="" username="" password="" />';
	xmlstr += '<threeg enable="" apn="" pin="" username="" password="" />';
	xmlstr += '</network>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			remote_data2ui(dvr_data);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
		
function remote_save_content()
{
	showInfo(langstr.save_setup);
	var ddns_s,pppoe_s;
	var prviders = document.getElementById("juan_envload#ddns@provider");   

	switch ($("#juan_envload\\#network\\@ddns_1")[0].checked)
	{
		case true: ddns_s = "yes";break;
		case false: ddns_s = "no";break;
		default: break;
	}
	switch ($("#juan_envload\\#network\\@pppoe_1")[0].checked)
	{
		case true: pppoe_s = "yes";break;
		case false: pppoe_s = "no";break;
		default: break;
	}

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<network>';
	xmlstr += '<pppoe enable="' + pppoe_s + '" username="' + $("#juan_envload\\#pppoe\\@usr")[0].value + '" password="' + $("#juan_envload\\#pppoe\\@pwd")[0].value + '" />';
	xmlstr += '<ddns enable="' + ddns_s + '" provider="' + prviders.options[prviders.selectedIndex].text + '" url="' + $("#juan_envload\\#ddns\\@url")[0].value + '" username="' + $("#juan_envload\\#ddns\\@usr")[0].value + '" password="' + $("#juan_envload\\#ddns\\@pwd")[0].value + '" />';
	xmlstr += '</network>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function remote_change()
{
	if($("#juan_envload\\#network\\@ddns_1")[0].checked == 1)
	{
		$("#juan_envload\\#ddns\\@provider")[0].disabled = false;			
		$("#juan_envload\\#ddns\\@url")[0].disabled = false;			
		$("#juan_envload\\#ddns\\@usr")[0].disabled = false;			
		$("#juan_envload\\#ddns\\@pwd")[0].disabled = false;			
	}
	else
	{
		$("#juan_envload\\#ddns\\@provider")[0].disabled = true;			
		$("#juan_envload\\#ddns\\@url")[0].disabled = true;			
		$("#juan_envload\\#ddns\\@usr")[0].disabled = true;			
		$("#juan_envload\\#ddns\\@pwd")[0].disabled = true;			
	}

	if($("#juan_envload\\#network\\@pppoe_1")[0].checked == 1)
	{
		$("#juan_envload\\#pppoe\\@usr")[0].disabled = false;			
		$("#juan_envload\\#pppoe\\@pwd")[0].disabled = false;			
	}
	else
	{
		$("#juan_envload\\#pppoe\\@usr")[0].disabled = true;			
		$("#juan_envload\\#pppoe\\@pwd")[0].disabled = true;			
	}
}

//network
function network_data2ui(dvr_data)
{
	switch (dvr_data.juan.conf.network.lan.dhcp)
	{
		case "yes": $("#juan_envload\\#network\\@dhcp_1")[0].checked = 1;break;
		case "no": $("#juan_envload\\#network\\@dhcp_0")[0].checked = 1;break;
		default:break;	
	};
	switch (dvr_data.juan.conf.network.esee.enable)
	{
		case "yes": $("#juan_envload\\#network\\@esee_1")[0].checked = 1;break;
		case "no": $("#juan_envload\\#network\\@esee_0")[0].checked = 1;break;
		default:break;	
	};
	switch (dvr_data.juan.conf.network.esee.id_disp)
	{
		case "yes": $("#juan_envload\\#network\\@id_disp_1")[0].checked = 1;break;
		case "no": $("#juan_envload\\#network\\@id_disp_0")[0].checked = 1;break;
		default:break;	
	};
	$("#juan_envload\\#network\\@mac")[0].value = dvr_data.juan.conf.network.mac;
	$("#juan_envload\\#network\\@ip")[0].value = dvr_data.juan.conf.network.lan.static_ip;
	$("#juan_envload\\#network\\@gateway")[0].value = dvr_data.juan.conf.network.lan.static_gateway;
	$("#juan_envload\\#network\\@submask")[0].value = dvr_data.juan.conf.network.lan.static_netmask;
	$("#juan_envload\\#network\\@dns")[0].value = dvr_data.juan.conf.network.lan.static_preferred_dns;
	$("#juan_envload\\#network\\@dns2")[0].value = dvr_data.juan.conf.network.lan.static_alternate_dns;
	$("#juan_envload\\#network\\@port")[0].value = dvr_data.juan.conf.network.lan.port0.value;
	ip_config_change();
}

function network_load_content()
{
	var xml = '';
	xml += '<juan ver="" seq="">';
	xml += '<conf type="read" user="admin" password="">';
	xml += '<network mac="">';
	xml += '<lan dhcp="" static_ip="" static_netmask="" static_gateway="" static_preferred_dns="" static_alternate_dns="" >'
	xml += '<port0 name="" value=""/>'
	xml += '</lan>'
	xml += '<esee enable="" id_disp=""/>';
	xml += '</network>';
	xml += '</conf>';
	xml += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xml, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			network_data2ui(dvr_data);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function network_save_content()
{
	showInfo(langstr.save_setup);
	var dhcp_s,esee_s;
	switch ($("#juan_envload\\#network\\@dhcp_1")[0].checked)
	{
		case true: dhcp_s = "yes";break;
		case false: dhcp_s = "no";break;
		default: break;	
	};
	switch ($("#juan_envload\\#network\\@esee_1")[0].checked)
	{
		case true: esee_s = "yes";break;
		case false: esee_s = "no";break;
		default:break;	
	};
	switch ($("#juan_envload\\#network\\@id_disp_1")[0].checked)
	{
		case true: id_disp_s = "yes";break;
		case false: id_disp_s = "no";break;
		default:break;	
	};
	var xml = '';
	xml += '<juan ver="1.0" seq="0">';
	xml += '<conf type="write" user="' + g_usr + '" password="' + g_pwd + '">';
	xml += '<network mac="' + $("#juan_envload\\#network\\@mac")[0].value + '">';
	xml += '<lan ';
	xml += 'dhcp="' + dhcp_s + '" ';
	xml += 'static_ip="' + $("#juan_envload\\#network\\@ip")[0].value + '" ';
	xml += 'static_netmask="' + $("#juan_envload\\#network\\@submask")[0].value + '" ';
	xml += 'static_gateway="' + $("#juan_envload\\#network\\@gateway")[0].value + '" ';
	xml += 'static_preferred_dns="' + $("#juan_envload\\#network\\@dns")[0].value + '" ';
	xml += 'static_alternate_dns="' + $("#juan_envload\\#network\\@dns2")[0].value + '" >';
	xml += '<port0 name="generic" value="' + $("#juan_envload\\#network\\@port")[0].value + '"/>';
	xml += '</lan>';
	xml += '<esee enable="' + esee_s + '" id_disp="' + id_disp_s + '"/>';
	xml += '</network>';
	xml += '</conf>';
	xml += '</juan>';
//	alert(xml);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xml, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	

}

var default_mac;
var default_ip;
var default_port;
function save_mac_value()
{
	var mac_obj=document.getElementById('juan_envload#network@mac');
	default_mac=mac_obj.value;
}
function save_ipaddr(id)
{
	var ipaddr_obj=document.getElementById(id);
	default_ip=ipaddr_obj.value;
}
function save_port()
{
	var port_obj=document.getElementById('juan_envload#network@port');
	default_port=port_obj.value;
}
function is_valid_mac()
{
	//mac地址正则表达嶍
	var mac_obj=document.getElementById('juan_envload#network@mac');
	var reg_name=/[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}/; 
	var reg_name_s=/[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}/; 
	if ((!reg_name.test(mac_obj.value)) && (!reg_name_s.test(mac_obj.value)))
	{ 
		alert(langstr.format_wrong+"22-24-21-19-BD-E4"); 
		mac_obj.value=default_mac;
		return false; 
	} 
	return true; 
}
function is_valid_ip(id)
{
	var ip_obj=document.getElementById(id);
	var re=/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;//正则表达嶿  
	if(re.test(ip_obj.value))     
	{     
	   if( RegExp.$1<256 && RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256)   
	   return true;     
	}     
	alert(langstr.format_wrong+"192.168.1.234");     
	ip_obj.value=default_ip;
	return false;   
}
function is_valid_port()
{
	var port_obj=document.getElementById('juan_envload#network@port');
	if(isNaN(port_obj.value)==true)
	{
		alert(langstr.port_wrong);
		port_obj.value=default_port;
	}
	return true;
}

function ip_config_change()
{
	if($("#juan_envload\\#network\\@dhcp_1")[0].checked ==1)
	{
		//alert("use dhcp");
		$("#juan_envload\\#network\\@ip")[0].disabled = true;
		$("#juan_envload\\#network\\@gateway")[0].disabled = true;
		$("#juan_envload\\#network\\@submask")[0].disabled = true;
		$("#juan_envload\\#network\\@dns")[0].disabled = true;
		$("#juan_envload\\#network\\@dns2")[0].disabled = true;
		$("#juan_envload\\#network\\@port")[0].disabled = true;
	}
	else
	{
		//alert("use static ip");
		$("#juan_envload\\#network\\@ip")[0].disabled = false;
		$("#juan_envload\\#network\\@gateway")[0].disabled = false;
		$("#juan_envload\\#network\\@submask")[0].disabled = false;
		$("#juan_envload\\#network\\@dns")[0].disabled = false;
		$("#juan_envload\\#network\\@dns2")[0].disabled = false;
		$("#juan_envload\\#network\\@port")[0].disabled = false;
	}
}

//alarmin
function alarmin_data2ui(dvr_data)
{
}
function alarmin_load_content()
{
}	
function alarmin_save_content()
{
}

//capture
function capture_data2ui(dvr_data)
{
}
function capture_load_content()
{
}		
function capture_save_content()
{
}

//ptz
function ptz_data2ui(dvr_data)
{
}
function ptz_load_content()
{
}	
function ptz_save_content()
{
}

//user
$(function(){
	$(".pp").blur(function(){
		
		var iii=0;
		for(iii=0;iii<3;iii++)
		{
			if(this==$(".pp").eq(iii).get(0)) break;
		}
		if($(this).get(0).value != $(".p").eq(iii).get(0).value )
		{
			$(this).get(0).value="";
			$(this).get(0).focus();
			alert("Confirm password is different from password, Please retry!");
		}
	});
});

//devinfo
function devinfo_load_content(bflag)
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<spec vin="" ain="" io_sensor="" io_alarm="" hdd="" sd_card="" />';
	xmlstr += '<info device_name="" device_model="" device_soc="" device_sn="" sensor_type="" hardware_version="" software_version="" build_date="" build_time="" />';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data);
			var dvr_data = xml2json.parser(data.xml, "", false);
			$("#juan\\#devinfo\\@name")[0].value = dvr_data.juan.conf.info.device_name;
			$("#juan\\#devinfo\\@model")[0].value = dvr_data.juan.conf.info.device_model;
			$("#juan\\#devinfo\\@hwver")[0].value = dvr_data.juan.conf.info.hardware_version;
			$("#juan\\#devinfo\\@swver")[0].value = dvr_data.juan.conf.info.software_version;
			$("#juan\\#devinfo\\@reldatetime")[0].value = dvr_data.juan.conf.info.build_date + " " + dvr_data.juan.conf.info.build_time;
			$("#juan\\#devinfo\\@alarmnum")[0].value = dvr_data.juan.conf.spec.io_alarm;
			$("#juan\\#devinfo\\@sdnum")[0].value = dvr_data.juan.conf.spec.sd_card;				
			//alert("current model:"+dvr_data.juan.conf.info.device_model);
			if(bflag==false){
				//alert(dvr_data.juan.conf.info.device_soc);
				video_load_content(dvr_data.juan.conf.info.device_soc, dvr_data.juan.conf.info.sensor_type);
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

var user_management_target = "";
function user_management_prepare_rm()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";
	if(confirm(langstr.delete_confirm))
	{
		user_management_save_del_usr();
	}
}
function user_management_save_del_usr()
{
//	show_loading("save_del_usr()");

	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<del_user name=\"" + user_management_target + "\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/del_user.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.delete_fail);	
			}
			else
			{
				user_management_load_content();
			}
//			user_management_data2ui(dvr_data);


			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function user_management_save_edit_usr()
{
	alert(document.getElementById('permit_admin').checked);

	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<edit_user name=\"" + user_management_target  + "\" admin=\"" + + "\" premit_live=\"\" premit_setting=\"" + + "\" premit_playback=\"" + + "\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/add_user.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.add_fail);	
			}
			else
			{
				user_management_load_content();
			}
//			user_management_data2ui(dvr_data);


			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function user_management_prepare_add()
{
	$("#tbl_add_user")[0].style.display = "block";
	$("#tbl_modify_pwd")[0].style.display = "none";
}


function user_management_save_new_usr()
{
	var use = document.getElementById("txt_new_usr").value;
	if(use==null || use=="")
	{
		alert(langstr.warning);
		document.getElementById('username').focus();

		return false;
		}	
	
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";

	user_management_target = $("#txt_new_usr")[0].value;
	//alert(user_management_target);
	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<add_user name=\"" + $("#txt_new_usr")[0].value + "\" password=\"" + $("#txt_new_pwd")[0].value + "\" admin=\"\" premit_live=\"\" premit_setting=\"\" premit_playback=\"\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/add_user.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.add_fail);	
			}
			else
			{
				user_management_load_content();
			}
//			user_management_data2ui(dvr_data);


			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function user_management_prepare_modify()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "block";
}
function user_management_prepare_save_modify_usr()
{
	if($("#txt_old_pwd")[0].value != g_pwd)
	{
		alert(langstr.old_pwd_wrong);
		return;
	}
	if($("#txt_modify_pwd")[0].value != $("#txt_repeat_pwd")[0].value)
	{
		alert(langstr.confirm_pwd_wrong);
		return;
	}

	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";

	user_management_dvr_target = g_usr;

	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<set_pass old_pass=\"" + $("#txt_old_pwd")[0].value + "\" new_pass=\"" + $("#txt_modify_pwd")[0].value + "\" />";
	xmlstr += "</user>";

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/set_pass.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.modify_pwd_fail);	
			}
			else{
				alert(langstr.modify_success);	
			}

			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function user_management_prepare_cancel()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";
	
	user_management_target = "";
}

function user_management_data2ui(dvr_data)
{
	var user_count = dvr_data.user.user_list.count;
	var tbl = $("#tbl_user_manage")[0];
//	alert(tbl.rows.length);
	for(var i = tbl.rows.length - 1; i >= 2; i--)
	{
		tbl.deleteRow(i);
	}
	if(eval("dvr_data.user.add_user") == "no")
	{
		document.getElementById('add_user_button').disabled=true;
	}else
	{
		document.getElementById('add_user_button').disabled=false;
	}	
	for(var i = 1; i < user_count+1; i++)
	{
		var tr = tbl.insertRow(tbl.rows.length);
		var td;
		var str;

		td = tr.insertCell(tr.cells.length);
		td.innerHTML = eval("dvr_data.user.user_list.user" + i + ".name");
		
		td = tr.insertCell(tr.cells.length);
		var permit_admin = "";
//		var permit_live = "";
		var permit_setting = "";
		var permit_playback = "";
		
		if(eval("dvr_data.user.user_list.user" + i + ".admin") == "yes")
		{
			permit_admin = "checked";
		}
//		if(eval("dvr_data.user.user_list.user" + i + ".permit_live") == "yes")
//		{
//			permit_live = "checked";
//		}
		if(eval("dvr_data.user.user_list.user" + i + ".permit_setting") == "yes")
		{
			permit_setting = "checked";
		}
		if(eval("dvr_data.user.user_list.user" + i + ".permit_playback") == "yes")
		{
			permit_playback = "checked";
		}
		str = "";
                    str += "<input type=\"checkbox\" id=\"permit_admin\" " + permit_admin + "/>"+(langstr.permit_admin)+"";
            //		str += "<input type=\"checkbox\" id=\"permit_live\" " + permit_live + ">permit_live";
                    str += "<input type=\"checkbox\" id=\"permit_setting\" " + permit_setting + "/>"+(langstr.permit_setting)+"";
                    str += "<input type=\"checkbox\" id=\"permit_playback\" " + permit_playback + "/>"+(langstr.playback)+"";
                    td.innerHTML = str;
		
		td = tr.insertCell(tr.cells.length);
		var edit_user = "";
		var del_user = "";
//		var set_pass = "";
		edit_user = "disabled"
		if(eval("dvr_data.user.user_list.user" + i + ".edit_user") == "no")
		{
			edit_user = "disabled";
		}
		if(eval("dvr_data.user.user_list.user" + i + ".del_user") == "no")
		{
			del_user = "disabled";
		}
//		if(eval("dvr_data.user.user_list.user" + i + ".set_pass") == "no")
//		{
//			set_pass = "disabled";
//		}
		str = "";
		str += "<button type=\"button\" id=\"edit_user\" " + edit_user + " onclick=\"user_management_target='" + eval("dvr_data.user.user_list.user" + i + ".name") + "';user_management_save_edit_usr()\">"+(langstr.save)+"</button>";
		str += "<button type=\"button\" id=\"del_user\" " + del_user + " onclick=\"user_management_target='" + eval("dvr_data.user.user_list.user" + i + ".name") + "';user_management_prepare_rm()\">"+(langstr.del_user)+"</button>";
//		str += "<input type=\"button\" id=\"set_pass\" value=\"设置\" " + set_pass + ">";
		td.innerHTML = str;
	}
}

function user_management_load_content()
{
//	var xmlstr = '';
//	xmlstr += '<juan ver="" seq="">';
//	xmlstr += '<conf type="read" user="admin" password="">';
//	xmlstr += '<spec vin="" ain="" io_sensor="" io_alarm="" hdd="" sd_card="" />';
//	xmlstr += '<info device_name="" device_model="" device_sn="" hardware_version="" software_version="" build_date="" build_time="" />';
//	xmlstr += '</conf>';
//	xmlstr += '</juan>';
////	alert(xmlstr);

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
//			alert("recv:" + data);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.login_fail);	
			}
			else
			{
				user_management_data2ui(dvr_data);
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}


//reboot
function reboot()
{
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<system operation="reboot" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
//		dataType: 'jsonp',
//		jsonp: 'jsoncallback',


		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data, "", false);
			setTimeout("showInfo(langstr.reboot_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
				$("#result").html(lang.reboot_failed);
				$("#result").css("color","red");
		}
	});	
}

//time
var strYear,strMonth,strDate,strHour,strMin,strSen;
Date.prototype.toFormatString=function()
{
	strYear=this.getUTCFullYear().toString();
	if (strYear.length<4)
	{
		var i = 4-strYear.length;
		for (var j = 0; j < i; j++)
		{
			strYear = "0" + strYear;
		}
	}
	strMonth=(this.getUTCMonth()+parseInt(1)).toString();
	strMonth=(strMonth.length==1)?("0"+strMonth):strMonth
	strDate=this.getUTCDate().toString();
	strDate=(strDate.length==1)?("0"+strDate):strDate
	strHour=this.getUTCHours().toString();
	strHour=(strHour.length==1)?("0"+strHour):strHour
	strMin=this.getUTCMinutes().toString();
	strMin=(strMin.length==1)?("0"+strMin):strMin
	strSen=this.getUTCSeconds().toString();
	strSen=(strSen.length==1)?("0"+strSen):strSen
}

function time_data2ui(dvr_data)
{
	var utc_devtime = parseInt(dvr_data.juan.setup.time.value)*1000;
	var devtime = new Date(utc_devtime);
	devtime.toFormatString();
	switch(dvr_data.juan.conf.datetime.date_separator)
	{
		case "-": $("#date_break")[0].selectedIndex = 0;break;
		case "/": $("#date_break")[0].selectedIndex = 1;break;
		case ".": $("#date_break")[0].selectedIndex = 2;break;
		default: break;
	}
	switch(dvr_data.juan.conf.datetime.date_format)
	{
		case "yyyymmdd": 
			$("#date_form")[0].selectedIndex = 0;
			break;
		case "mmddyyyy": 
			$("#date_form")[0].selectedIndex = 1;
			break;
		case "ddmmyyyy": 
			$("#date_form")[0].selectedIndex = 2;
			break;
		default: break;
	}
	$("#time_zone")[0].value = dvr_data.juan.conf.datetime.time_zone;
	$("#daylight_time")[0].value = dvr_data.juan.conf.datetime.day_saving_time;

	switch (dvr_data.juan.conf.datetime.ntp_sync)
	{
		case "yes": $("#juan_envload\\#time\\@ntp_1")[0].checked = 1;break;
		case "no": $("#juan_envload\\#time\\@ntp_0")[0].checked = 1;break;
		default:break;	
	};
	switch (dvr_data.juan.conf.datetime.ntp_user_domain)
	{
		case "": 
			$("#juan_envload\\#time\\@ntp_server")[0].selectedIndex = 0;
			break;
		case "": 
			$("#juan_envload\\#time\\@ntp_server")[0].selectedIndex = 1;
			break;
		case "": 
			$("#juan_envload\\#time\\@ntp_server")[0].selectedIndex = 2;
			break;
		default: break;
	}
	ntp_change();

	showtime();
}

function time_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<datetime date_format="" date_separator="" time_format="" time_zone="" day_saving_time="" ntp_sync="" ntp_user_domain=""	/>';
	xmlstr += '</conf>';
	xmlstr += '<setup type="read" user="admin" password="">';
	xmlstr += '<time value="" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			time_data2ui(dvr_data);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}

function time_save_content()
{
	showInfo(langstr.save_setup);
	var date_form,date_break;
	switch($("#date_form")[0].selectedIndex)
	{
		case 0: date_form = "yyyymmdd";break;
		case 1: date_form = "mmddyyyy";break;
		case 2: date_form = "ddmmyyyy";break;
		default: break;
	};
	switch($("#date_break")[0].selectedIndex)
	{
		case 0: date_break = "-";break;
		case 1: date_break = "/";break;
		case 2: date_break = ".";break;
		default: break;
	};
	var ntp_s,ntp_domain;
	if ($("#juan_envload\\#time\\@ntp_1")[0].checked == true)
	{
		ntp_s = "yes";
	}else ntp_s = "no";
	switch ($("#juan_envload\\#time\\@ntp_server")[0].selectedIndex)
	{
		case 0: 
			ntp_domain = "time.windows.com";
			break;
		case 1: 
			ntp_domain = "time.nist.gov";
			break;
		default: break;
	}

//	savetime();

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<datetime date_format="' + date_form + '" date_separator="' + date_break + '" time_zone="' + $("#time_zone")[0].value + '" day_saving_time="' + $("#daylight_time")[0].value + '" ntp_sync="' + ntp_s + '" ntp_user_domain="' + ntp_domain + '"	/>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function showtime()
{
	var time_show = "";
	var datebreak = $("#date_break")[0].value;
	switch ($("#date_form")[0].selectedIndex)
	{
		case 0:
			time_show = strYear+datebreak+strMonth+datebreak+strDate+"  ";
			break;
		case 1:
			time_show = strMonth+datebreak+strDate+datebreak+strYear+"  ";
			break;
		case 2:
			time_show = strDate+datebreak+strMonth+datebreak+strYear+"  ";
			break;
		default:
			break;
	}
	time_show += strHour+":"+strMin+":"+strSen;
	$("#curent_time")[0].value = time_show;
}
function savetime()
{
	//showInfo(langstr.sync_time_now);
	var currentset_date = new Date();
	currentset_date.setFullYear(parseInt(strYear, 10));
	currentset_date.setMonth(parseInt(strMonth, 10)-1);
	currentset_date.setDate(parseInt(strDate, 10));
	currentset_date.setHours(parseInt(strHour, 10));
	currentset_date.setMinutes(parseInt(strMin, 10));
	currentset_date.setSeconds(parseInt(strSen, 10));
	var currentset_utc = currentset_date.getTime()/1000;
// + currentset_date.getTimezoneOffset()*60;
//	alert(currentset_utc);

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<time value="' + currentset_utc + '" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
			//setTimeout("showInfo(langstr.sync_refresh)",disp_delaytime_ms);
			//setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function sync_pc_time()
{
	strYear = yy;
	strMonth = mm;
	strDate = dd;
	strHour = hh;
	strMin = mi;
	strSen = ss;
	showtime();
	savetime();
}

function manual_set_time()
{
	var date_sep,time_sep;
	date_sep = $("#m_date")[0].value.split("-");
	time_sep = $('.in');//$("#m_time")[0].value.split(":");
	strYear = date_sep[0]; strHour = $('.in').eq(0).val();
	strMonth = date_sep[1]; strMin = $('.in').eq(1).val();
	strDate = date_sep[2]; strSen = $('.in').eq(2).val();
	showtime();
	savetime();
}

function is_valid_zone()
{
	var obj_zone=document.getElementById('time_zone');
	var str=obj_zone.value;
	var a = str.match(/^-?[1-9]$|^-?1[1-2]$|^0$/);
	if (a == null) 
	{
		alert('时区范围：\n     [-12~12]');
		obj_zone.value="8";
		return false;
	}
	return true;
}

function is_valid_daylight()
{
	var obj_daylight=document.getElementById('daylight_time');
	var str=obj_daylight.value;
	var a = str.match(/^-?[1-3]$|^0$/);
	if (a == null) 
	{
		alert('夏令时范围：\n     [-3~3]');
		obj_daylight.value="0";
		return false;
	}
	return true;
}

function is_valid_time()
{
	obj_time=document.getElementById('m_time');
	var str=obj_time.value;
	var a = str.match(/^(\d{2})(:)?(\d{2})\2(\d{2})$/);
	if (a == null) 
	{
		alert(langstr.format_wrong+'13:23:05');
		obj_time.value="00:00:00";
		return false;
	}
	if (a[1]>24 || a[3]>60 || a[4]>60)
	{
		alert(langstr.format_wrong+'13:23:05');
		obj_time.value="00:00:00";
		return false;
	}
	return true;
}

function is_valid_date()
{
	obj_date=document.getElementById('m_date');
	var str=obj_date.value;
	var r = str.match(/^(\d{1,4})(-|\/)(\d{2})\2(\d{2})$/);
	if(r==null)
	{
		alert(langstr.format_wrong+"2012-01-01");
		obj_date.value="0000-00-00";
		return false;
	}
	var d= new Date(r[1], r[3]-1, r[4]);
	if ((d.getFullYear()==r[1]&&(d.getMonth()+1)==r[3]&&d.getDate()==r[4])==false)
	{
		alert(langstr.format_wrong+"2012-01-01");
		obj_date.value="0000-00-00";
		return false;
	}
	return true;
}

function ntp_change()
{
	if ($("#juan_envload\\#time\\@ntp_1")[0].checked == 1)
	{
		$("#juan_envload\\#time\\@ntp_server")[0].disabled = false;
	}else
	{
		$("#juan_envload\\#time\\@ntp_server")[0].disabled = true;
	}
}

//default_setting
function default_setting()
{
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<system operation="default factory" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
//		dataType: 'jsonp',
//		jsonp: 'jsoncallback',


		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data, "", false);
			setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
				$("#result").html(lang.restore_failed);
				$("#result").css("color","red");
		}
	});	
}




var upload_persent = 0;
$(function(){
	$('#swfupload-control').swfupload({
		upload_url: "/cgi-bin/upload.cgi",
		file_size_limit : "16384",
		file_types : "*.rom",
		file_types_description : "Upgrade File",
		file_upload_limit : "0",
		flash_url : "images/swfupload.swf",
		button_image_url : 'images/XPButtonUploadText_61x22.png',
		button_width : 61,
		button_height : 22,
//		button_text_top_padding : 5,
//		button_text_left_padding : 5,
		button_text_style : "font-size: 22pt;",
		button_text : langstr.upgrade,
		button_disabled : false,
		button_placeholder : $('#button')[0],
		debug: false,
		custom_settings : {something : "here"}
	})
	.bind('swfuploadLoaded', function(event){
		//$('#log').append('<li>Loaded</li>');
		$('#txt_status')[0].innerHTML = "";
	})
	.bind('fileQueued', function(event, file){
		//$('#log').append('<li>File queued - '+file.name+'</li>');
		// start the upload since it's queued
		$(this).swfupload('startUpload');
	})
	.bind('fileQueueError', function(event, file, errorCode, message){
		//$('#log').append('<li>File queue error - '+message+'</li>');
		$('#txt_status')[0].innerHTML = langstr.file_error+ message;
		$('#txt_progress')[0].innerHTML = "";
	})
	.bind('fileDialogStart', function(event){
		//$('#log').append('<li>File dialog start</li>');
	})
	.bind('fileDialogComplete', function(event, numFilesSelected, numFilesQueued){
		//$('#log').append('<li>File dialog complete</li>');
	})
	.bind('uploadStart', function(event, file){
		//$('#log').append('<li>Upload start - '+file.name+'</li>');
		$('#txt_status')[0].innerHTML = langstr.start_upload;
		$(this).swfupload('setButtonDisabled', true);
	})
	.bind('uploadProgress', function(event, file, bytesLoaded){
		//$('#log').append('<li>Upload progress - '+bytesLoaded+'</li>');
		var str = "";
		var persent = bytesLoaded/file.size*100 + "";
		upload_persent = bytesLoaded/file.size*100;
		for(var i = 0; i < bytesLoaded/file.size*10; i++){
			str += "|";
		}
		if(upload_persent < 100){
			persent = persent.substr(0, 2);
		}
		if(upload_persent >= 100){
			persent = persent.substr(0, 3);
		}
		str += persent + "%";
		$('#txt_progress')[0].innerHTML = str;
	})
	.bind('uploadSuccess', function(event, file, serverData){
		//$('#log').append('<li>Upload success - '+file.name+'</li>');
		$('#txt_status')[0].innerHTML = langstr.stop_upload;
	})
	.bind('uploadComplete', function(event, file){
		//$('#log').append('<li>Upload complete - '+file.name+'</li>');
		if(upload_persent >= 100)
		{
			$('#txt_status')[0].innerHTML = langstr.stop_upload;
			get_upgrade_rate();
		}
		else
		{
			$('#txt_status')[0].innerHTML = langstr.wait_reboot;
		}
		// upload has completed, lets try the next one in the queue
		$(this).swfupload('startUpload');
	})
	.bind('uploadError', function(event, file, errorCode, message){
		//$('#log').append('<li>Upload error - '+message+'</li>');
		$('#txt_status')[0].innerHTML = langstr.fail_upload;
	});
});	

var upgrade_persent = 0;
function get_upgrade_rate()
{
	$.ajax({ 
		type:"GET",
		url: "/cgi-bin/upgrade_rate.cgi?cmd=upgrade_rate", 
		processData: false, 
		cache: false,
		data: "", 
		async:true,
		beforeSend: function(XMLHttpRequest){
			//alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data);
			$('#txt_upgrade_status')[0].innerHTML = langstr.writing_firmware;
			upgrade_persent = parseInt(data);
			
			var str = "";
			var persent = upgrade_persent + "";
			for(var i = 0; i < persent/10; i++){
				str += "|";
			}
			if(persent != "100"){
				persent = persent.substr(0, 2);
			}
			str += persent + "%";
			$('#txt_upgrade_progress')[0].innerHTML = str;
		},
		complete: function(XMLHttpRequest, textStatus){
			//alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error");	
		}
	});
	
	if(upgrade_persent <= 99)
	{
		setTimeout("get_upgrade_rate()", 1000);
	}
	else
	{
		alert(langstr.upgrade_success);
	}
}
