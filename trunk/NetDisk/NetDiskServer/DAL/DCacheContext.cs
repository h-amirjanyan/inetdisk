using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Couchbase;
using Enyim.Caching.Memcached;

namespace NetDiskServer.DAL
{
    public class DCacheContext : IDisposable
    {
        private CouchbaseClient client = new CouchbaseClient();

        public bool Store(StoreMode mode, string key, object objectValue)
        {
            return client.Store(mode, key, objectValue);
        }

        public T Get<T>(string key)
        {
            return client.Get<T>(key);
        }

        public bool Remove(string key)
        {
            return client.Remove(key);
        }

        private bool disposed = false;
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    client.Dispose();
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