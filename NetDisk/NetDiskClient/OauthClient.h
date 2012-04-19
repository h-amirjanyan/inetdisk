#pragma once

#include <string>
#include "HttpClient.h"
#include "WebBrowser.h"
#include <json/json.h>
#include <atlbase.h>
//#include

namespace API
{
	using namespace std;
	using namespace Utils;

	class RequestTokenModel
	{
	public:
		string oauth_consumer_key;
		string oauth_signature_method;
		string oauth_signature;
		int oauth_timestamp;
		string oauth_nonce;
		string oauth_version;
	};

	class OauthClient
	{
	public:
		OauthClient(void);
		~OauthClient(void);
	public:
		bool GetTempToken(string& str_oauth_token,string &str_oauth_token_secret)
		{
			HttpClient* client = new HttpClient();
			if(client->GetBodyByUrl("http://127.0.0.1:5243/Open/RequestToken?oauth_consumer_key=testconsumerkey"))
			{
				Json::Reader reader;
				Json::Value value;
				if(reader.parse(client->m_strBuffer,value))
				{
					bool isNull = value["oauth_token"].isNull();
					if(!isNull)
					{
						str_oauth_token = value["oauth_token"].asString();
					}
					if(!value["oauth_token_secret"].isNull())
					{
						str_oauth_token_secret = value["oauth_token_secret"].asString();
					}
					return !isNull;
				}
			}
			return false;
			
			/*public string oauth_consumer_key { get; set; }

			public string oauth_signature_method { get; set; }

			public string oauth_signature { get; set; }

			public int oauth_timestamp { get; set; }

			public string oauth_nonce { get; set; }

			public string oauth_version { get; set; }*/


		}

		bool Authorize(string& str_oauth_token)
		{
			//打开浏览器获取oauth_token
			MyWebBrowser* bro = new MyWebBrowser();
			string* url = new string("http://127.0.0.1:5243/Open/Authorize?oauth_token=");//CA2W
			url->append(str_oauth_token);
			bool ret = bro->OpenUrl(CA2W(url->c_str()));
			delete bro;
			return ret;
		}

		bool AccessToken(string& str_oauth_token,string& str_oauth_token_secret,int& str_UserId)
		{

			HttpClient* client = new HttpClient();
			string* url = new string("http://127.0.0.1:5243/Open/AccessToken?oauth_token=");
			url->append(str_oauth_token);
			if(client->GetBodyByUrl(const_cast<char*>(url->c_str())))
			{
				Json::Reader reader;
				Json::Value value;
				if(reader.parse(client->m_strBuffer,value))
				{
					bool isNull = value["oauth_token"].isNull();
					if(!isNull)
					{
						str_oauth_token = value["oauth_token"].asString();
					}
					if(!value["oauth_token_secret"].isNull())
					{
						str_oauth_token_secret = value["oauth_token_secret"].asString();
					}
					if(!value["UserId"].isNull())
					{
						str_UserId = value["UserId"].asInt();
					}
					return !isNull;
				}
			}
			delete url;
			return false;
		}
		void Test(string url)
		{
			//CURL* curl;
			//CURLcode res;    
			//curl = curl_easy_init();
			//res = curl_easy_setopt(curl, CURLOPT_URL, url);
			//res = curl_easy_perform(curl); /* ignores error */
			//if(CURLE_OK == res)
			//{
			//	char* ct;
			//	res = curl_easy_getinfo(curl,CURLINFO_
			//}


			//curl_easy_cleanup(curl);

		}
	};
}

