using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Net;
using System.IO;
using System.Drawing.Imaging;
using System.Diagnostics;
using Microsoft.Win32;

namespace ImageDebuggerServer
{
    public partial class Form1 : Form
    {
        TcpListener listener;
        private string path;

        private delegate void StringDelegate(string str);
        private delegate void BitmapInfoDelegate(BitmapInfo bi);

        public Form1()
        {
            this.InitializeComponent();

            int port = 7391;

            RegistryKey rk = Registry.CurrentUser.CreateSubKey("Software\\TJaworski\\ImageDebugger");
            rk.SetValue("Path", Application.ExecutablePath);
            rk.SetValue("Port", port, RegistryValueKind.DWord);


            this.listener = new TcpListener(IPAddress.Any, port);
            this.listener.Start();
            this.listener.BeginAcceptSocket(new AsyncCallback(CallbackAccept), null);
            this.button2.Tag = 0;
            Debug("Serwer uruchomiony...");

            this.path = null;
        }

        void Debug(String line)
        {
            if (this.InvokeRequired)
                this.Invoke(new StringDelegate(Debug), line);
            else
            {

                if (line.StartsWith("+"))
                    this.textBox1.AppendText(line.Substring(1));
                else
                {
                    if (this.textBox1.Text.Length != 0)
                        this.textBox1.AppendText("\r\n");

                    this.textBox1.AppendText(DateTime.Now.ToString("HH:mm:ss") + " " + line);
                }

                this.textBox1.ScrollToCaret();
            }
        }

        void DispatchBitmapInfo(BitmapInfo bi)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new BitmapInfoDelegate(DispatchBitmapInfo), bi);
                return;
            }

            TabPage p = null;
            string name = bi.name;
            if (String.IsNullOrEmpty(name))
                name = "Image";
            foreach(TabPage page in this.tabControl1.TabPages)
                if (page.Text == bi.name)
                {
                    p = page;
                    break;
                }

            ImageBrowser ib = null;
            if (p == null)
            {
                p = new TabPage(name);
                this.tabControl1.TabPages.Add(p);

                ib = new ImageBrowser();
                p.Controls.Add(ib);
                ib.Dock = DockStyle.Fill;
            }
            else
                ib = p.Controls[0] as ImageBrowser;

            ib.SetImage(bi.image);
            ib.SetStatusText(string.Format("size={2}; min={0:N3}; max={1:N3}", bi.min, bi.max, bi.image.Size));
            ib.Invalidate();
        }


        void CallbackAccept(IAsyncResult ar)
        {
            try
            {
                Socket cli = this.listener.EndAcceptSocket(ar);
                Debug("Połączenie z " + cli.RemoteEndPoint.ToString());
                //cli.ReceiveTimeout = 5000;

                this.ProcessConnection(cli);


            }
            catch (Exception e)
            {
                //
                Debug(e.Message);
            }

            this.listener.BeginAcceptTcpClient(new AsyncCallback(CallbackAccept), null);

        }

        private void ProcessConnection(Socket cli)
        {

            DImage img = new DImage();
            byte[] b = new byte[1];

            while (true)
            {
                string line = "";
                while (true)
                {
                    cli.Receive(b);
                    if (b[0] == '\n')
                        break;
                    line += (char)b[0];
                }

                string[] command = line.Split(new char[]{' '}, StringSplitOptions.RemoveEmptyEntries);
                if (command.Length == 0)
                    continue;

                if (command[0] == "height")
                    img.size.Height = int.Parse(command[1]);
                if (command[0] == "name")
                    img.name = line.Substring(4).Trim();
                if (command[0] == "width")
                    img.size.Width = int.Parse(command[1]);
                if (command[0] == "ctype") // channel type
                    img.channel_type = (ChannelType)Enum.Parse(typeof(ChannelType), command[1]);
                if (command[0] == "itype") // image type
                    img.type = (ImageType)Enum.Parse(typeof(ImageType), command[1]);
                if (command[0] == "palette") // image type
                    img.palette_type = (PaletteType)Enum.Parse(typeof(PaletteType), command[1]);

                if (command[0] == "data") // image type
                {
                    Debug("Pobieranie obrazu... ");
                    img.ReadStream(cli);
                    cli.Close();

                    Debug("+OK");
                    BitmapInfo bi = img.GetBitmapInfo();

                    this.DispatchBitmapInfo(bi);
                    return;
                }

            }

        }

        private void button1_Click(object sender, EventArgs e)
        {
            while (this.tabControl1.TabPages.Count > 0)
                this.tabControl1.TabPages.RemoveAt(0);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            ImageBrowser.ZoomMode zm = ImageBrowser.ZoomMode.Fit;
            if ((int)button2.Tag == 1)
            {
                zm = ImageBrowser.ZoomMode.Normal;
                button2.Tag = 2;
            }
            else
            {
                zm = ImageBrowser.ZoomMode.Fit;
                button2.Tag = 1;
            }

            foreach (TabPage page in this.tabControl1.TabPages)
            {
                ImageBrowser ib = page.Controls[0] as ImageBrowser;
                ib.Zoom(zm);
            }

            this.Invalidate();
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            if (this.path == null)
                this.path = Path.GetDirectoryName(Application.ExecutablePath);

            if (this.tabControl1.SelectedTab == null)
                return;

            using (SaveFileDialog dg = new SaveFileDialog())
            {
                dg.Filter = "Pliki PNG (*.png)|*.png|Pliki BMP (*.bmp)|*.bmp";
                dg.FilterIndex = 0;
                dg.Title = "Zapisz obraz jako...";
                dg.InitialDirectory = this.path;

                dg.FileName = this.tabControl1.SelectedTab.Text + ".png";

                if (dg.ShowDialog() != DialogResult.OK)
                    return;

                this.path = Path.GetDirectoryName(dg.FileName);

                ImageBrowser ib = this.tabControl1.SelectedTab.Controls[0] as ImageBrowser;
                Bitmap bmp = ib.GetImage();

                switch (Path.GetExtension(dg.FileName).ToLower())
                {
                    case ".png":
                        bmp.Save(dg.FileName, ImageFormat.Png);
                        break;
                    case ".jpeg":
                    case ".jpg":
                        bmp.Save(dg.FileName, ImageFormat.Jpeg);
                        break;
                    case ".bmp":
                        bmp.Save(dg.FileName, ImageFormat.Bmp);
                        break;
                    case ".tiff":
                    case ".tif":
                        bmp.Save(dg.FileName, ImageFormat.Tiff);
                        break;

                    default:
                        string f = Path.GetFileNameWithoutExtension(dg.FileName) + ".png";
                        bmp.Save(f, ImageFormat.Png);
                        break;
                }
            }
        }



    }
}
