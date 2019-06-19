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
            UdpRacerServer.Start(opts.Port, 16);
            Console.WriteLine($"listening on port: {opts.Port}");


            if (opts.IPs != null && opts.IPs.Count > 0)
            {
                Console.WriteLine($"IPs parsed: {String.Join(";",opts.IPs)}");
                byte[] initialPacket = UdpRacerStart.CreateNetworkPackage(opts.IPs);

                Console.WriteLine("++++ UDP package");
                UdpRacerStart.PrintUdpRacerPackage(initialPacket);
                Console.WriteLine("++++ UDP package");

                using (UdpClient udpc = new UdpClient())
                {
                    var sendTo = new IPEndPoint(opts.IPs[0], opts.Port);

                    udpc.Send(
                            initialPacket,
                            initialPacket.Length,
                            sendTo);
                    Console.WriteLine($"sent initial package to: {sendTo}");
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
