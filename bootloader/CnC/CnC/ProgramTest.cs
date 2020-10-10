using CnC.Jobs;
using IntelHEX;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Newtonsoft.Json.Converters
{
    public sealed class HexStringJsonConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType) => typeof(int).Equals(objectType);

        public override bool CanRead => true;
        public override bool CanWrite => false;

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer) => throw new NotImplementedException();

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            string str = reader.Value as string;
            if (string.IsNullOrEmpty(str) || !str.StartsWith("0x"))
                throw new JsonSerializationException($"Unable to convert string to integer");
            return Convert.ToInt32(str, 16);
        }
    }
}

namespace CnC.Jobs
{
    public enum TaskType
    {
        ProgramFlashMemory,
        ProgramEepromMemory,
    }

    public enum CPUType
    {
        ATMega328P,
    }

    public class TaskEntry
    {
        [JsonProperty("BootloaderID")]
        [JsonConverter(typeof(Newtonsoft.Json.Converters.HexStringJsonConverter))]
        public int BootloaderID { get; set; }

        [JsonProperty("TaskType")]
        [JsonConverter(typeof(StringEnumConverter))]
        public TaskType TaskType { get; set; }

        [JsonProperty("FileName")]
        public string FileName { get; set; }

        [JsonProperty("CPU")]
        [JsonConverter(typeof(StringEnumConverter))]
        public CPUType CPU { get; set; }

        [JsonProperty("ProgrammableMemorySize")]
        //[JsonConverter(typeof(Newtonsoft.Json.Converters.HexStringJsonConverter))]
        public uint ProgrammableMemorySize { get; set; }
    }

    public class TaskContainer
    {
        [JsonProperty("Tasks")]
        public TaskEntry[] Tasks { get; set; }

        public TaskContainer()
        {
            this.Tasks = new TaskEntry[0];
        }
    }

}

namespace CnC
{




    public class BootloaderTaskProvider
    {
        private TaskEntry[] tasks;
        public TaskEntry[] Tasks => this.tasks;

        public int Count => this.tasks.Length;

        public BootloaderTaskProvider()
        {

        }

        public void LoadTasks(string taskDescriptionFile)
        {
            // Read tasks
            Jobs.TaskContainer task_container = null;
            try
            {
                string content = File.ReadAllText(taskDescriptionFile);
                task_container = JsonConvert.DeserializeObject<Jobs.TaskContainer>(content);
            }
            catch (IOException ioex)
            {
                throw new BootloaderException("Configuration load error", ioex);
            }
            catch (JsonException jex)
            {
                throw new BootloaderException("Configuration parsing error", jex);
            }

            // Verify tasks
            for(int i = 0; i < task_container.Tasks.Length; i++)
            {
                TaskEntry task = task_container.Tasks[i];
                try
                {
                    //todo: refactorize
                    if (task.TaskType == TaskType.ProgramEepromMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                        MemoryMap mm = new MemoryMap(65536);
                        IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        s.Load(task.FileName);
                    }

                    if (task.TaskType == TaskType.ProgramFlashMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                        MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                        IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        s.Load(task.FileName);
                    }
                }
                catch (Exception ex)
                {
                    throw new BootloaderException($"Task verification failed ({task.TaskType}, task #{i}): {ex.Message}", ex);
                }
            }

            // Seems ok
            this.tasks = task_container.Tasks;
        }
    }


    [Serializable]
    public class BootloaderException : ApplicationException
    {
        public BootloaderException() { }
        public BootloaderException(string message) : base(message) { }
        public BootloaderException(string message, Exception inner) : base(message, inner) { }
        protected BootloaderException(
          System.Runtime.Serialization.SerializationInfo info,
          System.Runtime.Serialization.StreamingContext context) : base(info, context) { }
    }

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
            }catch(Exception ex)
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
            Console.WriteLine($"   Unique bootloaders): {string.Join(",", btp.Tasks.Select(x => x.BootloaderID.ToString("X2")).Distinct())}");

            //
            // Wait for all serial ports to be connected and identified
            AVRBootloaderCnC cnc = new AVRBootloaderCnC();
            cnc.SendAdvertisementToEveryDetectedPort();

            // Scan all available serial ports for bootloaders
            cnc.AcquireBootloaderDevices((byte)last_bootloader_id);


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
                //fw.Write((uint)pos1 + 4, (byte)dev.address);

                //cnc.WriteFLASH(dev, fw);
                //cnc.VerifyFLASH(dev, fw);
            }


            foreach (Device dev in cnc.Devices)
                cnc.Reset(dev);


        }

    }

}
