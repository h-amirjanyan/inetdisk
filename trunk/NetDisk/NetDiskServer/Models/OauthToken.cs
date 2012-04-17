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
        public string UserId { get; set; }
    }
}