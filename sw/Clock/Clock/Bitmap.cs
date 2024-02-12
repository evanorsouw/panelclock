
using System.Drawing;
using System.IO.Ports;
using System.Reflection.Metadata.Ecma335;

namespace Clock
{
    public class Bitmap
    {
        private int _dx;
        private int _dy;
        private byte[] _mem;
        private int _oversample;
        private int _oversample2;
        private string _port = "com9";
        private SerialPort _comm;

        public Bitmap(int dx, int dy, int oversample)
        {
            _dx = dx;
            _dy = dy;
            _mem = new byte[5 + dx * dy * 3];
            _oversample = oversample;
            _oversample2 = (oversample - 1) * (oversample - 1);

            _mem[0] = 0x01;
            _mem[1] = 0;
            _mem[2] = 0;
            _mem[3] = (byte)(dx);
            _mem[4] = (byte)(dy);
        }

        public byte[] Command => _mem;

        public int CommandSize => _mem.Length;

        public void SetPixel(int x, int y, int color)
            => SetPixel(x, y, color, _oversample-1, _oversample-1);

        private int _lastcolor;
        private int _lastx;
        private int _lasty;

        public void SetPixel(int x, int y, int color, int dx, int dy)
        {
            x /= _oversample;
            y /= _oversample;
            var i = 5 + (y * _dx + x) * 3;

            var scale = dx * dy;
            var alpha = 63 * scale / _oversample2;
            alpha = Math.Min(63, Math.Max(0, alpha));
            byte r = clip(color >> 16, scale);
            byte g = clip(color >> 8, scale);
            byte b = clip(color, scale);

            _mem[i + 0] = r;
            _mem[i + 1] = g;
            _mem[i + 2] = b;

            if (_lastcolor != color)
            {
                WriteBytes(0x08, (byte)(color >> 16), (byte)(color >> 8), (byte)color);
                _lastcolor = color;
            }
            if (_lasty != y || _lastx+1 != x)
            {
                _lasty = y;
                _lastx = x;
                WriteBytes((byte)(0xC0 | alpha & 0x3F), (byte)x, (byte)y);
            }
            else
            {
                _lastx = (_lastx + 1) & 0xff;
                WriteBytes((byte)(0x80 | alpha));
            }
        }

        public void Clear()
        {
            Array.Clear(_mem, 5, _mem.Length - 5);
            var buf = new byte[] { 2, 0, 0, 64, 64, 0, 0, 0 };
            WriteBytes(buf, buf.Length);
        }

        public void SelectScreen(int d, int w)
        {
            WriteBytes(0x09, (byte)((d & 15) | ((w & 15) << 4)));
        }

        private byte clip(int v, int scale)
        {
            var v2 = (byte)((v & 0xFF) * scale / _oversample2);
            return v2;
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
