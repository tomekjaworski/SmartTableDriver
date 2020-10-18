using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CnC.Jobs
{
    public struct JobTypeDescriptor
    {
        // Job needs a physical and conneced device to be executed. For example a memory read/write command.
        // In contary a key press job does not need a device.
        public bool IsPhysicalDeviceNeeded { get; set; }
    }
}
