using Microsoft.Win32.SafeHandles;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
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
        public static async void Start(UdpClient udpSock, int port)
        {
            IPEndPoint nextNode = new IPEndPoint(0, port);

            while (true)
            {
                UdpReceiveResult data = await udpSock.ReceiveAsync().ConfigureAwait(false);
                Interlocked.Increment(ref Program._GLOBAL_packages);
                Program._G_recv = data.RemoteEndPoint;
                long nextIP = modifyBuffer(ref data);

                nextNode.Address.Address = nextIP;
                Program._G_sent = nextNode;
                udpSock.Send(data.Buffer, data.Buffer.Length, nextNode);
            }
        }
        private static UInt32 modifyBuffer(ref UdpReceiveResult data)
        {
            Span<UInt32> field = MemoryMarshal.Cast<byte, UInt32>(new Span<byte>(data.Buffer));
            
            ++field[1];    
            UInt32 nextIPIdx = (field[2] + 1) % field[3];
            field[2] = nextIPIdx;

            return field[4 + (Int32)nextIPIdx];
        }

        public static byte[] CreateNetworkPackage(IReadOnlyList<IPAddress> IPs)
        {
            byte[] packet;

            using (MemoryStream ms = new MemoryStream())
            {
                BinaryWriter bw = new BinaryWriter(ms);

                bw.Write((UInt32)42);              // id
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
