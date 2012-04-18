using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Helpers
{
    public static class Extension
    {
        //refer:http://codeclimber.net.nz/archive/2007/07/10/convert-a-unix-timestamp-to-a-.net-datetime.aspx
        public static double ConvertToUnixTimestamp(this DateTime date)
        {
            DateTime utc = date.ToUniversalTime();
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
            TimeSpan diff = utc - origin;
            return Math.Floor(diff.TotalSeconds);
        }
        public static DateTime ConvertFromUnixTimestamp(this double timestamp)
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
            return origin.AddSeconds(timestamp).ToLocalTime();
        }
    }

 
    
//Demo://ConvertToUnixTimestamp(DateTime.UtcNow)

//static DateTime ConvertFromUnixTimestamp(double timestamp)
//{
//    DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
//    return origin.AddSeconds(timestamp);
//}


//static double ConvertToUnixTimestamp(DateTime date)
//{
//    DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
//    TimeSpan diff = date - origin;
//    return Math.Floor(diff.TotalSeconds);
//}
}