using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    public class RPCDeleteModel : SyncBaseModel
    {
        public int Id { get; set; }

        public int LocalVersion { get; set; }
    }
}