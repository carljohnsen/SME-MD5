﻿using System;
using System.Linq;
using SME;

namespace MD5
{

	[ClockedProcess]
	public class WorkerMulticycle : SimpleProcess
	{
		[InputBus]
        public Block block;
		[InputBus]
		public Hashes input_hash;

        [OutputBus]
        public Hashes output = Scope.CreateBus<Hashes>();
		
		public WorkerMulticycle()
		{
			r  = new uint[Constants.r.Length];
			kk = new uint[Constants.kk.Length];
			gs = new uint[Constants.gs.Length];

			Array.Copy(Constants.r,  r,  r.Length);
			Array.Copy(Constants.kk, kk, kk.Length);
			Array.Copy(Constants.gs, gs, gs.Length);
		}

		uint[] r;
		uint[] kk;
		uint[] gs;

		uint h0 = 0x67452301;
        uint h1 = 0xefcdab89;
        uint h2 = 0x98badcfe;
        uint h3 = 0x10325476;

		uint a;
		uint b;
		uint c;
		uint d;

		int i;

		bool running = false;

		protected override void OnTick()
		{
			if (!running)
			{
				if (block.valid)
				{
					running = true;
					i = 0;
					a = h0;
					b = h1;
					c = h2;
					d = h3;
					output.valid = false;
					output.h0 = 0;
					output.h1 = 0;
					output.h2 = 0;
					output.h3 = 0;
				}
			}
			else
			{
				uint f;
				if (i < 16)
					f = (b & c) | ((~b) & d);
				else if (i < 32)
					f = (d & b) | ((~d) & c);
				else if (i < 48)
					f = b ^ c ^ d;
				else
					f = c ^ (b | (~d));

				uint tmp = d;
				d = c;
				c = b;
				uint x = a + f + kk[i] + block.w[(int)gs[i]];
				int c2 = (int)r[i];
				b = b + (((x) << (c2)) | ((x) >> (32 - (c2))));
				a = tmp;

				if (i == 63)
				{
					output.valid = true;
					output.h0 = h0 + a;
					output.h1 = h1 + b;
					output.h2 = h2 + c;
					output.h3 = h3 + d;
					running = false;
				}
				i++;
			}
		}
	}

	[ClockedProcess]
    public class WorkerSinglecycle : SimpleProcess
    {
        [InputBus]
        public Block block;
		[InputBus]
		public Hashes input_hash;

        [OutputBus]
        public Hashes output = Scope.CreateBus<Hashes>();

		public WorkerSinglecycle()
        {
			r  = new uint[Constants.r.Length];
			kk = new uint[Constants.kk.Length];
			gs = new uint[Constants.gs.Length];

			Array.Copy(Constants.r,  r,  r.Length);
			Array.Copy(Constants.kk, kk, kk.Length);
			Array.Copy(Constants.gs, gs, gs.Length);
		}

		uint[] r;
		uint[] kk;
		uint[] gs;

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
					uint f;
					if (i < 16)
						f = (b & c) | ((~b) & d);
					else if (i < 32)
						f = (d & b) | ((~d) & c);
					else if (i < 48)
						f = b ^ c ^ d;
					else
						f = c ^ (b | (~d));

					uint tmp = d;
					d = c;
					c = b;
					uint x = a + f + kk[i] + block.w[(int)gs[i]];
					int c2 = (int)r[i];
					b = b + (((x) << (c2)) | ((x) >> (32 - (c2))));
					a = tmp;
				}

				output.valid = true;
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
