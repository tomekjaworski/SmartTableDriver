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

        //public override bool CanRead => true;
        //public override bool CanWrite => false;

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            throw new NotImplementedException();
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            string str = reader.Value as string;
            if (string.IsNullOrEmpty(str) || !str.StartsWith("0x"))
                throw new JsonSerializationException($"Unable to convert string to integer");
            return Convert.ToUInt32(str, 16);
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
    }

    public class TaskContainer
    {
        [JsonProperty("Tasks")]
        public TaskEntry[] Tasks { get; set; }
    }

}

namespace CnC
{




    public class MultibootController
    {
        public MultibootController()
        {

        }

        public void ReadTasks(string taskDescriptionFile)
        {
            try
            {
                string content = File.ReadAllText(taskDescriptionFile);
                Jobs.TaskContainer task_container = JsonConvert.DeserializeObject<Jobs.TaskContainer>(content);

            }
            catch (IOException ioex)
            {
                throw new BootloaderException("Configuration load error", ioex);
            }
            catch (JsonException jex)
            {
                throw new BootloaderException("Configuration parsing error", jex);
            }
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

            Console.WriteLine("SmartTable bootloader C&C software by Tomasz Jaworski");
            random = new Random();

            MultibootController mbc = new MultibootController();
            mbc.ReadTasks("bootloader_tasks.json");




            IntelHEX16Storage loader;
            MemoryMap memory_flash = new MemoryMap(32 * 1024 - 2 * 1024);
            MemoryMap memory_eeprom = new MemoryMap(0x400);
            loader = new IntelHEX16Storage(memory_flash);
            //loader.Load(f1);

            loader = new IntelHEX16Storage(memory_eeprom);
            //loader.Load(f2);



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
