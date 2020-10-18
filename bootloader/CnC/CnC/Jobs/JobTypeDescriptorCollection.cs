using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CnC.Jobs
{
    public static class JobTypeDescriptorCollection
    {
        static Dictionary<JobType, JobTypeDescriptor> dict;

        static JobTypeDescriptorCollection()
        {
            dict = new Dictionary<JobType, JobTypeDescriptor>();


            dict[JobType.ReadBootloaderVersion] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = true
            };

            dict[JobType.ReadEepromMemory] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = true
            };

            dict[JobType.ReadFlashMemory] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = true
            };

            dict[JobType.Reboot] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = true
            };

            dict[JobType.WaitForKey] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = false
            };

            dict[JobType.WriteEepromMemory] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = true
            };

            dict[JobType.WriteFlashMemory] = new JobTypeDescriptor
            {
                IsPhysicalDeviceNeeded = true
            };

        }

        internal static JobTypeDescriptor GetDescriptor(JobType jobType)
        {
            if (dict.ContainsKey(jobType))
                return dict[jobType];

            throw new ArgumentException($"Job type {jobType} has no descriptor.");
        }
    }
}
