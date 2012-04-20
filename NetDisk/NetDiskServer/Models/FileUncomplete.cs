using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.ComponentModel.DataAnnotations;

namespace NetDiskServer.Models
{
    /// <summary>
    /// 暂存的文件上传信息,待上传完整之后才会更新到File表
    /// </summary>
    public class FileUncomplete
    {
        /// <summary>
        /// Gets or sets the id.
        /// </summary>
        /// <value>
        /// The id.
        /// </value>
        [Key]
        public int Id { get; set; }

        /// <summary>
        /// Gets or sets the name of the file.
        /// </summary>
        /// <value>
        /// The name of the file.
        /// </value>
        public string FileName { get; set; }

        /// <summary>
        /// Gets or sets the file path.
        /// </summary>
        /// <value>
        /// The file path.
        /// </value>
        public string FilePath { get; set; }

        /// <summary>
        /// Gets or sets the reversion.
        /// </summary>
        /// <value>
        /// The reversion.
        /// </value>
        public int Reversion { get; set; }



        public int FileSize { get; set; }


        /// <summary>
        /// Gets or sets the hash.
        /// 文件的hash编码采用md5或SHA1
        /// </summary>
        /// <value>
        /// The hash.
        /// </value>
        public string Hash { get; set; }


        /// <summary>
        /// Gets or sets the DFS path.
        /// 在DFS中的路径
        /// </summary>
        /// <value>
        /// The DFS path.
        /// 在DFS中的路径
        /// </value>
        public string DFSPath { get; set; }

        /// <summary>
        /// Gets or sets the owner.
        /// 外键支持
        /// </summary>
        /// <value>
        /// The owner.
        /// </value>
        public virtual NetDiskUser Owner { get; set; }
        /// <summary>
        /// Gets or sets a value indicating whether this instance is completed.
        /// 是否上传完整
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance is completed; otherwise, <c>false</c>.
        /// </value>
        public bool IsCompleted { get; set; }

        public string ClientName { get; set; }
    }
}