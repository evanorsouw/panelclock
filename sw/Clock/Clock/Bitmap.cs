
using System.Drawing;
using System.IO.Ports;
using System.Reflection.Metadata.Ecma335;

namespace Clock
{
    public class Bitmap
    {
        private int _dx;
        private int _dy;
        private string _port = "com9";
        private SerialPort _comm;

        public Bitmap(int dx, int dy)
        {
            _dx = dx;
            _dy = dy;
        }

        public void SetPixel(int x, int y, int color)
            => SetPixel(x, y, color, 63);

        private int _lastcolor;
        private int _lastx;
        private int _lasty;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="color"></param>
        /// <param name="alpha">alphfa value 0=transparant to oversample=opaque</param>
        public void SetPixel(int x, int y, int color, int alpha)
        {
            x /= 64;
            y /= 64;

            alpha = Math.Min(63, Math.Max(0, alpha));

            if (_lastcolor != color)
            {
                WriteBytes(0x08, (byte)(color >> 16), (byte)(color >> 8), (byte)color);
                _lastcolor = color;
            }
//            alpha = 63;
            if (_lasty != y || _lastx+1 != x)
            {
                _lasty = y;
                _lastx = x;
                WriteBytes((byte)(0xC0 | alpha), (byte)x, (byte)y);
            }
            else
            {
                _lastx = (_lastx + 1) & 0xff;
                WriteBytes((byte)(0x80 | alpha));
            }
        }

        public void Rect(int x, int y, int dx, int dy, int color)
        {
            WriteBytes(0x02, (byte)x, (byte)y, (byte)dx, (byte)dy, (byte)(color >> 16), (byte)(color >> 8), (byte)color);
        }

        public void Clear()
        {
            var buf = new byte[] { 2, 0, 0, 64, 64, 0, 0, 0 };
            WriteBytes(buf, buf.Length);
        }

        public void SelectScreen(int d, int w)
        {
            WriteBytes(0x09, (byte)((d & 15) | ((w & 15) << 4)));
        }

        private void Open()
        {
            Close();
            try
            {
                _comm = new SerialPort(_port, 115200, Parity.None, 8, StopBits.One);
                _comm.Open();
            }
            catch (Exception ex)
            {

                Console.WriteLine($"Failed to open commport='{_port}': {ex.Message}");
            }
        }

        private void Close()
        {
            if (_comm != null)
            {
                try
                {
                    _comm.Close();
                }
                catch (Exception)
                {
                    throw;
                }
                _comm = null;
            }
        }

        private void WriteBytes(params byte[] bytes)
            => WriteBytes(bytes, bytes.Length);

        private void WriteBytes(byte[] buffer, int n)
        {
            if (_comm == null || !_comm.IsOpen)
            {
                Open();
            }
            try
            {
                _comm.Write(buffer, 0, n);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to end count='{n}' bytes to led panel: {ex.Message}");
                Close();
            }
        }
    };
}
