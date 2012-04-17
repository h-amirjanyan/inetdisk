using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using NetDiskServer.Models;
using NetDiskServer.ViewModels;

namespace NetDiskServer.Controllers
{
    /// <summary>
    /// Oauth验证接口
    /// </summary>
    public class OpenController : Controller
    {
        /// <summary>
        /// Requests the token.
        /// </summary>
        /// <returns> </returns>
        public JsonResult RequestToken(Models.RequestTokenModel model)
        {
            return Json("", JsonRequestBehavior.AllowGet);
        }

        /// <summary>
        /// Authorizes this instance.
        /// This Action will redirect to a webpage 
        /// 在这个页面中会验证用户的口令，如果成功会跳转到call back。
        /// </summary>
        /// <returns></returns>
        [HttpGet]
        public ViewResult Authorize()
        {
            //return Json("", JsonRequestBehavior.AllowGet);
            //TODO:生成
            return View();
        }
        
        [HttpPost]
        public ViewResult Authorize2()
        {
            //验证数据，并跳转到AuthorizeCompleted
            return View();
        }

        public ViewResult AuthorizeCompleted()
        {
            int verifyCode = 0;//校验码
            return View(verifyCode);
        }

        /// <summary>
        /// Accesses the token.
        /// </summary>
        /// <returns></returns>
        public JsonResult AccessToken()
        {
            return Json("", JsonRequestBehavior.AllowGet);
        }
    }
}
