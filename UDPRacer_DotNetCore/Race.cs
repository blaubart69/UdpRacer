using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace UDPRacer
{
    class Race
    {
        /// <summary>
        /// typedef struct _DATA {
        ///     DWORD Id;
        ///     DWORD Hops;
        ///     DWORD Next;
        ///     DWORD Last;
        ///     SOCKADDR_IN NextAddrIn[NUMBER_OF_MAX_CHAIN];
        /// }
        /// DATA;
        /// </summary>
        /// <param name="port"></param>
        public static async void Start(int port)
        {
            UdpClient udpSock = new UdpClient(port);
            IPEndPoint nextNode = new IPEndPoint(0, port);

            while (true)
            {
                UdpReceiveResult data = await udpSock.ReceiveAsync().ConfigureAwait(false);
                Interlocked.Increment(ref Program._GLOBAL_packages);
                long nextIP;

                unsafe
                {
                    fixed (byte* bptr = &(data.Buffer[0]))
                    {
                        UInt32* hops = (UInt32*)(bptr +  4);
                        UInt32* next = (UInt32*)(bptr +  8);
                        UInt32* last = (UInt32*)(bptr + 12);

                        ++(*hops);
                        *next = (*next + 1) % *last;

                        UInt32* ips = (UInt32*)(bptr + 16);
                        nextIP = ips[*next];
                    }
                }
                
                nextNode.Address = new IPAddress(nextIP);
                udpSock.SendAsync(data.Buffer, data.Buffer.Length, nextNode).ConfigureAwait(false);
            }
        }

        public static byte[] CreateNetworkPackage(IReadOnlyList<IPAddress> IPs)
        {
            byte[] packet;

            using (MemoryStream ms = new MemoryStream())
            {
                BinaryWriter bw = new BinaryWriter(ms);

                bw.Write((UInt32)0x6619FEAF);      // id
                bw.Write((UInt32)0);               // hops
                bw.Write((UInt32)0);               // next
                bw.Write((UInt32)IPs.Count);       // number of Hosts

                foreach (IPAddress ip in IPs)
                {
                    bw.Write(ip.GetAddressBytes());
                }

                packet = ms.ToArray();
            }

            return packet;
        }

    }
}
