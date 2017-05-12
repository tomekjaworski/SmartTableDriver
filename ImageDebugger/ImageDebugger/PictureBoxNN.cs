using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Text;
using System.Windows.Forms;

namespace ImageDebuggerServer
{

    public class PictureBoxNN : PictureBox
    {
        private Image img;
        private InterpolationMode interpolation_mode;
        private PictureBoxSizeMode size_mode;

        public new Image Image {  get { return this.img; }
        set {
                this.img = value;
                this.UpdateImage();
            }
        }

        public new PictureBoxSizeMode SizeMode {
            get { return this.size_mode; }
            set {
                this.size_mode = value;
                this.UpdateImage();
                base.SizeMode = size_mode;
            }
        }

        public InterpolationMode InterpolationMode {
            get { return this.interpolation_mode; }
            set {
                this.interpolation_mode = value;
                this.Invalidate();
            }
        }



        public Bitmap ScaleBitmap(Bitmap source, Size new_size)
        {
            Bitmap bmp = new Bitmap(ClientSize.Width, ClientSize.Height, source.PixelFormat);
            Graphics g = Graphics.FromImage(bmp);
            Size source_size = source.Size;
            Rectangle dest = new Rectangle();

            float ratio = Math.Min((float)ClientRectangle.Height / (float)source_size.Height, (float)ClientRectangle.Width / (float)source_size.Width);
            dest.Width = (int)(ratio * source_size.Width);
            dest.Height = (int)(ratio * source_size.Height);
            dest.X = (ClientRectangle.Width - dest.Width) / 2;
            dest.Y = (ClientRectangle.Height - dest.Height) / 2;

            g.InterpolationMode = interpolation_mode;
            g.PixelOffsetMode = PixelOffsetMode.Half;
            g.DrawImage(source, dest, new Rectangle(0, 0, source.Width, source.Height), GraphicsUnit.Pixel);
            g.Dispose();
            return bmp;
        }

        private void UpdateImage()
        {
            Bitmap bmp = this.img as Bitmap;
            if (bmp == null)
                return;

            if (this.size_mode == PictureBoxSizeMode.Zoom)
                base.Image = ScaleBitmap(bmp, ClientSize);
            else
                base.Image = bmp;
        }

        protected override void OnPaint(PaintEventArgs paintEventArgs)
        {
            //paintEventArgs.Graphics.InterpolationMode = InterpolationMode;
            base.OnPaint(paintEventArgs);
        }

        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            this.UpdateImage();
            this.Invalidate();
        }
    }
}
