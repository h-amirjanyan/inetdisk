using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;
using NetDiskServer.ViewModels;

namespace NetDiskServer.ViewModels
{
    public class GetUpdateListViewModel : SyncBaseViewModel
    {
        public List<FileLiteModel> Files { get; set; }
    }

    public class FileLiteModel
    {
        public int Id { get; set; }
        public string FullPath { get; set; } // as: \path\demo.doc
        public string Hash { get; set; }
    }

    public class DownloadPrepareViewModel : SyncBaseViewModel
    {
        public bool IsExits { get; set; }

        public bool IsDeleted { get; set; }

        public int LastId { get; set; }

        public int LastReversion { get; set; }

        public string LastDFSPath { get; set; }

        public string LastHash { get; set; }
    }
}