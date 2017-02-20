using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TomoTableClient
{

    public class TableImage
    {
        private int[,] data, nmin, nmax;
        Bitmap cached_bitmap;
        int dot_size = 1;
        bool live_normalization;
        int adc_bits;

        public int Height { get { return (int)this.data.GetLongLength(1); } }
        public int Width { get { return (int)this.data.GetLongLength(0); } }

        public int DotSize { get { return this.dot_size; } set { this.dot_size = value; } }

        /// <summary>
        /// Table top image constructor
        /// </summary>
        /// <param name="bh">Number of boards on horizontal axis (width)</param>
        /// <param name="bv">Number of boards on vertial axis (height)</param>
        /// <param name="adc_bits">Number of bits of ADC (8, 10 or 12)</param>
        public TableImage(int bh, int bv, int adc_bits)
        {
            this.data = new int[bh * 10, bv * 10];
            this.nmin = new int[bh * 10, bv * 10];
            this.nmax = new int[bh * 10, bv * 10];

            this.live_normalization = false;
            this.adc_bits = adc_bits;
            this.Clear();

            //
            SetInitialNormalizationValues(0, (int)Math.Pow(2, this.adc_bits));
        }

        public void SetInitialNormalizationValues(int min, int max)
        {
            for (long x = 0; x < this.data.GetLongLength(0); x++)
                for (long y = 0; y < this.data.GetLongLength(1); y++)
                {
                    this.nmin[x, y] = min;
                    this.nmax[x, y] = max;
                }
        }

        private void Clear()
        {
            for (long x = 0; x < this.data.GetLongLength(0); x++)
                for (long y = 0; y < this.data.GetLongLength(1); y++)
                    this.data[x, y] = int.MinValue;
        }

        public void SetIntensity(int x, int y, int value)
        {
            if (x < 0 || x >= this.Width)
                throw new ArgumentOutOfRangeException("x");
            if (y < 0 || y >= this.Height)
                throw new ArgumentOutOfRangeException("y");

            if (y < 0 || y >= Math.Pow(2, this.adc_bits)) // for 12-bit ADC
                throw new ArgumentOutOfRangeException("value");

            this.data[x, y] = value;

            if (this.live_normalization)
            {
                this.nmin[x, y] = value;
                this.nmax[x, y] = value;
            }

            this.nmin[x, y] = Math.Min(this.nmin[x, y], value);
            this.nmax[x, y] = Math.Max(this.nmax[x, y], value);
        }

        public unsafe Bitmap ToBitmap()
        {
            int width = (int)this.data.GetLongLength(0);
            int height = (int)this.data.GetLongLength(1);

            if (this.cached_bitmap != null &&
                (width * this.dot_size != this.cached_bitmap.Width ||
                height * this.dot_size != this.cached_bitmap.Height))
            {
                this.cached_bitmap.Dispose();
                this.cached_bitmap = null;
            }

            if (this.cached_bitmap == null)
                this.cached_bitmap = new Bitmap(width * this.dot_size, height * this.dot_size, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            BitmapData bd = this.cached_bitmap.LockBits(new Rectangle(Point.Empty, this.cached_bitmap.Size), ImageLockMode.ReadWrite, this.cached_bitmap.PixelFormat);

            uint* pimg = (uint*)bd.Scan0;
            for (int px = 0; px < width; px++)
                for (int py = 0; py < height; py++)
                {
                    UInt32 color = 0xFF000000;
                    if (data[px, py] != int.MinValue)
                    {
                        double v = ((double)data[px, py] - (double)nmin[px, py]) / ((double)nmax[px, py] - (double)nmin[px, py]);
                        v = v * 255.0D;
                        UInt32 vi = (UInt32)v;
                        color |= (vi << 16) | (vi << 8) | vi;
                    }
                    else
                        color |= 0xDE6FDE;

                    for (int dx = 0; dx < this.dot_size; dx++)
                        for (int dy = 0; dy < this.dot_size; dy++)
                            pimg[dx + px * this.dot_size + (dy + py * this.dot_size) * bd.Stride / 4] = color;
                }

            this.cached_bitmap.UnlockBits(bd);
            return this.cached_bitmap;
        }
    }
}
