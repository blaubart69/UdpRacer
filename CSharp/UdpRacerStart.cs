using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace CSharp
{
    class UdpRacerStart
    {
        public static byte[] CreateNetworkPackage(IList<IPAddress> IPs)
        {
            byte[] packet;

            using (MemoryStream ms = new MemoryStream())
            {
                BinaryWriter bw = new BinaryWriter(ms);

                bw.Write((UInt32) 0x6619FEAF);      // id
                bw.Write((UInt32) 0);               // hops
                bw.Write((UInt32) 0);               // next
                bw.Write((UInt32) IPs.Count);       // last

                foreach (IPAddress ip in IPs )
                {
                    bw.Write( ip.GetAddressBytes() );
                }

                packet = ms.ToArray();
            }

            return packet;
        }
    }
}
