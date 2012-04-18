#pragma once

#include <string>
//#include

namespace API
{
	using namespace std;
	class OauthClient
	{
	public:
		OauthClient(void);
		~OauthClient(void);
	public:
		bool GetTempToken(string& str_oauth_token,string &oauth_token_secret)
		{

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

