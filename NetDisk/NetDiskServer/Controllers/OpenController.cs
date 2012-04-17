using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using NetDiskServer.Models;
using NetDiskServer.ViewModels;
using NetDiskServer.DAL;

namespace NetDiskServer.Controllers
{
    /// <summary>
    /// Oauth验证接口
    /// </summary>
    public class OpenController : Controller
    {
        private UnitOfWork unitOfWork = new UnitOfWork();

        /// <summary>
        /// Requests the token.
        /// </summary>
        /// <returns> </returns>
        public JsonResult RequestToken(Models.RequestTokenModel model)
        {
            ViewModels.RequestTokenModel ret = new ViewModels.RequestTokenModel();
            ret.oauth_callback_confirmed = false;
            ret.oauth_token = "test_token";
            ret.oauth_token_secret = "test_secret";
            return Json(ret, JsonRequestBehavior.AllowGet);
        }

        /// <summary>
        /// Authorizes this instance.
        /// This Action will redirect to a webpage 
        /// 在这个页面中会验证用户的口令，如果成功会跳转到call back。
        /// </summary>
        /// <returns></returns>
        [HttpGet]
        public ActionResult Authorize(Models.AuthorizeModel model)
        {
            
            //return Json("", JsonRequestBehavior.AllowGet);

            //ViewModels.Authorize2Model ret = new ViewModels.Authorize2Model();

            return View(model);
        }
        
        [HttpPost]
        public ActionResult Authorize(Models.Authorize2Model model)
        {
            var user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.Email == model.Email && u.Password == model.Password);
            if (user == null)
            {
                ModelState.AddModelError("", "密码不正确");
            }
            else //密码正确，把临时token换成正式的token，并存入缓存，与当前用户相关
            {
                string token = "test_token_ok";
                string tokenSecret = "test_token_secret_ok";
                OauthTokenPair tokenpair = new OauthTokenPair();
                tokenpair.oauth_token = token;
                tokenpair.oauth_token_secret = tokenSecret;
                tokenpair.UserId = user.UserName;

                unitOfWork.TokenReposity.Insert(tokenpair);

                return RedirectToAction("AuthorizeCompleted",123);
                
            }
            //验证数据，并跳转到AuthorizeCompleted
            return View();
        }

        public ActionResult AuthorizeCompleted(int verifyCode)
        {
               //int verifyCode = 0;//校验码
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

        protected override void Dispose(bool disposing)
        {
            unitOfWork.Dispose();
            base.Dispose(disposing);
        }
    }
}
