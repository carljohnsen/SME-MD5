using System;
using System.Linq;
using SME;

namespace MD5 
{
    
    public class Tester : SimulationProcess
	{
		[InputBus]
		public Hashes computed;

		[OutputBus]
		public Block block = Scope.CreateBus<Block>();
		[OutputBus]
		public Hashes initial_hash = Scope.CreateBus<Hashes>();

		public Tester(int rounds)
		{
			this.rounds = rounds;
		}

		uint[] build_block(uint[] input)
		{
			uint[] tmp = new uint[16];
			for (int i = 0; i < input.Length; i++)
				tmp[i] = input[i];
			tmp[input.Length] = 128;
			for (int i = input.Length+1; i < 15; i++)
				tmp[i] = 0;
			tmp[14] = (uint)(input.Length * 4 * 8);
			tmp[15] = 0;
			return tmp;
		}

		uint[] build_input(int size)
		{
			uint[] tmp = new uint[size];
			for (int i = 0; i < size; i++)
				tmp[i] = (uint)rnd.Next();
			return tmp;
		}

		byte[] from_uint(uint[] input)
		{
			byte[] tmp = new byte[input.Length << 2];
			for (int j = 0; j < input.Length; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					tmp[(j*4)+k] = (byte)((input[j] >> (k*8)) & 0xFF);
				}
			}
			return tmp;
		}

		uint h0 = 0x67452301;
        uint h1 = 0xefcdab89;
        uint h2 = 0x98badcfe;
        uint h3 = 0x10325476;
		Random rnd = new Random();
		int rounds;

		public async override System.Threading.Tasks.Task Run()
		{
			System.Security.Cryptography.MD5 hasher = System.Security.Cryptography.MD5.Create();
			await ClockAsync();

			for (int i = 0; i < rounds; i++)
			{
				uint[] tmp = build_input(rnd.Next(14));
				uint[] blk = build_block(tmp);
				for (int j = 0; j < blk.Length; j++)
					block.w[j] = blk[j];
				block.valid = true;
				initial_hash.valid = true;
				initial_hash.h0 = h0;
				initial_hash.h1 = h1;
				initial_hash.h2 = h2;
				initial_hash.h3 = h3;
				await ClockAsync();
				await ClockAsync();
				block.valid = false;
				while (!computed.valid)
				{
					await ClockAsync();
				}

				byte[] tmp_byte = from_uint(tmp);
				byte[] verified_output = hasher.ComputeHash(tmp_byte);
				byte[] computed_output = from_uint(new uint[]{computed.h0, computed.h1, computed.h2, computed.h3});
				if (!Enumerable.SequenceEqual(verified_output, computed_output))
				{
					Console.WriteLine("They don't match!");
					for (int j = 0; j < verified_output.Length; j++)
						Console.Write("{0:x2}", verified_output[j]);
					Console.WriteLine(" - verified");
					for (int j = 0; j < computed_output.Length; j++)
						Console.Write("{0:x2}", computed_output[j]);
					Console.WriteLine(" - computed");
				}
			}

			Console.WriteLine("Testing done");
		}
	}

}