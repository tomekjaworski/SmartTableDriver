using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Net.Sockets;
using System.Drawing.Imaging;

namespace ImageDebuggerServer
{

    public struct BitmapInfo
    {
        public Bitmap image;
        public float min;
        public float max;
        public string name;

    }

    public class DImage
    {
        public Size size;
        public ChannelType channel_type;
        public ImageType type;
        public PaletteType palette_type;
        public string name;

        byte[] recv_buffer;

        public DImage()
        {
            this.size = Size.Empty;

            this.channel_type = ChannelType.Invalid;
            this.type = ImageType.Invalid;
            this.palette_type = PaletteType.Invalid;
        }

        public int CalcRawDataSize()
        {
            int csize = 0;
            if (this.channel_type == ChannelType.U8) csize = 1;
            if (this.channel_type == ChannelType.U16) csize = 2;
            if (this.channel_type == ChannelType.U32) csize = 4;
            if (this.channel_type == ChannelType.Float) csize = 4;
            if (this.channel_type == ChannelType.Double) csize = 8;

            int channels = 0;
            if (this.type == ImageType.Luminance) channels = 1;

            return channels * csize * this.size.Height * this.size.Width;
        }

        internal byte[] ReadStream(Socket s)
        {
            int bytes_left = this.CalcRawDataSize();
            this.recv_buffer = new byte[bytes_left];

            int recv_pos = 0;
            while (bytes_left > 0)
            {
                int recvd = s.Receive(recv_buffer, recv_pos, bytes_left, SocketFlags.None);
                recv_pos += recvd;
                bytes_left -= recvd;
            }


            return recv_buffer;
        }

        internal unsafe BitmapInfo GetBitmapInfo()
        {
            BitmapInfo bi = new BitmapInfo();
            Bitmap bmp = new Bitmap(this.size.Width, this.size.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            BitmapData bd = bmp.LockBits(new Rectangle(Point.Empty, bmp.Size), ImageLockMode.WriteOnly, bmp.PixelFormat);

            UInt32* dest = (UInt32*)bd.Scan0;
            int h = size.Height;
            int w = size.Width;

            fixed (void* src = this.recv_buffer)
            {

                if (this.type == ImageType.Luminance)
                {
                    UInt32[] palette = Palette.GetPalette(palette_type);
                    int pmax = palette.Length - 1;

                    if (this.channel_type == ChannelType.Float)
                    {
                        float min, max;
                        float* fsrc = (float*)src;
                        CalculateRange(fsrc, this.size, out min, out max);

                        if (min == max)

                            for (int i = 0; i < w * h; i++)
                            {
                                *dest = palette[0];
                                dest++;
                            }
                        else
                            for (int r = 0; r < h; r++)
                            {
                                for (int c = 0; c < w; c++)
                                {
                                    int value = (int)(((*fsrc - min) * pmax) / (max - min));
                                    if (value >= 0 && value <= pmax)
                                        *dest = palette[value];

                                    fsrc++;
                                    dest++;
                                }
                            }

                        bi.min = min;
                        bi.max = max;

                    }

                    // -----------------------------


                    if (this.channel_type == ChannelType.U16)
                    {
                        ushort umin, umax;
                        ushort* ussrc = (ushort*)src;
                        CalculateRange(ussrc, this.size, out umin, out umax);

                        if (umin == umax)

                            for (int i = 0; i < w * h; i++)
                            {
                                *dest = palette[0];
                                dest++;
                            }
                        else
                            for (int r = 0; r < h; r++)
                            {
                                for (int c = 0; c < w; c++)
                                {
                                    int value = (((int)*ussrc - umin) * pmax) / (umax - umin);
                                    if (value >= 0 && value <= pmax)
                                        *dest = palette[value];
                                    ussrc++;
                                    dest++;
                                }
                            }

                        bi.min = umin;
                        bi.max = umax;
                    }

                    // -----------------------------

                    if (this.channel_type == ChannelType.U8)
                    {
                        byte bmin, bmax;
                        byte* bsrc = (byte*)src;
                        CalculateRange(bsrc, this.size, out bmin, out bmax);

                        if (bmin == bmax)

                            for (int i = 0; i < w * h; i++)
                            {
                                *dest = palette[0];
                                dest++;
                            }
                        else
                            for (int r = 0; r < h; r++)
                            {
                                for (int c = 0; c < w; c++)
                                {
                                    int value = (((int)*bsrc - bmin) * pmax) / (bmax - bmin);
                                    if (value >= 0 && value <= pmax)
                                        *dest = palette[value];
                                    bsrc++;
                                    dest++;
                                }
                            }

                        bi.min = bmin;
                        bi.max = bmax;
                    }
                }
            }


            bmp.UnlockBits(bd);

            bi.image = bmp;
            bi.name = this.name;
            return bi;
        }

        public unsafe void CalculateRange(float*src, Size size, out float min, out float max)
        {
            max = float.MinValue;
            min = float.MaxValue;

            int c = size.Height * size.Width;
            while (c-- > 0)
            {
                if (*src > max)
                    max = *src;
                if (*src < min)
                    min = *src;

                src++;
            }
        }

        public unsafe void CalculateRange(ushort* src, Size size, out ushort min, out ushort max)
        {
            max = ushort.MinValue;
            min = ushort.MaxValue;

            int c = size.Height * size.Width;
            while (c-- > 0)
            {
                if (*src > max)
                    max = *src;
                if (*src < min)
                    min = *src;

                src++;
            }
        }

        public unsafe void CalculateRange(byte* src, Size size, out byte min, out byte max)
        {
            max = byte.MinValue;
            min = byte.MaxValue;

            int c = size.Height * size.Width;
            while (c-- > 0)
            {
                if (*src > max)
                    max = *src;
                if (*src < min)
                    min = *src;

                src++;
            }
        }

    }

}
