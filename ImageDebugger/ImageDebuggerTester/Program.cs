using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Net;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Runtime.CompilerServices;
using System.Runtime.ConstrainedExecution;
using System.Security.Permissions;

namespace ImageDebuggerTester
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static unsafe void Main()
        {
            Random rnd = new Random();

            while (true)
            {

                int h = rnd.Next(10, 512);
                int w = rnd.Next(10, 512);
                float[] f = new float[h * w];
                for (int i = 0; i < w; i++)
                    for (int j = 0; j < h; j++)
                        f[i + w * j] = ((float)i / (float)(w - 1)) * ((float)j / (float)(h - 1));


                Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                s.Connect(new System.Net.IPEndPoint(IPAddress.Loopback, 7391));

                WriteLine(s, "height {0}", h);
                WriteLine(s, "width {0}", w);
                WriteLine(s, "ctype {0}", "Float");
                WriteLine(s, "itype {0}", "Luminance");
                WriteLine(s, "palette {0}", "Iron");
                WriteLine(s, "data");

                fixed (float* pf = f)
                    SendBuffer(s, (byte*)pf, h * w * 4);


                Thread.Sleep(100);
            }
        }

        private static unsafe void WriteLine(Socket s, string fmt, params object[] args)
        {
            string line = string.Format(fmt, args)+"\n";
            byte[] b = Encoding.ASCII.GetBytes(line);
            fixed (byte* ptr = b)
                SendBuffer(s, ptr, b.Length);
        }

        private static unsafe void SendBuffer(Socket s, byte* ptr, int p)
        {
            while (p > 0)
            {
                int sent = send(s.Handle, ptr, p, SocketFlags.None);
                if (sent == -1)
                {
                    int error_code = Marshal.GetLastWin32Error();
                    throw new SocketException(error_code);
                }

                p -= sent;
                ptr += sent;
            }
        }

        [DllImport("ws2_32.dll", SetLastError = true)]
        internal static extern unsafe int send([In] IntPtr socketHandle, [In] byte* pinnedBuffer, [In] int len, [In] SocketFlags socketFlags);

        [MethodImpl(MethodImplOptions.InternalCall), ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success), SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.UnmanagedCode)]
        public static extern int GetLastWin32Error();
 

 

    }
}
