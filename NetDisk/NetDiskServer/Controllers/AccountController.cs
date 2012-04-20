using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using NetDiskServer.Models;
using NetDiskServer.DAL;

namespace NetDiskServer.Controllers
{
    public class AccountController : Controller
    {
        //
        // GET: /Account/

        private UnitOfWork db = new UnitOfWork();

        public ActionResult Index()
        {
            return View();
        }

        [HttpGet]
        public ActionResult Register()
        {
            return View();
        }

        [HttpPost]
        public ActionResult Register(RegisterModel model)
        {
            //TODO:注册的时候检查是否唯一
            if (ModelState.IsValid)
            {
                NetDiskUser user = new NetDiskUser();
                user.Email = model.Email;
                user.RegisterDate = DateTime.Now;
                user.LastLoginDate = DateTime.Now;
                user.UserName = model.UserName;
                user.Password = model.ConfirmPassword;

                if (model.Password == model.ConfirmPassword)
                {

                    db.UserRepositiry.Insert(user);
                    db.Save();
                    return RedirectToAction("RegisterSuccess");
                }
                else
                {
                    ModelState.AddModelError("", "两次输入的密码不一样");
                }
            }

            // If we got this far, something failed, redisplay form
            return View(model);
            
        }

        public ActionResult RegisterSuccess()
        {
            return View();
        }

        protected override void Dispose(bool disposing)
        {
            db.Dispose();
            base.Dispose(disposing);
        }

    }
}
