using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace CnC.Jobs
{
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

}
