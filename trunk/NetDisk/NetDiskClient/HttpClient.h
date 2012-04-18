#pragma once

#include <curl/curl.h>
#include <string>
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

	public:
		char m_chErrorBuffer[CURL_ERROR_SIZE];
		string m_strBuffer;
	};

}

