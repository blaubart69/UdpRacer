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
        public static byte[] CreateNetworkPackage(IList<IPAddress> IPs, UInt16 port)
        {
            byte[] packet;

            using (MemoryStream ms = new MemoryStream())
            {
                BinaryWriter bw = new BinaryWriter(ms);

                bw.Write((UInt32) 0x6619FEAF);      // id
                bw.Write((UInt32) 0);               // hops
                bw.Write((UInt32) 0);               // next
                bw.Write((UInt32) IPs.Count);       // last

                /// typedef struct sockaddr_in {
                ///         short          sin_family;
                ///         USHORT sin_port;
                ///         IN_ADDR sin_addr;
                ///         CHAR sin_zero[8];
                ///     }
                ///     SOCKADDR_IN, * PSOCKADDR_IN;

                byte[] zero = new byte[8];

                foreach (IPAddress ip in IPs )
                {
                    bw.Write((UInt16) 0    ); // family
                    bw.Write((UInt16) port ); // family
                    bw.Write( ip.GetAddressBytes() );
                    bw.Write(zero);
                }

                packet = ms.ToArray();
            }

            return packet;
        }
        public static void PrintUdpRacerPackage(byte[] buffer)
        {
            unsafe
            {
                fixed (byte* bptr = &(buffer[0]))
                {
                    UInt32* id   = (UInt32*)(bptr +  0);
                    UInt32* hops = (UInt32*)(bptr +  4);
                    UInt32* next = (UInt32*)(bptr +  8);
                    UInt32* last = (UInt32*)(bptr + 12);

                    Console.WriteLine(
                          $"id\t{*id:X2}\n"
                      + $"hops\t{*hops}\n"
                      + $"next\t{*next}\n"
                      + $"last\t{*last}");

                    byte* sockaddr_in  = bptr + 16;
                    for (int i = 0; i < *last; ++i)
                    {
                        UInt32* ip = (UInt32*)(sockaddr_in + 4);
                        IPAddress ipObj = new IPAddress(*ip);
                        Console.WriteLine($"IP[{i}]\t{ipObj}");

                        sockaddr_in += 16;

                    }
                }
            }
        }
    }
}
