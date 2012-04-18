using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace NetDiskServer.Controllers
{
    //Upload Server
    public class UploadController : Controller
    {
        /// <summary>
        /// 上传文件，导入到DFS，把DFS路径记录到FileUncomplete表
        /// </summary>
        /// <returns></returns>
        public JsonResult Upload()
        {
            throw new NotImplementedException();
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
