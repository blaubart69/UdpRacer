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

        /// <summary>
        /// race for speed and fun
        /// </summary>
        /// <param name="IPs">IPs of the host. seperated by comma</param>
        /// <param name="inject">injet package and exit</param>
        /// <param name="port">UDP port to listen (default: 4444)</param>
        static void Main(string IPs, bool inject = false, int port = 4444)
        {
            if ( inject )
            {
                var hosts = IPs.Split(",").Select(ip => IPAddress.Parse(ip)).ToList();

                byte[] initialPacket = Race.CreateNetworkPackage(hosts);
                using (UdpClient udpc = new UdpClient())
                {
                    var firstHost = new IPEndPoint(hosts[0], port);
                    udpc.Send(initialPacket, initialPacket.Length, firstHost);
                    Console.WriteLine($"injected package to {firstHost}. size: {initialPacket.Length} bytes");
                }
                return;
            }

            Console.WriteLine($"listening on port {port}");
            Race.Start(port);

            using (ManualResetEvent loop = new ManualResetEvent(false))
            {
                while (!loop.WaitOne(1000))
                {
                    long pkgPerSec = Interlocked.Exchange(ref _GLOBAL_packages, 0);
                    Console.WriteLine($"{pkgPerSec}/s");
                }
            }
        }
    }
}
