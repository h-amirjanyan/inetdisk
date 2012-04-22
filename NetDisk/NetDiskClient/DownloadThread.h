#pragma once
#pragma warning(disable:4800)
#pragma warning(disable:4995)
#pragma warning(disable:4996)
#pragma warning(disable:210)
#include "TodoList.h"
#include "Log.h"
#include <process.h>
#include "HttpClient.h"
#include <json/json.h>
#include <string>
#include <atlbase.h>
#include "Common.h"
#include "macros.h"
#include "CppSQLite3.h"

using namespace std;
using namespace Utils;

class CDownloadThread
{
public:
	CDownloadThread(TodoList* _pTodolist,int& _lastSyncId);
	~CDownloadThread(void);


	void Create()
	{
		hThread = (HANDLE)_beginthreadex(NULL,0,CDownloadThread::StaticEntry,this,CREATE_SUSPENDED,&uThreadId);//����һ��Ĭ�Ϲ�����߳�
	}
	void Start()
	{
		if(!hThread)
			Create();
		ResumeThread(hThread);
	}

	static unsigned __stdcall StaticEntry(void * pThis)
	{
		CDownloadThread* self = (CDownloadThread*)pThis;
		while(1)
		{
			//�ٽ�����ʼ
			EnterCriticalSection(self->_critical);
			APP_TRACE("��ʼһ��ͬ������,�����߳̽����ٽ���");
			if(self->pTodolist)
			{
				if(self->pTodolist->GetSize() > 0 )
				{
					APP_TRACE("�ϴ����в�Ϊ�գ������ϴ����̣����̼߳�������");
					//Sleep(30*1000);
				}
				else
				{
					//��ʼһ��ͬ������
					self->DownloadFile();
				}
			}else
			{
				return -1;
			}
			//�ٽ�������
			LeaveCriticalSection(self->_critical);
			APP_TRACE("�´��������̽����������߳��˳��ٽ���");
			Sleep(30*1000);
		}
		return 0;
	}

	void DownloadFile()
	{
		USES_CONVERSION;

		HttpClient* client = new HttpClient();
		string* url = new string(HOST_URL);
		url->append("/");

		//��ȡ��Ҫͬ�����ļ��б�
		//RPC/GetUpdateList?oauth_token=test_token_ok&lastSyncId=0
		url->append("RPC/GetUpdateList");
		client->Clear();
		client->AddParam("oauth_token",oauth_token);
		char numbuf[20];
		sprintf(numbuf,"%d",lastSyncId);
		client->AddParam("lastSyncId",numbuf);
		client->Request(const_cast<char*>(url->c_str()));
		

		Json::Reader reader;
		Json::Value value;
		if(!reader.parse(client->m_strBuffer.c_str(),value))
		{
			APP_TRACE("��������%s����json����",url->c_str());
			goto EXIT;
		}
		//{"Files":[{"Id":4,"FullPath":"\\ApiPoolProtocol - Copy (2) - Copy.cpp","Hash":"be2ca9180c80f8cc21e43433250c3457"},{"Id":5,"FullPath":"\\Log_2012_4_20.txt","Hash":"a67a00a478cb6fefa4adcc65b6f41221"}],"ret":0,"msg":null}
		if(value["ret"].asInt() != 0)
		{
			APP_TRACE("����%s����,err info:%s",url->c_str(),value["msg"].asString().c_str());
			goto EXIT;
		}

		bool bAllSucceeded = true;
		{
			Json::Value arryObj = value["Files"];
			for (int i=0 ;i < arryObj.size() ;++ i)
			{
				//����ɹ��͸���lastsyncid�����������һ�θ��£�
				if(UpdateSingleFile(arryObj[i]["Id"].asInt(),arryObj[i]["FullPath"].asString(),arryObj[i]["Hash"].asString()))
				{
				/*	if(bAllSucceeded)
						lastSyncId = arryObj[i]["Id"].asInt();*/
				}
				else
				{
					bAllSucceeded = false;
				}
			}
		}
EXIT:
		SAFE_DELETE(client);
		SAFE_DELETE(url);
	}

	bool UpdateSingleFile(int id, string fullPath, string hash)
	{
		bool bUpdateResult = false;
		USES_CONVERSION;
		
		int lastVersionLocal = GetLastVersionLocal(fullPath);
		

		HttpClient* client = new HttpClient();
		string* url = new string(HOST_URL);
		url->append("/");

		url->append("RPC/DownloadPrepare");
		client->Clear();
		client->AddParam("oauth_token",oauth_token);
		char numbuf[20];
		sprintf(numbuf,"%d",id);
		client->AddParam("Id",numbuf);
		client->Request(const_cast<char*>(url->c_str()));

		Json::Reader reader;
		Json::Value value;
		if(!reader.parse(client->m_strBuffer.c_str(),value))
		{
			APP_TRACE("��������%s��json����",url->c_str());
			goto EXIT;
		}

		if(value["ret"].asInt() != 0)
		{
			APP_TRACE("����%s����err info:%s",url->c_str(),value["msg"].asString());
			
			goto EXIT;
		}

		if(!value["IsExits"].asBool())
		{
			APP_TRACE("�����%s������",fullPath.c_str());	//?��������ݴ����𣬹���
			bUpdateResult = true;
			goto EXIT;
		}

		if(value["IsDeleted"].asBool())
		{
			//delete file and record in db local
			bUpdateResult = DeleteLocal(fullPath);
			goto EXIT;
		}
		else
		{
			
			int lastVersionRemote = value["LastReversion"].asInt();
			if( lastVersionRemote > lastVersionLocal)
			{
				//���ظ��ļ����°汾
				SAFE_DELETE(url);
				url = new string(HOST_URL);
				url->append("/Download/Download");
				client->Clear();
				client->AddParam("DFSPath",value["LastDFSPath"].asString());
				string fullFileName(PATH_PREFIX);//for: D:\testdir\demo.doc
				fullFileName.append(fullPath);

				client->SaveFile(const_cast<char*>(url->c_str()),const_cast<char*>(fullFileName.c_str()));

				//���±������ݿ�汾��
				if(UpdateLocalDB(value["LastId"].asInt(),fullPath,lastVersionRemote,value["LastHash"].asString(),lastVersionLocal))
				{
					bUpdateResult = true;
					lastSyncId = value["LastId"].asInt();
				}
				else
				{
					bUpdateResult = false;
				}
			}
			else
			{
				//����Ҫ����
				bUpdateResult = true;
			}
		}
EXIT:
		SAFE_DELETE(url);
		SAFE_DELETE(client);
		if(bUpdateResult)
			lastSyncId = id;
		CCommon::SetLastSyncId(lastSyncId);
		return bUpdateResult;
	}

	bool DeleteLocal(string fullpath)
	{
		USES_CONVERSION;
		wstring fullFileName(PATH_PREFIXW);//for: D:\testdir\demo.doc
		fullFileName.append(CA2W(fullpath.c_str()));
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		char szSQL[256];
		sprintf(szSQL,"delete from Files where FileName like '%s';", fullpath.c_str());
		db.execDML(szSQL);
		DeleteFile(fullFileName.c_str());
		db.close();
		return true;
	}

	int GetLastVersionLocal(string fullpath)
	{
		//�������򷵻�-1
		//fullpath = fullpath.substr(1);
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		char buffer[100];
		sprintf(buffer,"select Reversion from Files where FileName like '%s'",fullpath.c_str());
		CppSQLite3Table t = db.getTable(buffer);
		bool ret = (bool)t.numRows();
		int version = -1;
		

		if(ret)
		{
			t.setRow(0);
			version = t.getIntField(0);
		}
		t.finalize();
		db.close();
		return version;
	}
	bool UpdateLocalDB(int id,string fullPath,int lastVersionRemote,string hash,int lastid)
	{
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		char szSQL[256];
		if(lastid != -1)
		{
			sprintf(szSQL,"update Files Set Id = %d ,Reversion = %d ,Hash = '%s' where FileName like '%s';",
				id,
				lastVersionRemote,
				hash.c_str(),
				fullPath.c_str());
		}
		else
		{
			sprintf(szSQL,"insert into Files (Id,FileName,Reversion,Hash) values(%d,'%s','%d','%s');",
				id,
				fullPath.c_str(),
				lastVersionRemote,
				hash.c_str()
				);
		}
	
		bool ret = (bool)db.execDML(szSQL);
		db.close();
		return ret;
	}

	TodoList*	pTodolist;
	HANDLE		hThread;
	unsigned	uThreadId;
	int&		lastSyncId;
	string		oauth_token;
	string		oauth_token_secret;
	string		strDBPath;


	CRITICAL_SECTION *_critical;	//�ٽ���
};

#pragma warning(default:4800)
#pragma warning(default:4996)
#pragma warning(default:4995)
#pragma warning(default:210)