using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using NetDiskServer.Helpers;
using NetDiskServer.ViewModels;

namespace NetDiskServer.Controllers
{
    //Upload Server
    public class UploadController : Controller
    {
        private static string DFS_BASEPATH = "C:\\DFSRoot";
        /// <summary>
        /// 上传文件，导入到DFS，把DFS路径记录到FileUncomplete表
        /// </summary>
        /// <returns></returns>
        public JsonResult Upload(string DFSPath,string hash)
        {
            SyncBaseViewModel viewModel = new SyncBaseViewModel();

            if (Request.Files["UploadFile"].HasFile())
            {
                try
                {
                    Request.Files["UploadFile"].SaveAs(DFS_BASEPATH + DFSPath);
                    viewModel.ret = 0;
                }
                catch (System.Exception ex)
                {
                    viewModel.ret = -1;
                    viewModel.msg = "save file failed,err info:" + ex.Message;
                }
                
            }
            else
            {
                viewModel.ret = -1;
                viewModel.msg = "there is no content in the file";
            }
            return Json(viewModel, JsonRequestBehavior.AllowGet);
        }

        /// <summary>
        /// 保存文件到DFS，把DFS路径记录到FileUncomplete表
        /// 返回保存成功
        /// </summary>
        /// <returns></returns>
        public JsonResult Modify()
        {
            throw new NotImplementedException();
        }
    }
}
