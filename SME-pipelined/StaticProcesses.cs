using System;
using System.Threading.Tasks;
using SME;
using System.Text;

namespace MD5Pipelined
{
    [InitializedBus, TopLevelInputBus]
    public interface Target : IBus
    {
        [InitialValue(1)] uint h0 { get; set; } // Init so they dont match
        uint h1 { get; set; }
        uint h2 { get; set; }
        uint h3 { get; set; }
    }

    [InitializedBus, TopLevelOutputBus]
    public interface CollectedResult : IBus
    {
        bool valid { get; set; }

        uint w0 { get; set; }
        uint w1 { get; set; }
    }

    public class Tester : SimulationProcess
    {
        [OutputBus]
        Target target;

        [InputBus]
        CollectedResult verified;

        System.Security.Cryptography.MD5 md5 = System.Security.Cryptography.MD5.Create();

        public async override Task Run()
        {
            byte[] hash = md5.ComputeHash(Encoding.ASCII.GetBytes("C       "));
            uint[] hs = new uint[4];
            for (int i = 0; i < 4; i++)
            {
                hs[i] = (uint)(
                    hash[i * 4] |
                    hash[i * 4 + 1] << 8 |
                    hash[i * 4 + 2] << 16 |
                    hash[i * 4 + 3] << 24
                    );
            }
            target.h0 = hs[0];
            target.h1 = hs[1];
            target.h2 = hs[2];
            target.h3 = hs[3];
            Console.WriteLine("{0:x8} {1:x8} {2:x8} {3:x8}", hs[0], hs[1], hs[2], hs[3]);
            await ClockAsync();
            while (!verified.valid)
                await ClockAsync();
            Console.WriteLine("!--- {0:x8} {1:x8} ---!", verified.w0, verified.w1);
        }
    }
}
