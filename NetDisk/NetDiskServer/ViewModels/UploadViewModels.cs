using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.ViewModels
{
    public class UploadPrepareViewModel : SyncViewModels
    {
        public int Id { get; set; }
        public string hash { get; set; }
        public string DFSPath { get; set; }
    }

    public class uploadCompleteViewModel : SyncViewModels
    {
        public int Id { get; set; }

        public int Reversion { get; set; }

        public bool Conflicted { get; set; }

        public string NewFilename { get; set; }
    }
}