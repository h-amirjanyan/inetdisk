using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace NetDiskServer.Helpers
{
    public static class Helpers
    {
        public static string GetFilePath(this string path)
        {
            int pos = path.LastIndexOf('\\');
            return path.Substring(0, pos + 1);
        }

        public static string GetFileName(this string path)
        {
            int pos = path.LastIndexOf('\\');
            return path.Substring(pos + 1);
        }

        public static bool HasFile(this HttpPostedFileBase file)
        {
            return (file != null && file.ContentLength > 0) ? true : false;
        }
    }
}