#pragma once
#define SAFE_DELETE(p)  if(p){delete p; p=NULL;}
#define HOST_URLW	_T("http://127.0.0.1:5243")
#define HOST_URL	"http://127.0.0.1:5243"
#define OAUTH_CONSUMERKEY _T("test")
#define PATH_PREFIX	"D:\\Netdisk"
#define PATH_PREFIXW _T("D:\\Netdisk")
#define CACHEPATH_PREFIX "D:\\NetdiskCache"
#define CACHEPATH_PREFIXW _T("D:\\NetdiskCache")
#define DB_PATHW	_T("D:\\NetdiskCache")
#define DB_PATH		"D:\\NetdiskCache"