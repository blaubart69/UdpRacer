using Spi;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace CSharp
{
    class Opts
    {
        public int Port;
        public IList<IPAddress> IPs;

        private Opts()
        {
        }

        public static bool ParseOpts(string[] args, out Opts opts)
        {
            opts = null;
            bool showhelp = false;

            string IPList = null;
            Opts tmpOpts = new Opts() { Port = 4444 };
            var cmdOpts = new BeeOptsBuilder()
                .Add('p', "port", OPTTYPE.VALUE, "udp port to listen",      o => tmpOpts.Port = Convert.ToInt32(o))
                .Add('i', "ip", OPTTYPE.VALUE, "ips for packet to inject",  o => IPList = o)
                .Add('h', "help", OPTTYPE.BOOL,  "show help",               o => showhelp = true)
                .GetOpts();


            var IPsString = Spi.BeeOpts.Parse(args, cmdOpts, (string unknownOpt) => Console.Error.WriteLine($"unknow option [{unknownOpt}]"));
            if (showhelp)
            {
                Spi.BeeOpts.PrintOptions(cmdOpts);
                Console.WriteLine(
                        "\nSample:"
                    + "UdpRacer.exe -i 10.2.2.2,10.1.1.1"
                    );
                return false;
            }

            if (!String.IsNullOrEmpty(IPList))
            {
                if (!TryParseIPs(
                    IpsAsString: IPList.Split(',').Select(i => i.Trim()),
                    IPs: out tmpOpts.IPs))
                {
                    return false;
                }
            }

            opts = tmpOpts;

            return true;
        }
        static bool TryParseIPs(IEnumerable<string> IpsAsString, out IList<IPAddress> IPs)
        {
            IPs = new List<IPAddress>();

            AddressFamily? mode = null;
            bool ok = true;

            foreach (string ip in IpsAsString)
            {
                IPAddress ParsedIP;
                if (IPAddress.TryParse(ip, out ParsedIP))
                {
                    IPs.Add(ParsedIP);
                    if (mode.HasValue)
                    {
                        if (mode != ParsedIP.AddressFamily)
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
