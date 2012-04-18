﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.ComponentModel.DataAnnotations;

namespace NetDiskServer.Models
{
    public class NameSpaceModels
    {
    }

    /// <summary>
    /// File Model
    /// </summary>
    public class File
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

        /// <summary>
        /// Gets or sets the create time.
        /// </summary>
        /// <value>
        /// The create time.
        /// </value>
        public DateTime CreateTime { get; set; }

        /// <summary>
        /// Gets or sets the last modify time.
        /// </summary>
        /// <value>
        /// The last modify time.
        /// </value>
        public DateTime LastModifyTime { get; set; }


        public int FileSize { get; set; }

        /// <summary>
        /// Gets or sets the type of the file.
        /// 1为文件，2为文件夹
        /// </summary>
        /// <value>
        /// The type of the file.
        /// </value>
        public int FileType { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this file is deleted.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance is deleted; otherwise, <c>false</c>.
        /// </value>
        public bool IsDeleted { get; set; }


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
    }
}