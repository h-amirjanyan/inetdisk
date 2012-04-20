using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;

namespace NetDiskServer.DAL
{
    public class FileUncompleteRepository : GenericRepository<FileUncomplete>
    {
        public FileUncompleteRepository(NetDiskServer.Models.NetdiskContext context)
            : base(context)
        {
        }
    }
}