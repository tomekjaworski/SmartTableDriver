using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace CnC.Jobs
{
    public class JobEntry
    {
        [JsonProperty("BootloaderID")]
        [JsonConverter(typeof(Newtonsoft.Json.Converters.HexStringJsonConverter))]
        public byte? BootloaderID { get; set; }

        [JsonProperty("JobType")]
        [JsonConverter(typeof(StringEnumConverter))]
        public JobType JobType { get; set; }

        [JsonProperty("FileName")]
        public string FileName { get; set; }

        [JsonProperty("CPU")]
        [JsonConverter(typeof(StringEnumConverter))]
        public CPUType CPU { get; set; }

        [JsonProperty("ProgrammableMemorySize")]
        public uint ProgrammableMemorySize { get; set; }
     
        [JsonProperty("Message")]
        public string WaitForKeyMessage { get; set; }
    }

}
