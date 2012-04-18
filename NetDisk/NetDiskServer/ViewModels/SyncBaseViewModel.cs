using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.ViewModels
{
    public class SyncBaseViewModel
    {
        public int ret { get; set; }

        public string msg { get; set; }

        //每个基类都有自己的数据域
        //public object data { get; set; }
    }
}