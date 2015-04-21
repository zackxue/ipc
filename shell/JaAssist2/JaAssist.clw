; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CJaAssistDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "JaAssist.h"

ClassCount=3
Class1=CJaAssistApp
Class2=CJaAssistDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_JAASSIST_DIALOG

[CLS:CJaAssistApp]
Type=0
HeaderFile=JaAssist.h
ImplementationFile=JaAssist.cpp
Filter=N

[CLS:CJaAssistDlg]
Type=0
HeaderFile=JaAssistDlg.h
ImplementationFile=JaAssistDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_EDIT2

[CLS:CAboutDlg]
Type=0
HeaderFile=JaAssistDlg.h
ImplementationFile=JaAssistDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_JAASSIST_DIALOG]
Type=1
Class=CJaAssistDlg
ControlCount=8
Control1=IDC_LIST_DEVICE,SysListView32,1350664193
Control2=IDC_BUTTON_REFRESH,button,1342242816
Control3=IDC_BUTTON_CONNECT,button,1476460544
Control4=IDC_BUTTON_DISCONNECT,button,1342242816
Control5=IDC_EDIT1,edit,1350631552
Control6=IDC_STATIC_STDIN,static,1342308352
Control7=IDC_STATIC_OUTPUT,static,1342308352
Control8=IDC_EDIT_OUTPUT,edit,1353777220

