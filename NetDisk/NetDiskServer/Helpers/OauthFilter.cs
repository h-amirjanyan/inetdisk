using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace NetDiskServer.Helpers
{
    public class OauthFilter : ActionFilterAttribute
    {
        public override void OnActionExecuting(ActionExecutingContext filterContext)
        {
            //验证oauth-token and oauth_token_secret
            string oauth_token = filterContext.RouteData.Values["oauth_token"] as string;
            if (-1 == TokenHelper.CheckToken(oauth_token))
            {
                filterContext.HttpContext.Response.Write("{\"ret\":-1;\"msg\"=\"can not find the token\"}");
                filterContext.HttpContext.Response.End();
            }
            
            base.OnActionExecuting(filterContext);
        }
    }
}