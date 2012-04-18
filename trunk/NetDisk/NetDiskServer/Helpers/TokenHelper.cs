using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.DAL;
using NetDiskServer.Models;

namespace NetDiskServer.Helpers
{
    public  class TokenHelper
    {
        private static UnitOfWork unitOfWork = new UnitOfWork();
        public static int CheckToken(string oauth_token)
        {
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(oauth_token);
            if (pair != null && oauth_token == pair.oauth_token)
                return pair.UserId;
            return -1;
        }

        //private bool disposed = false;
        //protected virtual void Dispose(bool disposing)
        //{
        //    if (!this.disposed)
        //    {
        //        if (disposing)
        //        {
        //            unitOfWork.Dispose();
        //        }
        //    }
        //    this.disposed = true;
        //}

        //public void Dispose()
        //{
        //    Dispose(true);
        //    GC.SuppressFinalize(this);
        //}
    }
}