using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharp
{
    struct Opts
    {
        public int Port;
        public IList<IPAddress> IPs;
    }
    class Program
    {
        static int Main(string[] args)
        {
            Opts opts;
            if ( ! ParseOpts(args, out opts) )
            {
                return 4;
            }

            StartListening(opts.Port);

            if ( opts.IPs != null && opts.IPs.Count > 0 )
            {
                InjectPacket(opts.IPs, opts.Port);
            }

            return 0;
        }
        static void InjectPacket(IList<IPAddress> IPs, int Port)
        {
            byte[] packet;
            using (MemoryStream ms = new MemoryStream())
            {
                BinaryWriter bw = new BinaryWriter(ms);

                bw.Write((UInt32)IPs.Count);
                bw.Write((UInt32)IPs[0]);

                packet = ms.ToArray();
            }
                
            
        }
        static bool ParseOpts(string[] args, out Opts opts)
        {
            bool showhelp = false;

            opts = default(Opts);
            Opts tmpOpts = new Opts() { Port = 4444 };
            var p = new Mono.Options.OptionSet() {
                { "p|port=",    "udp port to listen",           v => tmpOpts.Port  = int.Parse(v)      },
                { "h|help",     "show this message and exit",   v => showhelp = true                   },
            };
            var IPsString = p.Parse(args);
            if ( ! TryParseIPs(IPsString, out tmpOpts.IPs) )
            {
                return false;
            }

            if (showhelp)
            {
                p.WriteOptionDescriptions(Console.Out);
                Console.WriteLine(
                        "\nSample:"
                    +   "UdpRacer.exe -c 10.2.2.2,10.1.1.1"
                    );
                return false;
            }

            opts = tmpOpts;

            return true;
        }
        static bool TryParseIPs(IEnumerable<string> IpsAsString, out IList<IPAddress> IPs)
        {
            IPs = new List<IPAddress>();

            AddressFamily? mode = null;
            bool ok = true;

            foreach ( string ip in IpsAsString )
            {
                IPAddress ParsedIP;
                if ( IPAddress.TryParse(ip, out ParsedIP) )
                {
                    if ( mode.HasValue )
                    {
                        if ( mode != ParsedIP.AddressFamily )
                        {
                            Console.Error.WriteLine("E: you cannot mix IPv4 and IPv6 adresses");
                            ok = false;
                            break;
                        }
                    }
                    else
                    {
                        mode = ParsedIP.AddressFamily;
                    }
                }
                else
                {
                    Console.Error.WriteLine("E: Could not parse IP [{0}]", ip);
                    ok = false;
                }
            }

            return ok;
        }
    }
}
