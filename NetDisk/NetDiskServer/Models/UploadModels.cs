using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    public class UploadPrepareModel : SyncBaseModel
    {
        public string FilePath { get; set; }
        public string FileName {get;set;}
        public int FileSize { get; set; }
        public string Hash { get; set; }
    }

    public class UploadCompleteModel : SyncBaseModel
    {
        public int Id { get; set; }
    }
}