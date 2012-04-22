#pragma once

#include <curl/curl.h>
#include <string>
#include <map>

namespace Utils
{
	using namespace std;

	class HttpClient
	{
	public:
		HttpClient(void);
		~HttpClient(void);

	public:
//得到
		//long GetUrl(string& url,string& content)
		//{
		//	CURL* curl;
		//	CURLcode res;    
		//	long response_code = 0;
		//	curl = curl_easy_init();
		//	res = curl_easy_setopt(curl, CURLOPT_URL, url);
		//	res = curl_easy_perform(curl); /* ignores error */
		//	if(CURLE_OK == res)
		//	{
		//		//char* ct;
		//		
		//		res = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&response_code);
		//		if(res == CURLE_OK && response_code == 200) //下载成功
		//		{

		//		}
		//	}


		//	curl_easy_cleanup(curl);
		//}

		static int writer(char *data,size_t dataSize,size_t nmemb,string * writerData)
		{
			if(writerData == NULL)
				return 0;
			writerData->append(data,dataSize*nmemb);
			return dataSize * nmemb;
		}

		bool GetBodyByUrl(char* url)
		{
			CURL *conn = NULL;
			CURLcode code;
			
			if (!init(conn,url))
			{
				fprintf(stderr, "Connection initializion failed\n");
				return false;
			}
			

			// Retrieve content for the URL

			code = curl_easy_perform(conn);
			curl_easy_cleanup(conn);

			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to get '%s' [%s]\n", url, this->m_chErrorBuffer);

				return false;
			}

			// Parse the (assumed) HTML code

			//parseHtml(buffer, title);

			// Display the extracted title

			printf("Title: %s\n", this->m_strBuffer.c_str());

			//return EXIT_SUCCESS;
			return true;
		}

		bool init(CURL *&conn, char *url)
		{
			CURLcode code;

			conn = curl_easy_init();

			if (conn == NULL)
			{
				fprintf(stderr, "Failed to create CURL connection\n");

				exit(EXIT_FAILURE);
			}

			code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, this->m_chErrorBuffer);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set error buffer [%d]\n", code);

				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_URL, url);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set URL [%s]\n", this->m_chErrorBuffer);

				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set redirect option [%s]\n", this->m_chErrorBuffer);

				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, &writer);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set writer [%s]\n", this->m_chErrorBuffer);

				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &(this->m_strBuffer));
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set write data [%s]\n", this->m_chErrorBuffer);

				return false;
			}

			return true;
		}



		bool Request(char* url)
		{
			string strurl(url);
			CURL *conn = NULL;
			CURLcode code;


			struct curl_httppost *formpost=NULL;
			struct curl_httppost *lastptr=NULL;
			struct curl_slist *headerlist=NULL;
			static const char buf[] = "Expect:";

			conn = curl_easy_init();

			if (conn == NULL)
			{
				fprintf(stderr, "Failed to create CURL connection\n");

				exit(EXIT_FAILURE);
			}

			code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, this->m_chErrorBuffer);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set error buffer [%d]\n", code);

				return false;
			}

			if(m_params.size() > 0)
			{
				strurl += "?";
				map<string,string>::iterator it = m_params.begin();
				for (;it != m_params.end(); ++it )
				{
					if(it != m_params.begin())
						strurl += "&";
					strurl += it->first;
					strurl += "=";
					strurl += curl_easy_escape(conn,it->second.c_str(),it->second.length());
				}
			}

			code = curl_easy_setopt(conn, CURLOPT_URL, strurl.c_str());
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set URL [%s]\n", this->m_chErrorBuffer);
				return false;
			}

			if(!m_strFilefieldName.empty() && !m_strFileFullPath.empty())
			{
				headerlist = curl_slist_append(headerlist, buf);
				/* Fill in the file upload field */ 
				curl_formadd(&formpost,
					&lastptr,
					CURLFORM_COPYNAME, m_strFilefieldName.c_str(),
					CURLFORM_FILE, m_strFileFullPath.c_str(),
					CURLFORM_END);
				curl_easy_setopt(conn, CURLOPT_HTTPHEADER, headerlist);
				curl_easy_setopt(conn, CURLOPT_HTTPPOST, formpost);
			}

			code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set redirect option [%s]\n", this->m_chErrorBuffer);
				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, &writer);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set writer [%s]\n", this->m_chErrorBuffer);
				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &(this->m_strBuffer));
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set write data [%s]\n", this->m_chErrorBuffer);
				return false;
			}

			code = curl_easy_perform(conn);
			curl_easy_cleanup(conn);
			/* then cleanup the formpost chain */ 
			curl_formfree(formpost);
			/* free slist */ 
			curl_slist_free_all (headerlist);

			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to get '%s' [%s]\n", url, this->m_chErrorBuffer);
				return false;
			}
			printf("buffer: %s\n", this->m_strBuffer.c_str());

			return true;
		}
		
		bool SaveFile(char* url,char* filename)
		{
			string strurl(url);
			CURL *conn = NULL;
			CURLcode code;


			

			conn = curl_easy_init();

			if (conn == NULL)
			{
				fprintf(stderr, "Failed to create CURL connection\n");

				exit(EXIT_FAILURE);
			}

			code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, this->m_chErrorBuffer);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set error buffer [%d]\n", code);

				return false;
			}

			if(m_params.size() > 0)
			{
				strurl += "?";
				map<string,string>::iterator it = m_params.begin();
				for (;it != m_params.end(); ++it )
				{
					if(it != m_params.begin())
						strurl += "&";
					strurl += it->first;
					strurl += "=";
					strurl += curl_easy_escape(conn,it->second.c_str(),it->second.length());
				}
			}

			code = curl_easy_setopt(conn, CURLOPT_URL, strurl.c_str());
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set URL [%s]\n", this->m_chErrorBuffer);
				return false;
			}


			code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set redirect option [%s]\n", this->m_chErrorBuffer);
				return false;
			}

			code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, &write_file_data);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set writer [%s]\n", this->m_chErrorBuffer);
				return false;
			}
			FILE* pFile = fopen(filename,"wb"); //覆盖写 http://www.cplusplus.com/reference/clibrary/cstdio/fopen/
			code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, (void*)pFile);
			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to set write data [%s]\n", this->m_chErrorBuffer);
				return false;
			}

			code = curl_easy_perform(conn);

			//fclose(pFile);
			int hclose = fclose(pFile);
			
			curl_easy_cleanup(conn);

			if (code != CURLE_OK)
			{
				fprintf(stderr, "Failed to get '%s' [%s]\n", url, this->m_chErrorBuffer);
				return false;
			}
			printf("buffer: %s\n", this->m_strBuffer.c_str());

			return true;
		}

		static size_t write_file_data(char* buffer,size_t size,size_t nitems,void *outstream)
		{
			int written = fwrite(buffer,size,nitems,(FILE*)outstream);
			return written;
		}

		void AddParam(string index,string strvalue)
		{
			m_params.insert(map<string,string>::value_type(index,strvalue));
		}

		void Clear()
		{
			m_strFilefieldName = "";
			m_strFileFullPath = "";
			m_strBuffer = "";
			m_params.clear();
			memset(m_chErrorBuffer,0,CURL_ERROR_SIZE);
		}
	public:
		char m_chErrorBuffer[CURL_ERROR_SIZE];
		string m_strBuffer;
		map<string,string> m_params;

		string m_strFilefieldName;
		string m_strFileFullPath;
	};

}

