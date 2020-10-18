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
            ShowIntro();

            random = new Random();

            BootloaderJobsLoader btp = new BootloaderJobsLoader();
            Console.WriteLine($"Reading task file...");
            try
            {
                btp.LoadTasks("bootloader_tasks_real-subset.json");
            }
            catch (Exception ex)
            {
                ColorConsole.WriteLine(ConsoleColor.Red, "Error druing reading: " + ex.Message);
            }


            // Find highest bootloader id
            byte[] addresses_to_check = btp.Jobs.Where(x => x.BootloaderID.HasValue).Select(x => x.BootloaderID.Value).Distinct().ToArray();

            // Show some summary            
            Console.WriteLine($"Task summary:");
            Console.WriteLine($"   Found {btp.Count} task(s).");
            Console.WriteLine($"   Highest bootloader ID: 0x{addresses_to_check.Max():X2}");
            Console.WriteLine($"   Unique bootloaders (by ID): {btp.Jobs.Select(x => x.BootloaderID).Distinct().Count()}");
            Console.WriteLine($"   Unique bootloaders: {string.Join(",", addresses_to_check.Select(x => "0x" + x.ToString("X2")).Distinct())}");

            //
            // Wait for all serial ports to be connected and identified
            AVRBootloaderCnC cnc = new AVRBootloaderCnC();
            cnc.SendAdvertisementToEveryDetectedPort();

            // Scan all available serial ports for bootloaders
            cnc.AcquireBootloaderDevicesInParallel(addresses_to_check);



            // show found devices
            cnc.ShowDevices();


            for (int job_index = 0; job_index < btp.Count; job_index++)
            {
                JobEntry job_entry = btp.Jobs[job_index];
                JobTypeDescriptor job_type_descriptor = JobTypeDescriptorCollection.GetDescriptor(job_entry.JobType);
                ColorConsole.WriteLine(ConsoleColor.Cyan, $"Running task {job_index}: {job_entry.JobType} for device {job_entry.CPU} ID={job_entry.BootloaderID:X2}...");

                // Get the device
                BootloaderClient device = cnc.Devices.Where(x => x.BootloaderAddress == job_entry.BootloaderID).FirstOrDefault();
                if (device == null && job_type_descriptor.IsPhysicalDeviceNeeded)
                {
                    ColorConsole.WriteLine(ConsoleColor.Yellow, "   No proper device found.");
                    continue;
                }

                if (job_entry.JobType == JobType.ReadEepromMemory)
                {
                    MemoryMap mm = new MemoryMap(job_entry.ProgrammableMemorySize);
                    cnc.ReadEEPROM(device, mm);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Save(job_entry.FileName);
                }

                if (job_entry.JobType == JobType.ReadFlashMemory)
                {
                    MemoryMap mm = new MemoryMap(job_entry.ProgrammableMemorySize);
                    cnc.ReadFLASH(device, mm);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Save(job_entry.FileName);
                }

                if (job_entry.JobType == JobType.WriteEepromMemory)
                {
                    MemoryMap mm = new MemoryMap(job_entry.ProgrammableMemorySize);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Load(job_entry.FileName);
                    cnc.WriteEEPROM(device, mm);
                }


                if (job_entry.JobType == JobType.WriteFlashMemory)
                {
                    MemoryMap mm = new MemoryMap(job_entry.ProgrammableMemorySize);
                    IntelHEX16Storage storage = new IntelHEX16Storage(mm);
                    storage.Load(job_entry.FileName);
                    cnc.WriteFLASH(device, mm);
                }

                if (job_entry.JobType == JobType.Reboot)
                {
                    cnc.Reboot(device);
                }

                if (job_entry.JobType == JobType.WaitForKey)
                {
                    if (!string.IsNullOrEmpty(job_entry.WaitForKeyMessage))
                        ColorConsole.PressAnyKey(job_entry.WaitForKeyMessage);
                    else
                        ColorConsole.PressAnyKey();
                }

                if (job_entry.JobType == JobType.ReadBootloaderVersion)
                {
                    string ver = "";
                    cnc.ReadBootloaderVersion(device, ref ver);
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

        enum X
        {
            a = ConsoleColor.Red
        }

        private static void ShowIntro()
        {
            ColorConsole.WriteLine(ConsoleColor.White, @"  __  __  ___ __  __ ___    ___         _   _             _          ");
            ColorConsole.WriteLine(ConsoleColor.White, @" |  \/  |/ __|  \/  |   \  | _ )___ ___| |_| |___ __ _ __| |___ _ _  ");
            ColorConsole.WriteLine(ConsoleColor.White, @" | |\/| | (__| |\/| | |) | | _ / _ / _ |  _| / _ / _` / _` / -_| '_| ");
            ColorConsole.WriteLine(ConsoleColor.White, @" |_|  |_|\___|_|  |_|___/  |___\___\___/\__|_\___\__,_\__,_\___|_|   ");
            ColorConsole.WriteLine(ConsoleColor.White, @"═════════════════════════════════════════════════════════════════════");
            ColorConsole.WriteLine(ConsoleColor.White, @" Multi-Channel Multi-Device Bootloader, Tomasz Jaworski 2020");
            Console.WriteLine();
        }
    }

}
