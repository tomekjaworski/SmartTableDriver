using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TomoTableClient
{
    public partial class Form1 : Form
    {
        TableImage ti;
        FileStream fs;
        StreamReader sr;
        public Form1()
        {
            this.ti = new TableImage(1, 1, 10);
            ti.DotSize = 10;
            InitializeComponent();

            ti.SetIntensity(0, 0, 0);
            ti.SetInitialNormalizationValues(0, 300);

            fs = new FileStream("dane_od_izy.txt", FileMode.Open, FileAccess.Read);
            sr = new StreamReader(fs);


            this.pictureBox1.Image = ti.ToBitmap();
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            bool new_frame = false;
            bool frame_data = false;
            int row = 0;
            while (true)
            {
                string line = await sr.ReadLineAsync();
                if (line == null)
                    break; // end of stream

                line = line.Trim();
                if (line == "")
                {
                    if (frame_data)
                        break;
                    new_frame = true;
                    row = 0;
                    continue;
                }

                string[] sv = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                int[] measures = sv.Select(x => int.Parse(x)).ToArray();

                for (int i = 0; i < 10; i++)
                    this.ti.SetIntensity(i, row, measures[i]);

                row++;
                frame_data = true;
            }

            this.pictureBox1.Image = ti.ToBitmap();

        }

    }


}
