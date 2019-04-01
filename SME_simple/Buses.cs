using System;
using SME;

namespace MD5
{

    [InitializedBus]
    public interface Hashes : IBus
    {
        bool valid { get; set; }

        uint h0 { get; set; }
        uint h1 { get; set; }
        uint h2 { get; set; }
        uint h3 { get; set; }
    }

    //[InitializedBus]
    public interface Block : IBus
    {
        [InitialValue(false)]
        bool valid { get; set; }
        [FixedArrayLength(16)]
        IFixedArray<uint> w { get; set; }
    }

}