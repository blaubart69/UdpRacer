using System;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace UDPRacer
{
    class Program
    {
        public static long _GLOBAL_packages;
        public static IPEndPoint _G_recv = null;
        public static IPEndPoint _G_sent = null;

        /// <summary>
        /// race for speed and fun
        /// </summary>
        /// <param name="IPs">IPs of the host. seperated by comma</param>
        /// <param name="inject">injet n packages and exit</param>
        /// <param name="port">UDP port to listen (default: 4444)</param>
        static void Main(string IPs, int inject = 0, int port = 4444)
        {
            if ( inject > 0 )
            {
                var hosts = IPs.Split(",").Select(ip => IPAddress.Parse(ip)).ToList();

                byte[] initialPacket = Race.CreateNetworkPackage(hosts);
                using (UdpClient udpc = new UdpClient())
                {
                    var firstHost = new IPEndPoint(hosts[0], port);
                    for (int i = 0; i < inject; ++i)
                    {
                        udpc.Send(initialPacket, initialPacket.Length, firstHost);
                    }
                    Console.WriteLine($"injected {inject} packages to {firstHost}. size: {initialPacket.Length} bytes");
                }
                return;
            }

            using (UdpClient udpSock = new UdpClient(port))
            {
                for (int i = 0; i < Environment.ProcessorCount; ++i)
                {
                    Race.Start(udpSock, port);
                }
                Console.WriteLine($"listening on port {port}. started {Environment.ProcessorCount} send/recv loops");

                using (ManualResetEvent loop = new ManualResetEvent(false))
                {
                    while (!loop.WaitOne(2000))
                    {
                        long pkgPerSec = Interlocked.Exchange(ref _GLOBAL_packages, 0) / 2;
                        if (pkgPerSec > 0)
                        {
                            Console.WriteLine($"{DateTime.Now}\t{pkgPerSec,6}/s\tfrom: {_G_recv?.ToString()}\tto: {_G_sent?.ToString()}");
                        }
                    }
                }
            }
        }
    }
}
