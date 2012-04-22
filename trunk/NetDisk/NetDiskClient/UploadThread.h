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
			//临界区开始
			EnterCriticalSection(self->_critical);
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
			//临界区结束
			LeaveCriticalSection(self->_critical);
			if(self->pTodolist->GetSize() == 0)
			{
				APP_TRACE("没有文件需要上传，进入休眠状态");
				//Sleep(10*60*1000);//休眠10s
				Sleep(30*1000);
				APP_TRACE("上传线程休眠结束");
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
		string* url = new string(HOST_URL);

		Json::Reader reader;
		Json::Value value;

		unsigned char buf[16];
		MD5Calc* calc = new MD5Calc();
		bool bOperateResult = false;
		string hash;
		

		string strFilename(W2CA(const_cast<TCHAR*>(operate->m_strfilename.c_str()))); //for:\test\demo.doc
		string netdisk_prefix(PATH_PREFIX);
		strFilename = strFilename.substr(netdisk_prefix.length());

		if(operate->m_operate == E_FILE_CREATE || operate->m_operate == E_FILE_UPDATE_FILE)
		{
			calc->mdfile(const_cast<char*>(W2CA(operate->m_strfilename.c_str())),buf);
			hash = calc->print_digest(buf);
		}
		wstring chFilename(CA2W(const_cast<char*>(hash.c_str()))); //hash
		

		wstring wstrLongLocalPath(operate->m_strfilename.c_str());
		BakCacheFile(wstrLongLocalPath,chFilename);//TODO:操作完成之后删除hash,可选

		if(operate->m_operate == E_FILE_CREATE)
		{
			

			url->append("/RPC/UploadPrepare");
			client->Clear();//清理上下文


			//现在本地数据库中查询是否存在，hash是否更改
			bool isExits = IsExitsInLocalDB(strFilename);
			string oldHash ;
			if(isExits) //不存在
			{
				GetOldHash(strFilename,&oldHash);
			}

			if(!isExits) //不存在
			{
				
				//先把文件复制到临时文件夹备份 cache/hash，避免上传的操作的hash和现在的hash不一致的情况
				
				client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
				client->m_params.insert(map<string,string>::value_type("FileName",strFilename));
				client->m_params.insert(map<string,string>::value_type("FileSize","0"));
				client->m_params.insert(map<string,string>::value_type("Hash",hash));
				client->Request(const_cast<char*>(url->c_str()));
				SAFE_DELETE(url);
				//分析返回的结果，判断

				
				if(!reader.parse(client->m_strBuffer,value))
				{
					MessageBox(NULL,_T("/RPC/UploadPrepare 返回结果解析失败,查看日志"),_T("错误"),NULL);
					APP_TRACE("/RPC/UploadPrepare,%s返回结果解析失败",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}

				int ret = value["ret"].asInt();
				if(ret != 0)
				{
					APP_TRACE("上传%s失败,原因为:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
					goto EXIT;
				}

				
				//TODO判断服务器上该路径的该hash的文件是否存在，如果存在则是没有插入到本地数据库，此时需要重插
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

				//上传文件
				//上传文件到DFS
				//如果上传成功，则高数RPC上传成功,RPC从临时表转移到正是表，并返回新的fileId
				url = new string(HOST_URL);
				url->append("/Upload/Upload");
				client->Clear();//清理上下文
				//client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
				client->m_params.insert(map<string,string>::value_type("hash",hash));
				client->m_params.insert(map<string,string>::value_type("DFSPath",strDfsPath));
				client->m_strFilefieldName = "UploadFile";
				//client->m_strFileFullPath = W2CA(wstrLongLocalPath.c_str());//上传的是本地cache
				chFilename.insert(0,_T("\\"));
				chFilename.insert(0,CACHEPATH_PREFIXW);
				client->m_strFileFullPath = W2CA(chFilename.c_str());
				client->Request(const_cast<char*>(url->c_str()));

				//分析上传结果
				bool jsonRet = reader.parse(client->m_strBuffer,value);
				if(!jsonRet)
				{
					MessageBox(NULL,_T("/Upload/Upload,上传文件失败,查看日志，无法解析返回json"),_T("错误"),NULL);
					APP_TRACE("/Upload/Upload,上传%s失败,无法解析返回json",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}

				if (0 != value["ret"].asInt())
				{
					APP_TRACE("上传%s失败,原因为:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
					goto EXIT;
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
					MessageBox(NULL,_T("UploadComplete失败,无法解析json"),_T("错误"),NULL);
					APP_TRACE("上传%s失败，UploadComplete失败,,无法解析json",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}

				if (0 != value["ret"].asInt())
				{
					APP_TRACE("UploadComplete%s失败,原因为:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
					goto EXIT;
				}

				//本地收到RPC的返回，判断是否是冲突消息，如果冲突，按照返回的文件名把本地文件改名
				char sqlbuf[256];
				if(value["Conflicted"].asBool())
				{
					//改名
					wstring newpath(PATH_PREFIXW);
					newpath.append(CA2W(value["NewFilename"].asString().c_str()));
					MoveFile(wstrLongLocalPath.c_str(),newpath.c_str());
				}
				//插入新纪录
				sprintf(sqlbuf,"insert into Files (Id,FileName,Reversion,Hash) values(%d,'%s','%d','%s');",
					value["Id"].asInt(),
					value["NewFilename"].asString().c_str(),
					value["Reversion"].asInt(),
					hash.c_str()
					);


				if(!UpdateDB(sqlbuf))
				{
					MessageBox(NULL,_T("更新数据库失败"),_T("错误"),NULL);
					APP_TRACE("更新数据库%s失败，更新数据库失败",W2CA(wstrLongLocalPath.c_str()));
					goto EXIT;
				}
				bOperateResult = true;
						 
			}
			else if(isExits && (oldHash != hash))
			{
				//走更新流程	,赞不实现，情况是文件在客户端没有打开的时候删除再创建
				//暂时不实现，一切修改必须在客户端打开的情况下
				bOperateResult = true;
				goto EXIT;
			}
			else if(isExits && (oldHash == hash))
			{
				//删除再找回，且本地没有提交到服务器或从服务器刚刚下载
				//此情况忽略
				bOperateResult = true;
				goto EXIT;
			}
		}
		else if (operate->m_operate == E_FILE_UPDATE_FILE)
		{
			/*calc->mdfile(const_cast<char*>(W2CA(operate->m_strfilename.c_str())),buf);
			hash = calc->print_digest(buf);*/

			//修改文件
			//查看修改过后的hash是否和localDB相同，相同就不操作，返回操作成功
			//不相同就发起更新请求
			bOperateResult = false;
			bool bIsExitsLocalDB = IsExitsInLocalDB(strFilename);
			if(bIsExitsLocalDB)
			{
				string oldHash;
				GetOldHash(strFilename,&oldHash);
				if(oldHash == hash) //为下载线程的修改
				{
					APP_TRACE("本地没有修改，不更新%s",operate->m_strfilename.c_str());
					bOperateResult = true;
					goto EXIT;					
				}
				
				SAFE_DELETE(url);
				url = new string(HOST_URL);
				url->append("/");
				url->append("RPC/ModifyPrepare");
				client->Clear();
				int localId = 0;
				int localVersion = -1;
				GetFileIdVer(localId,localVersion,strFilename);
				char verbuf[10];
				char idbuf[10];
				
				sprintf(verbuf,"%d",localVersion);
				sprintf(idbuf,"%d",localId);
				client->AddParam("oauth_token",oauth_token);
				client->AddParam("LocalId",idbuf);
				client->AddParam("LocalPath",strFilename.c_str());
				client->AddParam("LocalVersion",verbuf);
				client->AddParam("NewHash",hash);
				client->AddParam("NewFileSize","0");
				client->Request(const_cast<char*>(url->c_str()));
				
				if(!reader.parse(client->m_strBuffer,value))
				{
					APP_TRACE("请求%s失败,err info:解析json字符串失败",url->c_str());
					goto EXIT;
				}

				if(value["ret"] != 0)
				{
					APP_TRACE("请求%s失败,err info:%s",url->c_str(),value["msg"].asString().c_str());
					goto EXIT;
				}
			
				if(value["IsOutOfData"].asBool()/* && !value["IsDeleted"].asBool()*/)
				{
					//直接把本地文件改名
					wstring bakfilename(operate->m_strfilename);
					bool bCopy = false;
					do 
					{
						bakfilename.append(_T("_ConfictedCopy"));
						bCopy = CopyFile(operate->m_strfilename.c_str(),bakfilename.c_str(),TRUE);
					} while (!bCopy);
					
					bOperateResult = true;
					goto EXIT;
				}
				//else if(value["IsOutOfData"].asBool() && value["IsDeleted"].asBool())
				//{
				//	//本地这个版本被标记为删除
				//}
				else
				{
					//按照返回的数据请求upload/upload,

					string strDfsPath = value["DFSPath"].asString();
					int Id = value["Id"].asInt();

					url = new string(HOST_URL);
					url->append("/Upload/Upload");
					client->Clear();//清理上下文
					//client->m_params.insert(map<string,string>::value_type("oauth_token",oauth_token));
					client->m_params.insert(map<string,string>::value_type("hash",hash));
					client->m_params.insert(map<string,string>::value_type("DFSPath",strDfsPath));
					client->m_strFilefieldName = "UploadFile";
					//client->m_strFileFullPath = W2CA(wstrLongLocalPath.c_str());//上传的是本地cache
					chFilename.insert(0,_T("\\"));
					chFilename.insert(0,CACHEPATH_PREFIXW);
					client->m_strFileFullPath = W2CA(chFilename.c_str());
					client->Request(const_cast<char*>(url->c_str()));

					//分析上传结果
					bool jsonRet = reader.parse(client->m_strBuffer,value);
					if(!jsonRet)
					{
						MessageBox(NULL,_T("/Upload/Upload,上传文件失败,查看日志，无法解析返回json"),_T("错误"),NULL);
						APP_TRACE("/Upload/Upload,上传%s失败,无法解析返回json",W2CA(wstrLongLocalPath.c_str()));
						goto EXIT;
					}

					if (0 != value["ret"].asInt())
					{
						APP_TRACE("上传%s失败,原因为:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
						goto EXIT;
					}

					//请求RPC/ModifyComplete

					SAFE_DELETE(url);
					url = new string(HOST_URL);
					url->append("/RPC/ModifyComplete");
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
						MessageBox(NULL,_T("UploadComplete失败,无法解析json"),_T("错误"),NULL);
						APP_TRACE("上传%s失败，UploadComplete失败,,无法解析json",W2CA(wstrLongLocalPath.c_str()));
						goto EXIT;
					}

					if (0 != value["ret"].asInt())
					{
						APP_TRACE("UploadComplete%s失败,原因为:%s",W2CA(wstrLongLocalPath.c_str()),value["msg"].asString().c_str());
						goto EXIT;
					}

					//本地收到RPC的返回，判断是否是冲突消息，如果冲突，按照返回的文件名把本地文件改名
					char sqlbuf[256];
					if(value["Conflicted"].asBool())
					{
						//改名
						wstring newpath(PATH_PREFIXW);
						newpath.append(CA2W(value["NewFilename"].asString().c_str()));
						MoveFile(wstrLongLocalPath.c_str(),newpath.c_str());
						//插入新纪录
						sprintf(sqlbuf,"insert into Files (Id,FileName,Reversion,Hash) values(%d,'\\%s','%d','%s');",
							value["Id"].asInt(),
							value["NewFilename"].asString().c_str(),
							value["Reversion"].asInt(),
							hash.c_str()
							);
					}
					else
					{
						//修改数据库
						sprintf(sqlbuf,"update Files Set Id = %d ,Reversion = %d,Hash ='%s' where FileName like '\\%s'",
							value["Id"].asInt(),
							value["Reversion"].asInt(),
							hash.c_str(),
							value["NewFilename"].asString().c_str());
					}

					if(!UpdateDB(sqlbuf))
					{
						MessageBox(NULL,_T("更新数据库失败"),_T("错误"),NULL);
						APP_TRACE("更新数据库%s失败，更新数据库失败",W2CA(wstrLongLocalPath.c_str()));
						goto EXIT;
					}
					bOperateResult = true;

				}

				goto EXIT;
			}
			else
			{
				//本地数据库还没有记录,把当前操作变成新建操作，加入到操作队列
				operate->m_operate = E_FILE_CREATE;
				bOperateResult = false;
				goto EXIT;
			}
		}
		else if (operate->m_operate == E_FILE_DELETE)
		{
			//删除
			//发起删除请求，标记remote db中响应的版本为删除，以路径和hash判断
			url->append("/RPC/DeleteFile");

			client->Clear();
			client->AddParam("oauth_token",oauth_token);
			int id = -1;
			int localversion = -1;
			char idbuf[10];
			char verbuf[10];
			GetFileIdVer(id,localversion,strFilename);
			itoa(id,idbuf,10);
			itoa(localversion,verbuf,10);

			
			client->AddParam("Id",idbuf);
			client->AddParam("LocalVersion",verbuf);
			client->Request(const_cast<char*>(url->c_str()));
			if(!reader.parse(client->m_strBuffer,value))
			{
				APP_TRACE("请求%s解析json失败",url->c_str());
				goto EXIT;
			}

			if (value["ret"].asInt() != 0)
			{
				APP_TRACE("请求%s失败,err info:%s",url->c_str(),value["msg"].asString().c_str());
				goto EXIT;
			}

			if(value["IsOutOfData"].asBool())
			{
				APP_TRACE("delete操作：%s文件已经过期",operate->m_strfilename.c_str());
				/*wstring bakfilename(operate->m_strfilename);
				bakfilename.append(_T("_ConfictedCopy"));*/
				wstring localFileName(operate->m_strfilename);
				bool bDel = false;
				do 
				{
					localFileName.append(_T("_ConfictedCopyDL"));
					bDel = CopyFile(operate->m_strfilename.c_str(),localFileName.c_str(),true);
				} while (!bDel);
				bOperateResult = true;
			}
			else
			{
				char szbuf[300];
				sprintf(szbuf,"delete from Files where Id = %d",id);
				DeleteFile(operate->m_strfilename.c_str());
				bOperateResult = true;
			}

			goto EXIT;
		}
		else
		{
			APP_TRACE("不支持的操作类型");
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
	void GetFileIdVer(int &id,int &localversion,string strFilename)
	{
		CppSQLite3DB db;
		db.open(strDBPath.c_str());
		char buffer[100];
		sprintf(buffer,"select Id,Reversion from Files where FileName like '%s'",strFilename.c_str());
		CppSQLite3Table t = db.getTable(buffer);
		bool ret = (bool)t.numRows();

		if(ret)
		{
			t.setRow(0);
			id = t.getIntField(0);
			localversion = t.getIntField(1);
		}
		t.finalize();
		db.close();
	}

	void GetOldHash(string strFilename,string* strOldHash)
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
			*strOldHash = t.getStringField(0);
		}
		t.finalize();
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

	CRITICAL_SECTION *_critical;
};

#pragma warning(default:4995) 
#pragma warning(default:4800)
#pragma warning(default:4996)