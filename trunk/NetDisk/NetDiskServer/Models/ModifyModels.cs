using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    public class ModifyPrepareModel : SyncBaseModel
    {
        /// <summary>
        /// Gets or sets the local id.
        /// </summary>
        /// <value>
        /// The local id.
        /// </value>
        public int LocalId { get; set; }

        /// <summary>
        /// Gets or sets the local path.
        /// </summary>
        /// <value>
        /// The local path.
        /// </value>
        public string LocalPath { get; set; }

        /// <summary>
        /// Gets or sets the local version.
        /// </summary>
        /// <value>
        /// The local version.
        /// </value>
        public int LocalVersion { get; set; }

        /// <summary>
        /// Gets or sets the new hash.
        /// </summary>
        /// <value>
        /// The new hash.
        /// </value>
        public string NewHash { get; set; }


        public int NewFileSize { get; set; }
    }


    public class ModifyCompleteModel : SyncBaseModel
    {
        public int Id { get; set; }
    }
}