using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Data;
using System.Data.Entity;
using NetDiskServer.Models;

namespace NetDiskServer.DAL
{
    public class UserRepository : GenericRepository<NetDiskUser>
    {
        public UserRepository(NetdiskContext context)
            : base(context)
        {

        }

        public bool Register(string username,string email, string password)
        {
            NetDiskUser newuser = new NetDiskUser();
            newuser.Email = email;
            newuser.UserName = username;
            newuser.RegisterDate = DateTime.Now;

            if (context.NetdiskUsers.Where(u => u.Email == email).Count() > 0)
                return false;

            context.NetdiskUsers.Add(newuser);

            return true;

        }
    }
}