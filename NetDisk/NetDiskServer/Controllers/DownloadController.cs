using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace NetDiskServer.Controllers
{
    public class DownloadController : Controller
    {
        private static string DFS_BASEPATH = "C:\\DFSRoot";
        /// <summary>
        /// 根据客户端提供的DFS目录信息，直接返回该文件
        /// </summary>
        /// <returns></returns>
        public FileResult Download(string DFSPath)
        {
            if (DFSPath[0] != '\\')
                DFSPath = "\\" + DFSPath;
            return File(DFS_BASEPATH + DFSPath, "application/octet-stream");
        }
    }
}
