using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    [Serializable]
    public class OauthTokenPair
    {
        public string oauth_token { get; set; }
        public string oauth_token_secret { get; set; }
        public int UserId { get; set; }
    }

    [Serializable]
    public class TempToken2OfficialMap
    {
        public string oauth_temp_token { get; set; }

        public string oauth_token { get; set; }
    }
}