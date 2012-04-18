using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    //暂时只验证oauth_token
    public class SyncBaseModel
    {
        public string oauth_consumer_key { get; set; }
        public string oauth_token { get; set; }
        public string signature_method { get; set; }
        public string oauth_signature { get; set; }
        public string timestamp { get; set; }
        public string oauth_nonce { get; set; }
        public string oauth_version { get; set; }
    }
}