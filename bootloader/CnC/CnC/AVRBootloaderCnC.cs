using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CnC
{

    

    public class AVRBootloaderCnC
    {
        List<SerialPort> available_ports;
        List<string> excluded_port_names;

        List<BootloaderClient> discovered_devices;

        public BootloaderClient[] Devices => this.discovered_devices.ToArray();

        Random random;


        public AVRBootloaderCnC()
        {
            this.available_ports = new List<SerialPort>();
            this.excluded_port_names = new List<string>();
            this.discovered_devices = new List<BootloaderClient>();
            this.random =  new Random();
        }

        internal void AcquireBootloaderDevices(byte max_addr)
        {
            foreach (SerialPort sp in available_ports)
                this.discovered_devices.AddRange(AcquireDevicesOnSerialPort(sp, max_addr));

            this.discovered_devices.Sort((x, y) => x.BootloaderAddress - y.BootloaderAddress);
        }


        private async  Task SerialPortOpenerThread(CancellationToken ct)
        {
            List<string> port_names_to_open = new List<string>();

            while (!ct.IsCancellationRequested) {
                lock (this.available_ports)
                    foreach (string port_name in SerialPort.GetPortNames()) {
                        SerialPort sp = available_ports.Find(x => x.PortName == port_name);
                        if (sp != null)
                            continue; // ok, this port was previoulsy opened

                        port_names_to_open.Add(port_name);
                    }

                while (port_names_to_open.Count > 0)
                {
                    String port_name = port_names_to_open[0];
                    port_names_to_open.Remove(port_name);
                    try
                    {
                        SerialPort sp = new SerialPort(port_name, 19200, Parity.Even, 8, StopBits.One);

                        sp.ReadTimeout = 200;
                        sp.Open();
                        lock (this.available_ports)
                            this.available_ports.Add(sp);
                    }
                    catch (Exception ex)
                    {
                        // Console.WriteLine("Failed");
                    }

                }

                await Task.Delay(100);
            }
        }

        public bool Reboot(BootloaderClient dev)
        {
            Console.Write("Rebooting {0:X2}... ", dev.BootloaderAddress);
            Message reboot_message = new Message(dev.BootloaderAddress, MessageType.Reboot);
            Message msg = SendAndWaitForResponse(dev, reboot_message, 200, false);

            if (msg != null)
                Console.WriteLine("Ok.");
            else
                Console.WriteLine("Failed.");

            return msg != null;
        }

        public bool Ping(BootloaderClient dev, int timeout)
        {
            int x = this.random.Next();
            byte[] payload = BitConverter.GetBytes(x);

            Message ping_message = new Message(dev.BootloaderAddress, MessageType.Ping, payload);
            Message msg = SendAndWaitForResponse(dev, ping_message, timeout, false);

            // check if there was a response
            if (msg == null)
                return false;
            
            // check if the received payload size is the same as sent
            if (msg.Payload.Length != 4)
                return false;

            // compare the contents
            for (int i = 0; i < 4; i++)
                if (payload[i] != msg.Payload[i])
                    return false;
                    
            // well, ok then
            return true;
        }


        public void SendAdvertisementToEveryDetectedPort()
        {
            ColorConsole.WriteLine(ConsoleColor.Black, ConsoleColor.Yellow, "*** TURN ON all devices and press any key to processed...");
            char[] anim = { '/', '-', '\\', '|' };
            int anim_counter = 0;
            int cx = 0;

            Console.CursorVisible = false;
            cx = Console.CursorLeft;

            CancellationTokenSource cts = new CancellationTokenSource();
            Task t = SerialPortOpenerThread(cts.Token);

            while (!Console.KeyAvailable) {

                Thread.Sleep(100);

                Console.CursorLeft = cx;
                ColorConsole.Write(ConsoleColor.Yellow, "Sending C&C Advertisement to {0} serial ports: ", this.available_ports.Count);
                Console.Write(anim[anim_counter++ % 4]);

                // send advertisement to ports on list
                List<SerialPort> lost_ports = new List<SerialPort>();
                byte[] req = new byte[] { (byte)'A' };

                lock (this.available_ports)
                    foreach (SerialPort out_port in available_ports)
                        try {
                            //if (out_port.IsOpen)
                                out_port.Write(req, 0, 1);
                            //else
                                //out_port.Dispose();
                        }
                        catch (Exception ex) {
                            lost_ports.Add(out_port);
                        }

                // remove lost ports
                lock (this.available_ports)
                    foreach (SerialPort lp in lost_ports)
                        this.available_ports.Remove(lp);

            }

            cts.Cancel();
            Task.WaitAll(t);
        }
        
        public BootloaderClient[] AcquireDevicesOnSerialPort(SerialPort sp, byte max_addr)
        {
            Console.WriteLine("\nSending PING to serial port {0}... ", sp.PortName);
            Console.CursorVisible = false;

            bool intro = true;
            int cx = 0, cy = 0;

            int timeout = 100;
            int counter = 0;

            sp.ReadTimeout = 20;
            byte[] buffer = new byte[1024];
            MessageExtractor me = new MessageExtractor();
            List<BootloaderClient> endpoints = new List<BootloaderClient>();

            // scan through 0x00 - 0xEF. Range 0xF0 - 0xFF is reserved
            max_addr = Math.Min(max_addr, (byte)0xF0);

            for (byte naddress = 0x00; naddress <= max_addr; naddress++) {
              //i = 0x12;
                if (intro) {
                    Console.Write(" Looking for device ");
                    cx = Console.CursorLeft;
                    cy = Console.CursorTop;
                    intro = false;
                }

                Console.SetCursorPosition(cx, cy);
                ColorConsole.Write(ConsoleColor.Green, "0x{0:X2}", naddress);


                // send ping do selected device
                sp.DiscardInBuffer();
                sp.DiscardOutBuffer();
                me.Discard();

                Message ping_message = new Message(naddress, MessageType.Ping);
                Message msg = SendAndWaitForResponse(new BootloaderClient(sp, naddress), ping_message, timeout, false, 0);

                if (msg != null) {
                    Console.WriteLine(" Found!");
                    counter++;
                    intro = true;
                    endpoints.Add(new BootloaderClient(sp, naddress));
                }

                //break;
            }
            Console.WriteLine();

            Console.SetCursorPosition(0, cy);
            Console.WriteLine(" Done. Found {0} devices on serial port {1}.", counter, sp.PortName);
            Console.CursorVisible = true;
            return endpoints.ToArray();
        }

        class ScanningTask
        {
            public int Row;

            public Thread Thread { get; internal set; }
            public byte[] AddressList { get; internal set; }
            public SerialPort SerialPort { get; internal set; }
            public List<BootloaderClient> EndpointsCollection { get; internal set; }
        }

        internal void AcquireBootloaderDevicesInParallel(byte[] addresses)
        {
            Console.WriteLine();
            ScanningTask[] tasks = new ScanningTask[available_ports.Count];
            int top = Console.CursorTop;
            List<BootloaderClient> endpoints = new List<BootloaderClient>();


            for (int i = 0; i < available_ports.Count; i++)
            {
                tasks[i] = new ScanningTask()
                {
                    Row = top + i,
                    Thread = new Thread(new ParameterizedThreadStart(Ack)),
                    AddressList = addresses,
                    SerialPort = available_ports[i],
                    EndpointsCollection = endpoints
                };
                tasks[i].Thread.Start(tasks[i]);
            }

            for (int i = 0; i < available_ports.Count; i++)
                tasks[i].Thread.Join();

            Console.CursorTop += available_ports.Count;

            this.discovered_devices.AddRange(endpoints);
            this.discovered_devices.Sort((x, y) => x.BootloaderAddress - y.BootloaderAddress);
        }

        internal void AcquireBootloaderDevicesInParallel(byte max_addr)
        {
            var addresses = Enumerable.Range(0, (int)max_addr + 1).Select(x => (byte)x).ToArray();
            AcquireBootloaderDevicesInParallel(addresses);
        }

        private void Ack(object obj)
        {
            ScanningTask task = obj as ScanningTask;
            int col = 0;
            ColorConsole.WriteXY(0, task.Row, task.SerialPort.PortName);
            col += 3 + 3 + 1;

            var sp = task.SerialPort;
            var me = new MessageExtractor();
            int timeout = 100;
            var endpoints = new List<BootloaderClient>();

            foreach (byte scanned_address in task.AddressList)
            {
                // Show scanned address
                int ccol = col;
                ColorConsole.WriteXY(ccol, task.Row, ConsoleColor.Green, $"0x{scanned_address:X2}");

                // send ping do selected device
                sp.DiscardInBuffer();
                sp.DiscardOutBuffer();
                me.Discard();

                Message ping_message = new Message(scanned_address, MessageType.Ping);
                Message msg = SendAndWaitForResponse(new BootloaderClient(sp, scanned_address), ping_message, timeout, false, 0);

                if (msg != null)
                    endpoints.Add(new BootloaderClient(sp, scanned_address));


                // Show list of found clients
                ccol += 5;
                ColorConsole.WriteXY(ccol, task.Row, ConsoleColor.Yellow, "[" + string.Join(",", endpoints.Select(x => x.BootloaderAddress.ToString("X2"))) + "]");
            }

            lock (task.EndpointsCollection)
                task.EndpointsCollection.AddRange(endpoints);
        }

        public void ShowDevices()
        {
            Console.WriteLine("\nListing {0} discovered device(s): ", discovered_devices.Count);

            // group them against serial port
            Dictionary<string, List<BootloaderClient>> devs = new Dictionary<string, List<BootloaderClient>>();
            foreach (BootloaderClient dev in this.discovered_devices)
            {
                if (!devs.ContainsKey(dev.Port.PortName))
                    devs[dev.Port.PortName] = new List<BootloaderClient>();

                devs[dev.Port.PortName].Add(dev);
            }

            // sort them
            foreach (List<BootloaderClient> sp_devs in devs.Values)
                sp_devs.Sort((x, y) => x.BootloaderAddress - y.BootloaderAddress);

            // and show them
            foreach (string key in devs.Keys)
            {
                List<BootloaderClient> sp_devs = devs[key];
                Console.Write(" Port {0:010}: ", key.PadRight(5));
                ColorConsole.WriteLine(ConsoleColor.Yellow, string.Join(" ", sp_devs.Select(x => x.BootloaderAddress.ToString("X2"))));
            }

            // and show them again, but together for better view in case of holes in addresses

            discovered_devices.Sort((x, y) => x.BootloaderAddress - y.BootloaderAddress);
            Console.Write(" Summary list: ");
            ColorConsole.WriteLine(ConsoleColor.Yellow, string.Join(" ", discovered_devices.Select(x => x.BootloaderAddress.ToString("X2"))));
            ColorConsole.WriteLine();
        }

        public void ReadBootloaderVersion(BootloaderClient dev, ref string ver)
        {
            Console.CursorVisible = false;


            Message msg_readfwver = new Message(dev.BootloaderAddress, MessageType.ReadBootloaderVersion);
            Message response = SendAndWaitForResponse(dev, msg_readfwver, 2000);

            ver = Encoding.ASCII.GetString(response.Payload, 0, response.Payload.Length - 1);

            Console.CursorVisible = true;
            Console.WriteLine($"   Reading bootloader fw version: [{ver}]");

        }


        public void ReadSignature(BootloaderClient endpoint, out byte[] signature)
        {
            Console.CursorVisible = false;
            signature = null;

            Console.Write("   Reading AVR CPU signature (32b): ");

            Message msg_readsig = new Message(endpoint.BootloaderAddress, MessageType.ReadSignature);
            Message response = SendAndWaitForResponse(endpoint, msg_readsig, 2000);

            signature = response.Payload;

            Console.CursorVisible = true;
            Console.WriteLine("   Done ({0:X2} {1:X2} {2:X2}).", signature[0], signature[2], signature[4]);
        }

        public void ReadFLASH(BootloaderClient endpoint, MemoryMap dest)
        {
            Console.CursorVisible = false;

            Console.Write("   Reading FLASH memory ({0}kB):    ", dest.Size / 1024);
            ConsoleProgressBar cpb = new ConsoleProgressBar(0, dest.Size);

            for (uint addr = 0; addr < dest.Size; addr += 128, cpb.Progress = addr) {
                Message msg_readpage = new Message(endpoint.BootloaderAddress, MessageType.ReadFlashPage, new byte[] { (byte)(addr & 0xFF), (byte)(addr >> 8), });

                Message response = SendAndWaitForResponse(endpoint, msg_readpage, 2000);
                dest.Write(addr, response.Payload, 0, 128);
            }

            Console.CursorVisible = true;
            Console.WriteLine("Done.");
        }

        public void WriteFLASH(BootloaderClient endpoint, MemoryMap source)
        {
            Console.CursorVisible = false;

            Console.Write("   Writing FLASH memory ({0}kB):    ", source.Size / 1024);
            ConsoleProgressBar cpb = new ConsoleProgressBar(0, source.Size);

            for (uint addr = 0; addr < source.Size; addr += 128, cpb.Progress = addr) {

                byte[] payload = new byte[2 + 128];
                payload[0] = (byte)(addr & 0xFF);
                payload[1] = (byte)((addr >> 8) & 0xFF);
                source.Read(addr, payload, 2, 128);

                Message msg_write = new Message(endpoint.BootloaderAddress, MessageType.WriteFlashPage, payload);

                Message response = SendAndWaitForResponse(endpoint, msg_write, 2000);
            }

            Console.CursorVisible = true;
            Console.WriteLine("Done.");
        }

        public bool VerifyFLASH(BootloaderClient endpoint, MemoryMap expected)
        {
            Console.CursorVisible = false;

            Console.Write("   Verifying FLASH memory ({0}kB):  ", expected.Size / 1024);
            ConsoleProgressBar cpb = new ConsoleProgressBar(0, expected.Size);
            MemoryMap mmread = new MemoryMap(expected.Size);

            for (uint addr = 0; addr < expected.Size; addr += 128, cpb.Progress = addr) {
                Message msg_readpage = new Message(endpoint.BootloaderAddress, MessageType.ReadFlashPage, new byte[] { (byte)(addr & 0xFF), (byte)(addr >> 8), });

                Message response = SendAndWaitForResponse(endpoint, msg_readpage, 2000);
                mmread.Write(addr, response.Payload, 0, 128);
            }

            UInt32 difference_address = 0;
            bool result = expected.BinaryCompare(mmread, ref difference_address);

            Console.CursorVisible = true;
            if (result)
                Console.WriteLine("Correct.");
            else {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Failed.");
                Console.ForegroundColor = ConsoleColor.Gray;

                byte expected_byte = expected.ReadByte(difference_address);
                byte existing_byte = mmread.ReadByte(difference_address);

                throw new MemoryVerificationException("FLASH", difference_address, expected_byte, existing_byte);
            }
            return result;
        }

        public void ReadEEPROM(BootloaderClient endpoint, MemoryMap dest)
        {
            Console.CursorVisible = false;

            Console.Write("   Reading EEPROM memory ({0}kB):   ", dest.Size / 1024);
            ConsoleProgressBar cpb = new ConsoleProgressBar(0, dest.Size);

            for (uint addr = 0; addr < dest.Size; addr += 128, cpb.Progress = addr) {
                Message msg_readpage = new Message(endpoint.BootloaderAddress, MessageType.ReadEepromPage, new byte[] { (byte)(addr & 0xFF), (byte)(addr >> 8), });

                Message response = SendAndWaitForResponse(endpoint, msg_readpage, 2000);
                dest.Write(addr, response.Payload, 0, 128);
            }

            Console.CursorVisible = true;
            Console.WriteLine("Done.");
        }

        public bool VerifyEEPROM(BootloaderClient endpoint, MemoryMap expected)
        {
            Console.CursorVisible = false;

            Console.Write("   Verifying EEPROM memory ({0}kB): ", expected.Size / 1024);
            ConsoleProgressBar cpb = new ConsoleProgressBar(0, expected.Size);
            MemoryMap mmread = new MemoryMap(expected.Size);

            for (uint addr = 0; addr < expected.Size; addr += 128, cpb.Progress = addr) {
                Message msg_readpage = new Message(endpoint.BootloaderAddress, MessageType.ReadEepromPage, new byte[] { (byte)(addr & 0xFF), (byte)(addr >> 8), });

                Message response = SendAndWaitForResponse(endpoint, msg_readpage, 2000);
                mmread.Write(addr, response.Payload, 0, 128);
            }

            UInt32 difference_address = 0;
            bool result = expected.BinaryCompare(mmread, ref difference_address);

            Console.CursorVisible = true;
            if (result)
                Console.WriteLine("Correct.");
            else {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Failed.");
                Console.ForegroundColor = ConsoleColor.Gray;

                byte expected_byte = expected.ReadByte(difference_address);
                byte existing_byte = mmread.ReadByte(difference_address);

                throw new MemoryVerificationException("FLASH", difference_address, expected_byte, existing_byte);
            }
            return result;
        }

        public void WriteEEPROM(BootloaderClient endpoint, MemoryMap source)
        {
            Console.CursorVisible = false;

            Console.Write("   Writing EEPROM memory ({0}kB):   ", source.Size / 1024);
            ConsoleProgressBar cpb = new ConsoleProgressBar(0, source.Size);

            for (uint addr = 0; addr < source.Size; addr += 128, cpb.Progress = addr) {

                byte[] payload = new byte[2 + 128];
                payload[0] = (byte)(addr & 0xFF);
                payload[1] = (byte)((addr >> 8) & 0xFF);
                source.Read(addr, payload, 2, 128);

                Message msg_write = new Message(endpoint.BootloaderAddress, MessageType.WriteEepromPage, payload);

                Message response = SendAndWaitForResponse(endpoint, msg_write, 2000);
                if (response.Type != MessageType.WriteEepromPage)
                    throw new CnCException("response.Type");
            }

            Console.CursorVisible = true;
            Console.WriteLine("Done.");
        }

        private void PurgeSerialPorts()
        {
            Thread.Sleep(100);
            foreach (SerialPort sp in this.available_ports) {
                sp.DiscardInBuffer();
                sp.DiscardOutBuffer();
            }
        }

        private Message SendAndWaitForResponse(BootloaderClient ep, Message request, int timeout, bool throw_timeout_exception = true, int retries = 3)
        {
            Debug.Assert(ep.BootloaderAddress == request.Address);

            MessageExtractor me = new MessageExtractor();
            byte[] buffer = new byte[1024];
            byte[] empty = new byte[128+4+5];

            Message msg = null;

            while (retries-- >= 0) {

                // setup serial port
                ep.Port.DiscardInBuffer();
                ep.Port.DiscardOutBuffer();
                ep.Port.ReadTimeout = 200;

                // send data
                ep.Port.Write(request.Binary, 0, request.BinarySize);
                Thread.Sleep(0);
                //Debug.WriteLine("sent " + request.BinarySize.ToString());

                // and wait for response
                DateTime start = DateTime.Now;
                do {
                    int read = -1;
                    try {
                        read = ep.Port.Read(buffer, 0, buffer.Length);
                    }
                    catch (TimeoutException tex) {
                        Debug.WriteLine("TO");
                        continue; // ignore timeouts
                    }
                    catch (Exception ex) {
                        Debug.WriteLine("EX");
                        break; // shit happens
                    }

                    me.AddData(buffer, read);
                    if (me.TryExtract(ref msg, request.Address, request.Type))
                        break; // ok, got message!

                } while ((DateTime.Now - start).TotalMilliseconds <= timeout && timeout != -1);

                // if message was correctly received then stop communication
                if (msg != null)
                    break;
                Debug.WriteLine("RETRY");
            }
            if (msg == null && throw_timeout_exception)
                throw new TimeoutException(string.Format("No response from bootloader device 0x{0:X2} on {1}", ep.BootloaderAddress, ep.Port.PortName));

            return msg;
        }

    }
}
