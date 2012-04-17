using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.ViewModels
{
    public class RequestTokenModel
    {
        /// <summary>
        /// Gets or sets the oauth_token_secret.
        /// </summary>
        /// <value>
        /// The oauth_token_secret.
        /// </value>
        public string oauth_token_secret { get; set; }

        /// <summary>
        /// Gets or sets the oauth_token.
        /// </summary>
        /// <value>
        /// The oauth_token.
        /// </value>
        public string oauth_token { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this <see cref="RequestTokenViewModel"/> is oauth_callback_confirmed.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if oauth_callback_confirmed; otherwise, <c>false</c>.
        /// </value>
        public bool oauth_callback_confirmed { get; set; }
    }


    public class Authorize2Model
    {
        public string oauth_verifier { get; set; }
    }

    public class AccessTokenModel
    {
        public string oauth_token_secret { get; set; }

        public string oauth_token { get; set; }

        public int UserId { get; set; }

        public string charged_dir { get; set; }
    }
}