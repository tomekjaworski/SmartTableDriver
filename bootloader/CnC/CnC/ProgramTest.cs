using CnC.Jobs;
using IntelHEX;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;


namespace CnC
{

    class Program
    {
        static Random random;
        static void Main(string[] args)
        {

            Console.WriteLine("SmartTable bootloader C&C software by Tomasz Jaworski\n");
            random = new Random();

            BootloaderTaskProvider btp = new BootloaderTaskProvider();
            Console.WriteLine($"Reading task file...");
            try
            {
                btp.LoadTasks("bootloader_tasks.json");
            }
            catch (Exception ex)
            {
                ColorConsole.WriteLine(ConsoleColor.Red, "Error druing reading: " + ex.Message);
            }


            // Find highest bootloader id
            int last_bootloader_id = btp.Tasks.Select(x => x.BootloaderID).Max();

            // Show some summary            
            Console.WriteLine($"Task summary:");
            Console.WriteLine($"   Found {btp.Count} task(s).");
            Console.WriteLine($"   Highest bootloader ID:  {last_bootloader_id}");
            Console.WriteLine($"   Unique bootloaders (by ID): {btp.Tasks.Select(x => x.BootloaderID).Distinct().Count()}");
            Console.WriteLine($"   Unique bootloaders: {string.Join(",", btp.Tasks.Select(x => x.BootloaderID.ToString("X2")).Distinct())}");

            //
            // Wait for all serial ports to be connected and identified
            AVRBootloaderCnC cnc = new AVRBootloaderCnC();
            cnc.SendAdvertisementToEveryDetectedPort();

            // Scan all available serial ports for bootloaders
            cnc.AcquireBootloaderDevices((byte)(1 + last_bootloader_id));


            //IntelHEX16Storage loader;
            //MemoryMap memory_flash = new MemoryMap(32 * 1024 - 2 * 1024);
            //MemoryMap memory_eeprom = new MemoryMap(0x400);
            //loader = new IntelHEX16Storage(memory_flash);
            ////loader.Load(f1);

            //loader = new IntelHEX16Storage(memory_eeprom);
            ////loader.Load(f2);



            //MemoryMap fw = new MemoryMap(32*1024-2*1024);
            //IntelHEX16Storage st = new IntelHEX16Storage(fw);
            //st.Load(@"d:\SystemDocuments\SmartTableDriver\SmartTableFirmware\Debug\SmartTableFirmware.hex");

            //int pos1 = fw.FindSequence(new byte[] { 0xaa, 0x11, 0x0d, 0x4d });
            //int pos2 = fw.FindSequence(new byte[] { 0x75, 0x87, 0x60, 0x64 });
            //Debug.Assert(pos2 == pos1 + 5);

            //fw.Dump("test.txt");




            // show found devices
            cnc.ShowDevices();


            for (int task_index = 0; task_index < btp.Count; task_index++)
            {
                TaskEntry task_entry = btp.Tasks[task_index];
                Console.WriteLine($"Running task {task_index}: {task_entry.TaskType} for device {task_entry.CPU} ID={task_entry.BootloaderID:X2}...");

                // Get the device
                Device device = cnc.Devices.Where(x => x.address == task_entry.BootloaderID).FirstOrDefault();
                if (device == null)
                {
                    ColorConsole.WriteLine(ConsoleColor.Yellow, "   No proper device found.");
                    continue;
                }

                if (task_entry.TaskType == TaskType.ReadEepromMemory)
                {
                    MemoryMap mm = new MemoryMap(task_entry.ProgrammableMemorySize);
                    cnc.ReadEEPROM(device, mm);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Save(task_entry.FileName);
                }

                if (task_entry.TaskType == TaskType.ReadFlashMemory)
                {
                    MemoryMap mm = new MemoryMap(task_entry.ProgrammableMemorySize);
                    cnc.ReadFLASH(device, mm);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Save(task_entry.FileName);
                }

                if (task_entry.TaskType == TaskType.WriteEepromMemory)
                {
                    MemoryMap mm = new MemoryMap(task_entry.ProgrammableMemorySize);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Load(task_entry.FileName);
                    cnc.WriteEEPROM(device, mm);
                }


                if (task_entry.TaskType == TaskType.WriteFlashMemory)
                {
                    MemoryMap mm = new MemoryMap(task_entry.ProgrammableMemorySize);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Load(task_entry.FileName);
                    cnc.WriteFLASH(device, mm);
                }

                if (task_entry.TaskType == TaskType.Reboot)
                {
                    cnc.Reboot(device);
                }

                //if (task_entry.TaskType == TaskType.ReadFlashMemory)
                //{
                //    MemoryMap mm = new MemoryMap(task_entry.ProgrammableMemorySize);
                //    cnc.ReadFLASH(device, mm);
                //    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                //    storage.Save(task_entry.FileName);
                //}

            }

            //Console.WriteLine("Reading bootloader version and signature");
            //foreach (Device dev in cnc.Devices)
            //{
            //    // read bootloader version and timestamp
            //    string ver = "";
            //    cnc.ReadVersion(dev, ref ver);

            //    // read CPU signature
            //    byte[] bsig = null;
            //    cnc.ReadSignature(dev, out bsig);

            //}

            //Console.WriteLine("Writing firmare...");

            //foreach (Device dev in cnc.Devices)
            //{

            //    // preapre modified firmare
            //    //fw.Write((uint)pos1 + 4, (byte)dev.address);

            //    //cnc.WriteFLASH(dev, fw);
            //    //cnc.VerifyFLASH(dev, fw);
            //}


            //foreach (Device dev in cnc.Devices)
            //    cnc.Reboot(dev);


            ColorConsole.PressAnyKey();

        }

    }

}
