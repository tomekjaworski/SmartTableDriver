using Newtonsoft.Json;

namespace CnC.Jobs
{
    public class JobEntryCollection
    {
        [JsonProperty("Jobs")]
        public JobEntry[] Jobs { get; set; }

        public JobEntryCollection()
        {
            this.Jobs = new JobEntry[0];
        }
    }

}
