namespace CnC.Jobs
{
    public enum JobType
    {
        WriteFlashMemory,
        Reboot,

        ReadFlashMemory,
        ReadEepromMemory,
        WriteEepromMemory,

        ReadBootloaderVersion,
        WaitForKey,
    }

}
