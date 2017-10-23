using SME;

namespace MD5
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
