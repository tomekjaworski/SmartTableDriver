using IntelHEX;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CnC
{

    public class ModuleEntry
    {
        [JsonProperty("bootloader_id")]
        public int BootloaderID { get; set; }

        [JsonProperty("flash")]
        public string FlashSourceFileName { get; set; }

        [JsonProperty("eeprom")]
        public string EepromSourceFileName { get; set; }
    }

    public class Configuration
    {
        [JsonProperty("modules")]
        public ModuleEntry[] Modules { get; set; }
    }

    class Program
    {
        static Random random;
        static void Main(string[] args)
        {

            string content = File.ReadAllText("configuration.json");
            var x = JsonConvert.DeserializeObject< Configuration>(content);

            string f1 = @"C:\Users\Tomek\Documents\SmartTableDriver\SmartTableFirmware\Debug\SmartTableFirmware.hex";
            string f2 = @"C:\Users\Tomek\Documents\SmartTableDriver\SmartTableFirmware\Eeprom\13.hex";

            IntelHEX16Storage loader;
            MemoryMap memory_flash = new MemoryMap(32 * 1024 - 2 * 1024);
            MemoryMap memory_eeprom = new MemoryMap(0x400);
            loader = new IntelHEX16Storage(memory_flash);
            loader.Load(f1);

            loader = new IntelHEX16Storage(memory_eeprom);
            loader.Load(f2);


            Console.WriteLine("SmartTable bootloader C&C software by Tomasz Jaworski");
            random = new Random();

            MemoryMap fw = new MemoryMap(32*1024-2*1024);
            IntelHEX16Storage st = new IntelHEX16Storage(fw);
            st.Load(@"d:\SystemDocuments\SmartTableDriver\SmartTableFirmware\Debug\SmartTableFirmware.hex");

            int pos1 = fw.FindSequence(new byte[] { 0xaa, 0x11, 0x0d, 0x4d });
            int pos2 = fw.FindSequence(new byte[] { 0x75, 0x87, 0x60, 0x64 });
            Debug.Assert(pos2 == pos1 + 5);

            fw.Dump("test.txt");



            AVRBootloaderCnC cnc = new AVRBootloaderCnC();
            cnc.SendAdvertisementToEveryDetectedPort();
            cnc.AcquireBootloaderDevices(0x20);

            // show found devices
            cnc.ShowDevices();


            Console.WriteLine("Reading bootloader version and signature");
            foreach (Device dev in cnc.Devices)
            {
                // read bootloader version and timestamp
                string ver = "";
                cnc.ReadVersion(dev, ref ver);

                // read CPU signature
                byte[] bsig = null;
                cnc.ReadSignature(dev, out bsig);
            }

            Console.WriteLine("Writing firmare...");

            foreach (Device dev in cnc.Devices) {

                // preapre modified firmare
                fw.Write((uint)pos1 + 4, (byte)dev.address);

                cnc.WriteFLASH(dev, fw);
                cnc.VerifyFLASH(dev, fw);
            }


            foreach (Device dev in cnc.Devices)
                cnc.Reset(dev);


        }

    }

}
