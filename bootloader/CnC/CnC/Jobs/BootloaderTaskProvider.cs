using CnC.Jobs;
using IntelHEX;
using Newtonsoft.Json;
using System;
using System.IO;


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
                throw new BootloaderException($"Configuration load error: {ioex.Message}", ioex);
            }
            catch (JsonException jex)
            {
                throw new BootloaderException($"Configuration parsing error: {jex.Message}", jex);
            }

            // Verify tasks
            for(int i = 0; i < task_container.Tasks.Length; i++)
            {
                TaskEntry task = task_container.Tasks[i];
                try
                {
                    //todo: refactorize
                    if (task.TaskType == TaskType.WriteEepromMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                        MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                        IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        s.Load(task.FileName);
                    }

                    if (task.TaskType == TaskType.WriteFlashMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                        MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                        IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        s.Load(task.FileName);
                    }
                    if (task.TaskType == TaskType.ReadFlashMemory || task.TaskType == TaskType.ReadEepromMemory)
                    {
                        //TODO: replace fake load into fake memory with a clear verification procedure
                      //  MemoryMap mm = new MemoryMap(task.ProgrammableMemorySize);
                       // IntelHEX16Storage s = new IntelHEX16Storage(mm);
                        //s.Load(task.FileName);
                    }
                    if (task.TaskType == TaskType.Reboot)
                    {
                        //?
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

}
