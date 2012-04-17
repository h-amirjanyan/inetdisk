using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    public class RequestTokenModel
    {
        public string oauth_consumer_key { get; set; }

        public string oauth_signature_method { get; set; }

        public string oauth_signature { get; set; }

        public int oauth_timestamp { get; set; }

        public string oauth_nonce { get; set; }

        public string oauth_version { get; set; }

        /// <summary>
        /// Gets or sets the oauth_callback.
        /// </summary>
        /// <value>
        /// The oauth_callback url.
        /// </value>
        public string oauth_callback { get; set; }
    }

    public class AuthorizeModel
    {
        /// <summary>
        /// Gets or sets the temp oauth_token.
        /// </summary>
        /// <value>
        /// The oauth_token.
        /// </value>
        public string oauth_token { get; set; }
    }

    /// <summary>
    /// authorise 页面提交的结果，验证过后会跳转到指定的callback
    /// </summary>
    public class Authorize2Model
    {
        public string Email { get; set; }

        public string Password { get; set; }

        public bool allow { get; set; }
    }

    public class AccessTokenModel
    {
        public string oauth_consumer_key { get; set; }

        public string oauth_signature_method { get; set; }

        public string oauth_signature { get; set; }

        public int timestamp { get; set; }

        public string oauth_nonce { get; set; }

        public string oauth_version { get; set; }

        public string oauth_token { get; set; }

        public string oauth_verifier { get; set; }
    }
}
