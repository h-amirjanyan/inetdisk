using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;

namespace NetDiskServer.ViewModels
{
    public class ModifyPrepareViewModel : SyncBaseViewModel
    {
        public bool IsOutOfData { get; set; }

        public bool IsDeleted { get; set; } //该版本是不是已经被删除

        public int Id { get; set; } //fileUnComplete临时id
        public string DFSPath { get; set; } //DFS路径
    }
    
    public class ModifyCompleteViewModel : SyncBaseViewModel
    {
        public bool IsDeleted { get; set; } //该版本是不是已经被删除

        public int Id { get; set; }

        public int Reversion { get; set; }

        public bool Conflicted { get; set; }

        public string NewFilename { get; set; }
    }
}