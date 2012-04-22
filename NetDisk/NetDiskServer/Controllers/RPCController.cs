using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using NetDiskServer.Models;
using NetDiskServer.DAL;
using NetDiskServer.ViewModels;
using NetDiskServer.Helpers;

namespace NetDiskServer.Controllers
{
    public class RPCController : Controller
    {

        private UnitOfWork unitOfWork = new UnitOfWork();
        private NetdiskContext db = new NetdiskContext();
        #region Create

        /// <summary>
        /// Uploads prepare.
        /// 此时会把文件记录到临时表，DFS为空，reversion从file表中获取，该文件ID不代表在File中的ID
        /// </summary>
        /// <returns></returns>
        public JsonResult UploadPrepare(UploadPrepareModel model)
        {
            //FileName
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);
            db.NetdiskUsers.Attach(user);
            FileUncomplete fileUncomplete = new FileUncomplete
                {
                    FileName = model.FileName.GetFileName(),
                    FilePath = model.FileName.GetFilePath(),
                    FileSize = model.FileSize,
                    Hash = model.Hash,
                    IsCompleted = false,
                    Owner = user
                };
            UploadPrepareViewModel viewmodel = new UploadPrepareViewModel();

            //以该hash的的相同路径的最新版本作为当前版本
            File thisVersion = db.Files.Where(f => f.FileName == fileUncomplete.FileName && f.FilePath == fileUncomplete.FilePath && f.Hash == model.Hash).OrderByDescending(f => f.Id).FirstOrDefault();

            if (thisVersion != null)
                viewmodel.IsExitsRemote = true;
            else
                viewmodel.IsExitsRemote = false;

            if (viewmodel.IsExitsRemote)
            {
                viewmodel.RemoteId = thisVersion.Id;
                viewmodel.RemoteHash = thisVersion.Hash;
                viewmodel.RemoteReversion = thisVersion.Reversion;
            }
            else
            {
                try
                {

                    db.FileUncomplete.Add(fileUncomplete);
                    db.SaveChanges();
                    viewmodel.ret = 0;
                    viewmodel.Id = fileUncomplete.Id;
                    viewmodel.hash = fileUncomplete.Hash;
                    viewmodel.DFSPath = "\\" + fileUncomplete.Hash;
                }
                catch (System.Exception ex)
                {
                    viewmodel.ret = -1;
                    viewmodel.msg = "save file to FileUncomplete failed!err info:" + ex.Message;
                }
            }

            return Json(viewmodel, JsonRequestBehavior.AllowGet);
        }

        /// <summary>
        /// Uploads complete.
        /// 把临时记录删除，并记录到File表
        /// 如果有冲突，即该版本号已经存在，则返回冲突消息
        /// 同时发布更新消息（更新消息发布到以该用户UserId为channelId的channel上），所有以该帐号登录的客户端都要订阅该Channel
        /// 客户端收到该请求的返回之后应该更新本地的文件ID与版本号
        /// </summary>
        /// <returns></returns>
        public JsonResult UploadComplete(UploadCompleteModel model)
        {
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);
            db.NetdiskUsers.Attach(user);
            FileUncomplete fileUncomplete = db.FileUncomplete.SingleOrDefault(f => f.Id == model.Id);
            uploadCompleteViewModel viewModel = new uploadCompleteViewModel();

            if (fileUncomplete == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "unComplete recorde is miss";
            }

            File file = new File
            {
                CreateTime = DateTime.Now,
                DFSPath = '\\' + fileUncomplete.Hash,
                Hash = fileUncomplete.Hash,
                FileName = fileUncomplete.FileName,
                FilePath = fileUncomplete.FilePath,
                FileSize = fileUncomplete.FileSize,
                FileType = 1,
                IsDeleted = false,
                LastModifyTime = DateTime.Now,
                Owner = user,
                Reversion = 0
            };

            if (db.Files.Count(a => a.FilePath == file.FilePath && a.FileName == file.FileName && !a.IsDeleted) > 0)
            {
                //已经有文件，返回冲突消息
                file.FileName = file.FileName + "_conflict_copy";
                viewModel.Conflicted = true;
                viewModel.NewFilename = file.FilePath + file.FileName;
            }
            else
            {
                viewModel.Conflicted = false;
                try
                {
                    File last = db.Files.Where(f=>f.FilePath == file.FilePath && f.FileName == file.FileName).OrderByDescending(f => f.Reversion).FirstOrDefault();
                    if (last != null)
                    {
                        file.Reversion = last.Reversion + 1;
                    }
                    else
                    {
                        file.Reversion = 0;
                    }

                    db.Files.Add(file);
                    db.SaveChanges();
                    viewModel.Id = file.Id;
                    viewModel.ret = 0;
                    viewModel.NewFilename = file.FilePath + file.FileName;
                    viewModel.Reversion = file.Reversion;

                    db.FileUncomplete.Remove(fileUncomplete);
                    db.SaveChanges();
                }
                catch (System.Exception ex)
                {
                    viewModel.ret = -1;
                    viewModel.msg = "save db failed,err info:"+ ex.Message;
                }
            }
            return Json(viewModel, JsonRequestBehavior.AllowGet);
        }
        #endregion

        #region modify
        /// <summary>
        /// 记录修改数据到fileuncomplete表
        /// 返回文件id等，客户端收到之后应该完整上传
        /// </summary>
        /// <returns></returns>
        public JsonResult ModifyPrepare(ModifyPrepareModel model)
        {
            //根据客户端提供的路径查找最新版，看版本Id是否过期，如果过期则返回过期，客户端提示版本冲突，创建新文件重新提交
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);
            ModifyPrepareViewModel viewModel = new ModifyPrepareViewModel();
            if (user == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "cannot find the user in cache, relogin may help";
                return Json(viewModel, JsonRequestBehavior.AllowGet);
            }
           

            db.NetdiskUsers.Attach(user);
            FileUncomplete fileUncomplete = new FileUncomplete
                {
                    FileName = model.LocalPath.GetFileName(),
                    FilePath = model.LocalPath.GetFilePath(),
                    FileSize = model.NewFileSize,
                    Hash = model.NewHash,
                    IsCompleted = false,
                    Owner = user
                };
            
            
            File lastversion = db.Files.Where(f => f.FileName == fileUncomplete.FileName && f.FilePath == fileUncomplete.FilePath).OrderByDescending(f => f.Id).FirstOrDefault();
            if (lastversion == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "can not find the file specified";
                return Json(viewModel, JsonRequestBehavior.AllowGet);
            }
            else
            {
                if (model.LocalId < lastversion.Id) //过期
                {
                    viewModel.IsOutOfData = true;
                }
                else if (model.LocalId == lastversion.Id && lastversion.IsDeleted) //过期，最后一个版本已经被删除
                {
                    viewModel.IsOutOfData = true;
                    viewModel.IsDeleted = true;
                }
                else
                {
                    viewModel.IsOutOfData = false;
                    viewModel.IsDeleted = false;
                    try
                    {
                        fileUncomplete.Reversion = lastversion.Reversion + 1;
                        db.FileUncomplete.Add(fileUncomplete);
                        db.SaveChanges();
                        viewModel.ret = 0;
                        viewModel.Id = fileUncomplete.Id;
                        viewModel.DFSPath = "\\" + fileUncomplete.Hash;
                    }
                    catch (System.Exception ex)
                    {
                        viewModel.ret = -1;
                        viewModel.msg = "save file to FileUncomplete failed!err info:" + ex.Message;
                    }
                }
            }


            return Json(viewModel, JsonRequestBehavior.AllowGet);
         }

        /// <summary>
        /// 接收客户端上传成功的请求，把文件修改从fileuncomplete移动到file表
        /// 同时检测冲突，并发布文件更新通知
        /// 
        /// 客户端收到之后更新本地数据
        /// </summary>
        /// <returns></returns>
        public JsonResult ModifyComplete(ModifyCompleteModel model)
        {

            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);
            db.NetdiskUsers.Attach(user);
            FileUncomplete fileUncomplete = db.FileUncomplete.SingleOrDefault(f => f.Id == model.Id);
            uploadCompleteViewModel viewModel = new uploadCompleteViewModel();
            if (fileUncomplete == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "unComplete recorde is miss";
            }

            File file = new File
            {
                CreateTime = DateTime.Now,
                DFSPath = fileUncomplete.Hash,
                Hash = fileUncomplete.Hash,
                FileName = fileUncomplete.FileName,
                FilePath = fileUncomplete.FilePath,
                FileSize = fileUncomplete.FileSize,
                FileType = 1,
                IsDeleted = false,
                LastModifyTime = DateTime.Now,
                Owner = user,
                Reversion = fileUncomplete.Reversion
            };

            File lastversion =  db.Files.Where(f => f.FileName == fileUncomplete.FileName && f.FilePath == fileUncomplete.FilePath).OrderByDescending(f => f.Id).FirstOrDefault();
            if(lastversion.IsDeleted || lastversion.Reversion >= file.Reversion)
            {
                //该版本已经被删除或者有更新版本
                file.FileName = file.FileName + "_conflict_copy_MC";
                viewModel.Conflicted = true;
                viewModel.NewFilename = file.FileName;
            }
            else
            {
                viewModel.Conflicted = false;
                try
                {
                   
                    db.Files.Add(file);
                    db.SaveChanges();
                    viewModel.Id = file.Id;
                    viewModel.ret = 0;
                    viewModel.NewFilename = file.FileName;
                    viewModel.Reversion = file.Reversion;

                    db.FileUncomplete.Remove(fileUncomplete);
                    db.SaveChanges();
                }
                catch (System.Exception ex)
                {
                    viewModel.ret = -1;
                    viewModel.msg = "save db failed,err info:" + ex.Message;
                }
            }
            return Json(viewModel, JsonRequestBehavior.AllowGet);
        }
        #endregion


        #region download
        //（1）客户端将上次同步版本号发送给RPC
        //（2）RPC查询出数据库中该用户需要同步的目录下所有版本大于客户端上次同步版本号的文件与文件夹。
        //（3）将查询出的文件与文件夹按照版本号升序排列，如果文件夹和文件版本号相同，文件夹要在文件之前。如果文件和文件或者文件夹和文件夹版本相同按照ID大小排序。
        //（4）将这些数据转化成Json格式返回给客户端。
        /// <summary>
        /// 获取更新列表
        /// </summary>
        /// <returns></returns>
        public JsonResult GetUpdateList(GetUpdateListModel model)
        {
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);

            GetUpdateListViewModel viewModel = new GetUpdateListViewModel();
            if (user == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "cannot find the user specified!";
            }
            else
            {
                List<File> fileList = db.Files.Where(f => f.Id >= model.lastSyncId && f.Owner.UserId == user.UserId).OrderBy(f => f.Id).ToList();
                viewModel.Files = new List<FileLiteModel>();
                foreach (var item in fileList)
                {
                    FileLiteModel lite = new FileLiteModel
                    {
                        Hash = item.Hash,
                        Id = item.Id,
                        FullPath = item.FilePath + item.FileName
                    };
                    viewModel.Files.Add(lite);
                }
                viewModel.ret = 0;
            }
            return Json(viewModel, JsonRequestBehavior.AllowGet);
        }

        /// <summary>
        /// Downloads  prepare.
        /// 客户端提交需要更新的文件Id,以及本地版本号
        /// 服务端判断是否需要更新、删除
        /// 更新则直接找到最新版本，通知客户端完全下载
        /// </summary>
        /// <returns></returns>
        public JsonResult DownloadPrepare(DownloadPrepareModel model)
        {
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);
            DownloadPrepareViewModel viewModel = new DownloadPrepareViewModel();
            if (user == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "cannot find the user specified!";
            }
            else
            {
                viewModel.ret = 0;
                File file = db.Files.SingleOrDefault(f=>f.Id == model.Id);

                if(file == null)
                {
                    viewModel.IsExits = false;
                }
                else
                {
                    File lastVersion = db.Files.Where(f => f.FilePath == file.FilePath && f.FileName == file.FileName).OrderByDescending(f => f.Id).FirstOrDefault();
                    viewModel.IsExits = true;
                    viewModel.IsDeleted = lastVersion.IsDeleted;
                    viewModel.LastId = lastVersion.Id;
                    viewModel.LastDFSPath = lastVersion.DFSPath;
                    viewModel.LastReversion = lastVersion.Reversion;
                    viewModel.LastHash = lastVersion.Hash;
                }
            }

            return Json(viewModel, JsonRequestBehavior.AllowGet);
        }

        ///// <summary>
        ///// Downloads  complete.
        ///// 客户端通知服务端下载完毕，此时客户端应该已经更新了本地的版本号等信息
        ///// </summary>
        ///// <returns></returns>
        //public JsonResult DownloadComplete()
        //{
        //    throw new NotImplementedException();
        //}
        #endregion


        #region RPC操作


        //（1）Client向RPC发送新增文件夹的名称和父目录Id
        //（2）RPC接收到参数后检查当前父目录下是否有同名文件夹，如果有则进入（3），如果没有就跳到步骤（4）
        //（3）获取数据库中同名目录的ID，如果文件夹最新版本为删除版本，则新增版本。如果为普通版本，则不变。
        //（4）新增文件夹ID，在数据库中添加文件夹，新增版本号。
        //（5）检查各级父目录是否有删除标志，如果有删除标志需要重置该标志。
        public JsonResult AddNewFolder()
        {
            throw new NotImplementedException();
        }

        //（1）删除文件类似于一个特殊的文件修改操作。Client只向RPC服务器发送一次请求，包括要删除的文件ID。 
        //（2）RPC接收该请求后，先检查数据块，如果文件不存在或已删除，则直接取消操作。
        //（3）如果文件存在且未删除且和本地版本相同，新增删除版本号,并通知其他Client同步删除该文件。
        //（4）删除文件要求只能删除版本小于本地文件的版本，也就是说如果服务器文件版本大于本地文件版本，删除文件操作将取消。
        public JsonResult DeleteFile(RPCDeleteModel model)
        {
            OauthTokenPair pair = unitOfWork.TokenReposity.GetTokenPair(model.oauth_token);
            NetDiskUser user = unitOfWork.UserRepositiry.dbSet.SingleOrDefault(u => u.UserId == pair.UserId);
            RPCDeleteViewModel viewModel = new RPCDeleteViewModel();
            if (user == null)
            {
                viewModel.ret = -1;
                viewModel.msg = "cannot find the user specified!";
            }
            else
            {
                File fileSpecified = db.Files.Where(f => f.Id == model.Id).SingleOrDefault();
                if (fileSpecified == null)
                {
                    viewModel.ret = 0;
                    viewModel.msg = "指定版本不存在，直接删除";
                    viewModel.IsConficted = true; //让本地重命名
                }
                else
                {
                    File last = db.Files.Where(f => f.FilePath == fileSpecified.FilePath && f.FileName == fileSpecified.FileName).OrderByDescending(f => f.Id).FirstOrDefault();
                    if (fileSpecified.Reversion == last.Reversion && !last.IsDeleted)
                    {
                        last.IsDeleted = true;
                        db.SaveChanges();
                        viewModel.IsConficted = false;
                    }
                    else
                    {
                        viewModel.IsConficted = true;
                    }
                }
            }
            return Json(viewModel, JsonRequestBehavior.AllowGet);
        }

        //（1）删除文件夹要求不能删除文件夹中未同步到本地的文件或文件夹。
        //（2）Client向RPC发送删除文件夹请求包含（a）文件夹的ID、（b）本地最后更新版本号、（c）客户端上文件夹下大于最后更新版本号的文件和文件夹以及他们的版本号。
        //     文件夹ID：根据ID删除文件夹 
        //     本地最后更新版本号：表示客户端上次同步的时间戳，服务器上版本号大于该最后更新版本号的文件和文件夹是未被同步到本地的服务器文件，添加到删除忽略列表。
        //     客户端文件夹下大于最后更新版本号的文件和文件夹以及他们的版本号：表示本地修改已同步到服务器上，但是提交修改之后服务器还没有经过版本更新，因此这些文件和文件夹的版本是大于最后更新版本号的，他们可能是本地和服务器一直的文件但由于比最后更新版本号大，会被添加到删除忽略列表中，对应这些文件需要比对客户端版本号与服务器版本号，如果他们的版本号等于服务器上该文件和文件夹的最新版本号就要被剔除出删除忽略列表，如果小于则继续保留。
        //（3）将删除忽略列表中的文件和文件夹的父目录也加入删除忽略列表。
        //（4）将删除忽略列表以外的其他的文件加上删除版本号。
        //（5）将删除忽略列表以外的其他的文件夹加上删除标志，并且更新版本号。

        public JsonResult DeleteFolder()
        {
            throw new NotImplementedException();
        }

        // （1）客户端先检查拷贝到的目录ID是否存在，如果不存在就等待目录ID生成后才执行后续操作。
        //（2）客户端再检查copy后的目录是否有重名文件：
        //	如果不存在同名文件则作为执行新文件上传流程（3.1），以原文件的MD5作为新文件上传参数中的MD5传给RPC。
        //	如果存在存在重名文件则分以下情况处理：
        //            (a)Web端：对copy后文件（A.txt）重命名，改成A(1).txt,如果A(1).txt,也已经存在就改成A(2).txt，依次类推。生成新的文件ID，生成版本号，docID与被copy文件当前版本的docID一致。
        //            (b)客户端：执行重名文件的修改流程（3.2），以原文件的MD5作为文件修改参数中的MD5传给RPC。
        //(4)服务器对拷贝操作和文件修改操作的接口都是同一个修改文件接口，只是客户端操作上略有区别。
        public JsonResult CopyFile()
        {
            throw new NotImplementedException();
        }

        //（1）文件重命名操作时move操作的一种，采用相同的API接口。
        //   （2）客户端检查目标目录ID是否存在，如果不存在就等待目录ID生成后才执行后续操作。
        //（3）客户端再检查move后的目录是否有重名文件：
        //	如果不存在同名文件，则执行文件修改流程（3.2），并将原文件的MD5作为修改后文件的MD5提交，避免重复计算MD5。
        //	如果存在同名文件，先对同名文件执行删除操作（3.4.3），再执行文件修改操作（3.2），并将原文件的MD5作为修改后文件的MD5提交，避免重复计算MD5。
        //(4)服务器对move操作和文件修改操作的接口都是同一个修改文件接口，只是客户端操作上略有区别。
        public JsonResult MoveFile()
        {
            throw new NotImplementedException();
        }


        //拷贝文件夹
        //（1）拷贝文件夹就是就是依次对拷贝后新生成的文件和文件夹的上传过程，需要分文件和文件夹两部分处理。
        //（2）首先客户端对文件夹以及所有拷贝的子文件夹（如果有同名按了取消的不计入在内）按照执行新建文件夹操作（3.4.2），执行顺序与客户端拷贝执行的顺序一致。
        //（3）接下来按照客户端文件更新时间来更新所有文件，文件拷贝流程按照（3.4.5）规则执行。
        public JsonResult CopyFolder()
        {
            throw new NotImplementedException();
        }

        //文件夹Move、重命名操作：
        //（1）文件夹move与重命名是同一种操作。
        //（2）向RPC服务器发送要move的文件夹ID，move到的目录ID，move后的文件夹名称。
        //（3）检查move到的父目录ID，是否存在，如果不存在就取消move操作
        //（5）检查是否有重名目录：
        //        （a）有重名目录，对原有目录执行删除操作，然后对目录下所有一级子目录（同3.4.8递归）与子文件（同3.4.6）执行move操作。
        //        （b）没有重名目录，则直接修改move目录的父目录ID或目录名称，修改版本号，最后对目录下所有子文件和子目录发起修改操作，修改它们的Path。
        public JsonResult MoveFolder()
        {
            throw new NotImplementedException();
        }
        #endregion

        protected override void Dispose(bool disposing)
        {
            unitOfWork.Dispose();
            db.Dispose();
            base.Dispose(disposing);
        }
    }
}
