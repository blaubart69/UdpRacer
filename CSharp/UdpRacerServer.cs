using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CSharp
{
    class UdpRacerServer
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
        public static async void Run(int port)
        {
            UdpClient udpSock = new UdpClient(port);

            IPEndPoint nextNode = new IPEndPoint(0, port);

            while (true)
            {
                UdpReceiveResult data = await udpSock.ReceiveAsync().ConfigureAwait(false);

                Interlocked.Increment(ref Program._packages);

                long nextIP;

                unsafe
                {
                    fixed (byte* bptr = &data.Buffer[0])
                    {
                        long* hops  = (long*)bptr[4];
                        long* next  = (long*)bptr[8];
                        long* last  = (long*)bptr[12];

                        ++(*hops);
                        *next = (*next + 1) % *last;

                        long* ips = (long*)(bptr + 16);
                        nextIP = ips[*next];
                    }
                }

                nextNode.Address = new IPAddress(nextIP);
                udpSock.SendAsync(data.Buffer, data.Buffer.Length, nextNode).ConfigureAwait(false);
            }
        }
    }
}
