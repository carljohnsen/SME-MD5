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
                var tester = new Tester();
                var worker = new Worker();

                tester.computed = worker.output;
                worker.block = tester.block;
                worker.input_hash = tester.initial_hash;

                Simulation.Current.AddTopLevelInputs(worker.block, worker.input_hash);
                Simulation.Current.AddTopLevelOutputs(worker.output);

                Simulation.Current
                    .BuildCSVFile()
                    .BuildVHDL()
                    .Run();
            }
        }
    }
}
