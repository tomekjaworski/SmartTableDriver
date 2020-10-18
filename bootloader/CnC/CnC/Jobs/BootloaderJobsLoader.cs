using CnC.Jobs;
using IntelHEX;
using Newtonsoft.Json;
using System;
using System.IO;


namespace CnC
{
    public class BootloaderJobsLoader
    {
        private JobEntry[] jobs;
        public JobEntry[] Jobs => this.jobs;

        public int Count => this.jobs.Length;

        public BootloaderJobsLoader()
        {

        }

        public void LoadTasks(string taskDescriptionFile)
        {
            // Read tasks
            Jobs.JobEntryCollection collection = null;
            try
            {
                string content = File.ReadAllText(taskDescriptionFile);
                collection = JsonConvert.DeserializeObject<Jobs.JobEntryCollection>(content);
            }
            catch (IOException ioex)
            {
                throw new BootloaderException($"Job list load error: {ioex.Message}", ioex);
            }
            catch (JsonException jex)
            {
                throw new BootloaderException($"Job list parsing error: {jex.Message}", jex);
            }

            // Verify jobs
            for(int i = 0; i < collection.Jobs.Length; i++)
            {
                JobEntry task = collection.Jobs[i];
                try
                {
                    //todo: refactorize
                    if (task.JobType == JobType.WriteEepromMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                        MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                        IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        s.Load(task.FileName);
                    }

                    if (task.JobType == JobType.WriteFlashMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                        MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                        IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        s.Load(task.FileName);
                    }
                    if (task.JobType == JobType.ReadFlashMemory || task.JobType == JobType.ReadEepromMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                      //  MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                       // IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        //s.Load(task.FileName);
                    }
                    if (task.JobType == JobType.Reboot)
                    {
                        //?
                    }
                }
                catch (Exception ex)
                {
                    throw new BootloaderException($"Job verification failed ({task.JobType}, task #{i}): {ex.Message}", ex);
                }
            }

            // Seems ok
            this.jobs = collection.Jobs;
        }
    }

}
