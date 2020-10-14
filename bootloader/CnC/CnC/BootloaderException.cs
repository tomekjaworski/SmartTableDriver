using System;


namespace CnC
{
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

}
