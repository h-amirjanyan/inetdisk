using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using NetDiskServer.Models;

namespace NetDiskServer.DAL
{
    public class FileRepository : GenericRepository<File>
    {
        public FileRepository(NetdiskContext context)
            : base(context)
        {
        }

        //文件操作

        /// <summary>
        /// Gets the update list.
        /// </summary>
        /// <param name="path">The path.</param>
        /// <param name="user">The user.</param>
        /// <param name="lastSuccessUpdateTimeStamp">The last success update time stamp.</param>
        /// <returns></returns>
        public IEnumerable<File> GetUpdateList(string path, NetDiskUser user, int lastSuccessUpdateTimeStamp)
        {
            throw  new  NotImplementedException();
        }

        /// <summary>
        /// Creates the file.
        /// </summary>
        /// <param name="filename">The filename.</param>
        /// <param name="path">The path.</param>
        /// <param name="user">The user.</param>
        /// <param name="isFolder">if folder or not.</param>
        /// <returns></returns>
        public bool CreateFile(string filename, string path, NetDiskUser user,bool isFolder)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Deletes the file.
        /// 删除操作是在最新版本的文件上打上删除标记
        /// </summary>
        /// <param name="fildId">The fild id.</param>
        /// <param name="user">The user.</param>
        /// <returns></returns>
        public bool DeleteFile(int fildId, NetDiskUser user)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Deletes the file.
        /// </summary>
        /// <param name="path">The path.</param>
        /// <param name="filename">The filename.</param>
        /// <param name="user">The user.</param>
        /// <returns></returns>
        public bool DeleteFile(string path, string filename, NetDiskUser user)
        {
            bool ret = false;
            throw new NotImplementedException();
            return ret;
        }

        /// <summary>
        /// Copies the file.
        /// 如果reversion为0，默认拷贝最新版本
        /// </summary>
        /// <param name="srcPath">The SRC path.</param>
        /// <param name="targetPath">The target path.</param>
        /// <param name="srcFilename">The SRC filename.</param>
        /// <param name="targetFilename">The target filename.</param>
        /// <param name="overwrite">if set to <c>true</c> [overwrite].</param>
        /// <param name="reversion">The reversion.</param>
        /// <returns></returns>
        public bool CopyFile(string srcPath, string targetPath, string srcFilename, string targetFilename,bool overwrite,int reversion = 0)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Copies the file.
        /// </summary>
        /// <param name="fileId">The file id.</param>
        /// <param name="targetPath">The target path.</param>
        /// <param name="targetFilename">The target filename.</param>
        /// <param name="overwrite">if set to <c>true</c> [overwrite].</param>
        /// <returns></returns>
        public bool CopyFile(int fileId, string targetPath, string targetFilename, bool overwrite)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Moves the file.
        /// </summary>
        /// <param name="srcPath">The SRC path.</param>
        /// <param name="targetPath">The target path.</param>
        /// <param name="filename">The filename.</param>
        /// <param name="overwrite">if set to <c>true</c> [overwrite].</param>
        /// <param name="reversion">The reversion.</param>
        /// <returns></returns>
        public bool MoveFile(string srcPath, string targetPath, string filename,NetDiskUser user, bool overwrite,int reversion = 0)
        {
            throw new NotImplementedException();
        }

        public bool MoveFile(int fileId, string targetPath, bool overwrite,NetDiskUser user)
        {
            throw new NotImplementedException();
        }


        /// <summary>
        /// Renames the specified SRC path.
        /// 默认操作是在当前的路径中删除，在新位置创建，或覆盖
        /// </summary>
        /// <param name="srcPath">The SRC path.</param>
        /// <param name="srcFilename">The SRC filename.</param>
        /// <param name="targetFilename">The target filename.</param>
        /// <param name="overwrite">if set to <c>true</c> [overwrite].</param>
        /// <param name="reversion">The reversion.</param>
        /// <returns></returns>
        public bool Rename(string srcPath, string srcFilename, string targetFilename,NetDiskUser user, bool overwrite, int reversion = 0)
        {
            throw new NotImplementedException();
        }


        public IEnumerable<File> GetFiles(string path, NetDiskUser user)
        {
            throw new NotImplementedException();
        }
        
            
    }
}