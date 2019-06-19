using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace CSharp
{
    static class UdpRacerServer
    {
        public static void Start(int port, int howMany)
        {
            UdpClient udpSock = new UdpClient(port, AddressFamily.InterNetwork);
            for (int i=0; i<howMany;++i)
            {
                RunOne(udpSock, port);
            }
            Console.Error.WriteLine($"started {howMany} async receive tasks");
        }
        /// <summary>
        /// typedef struct _DATA {
        ///     DWORD Id;
        ///     DWORD Hops;
        ///     DWORD Next;
        ///     DWORD Last;
        ///     SOCKADDR_IN NextAddrIn[NUMBER_OF_MAX_CHAIN];
        /// }
        /// DATA;
        /// 
        /// typedef struct sockaddr_in {
        ///         short          sin_family;
        ///         USHORT sin_port;
        ///         IN_ADDR sin_addr;
        ///         CHAR sin_zero[8];
        ///     }
        ///     SOCKADDR_IN, * PSOCKADDR_IN;
        /// 
        /// </summary>
        /// <param name="port"></param>
        public static async void RunOne(UdpClient udpSock, int port)
        {
            IPEndPoint nextNode = new IPEndPoint(0, port);

            while (true)
            {
                #if DEBUG
                Console.WriteLine("issuing ReceiveAsync()");
                #endif
                try
                {
                    UdpReceiveResult data = await udpSock.ReceiveAsync().ConfigureAwait(false);
                
                    Interlocked.Increment(ref Program._packages);
                    UInt32 nextIP;

                    unsafe
                    {
                        fixed (byte* bptr = &(data.Buffer[0]))
                        {
                            UInt32* hops  = (UInt32*)(bptr +  4);
                            UInt32* next  = (UInt32*)(bptr +  8);
                            UInt32* last  = (UInt32*)(bptr + 12);

    #if DEBUG
                            Console.WriteLine($"package received! hops, next, last\t{*hops},{*next},{*last}");
    #endif
                            ++(*hops);
                            *next = (*next + 1) % *last;

                            byte* sockaddr_in_next = (bptr + 16) + (*next * 16);
                            UInt32* nextIPptr = (UInt32*)(sockaddr_in_next + 4);

                            nextIP = *nextIPptr;
                        }
                    }

                    nextNode.Address = new IPAddress(nextIP);
    #if DEBUG
                    Console.Error.WriteLine($"sending to IP {nextNode}...");
    #endif
                    udpSock.SendAsync(data.Buffer, data.Buffer.Length, nextNode).ConfigureAwait(false);
                }
                catch (SocketException sex)
                {
                    Console.Error.WriteLine($"ReceiveAsyncEx: {sex.Message}");
                }
            }
        }
    }
}
