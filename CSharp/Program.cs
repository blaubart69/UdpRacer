using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace CSharp
{
    class Program
    {
        public static long _packages;

        static int Main(string[] args)
        {
            Opts opts;
            
            if ( ! Opts.ParseOpts(args, out opts) )
            {
                return 4;
            }

            _packages = 0;
            UdpRacerServer.Run(opts.Port);

            if (opts.IPs != null && opts.IPs.Count > 0)
            {
                byte[] initialPacket = UdpRacerStart.CreateNetworkPackage(opts.IPs);
                using (UdpClient udpc = new UdpClient())
                {
                    udpc.Send(
                            initialPacket,
                            initialPacket.Length,
                            new IPEndPoint(opts.IPs[0], opts.Port));
                    Console.WriteLine($"injected package to {opts.IPs[0]}");
                }
            }

            ManualResetEvent ended = new ManualResetEvent(false);
            while ( ! ended.WaitOne(1000) )
            {
                long pkgPerSec = Interlocked.Exchange(ref _packages, 0);
                Console.WriteLine($"{pkgPerSec}");
            }

            return 0;
        }
    }
}
