using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    public class GetUpdateListModel : SyncBaseModel
    {
        /// <summary>
        /// Gets or sets the last sync file id.
        /// </summary>
        /// <value>
        /// The last sync file id.
        /// </value>
        public int lastSyncId { get; set; }

    }

    public class DownloadPrepareModel : SyncBaseModel
    {
        public int Id { get; set; }

        public int ReversionLocal { get; set; }

    }
}