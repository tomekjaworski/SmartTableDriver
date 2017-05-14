using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ImageDebuggerServer
{
    public partial class ImageBrowser : UserControl
    {
        public ImageBrowser()
        {
            InitializeComponent();
        }

        public void SetStatusText(string str)
        {
            this.label1.Text = str;
        }

        public void SetImage(Image img)
        {
            this.pictureBox1.Image = img;
            this.pictureBox1.Invalidate();
        }

        public void Clear()
        {
            this.pictureBox1.Image = null;
            this.pictureBox1.Invalidate();
        }

        public enum ZoomMode
        {
            Normal,
            Fit
        }

        public void Zoom(ZoomMode mode)
        {
            if (mode == ZoomMode.Normal)
            {
                this.pictureBox1.SizeMode = PictureBoxSizeMode.AutoSize;
                this.pictureBox1.Dock = DockStyle.None;
                this.pictureBox1.Location = Point.Empty;
                this.pictureBox1.Invalidate();
            }

            if (mode == ZoomMode.Fit)
            {
                this.pictureBox1.SizeMode = PictureBoxSizeMode.Zoom;
                this.pictureBox1.Dock = DockStyle.Fill;
                this.pictureBox1.Invalidate();
            }

        }


        internal Bitmap GetImage()
        {
            //throw new NotImplementedException();
            return this.pictureBox1.Image as Bitmap;
        }
    }
}
