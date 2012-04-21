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

        public bool IsExitsRemote { get; set; }

        /// <summary>
        /// Gets or sets the remote id.
        /// </summary>
        /// <value>
        /// The remote id.
        /// 如果服务端已经存在，返回最新版版本的Id,版本,hash等信息，方便判断是否更新本地文件
        /// </value>
        public int RemoteId { get; set; }
        public int RemoteReversion { get; set; }
        public string RemoteHash { get; set; }
    }

    public class uploadCompleteViewModel : SyncViewModels
    {
        public int Id { get; set; }

        public int Reversion { get; set; }

        public bool Conflicted { get; set; }

        public string NewFilename { get; set; }
    }
}