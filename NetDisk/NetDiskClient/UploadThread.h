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
		hThread = (HANDLE)_beginthreadex(NULL,0,CUploadThread::StaticEntry,this,CREATE_SUSPENDED,&uThreadId);//����һ��Ĭ�Ϲ�����߳�
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
				//��ʼ�ϴ�����
				APP_TRACE("��ʼ�ϴ�����");
				CFileOperaType* operate = NULL;
				if(self->pTodolist->Get(operate))
				{
					if(self->UploadFile(operate))
					{
						APP_TRACE("����%s�ɹ�",W2CA(operate->m_strfilename.c_str()));
						self->pTodolist->Remove(operate);
						SAFE_DELETE(operate);
					}
					else
					{
						APP_TRACE("�ϴ�%sʧ�ܣ��ϴ��߳�����10s",W2CA(operate->m_strfilename.c_str()));
						Sleep(10*1000);//��Ϣ10s�ڴ˳���
					}
				}
				
			}
			else
			{
				APP_TRACE("û���ļ���Ҫ�ϴ���С˯һ��");
				Sleep(30*1000);//����30s
			}
		}
	}

	bool UploadFile(CFileOperaType* operate)
	{
		USES_CONVERSION;
		//��9���ȵ����ļ����ݵ�DFS����DDB�д����ļ��Ͱ汾�š�
		//	��10�������ϴ����򣬱������ݿ�ʱ�����´���
		//	a��������ִ���ͬĿ¼ͬ���ļ�(A.txt)���Ҳ�����ɾ���ļ����������ļ�ID��version,�޸��ļ���ΪA(xxx`s conflict copy)�������ظ��û���ͻ��Ϣ��
		//	b) ������ִ���ͬĿ¼ͬ���ļ�(A.txt)������ɾ�����������ļ�ID��version���ļ������䡣
		//	c) ���û��ͬĿ¼ͬ���ļ�(A.txt)���������ļ�ID��version���ļ������䡣
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
			client->Clear();//����������
			string strFilename(W2CA(const_cast<TCHAR*>(operate->m_strfilename.c_str())));
			string netdisk_prefix(PATH_PREFIX);
			strFilename = strFilename.substr(netdisk_prefix.length());

			//���ڱ������ݿ��в�ѯ�Ƿ���ڣ�hash�Ƿ����
			bool isExits = IsExitsInLocalDB(strFilename);
			string oldHash ;
			if(isExits) //������
			{
				GetOldHash(strFilename,&oldHash);
			}

			if(!isExits || (oldHash != hash))
			{
				wstring chFilename(CA2W(const_cast<char*>(hash.c_str())));
				//�Ȱ��ļ����Ƶ���ʱ�ļ��б��� cache/hash�������ϴ��Ĳ�����hash�����ڵ�hash��һ�µ����
				BakCacheFile(longpath,chFilename);//TODO:�������֮��ɾ��hash,��ѡ
				client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
				client->m_params.insert(map<string,string>::value_type("FileName",strFilename));
				client->m_params.insert(map<string,string>::value_type("FileSize","0"));
				client->m_params.insert(map<string,string>::value_type("Hash",hash));
				client->Request(const_cast<char*>(url->c_str()));
				SAFE_DELETE(url);
				//�������صĽ�����ж�

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
							APP_TRACE("�ϴ�%sʧ��,ԭ��Ϊ:%s",W2CA(longpath.c_str()),value["msg"].asString().c_str());
							return false;
						}
						string strDfsPath = value["DFSPath"].asString();
						int Id = value["Id"].asInt();

						//�ϴ��ļ�
						 url = new string(HOST_URL);
						 url->append("/Upload/Upload");
						 client->Clear();//����������
						 //client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
						 client->m_params.insert(map<string,string>::value_type("hash",hash));
						 client->m_params.insert(map<string,string>::value_type("DFSPath",strDfsPath));
						 client->m_strFilefieldName = "UploadFile";
						 client->m_strFileFullPath = W2CA(longpath.c_str());
						 client->Request(const_cast<char*>(url->c_str()));
						 
						 //�����ϴ����
						 bool jsonRet = reader.parse(client->m_strBuffer,value);
						 if(!jsonRet)
						 {
							 MessageBox(NULL,_T("�ϴ��ļ�ʧ��,�鿴��־���޷���������json"),_T("����"),NULL);
							 APP_TRACE("�ϴ�%sʧ��,�޷���������json",W2CA(longpath.c_str()));
							 return false;
						 }
						 if (0 != value["ret"].asInt())
						 {
							 APP_TRACE("�ϴ�%sʧ��,ԭ��Ϊ:%s",W2CA(longpath.c_str()),value["msg"].asString().c_str());
							 return false;
						 }
						 

						 SAFE_DELETE(url);
						 url = new string(HOST_URL);
						 url->append("/RPC/UploadComplete");
						 client->Clear();//����������
						 client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
						 char idbuffer[20];
						 itoa(Id,idbuffer,10);
						 client->m_params.insert(map<string,string>::value_type("Id",idbuffer));

						 client->Request(const_cast<char*>(url->c_str()));
						 SAFE_DELETE(url);
						 jsonRet = reader.parse(client->m_strBuffer,value);
						 if(!jsonRet)
						 {
							 MessageBox(NULL,_T("UploadCompleteʧ��"),_T("����"),NULL);
							 APP_TRACE("�ϴ�%sʧ�ܣ�UploadCompleteʧ��",W2CA(longpath.c_str()));
							 return false;
						 }
						 if (0 != value["ret"].asInt())
						 {
							 APP_TRACE("UploadComplete%sʧ��,ԭ��Ϊ:%s",W2CA(longpath.c_str()),value["msg"].asString().c_str());
							 return false;
						 }
						 char sqlbuf[256];
						 if(value["Conflicted"].asBool())
						 {
							 //����
							 wstring newpath(PATH_PREFIXW);
							 newpath.append(CA2W(value["NewFilename"].asString().c_str()));
							 MoveFile(longpath.c_str(),newpath.c_str());
						 }
						 //�����¼�¼
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
							// //�޸�ԭ��¼
						 //}

						 if(!UpdateDB(sqlbuf))
						 {
							 MessageBox(NULL,_T("�������ݿ�ʧ��"),_T("����"),NULL);
							 APP_TRACE("�������ݿ�%sʧ�ܣ��������ݿ�ʧ��",W2CA(longpath.c_str()));
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
						MessageBox(NULL,_T("��ʼ�ϴ��ļ�ʧ��,�鿴��־"),_T("����"),NULL);
						APP_TRACE("��ʼ�ϴ�%sʧ��",W2CA(longpath.c_str()));
						return false;
					}
					return false;
				}
				return false;
				//�ϴ��ļ���DFS

				//����ϴ��ɹ��������RPC�ϴ��ɹ�,RPC����ʱ��ת�Ƶ����Ǳ��������µ�fileId


				//�����յ�RPC�ķ��أ��ж��Ƿ��ǳ�ͻ��Ϣ�������ͻ�����շ��ص��ļ����ѱ����ļ�����
			}
			//�и��Ļ򲻴�������Upload
			//û����ֱ����������
			//oauth_token
			
			SAFE_DELETE(url);
			
			//����������ж��Ƿ�
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
			APP_TRACE("��֧�ֵĲ�������");
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
		//���ļ��ƶ���cache�ļ���
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