#pragma once
#include "TodoList.h"
#include "Log.h"
#include <process.h>
#include "macros.h"
//#include <sqlite/sqlite3.h>
#include "MD5Calc.h"
#include <atlbase.h>
#include "HttpClient.h"
#include "OauthClient.h"
#include <atlbase.h>
#include "CppSQLite3.h"
#pragma warning(disable:4995) 
#pragma warning(disable:4800)
#pragma warning(disable:4996)
class CUploadThread
{
public:
	CUploadThread(TodoList* _pTodolist);
	~CUploadThread(void);

	void Create()
	{
		hThread = (HANDLE)_beginthreadex(NULL,0,CUploadThread::StaticEntry,this,CREATE_SUSPENDED,&uThreadId);//创建一个默认刮起的线程
	}
	void Start()
	{
		if(!hThread)
			Create();
		ResumeThread(hThread);
	}

	static unsigned __stdcall StaticEntry(void *pThis)
	{
		USES_CONVERSION;
		CUploadThread* self = (CUploadThread*) pThis;
		while(1)
		{
			if(self->pTodolist->GetSize() > 0)
			{
				//开始上传流程
				APP_TRACE("开始上传流程");
				CFileOperaType* operate = NULL;
				if(self->pTodolist->Get(operate))
				{
					if(self->UploadFile(operate))
					{
						APP_TRACE("处理%s成功",W2CA(operate->m_strfilename.c_str()));
						self->pTodolist->Remove(operate);
						SAFE_DELETE(operate);
					}
					else
					{
						APP_TRACE("上传%s失败，上传线程休眠10s",W2CA(operate->m_strfilename.c_str()));
						Sleep(10*1000);//休息10s在此尝试
					}
				}
				
			}
			else
			{
				APP_TRACE("没有文件需要上传，小睡一会");
				Sleep(30*1000);//休眠30s
			}
		}
	}

	bool UploadFile(CFileOperaType* operate)
	{
		USES_CONVERSION;
		//（9）先导入文件内容到DFS再在DDB中存入文件和版本号。
		//	（10）根据上传规则，保存数据库时做以下处理：
		//	a）如果发现存在同目录同名文件(A.txt)，且不是已删除文件，生成新文件ID和version,修改文件名为A(xxx`s conflict copy)，并返回给用户冲突消息。
		//	b) 如果发现存在同目录同名文件(A.txt)，且已删除，生成新文件ID和version，文件名不变。
		//	c) 如果没有同目录同名文件(A.txt)，生成新文件ID和version，文件名不变。
		 Utils::HttpClient* client = new Utils::HttpClient();
		 unsigned char buf[16];
		 MD5Calc* calc = new MD5Calc();
		if(operate->m_operate == E_FILE_CREATE)
		{
			//LPCSTR
			//const_cast<char*>(W2CA(const_cast<TCHAR*>(operate->m_strfilename.c_str())))
			calc->mdfile(const_cast<char*>(W2CA(operate->m_strfilename.c_str())),buf);
			string hash = calc->print_digest(buf);
			wstring longpath(operate->m_strfilename.c_str());
			string* url = new string(HOST_URL);
			url->append("/RPC/UploadPrepare");
			client->Clear();//清理上下文
			string strFilename(W2CA(const_cast<TCHAR*>(operate->m_strfilename.c_str())));
			string netdisk_prefix(PATH_PREFIX);
			strFilename = strFilename.substr(netdisk_prefix.length());

			//现在本地数据库中查询是否存在，hash是否更改
			bool isExits = IsExitsInLocalDB(strFilename);
			string oldHash ;
			if(isExits) //不存在
			{
				GetOldHash(strFilename,&oldHash);
			}

			if(!isExits || (oldHash != hash))
			{
				wstring chFilename(CA2W(const_cast<char*>(hash.c_str())));
				//先把文件复制到临时文件夹备份 cache/hash，避免上传的操作的hash和现在的hash不一致的情况
				BakCacheFile(longpath,chFilename);//TODO:操作完成之后删除hash,可选
				client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
				client->m_params.insert(map<string,string>::value_type("FileName",strFilename));
				client->m_params.insert(map<string,string>::value_type("FileSize","0"));
				client->m_params.insert(map<string,string>::value_type("Hash",hash));
				client->Request(const_cast<char*>(url->c_str()));
				SAFE_DELETE(url);
				//分析返回的结果，判断

				Json::Reader reader;
				Json::Value value;
				if(reader.parse(client->m_strBuffer,value))
				{
					bool isNull = value["ret"].isNull();
					if(!isNull)
					{
						int ret = value["ret"].asInt();
						if(ret != 0)
						{
							APP_TRACE("上传%s失败,原因为:%s",W2CA(longpath.c_str()),value["msg"].asString().c_str());
							return false;
						}
						string strDfsPath = value["DFSPath"].asString();
						int Id = value["Id"].asInt();

						//上传文件
						 url = new string(HOST_URL);
						 url->append("/Upload/Upload");
						 client->Clear();//清理上下文
						 //client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
						 client->m_params.insert(map<string,string>::value_type("hash",hash));
						 client->m_params.insert(map<string,string>::value_type("DFSPath",strDfsPath));
						 client->m_strFilefieldName = "UploadFile";
						 client->m_strFileFullPath = W2CA(longpath.c_str());
						 client->Request(const_cast<char*>(url->c_str()));
						 
						 //分析上传结果
						 bool jsonRet = reader.parse(client->m_strBuffer,value);
						 if(!jsonRet)
						 {
							 MessageBox(NULL,_T("上传文件失败,查看日志，无法解析返回json"),_T("错误"),NULL);
							 APP_TRACE("上传%s失败,无法解析返回json",W2CA(longpath.c_str()));
							 return false;
						 }
						 if (0 != value["ret"].asInt())
						 {
							 APP_TRACE("上传%s失败,原因为:%s",W2CA(longpath.c_str()),value["msg"].asString().c_str());
							 return false;
						 }
						 

						 SAFE_DELETE(url);
						 url = new string(HOST_URL);
						 url->append("/RPC/UploadComplete");
						 client->Clear();//清理上下文
						 client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
						 char idbuffer[20];
						 itoa(Id,idbuffer,10);
						 client->m_params.insert(map<string,string>::value_type("Id",idbuffer));

						 client->Request(const_cast<char*>(url->c_str()));
						 SAFE_DELETE(url);
						 jsonRet = reader.parse(client->m_strBuffer,value);
						 if(!jsonRet)
						 {
							 MessageBox(NULL,_T("UploadComplete失败"),_T("错误"),NULL);
							 APP_TRACE("上传%s失败，UploadComplete失败",W2CA(longpath.c_str()));
							 return false;
						 }
						 if (0 != value["ret"].asInt())
						 {
							 APP_TRACE("UploadComplete%s失败,原因为:%s",W2CA(longpath.c_str()),value["msg"].asString().c_str());
							 return false;
						 }
						 char sqlbuf[256];
						 if(value["Conflicted"].asBool())
						 {
							 //改名
							 wstring newpath(PATH_PREFIXW);
							 newpath.append(CA2W(value["NewFilename"].asString().c_str()));
							 MoveFile(longpath.c_str(),newpath.c_str());
						 }
						 //插入新纪录
						 sprintf(sqlbuf,"insert into Files (Id,FileName,Reversion,Hash) values(%d,'%s','%d','%s');",
							 value["Id"].asInt(),
							 value["NewFilename"].asString().c_str(),
							 value["Reversion"].asInt(),
							 hash.c_str()
							 );
						 //else
						 //{
							// sprintf(sqlbuf,"update Files set Id = %d, Reversion = %d,Hash = '%s'  where FileName = '%s';",
							//	 value["Id"].asInt(),
							//	 value["Reversion"].asInt(),
							//	 hash.c_str(),
							//	 strFilename.c_str()
							//	 );
							//	 
							// //修改原纪录
						 //}

						 if(!UpdateDB(sqlbuf))
						 {
							 MessageBox(NULL,_T("更新数据库失败"),_T("错误"),NULL);
							 APP_TRACE("更新数据库%s失败，更新数据库失败",W2CA(longpath.c_str()));
							 return false;
						 }

						 
						
						 /*public int Id { get; set; }

						 public int Reversion { get; set; }

						 public bool Conflicted { get; set; }

						 public string NewFilename { get; set; }*/

						 SAFE_DELETE(client);
						 return true;
					}
					else
					{
						MessageBox(NULL,_T("开始上传文件失败,查看日志"),_T("错误"),NULL);
						APP_TRACE("开始上传%s失败",W2CA(longpath.c_str()));
						return false;
					}
					return false;
				}
				return false;
				//上传文件到DFS

				//如果上传成功，则高数RPC上传成功,RPC从临时表转移到正是表，并返回新的fileId


				//本地收到RPC的返回，判断是否是冲突消息，如果冲突，按照返回的文件名把本地文件改名
			}
			//有更改或不存在则走Upload
			//没更改直接跳过更新
			//oauth_token
			
			SAFE_DELETE(url);
			
			//分析结果，判断是否
			return true;
		}
		else if (operate->m_operate == E_FILE_UPDATE_FILE)
		{
		}
		else if (operate->m_operate == E_FILE_DELETE)
		{

		}
		else
		{
			APP_TRACE("不支持的操作类型");
		}
		SAFE_DELETE(calc);
		SAFE_DELETE(client);
		return false;
	}

	bool IsExitsInLocalDB(string strFilename)
	{
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		char buffer[100];
		sprintf(buffer,"select * from Files where FileName like '%s'",strFilename.c_str());
		CppSQLite3Table t = db.getTable(buffer);
		bool ret = (bool)t.numRows();
		t.finalize();
		db.close();
		return ret;
	}

	void GetOldHash(string strFilename,string* strOldFilename)
	{
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		char buffer[100];
		sprintf(buffer,"select Hash from Files where FileName like '%s'",strFilename.c_str());
		CppSQLite3Table t = db.getTable(buffer);
		bool ret = (bool)t.numRows();
		t.finalize();

		if(ret)
		{
			t.setRow(0);
			*strOldFilename = t.getStringField(0);
		}
		db.close();
	}

	void BakCacheFile(wstring strFilename,wstring hash)
	{
		//把文件移动到cache文件夹
		hash.insert(0,_T("\\"));
		hash.insert(0,CACHEPATH_PREFIXW);
		CopyFile(strFilename.c_str(),hash.c_str() ,TRUE);
	}

	bool UpdateDB(char* szSQL)
	{//execDML
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		bool ret = (bool)db.execDML(szSQL);
		db.close();
		return true;
	}

	int GetFileSize(wstring& filename)
	{
		return 0;
	}


	TodoList*	pTodolist;
	HANDLE		hThread;
	unsigned	uThreadId;

	string		oauth_token;
	string		oauth_token_secret;
	string		strDBPath;
};

#pragma warning(default:4995) 
#pragma warning(default:4800)
#pragma warning(default:4996)