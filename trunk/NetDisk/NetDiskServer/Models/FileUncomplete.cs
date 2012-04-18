using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace NetDiskServer.Models
{
    /// <summary>
    /// 暂存的文件上传信息,待上传完整之后才会更新到File表
    /// </summary>
    public class FileUncomplete : File
    {
        /// <summary>
        /// Gets or sets a value indicating whether this instance is completed.
        /// 是否上传完整
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance is completed; otherwise, <c>false</c>.
        /// </value>
        public bool IsCompleted { get; set; }
    }
}