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
        private DCacheContext dChacheContext = new DCacheContext();

        private FileRepository filesRepository;
        private UserRepository userRepository;
        private ConsumerRepository consumerRepository;
        private TokenRepository tokenRepository;
        private FileUncompleteRepository fileUncompleteRepository;

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

        public ConsumerRepository ConsumerRepository
        {
            get
            {
                if (this.consumerRepository == null)
                {
                    this.consumerRepository = new ConsumerRepository(context);
                }
                return consumerRepository;
            }
        }

        public TokenRepository TokenReposity
        {
            get
            {
                if (this.tokenRepository == null)
                {
                    this.tokenRepository = new TokenRepository(dChacheContext);
                }
                return tokenRepository;
            }
        }

        public FileUncompleteRepository FileUncompleteRepository
        {
            get
            {
                if (this.fileUncompleteRepository == null)
                {
                    this.fileUncompleteRepository = new FileUncompleteRepository(context);
                }
                return this.fileUncompleteRepository;
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
                    dChacheContext.Dispose();
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