using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;

namespace NetDiskServer.DAL
{
    public class UnitOfWork : IDisposable
    {
        private NetdiskContext context = new NetdiskContext();
        private FileRepository filesRepository;
        private UserRepository userRepository;

        public FileRepository FilesRepository
        {
            get
            {
                if (this.filesRepository == null)
                {
                    this.filesRepository = new FileRepository(context);
                }
                return filesRepository;
            }
        }

        public UserRepository UserRepositiry
        {
            get
            {
                if (this.userRepository == null)
                {
                    this.userRepository = new UserRepository(context);
                }
                return userRepository;
            }
        }

        public void Save()
        {
            context.SaveChanges();
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
                this.disposed = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
    }
}