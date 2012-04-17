using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;
using Enyim.Caching.Memcached;

namespace NetDiskServer.DAL
{
    public class TokenRepository : IDisposable
    {
        private DCacheContext context;
        public TokenRepository(DCacheContext context)
        {
            this.context = context;
        }

        public bool Insert(OauthTokenPair tokenPair)
        {
            return this.context.Store(StoreMode.Add, tokenPair.oauth_token, tokenPair);
        }

        public bool Delete(OauthTokenPair tokenPair)
        {
            return this.context.Remove(tokenPair.oauth_token);
        }

        private bool disposed = false;
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    context.Dispose();
                }
            }
            this.disposed = true;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
    }
}