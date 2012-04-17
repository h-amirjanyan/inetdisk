using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    public class Consumer
    {
        public string consumer_key { get; set; }

        public string consumer_secret { get; set; }

        public DateTime CreateTime { get; set; }

        public string DeveloperEmail { get; set; }
    }
}