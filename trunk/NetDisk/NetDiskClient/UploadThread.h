#pragma once
#include "TodoList.h"
#include "Log.h"
#include <process.h>
#include "macros.h"
//#include <sqlite/sqlite3.h>
#include "MD5Calc.h"
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
			//�ٽ�����ʼ
			EnterCriticalSection(self->_critical);
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
			//�ٽ�������
			LeaveCriticalSection(self->_critical);
			if(self->pTodolist->GetSize() == 0)
			{
				APP_TRACE("û���ļ���Ҫ�ϴ�����������״̬");
				//Sleep(10*60*1000);//����10s
				Sleep(30*1000);
				APP_TRACE("�ϴ��߳����߽���");
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
		 string* url = new string(HOST_URL);
		 unsigned char buf[16];
		 MD5Calc* calc = new MD5Calc();
		bool bOperateResult = false;
		if(operate->m_operate == E_FILE_CREATE)
		{
			
			calc->mdfile(const_cast<char*>(W2CA(operate->m_strfilename.c_str())),buf);
			string hash = calc->print_digest(buf);

			wstring wstrLongLocalPath(operate->m_strfilename.c_str());

			
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

			if(!isExits) //������
			{
				wstring chFilename(CA2W(const_cast<char*>(hash.c_str())));
				//�Ȱ��ļ����Ƶ���ʱ�ļ��б��� cache/hash�������ϴ��Ĳ�����hash�����ڵ�hash��һ�µ����
				BakCacheFile(wstrLongLocalPath,chFilename);//TODO:�������֮��ɾ��hash,��ѡ
				client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
				client->m_params.insert(map<string,string>::value_type("FileName",strFilename));
				client->m_params.insert(map<string,string>::value_type("FileSize","0"));
				client->m_params.insert(map<string,string>::value_type("Hash",hash));
				client->Request(const_cast<char*>(url->c_str()));
				SAFE_DELETE(url);
				//�������صĽ�����ж�

				Json::Reader reader;
				Json::Value value;
				if(!reader.parse(client->m_strBuffer,value))
				{
					MessageBox(NULL,_T("/RPC/UploadPrepare ���ؽ������ʧ��,�鿴��־"),_T("����"),NULL);
					APP_TRACE("/RPC/UploadPrepare,%s���ؽ������ʧ��",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}

				int ret = value["ret"].asInt();
				if(ret != 0)
				{
					APP_TRACE("�ϴ�%sʧ��,ԭ��Ϊ:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
					goto EXIT;
				}

				
				//TODO�жϷ������ϸ�·���ĸ�hash���ļ��Ƿ���ڣ������������û�в��뵽�������ݿ⣬��ʱ��Ҫ�ز�
				if(value["IsExitsRemote"].asBool())
				{
					char szReinsert[356];
					sprintf(szReinsert,"insert into Files (Id,FileName,Reversion,Hash) values(%d,'%s','%d','%s');",
						value["RemoteId"].asString().c_str(),
							strFilename.c_str(),
							value["RemoteReversion"].asInt(),
							value["RemoteHash"].asString().c_str());
					UpdateDB(szReinsert);
					bOperateResult = true;
					goto EXIT;
				}
				


				string strDfsPath = value["DFSPath"].asString();
				int Id = value["Id"].asInt();

				//�ϴ��ļ�
				//�ϴ��ļ���DFS
				//����ϴ��ɹ��������RPC�ϴ��ɹ�,RPC����ʱ��ת�Ƶ����Ǳ��������µ�fileId
				url = new string(HOST_URL);
				url->append("/Upload/Upload");
				client->Clear();//����������
				//client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
				client->m_params.insert(map<string,string>::value_type("hash",hash));
				client->m_params.insert(map<string,string>::value_type("DFSPath",strDfsPath));
				client->m_strFilefieldName = "UploadFile";
				//client->m_strFileFullPath = W2CA(wstrLongLocalPath.c_str());//�ϴ����Ǳ���cache
				chFilename.insert(0,_T("\\"));
				chFilename.insert(0,CACHEPATH_PREFIXW);
				client->m_strFileFullPath = W2CA(chFilename.c_str());
				client->Request(const_cast<char*>(url->c_str()));

				//�����ϴ����
				bool jsonRet = reader.parse(client->m_strBuffer,value);
				if(!jsonRet)
				{
					MessageBox(NULL,_T("/Upload/Upload,�ϴ��ļ�ʧ��,�鿴��־���޷���������json"),_T("����"),NULL);
					APP_TRACE("/Upload/Upload,�ϴ�%sʧ��,�޷���������json",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}

				if (0 != value["ret"].asInt())
				{
					APP_TRACE("�ϴ�%sʧ��,ԭ��Ϊ:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
					goto EXIT;
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
					MessageBox(NULL,_T("UploadCompleteʧ��,�޷�����json"),_T("����"),NULL);
					APP_TRACE("�ϴ�%sʧ�ܣ�UploadCompleteʧ��,,�޷�����json",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}

				if (0 != value["ret"].asInt())
				{
					APP_TRACE("UploadComplete%sʧ��,ԭ��Ϊ:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
					goto EXIT;
				}

				//�����յ�RPC�ķ��أ��ж��Ƿ��ǳ�ͻ��Ϣ�������ͻ�����շ��ص��ļ����ѱ����ļ�����
				char sqlbuf[256];
				if(value["Conflicted"].asBool())
				{
					//����
					wstring newpath(PATH_PREFIXW);
					newpath.append(CA2W(value["NewFilename"].asString().c_str()));
					MoveFile(wstrLongLocalPath.c_str(),newpath.c_str());
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
					APP_TRACE("�������ݿ�%sʧ�ܣ��������ݿ�ʧ��",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}
				bOperateResult = true;
						 
			}
			else if(isExits && (oldHash != hash))
			{
				//�߸�������	,�޲�ʵ�֣�������ļ��ڿͻ���û�д򿪵�ʱ��ɾ���ٴ���,���߿ͻ�������֮������������
				bOperateResult = true;
				goto EXIT;
			}
			else if(isExits && (oldHash == hash))
			{
				//ɾ�����һأ��ұ���û���ύ����������ӷ������ո�����
				//���������
				bOperateResult = true;
				goto EXIT;
			}
		}
		else if (operate->m_operate == E_FILE_UPDATE_FILE)
		{
			//�޸��ļ�
			bOperateResult = false;
			//todo
			goto EXIT;
		}
		else if (operate->m_operate == E_FILE_DELETE)
		{
			//ɾ��
			//TODO
			goto EXIT;
		}
		else
		{
			APP_TRACE("��֧�ֵĲ�������");
			bOperateResult = true;
			goto EXIT;
		}

	EXIT:
		SAFE_DELETE(calc);
		SAFE_DELETE(client);
		SAFE_DELETE(url);
		return bOperateResult;
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
		

		if(ret)
		{
			t.setRow(0);
			*strOldFilename = t.getStringField(0);
		}
		t.finalize();
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

	CRITICAL_SECTION *_critical;
};

#pragma warning(default:4995) 
#pragma warning(default:4800)
#pragma warning(default:4996)