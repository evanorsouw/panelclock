using System;
using System.Drawing;
using System.IO.Ports;
using System.Threading;

namespace WhiteMagic.PanelClock.Display
{
    public class LedPanelDisplay : IDisplay
    {
        private string _port;
        private SerialPort _comm;
        private int _hPanels;
        private int _vPanels;
        private byte[] _writeBuffer;
        private bool _initialized;

        public LedPanelDisplay(string port, int dx, int dy)
        {
            _port = port;

            if (dx % 64 != 0 || dy % 64 != 0)
                throw new Exception("display dimensions must be multiples of 64");

            _hPanels = dx/64;
            _vPanels = dy/64;

            _writeBuffer = new byte[64 * (64 * 3 + 5) * _vPanels * _hPanels];
            _initialized = false;
        }

        public int Width => _hPanels*64;

        public int Height => _vPanels*64;

        public void Show(Bitmap bitmap)
        {
            if (!_initialized)
            {
                _initialized = true;
                Initialize();
            }
            var bytes = _writeBuffer;
            var i = 0;
            bytes[i++] = 1;
            bytes[i++] = 0;
            bytes[i++] = 0;
            for (int y = 0; y < 64; ++y)
            {
                bytes[i++] = 2;
                bytes[i++] = (byte)128;
                for (int x = 0; x < 128; ++x)
                {
                    var pix = bitmap.GetPixel(x, y);
                    //bytes[i++] = (byte)(255 - pix.R);
                    //bytes[i++] = (byte)(255 - pix.G);
                    //bytes[i++] = (byte)(255 - pix.B);
                    bytes[i++] = pix.R;
                    bytes[i++] = pix.G;
                    bytes[i++] = pix.B;
                }
            }
            WriteBytes(_writeBuffer, i);
        }

        public void SetBrightness(int b)
        {
            WriteBytes(new byte[] { 4, (byte)b }, 2);
        }

        public void SetPixel(int x, int y, int color)
        {
            SetPixel(x * _hPanels * 64, y, color);
        }

        public void SetPixel(int addr, int color)
        {
            var bytes = new byte[20];

            int i = 0;
            bytes[i++] = 1;
            bytes[i++] = (byte)(addr >> 8);
            bytes[i++] = (byte)addr;
            bytes[i++] = 2;
            bytes[i++] = (byte)1;
            bytes[i++] = (byte)((color >> 16) & 0xFF);
            bytes[i++] = (byte)((color >> 8) & 0xFF);
            bytes[i++] = (byte)((color >> 0) & 0xFF);
            WriteBytes(bytes, i);
        }

        private void Initialize()
        {
            var buffer = new byte[1024];

            var i = 0;
            for (var j=0; j<256; ++j)
            {
                buffer[i++] = 255;
            }

            WriteBytes(buffer, i);
            Thread.Sleep(100);
        }

        private void Open()
        {
            Close();
            try
            {
                _comm = new SerialPort(_port, 3000000, Parity.None, 8, StopBits.One);
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
    }
}
