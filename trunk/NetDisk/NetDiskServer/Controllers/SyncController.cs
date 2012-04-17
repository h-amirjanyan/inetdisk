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
    public class SyncController : Controller
    {
        private NetdiskContext db = new NetdiskContext();
        //
        // GET: /Sync/

        //private UnitOfWork db = new UnitOfWork();
        public ActionResult Index()
        {
            //var test = db.FilesRepository.GetFiles("/", null);
            var test = db.Files.ToList();
            return View();
        }

        protected override void Dispose(bool disposing)
        {
            db.Dispose();
            base.Dispose(disposing);
        }


    }
}
