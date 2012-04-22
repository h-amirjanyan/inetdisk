#pragma once
#define SAFE_DELETE(p)  if(p){delete p; p=NULL;}
#define HOST_URLW	_T("http://NetdiskServer:5243")
#define HOST_URL	"http://NetdiskServer:5243"
#define	CUSUMER_KEY	"oauth_consumer_key"
#define	CUSUMER_KEYW	_T("oauth_consumer_key")

#define OAUTH_CONSUMERKEY _T("test")
#define PATH_PREFIX	"C:\\Netdisk"
#define PATH_PREFIXW _T("C:\\Netdisk")
#define CACHEPATH_PREFIX "C:\\NetdiskCache"
#define CACHEPATH_PREFIXW _T("C:\\NetdiskCache")
#define DB_PATHW	_T("C:\\NetdiskCache")
#define DB_PATH		"C:\\NetdiskCache"

#define DISK_DRIVERW _T("C:\\")
#define DISK_DRIVER	"C:\\"