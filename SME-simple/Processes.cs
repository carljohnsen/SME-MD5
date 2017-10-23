using System;
using System.Threading.Tasks;
using SME;

namespace MD5
{
	[InitializedBus, TopLevelOutputBus]
	public interface CollectedOutput : IBus
	{
		bool valid { get; set; }
		uint w0 { get; set; }
		uint w1 { get; set; }
	}

	public class Tester : SimulationProcess
	{
		[InputBus]
		CollectedOutput collected;

		public async override System.Threading.Tasks.Task Run()
		{
			await ClockAsync();
			while (!collected.valid) await ClockAsync();
			Console.WriteLine("!-- {0:x8} {1:x8} --!", collected.w0, collected.w1);
		}
	}
}
