using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;

namespace NetDiskServer.DAL
{
    public class ConsumerRepository : GenericRepository<Consumer>
    {
        public ConsumerRepository(NetdiskContext context)
            : base(context)
        {

        }
    }
}