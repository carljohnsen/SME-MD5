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

		Random rnd = new Random();

		public async override System.Threading.Tasks.Task Run()
		{
			System.Security.Cryptography.MD5 hasher = System.Security.Cryptography.MD5.Create();
			await ClockAsync();

			for (int i = 0; i < 100; i++)
			{
				uint[] tmp = build_input(rnd.Next(14));
				uint[] blk = build_block(tmp);
				for (int j = 0; j < blk.Length; j++)
					block.w[j] = blk[j];
				block.valid = true;
				initial_hash.valid = false;
				await ClockAsync();
				await ClockAsync(); 

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

	[ClockedProcess]
    public class Worker : SimpleProcess
    {
        [InputBus]
        public Block block;
		[InputBus]
		public Hashes input_hash;

        [OutputBus]
        public Hashes output = Scope.CreateBus<Hashes>();

        readonly uint[] r = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

        readonly uint[] kk = {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
            0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
            0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
            0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
            0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
            0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
            0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

        uint h0, h1, h2, h3;

        protected override void OnTick()
        {
			if (block.valid)
			{
				if (input_hash.valid)
				{
					h0 = input_hash.h0;
					h1 = input_hash.h1;
					h2 = input_hash.h2;
					h3 = input_hash.h3;
				}
				else
				{
					h0 = 0x67452301;
					h1 = 0xefcdab89;
					h2 = 0x98badcfe;
					h3 = 0x10325476;
				}

				uint a = h0;
				uint b = h1;
				uint c = h2;
				uint d = h3;

				for (int i = 0; i < 64; i++)
				{
					uint f, g;
					if (i < 16)
					{
						f = (b & c) | ((~b) & d);
						g = (uint)i;
					}
					else if (i < 32)
					{
						f = (d & b) | ((~d) & c);
						g = (uint)(5 * i + 1) % 16;
					}
					else if (i < 48)
					{
						f = b ^ c ^ d;
						g = (uint)(3 * i + 5) % 16;
					}
					else
					{
						f = c ^ (b | (~d));
						g = (uint)(7 * i) % 16;
					}

					uint tmp = d;
					d = c;
					c = b;
					uint x = a + f + kk[i] + block.w[(int)g];
					int c2 = (int)r[i];
					b = b + (((x) << (c2)) | ((x) >> (32 - (c2))));
					a = tmp;
				}

				output.valid = false;
				output.h0 = h0 + a;
				output.h1 = h1 + b;
				output.h2 = h2 + c;
				output.h3 = h3 + d;
			}
			else
			{
				output.valid = false;
				output.h0 = 0;
				output.h1 = 0;
				output.h2 = 0;
				output.h3 = 0;
			}
        }
	}

}
