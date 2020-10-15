using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CnC
{
    public class BootloaderClient
    {
        public byte BootloaderAddress => this.address;
        public SerialPort Port => this.sp;

        private byte address;
        private SerialPort sp;

        public BootloaderClient(SerialPort sp, byte bootloaderAdderess)
        {
            this.sp = sp;
            this.address = bootloaderAdderess;
        }

        public override string ToString()
        {
            return string.Format("0x{0:X2} on {1}", address, sp.PortName);
        }
    }
}
