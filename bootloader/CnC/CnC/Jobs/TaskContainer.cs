using Newtonsoft.Json;

namespace CnC.Jobs
{
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
