using System;
using SME;

namespace MD5
{
    class Program
    {
        static void Main(string[] args)
        {
            using (new Simulation())
            {
                var tester = new Tester(100);
                var worker = new WorkerPipelined(tester.block, tester.initial_hash);
                
                tester.computed = worker.output_hash;

                Simulation.Current.AddTopLevelInputs(worker.input_block, worker.input_hash);
                Simulation.Current.AddTopLevelOutputs(worker.output_hash);

                Simulation.Current
                    .BuildCSVFile()
                    .BuildVHDL()
                    .Run();
            }
        }
    }
}
