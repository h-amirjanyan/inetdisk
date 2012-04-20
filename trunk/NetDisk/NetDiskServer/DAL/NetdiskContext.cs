using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Data.Entity;
using System.Data.Entity.ModelConfiguration.Conventions;

namespace NetDiskServer.Models
{
    public class NetdiskContext : DbContext
    {
        /// <summary>
        /// Gets or sets the files.
        /// </summary>
        /// <value>
        /// The files.
        /// </value>
        public DbSet<File> Files { get; set; }

        
        public DbSet<FileUncomplete> FileUncomplete { get; set; }

        /// <summary>
        /// Gets or sets the netdisk users.
        /// </summary>
        /// <value>
        /// The netdisk users.
        /// </value>
        public DbSet<NetDiskUser> NetdiskUsers { get; set; }

        protected override void OnModelCreating(DbModelBuilder modelBuilder)
        {
            //base.OnModelCreating(modelBuilder);
            //modelBuilder.Conventions.Remove<PluralizingTableNameConvention>();

            modelBuilder.Entity<File>()
                .HasOptional(p => p.Owner);

            modelBuilder.Entity<FileUncomplete>()
                .HasOptional(p => p.Owner);
        }
    }
}