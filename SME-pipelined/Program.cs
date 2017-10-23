using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SME;

namespace MD5Pipelined
{
    class Program
    {
        static void Main(string[] args)
        {
            new Simulation()
                .BuildCSVFile()
                .BuildVHDL()
                .Run(typeof(Program).Assembly);
        }
    }
}
