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
            bytes[i++] = 64;
            bytes[i++] = 64;
            for (int y = 0; y < 64; ++y)
            {
                for (int x = 0; x < 64; ++x)
                {
                    var pix = bitmap.GetPixel(32+x, y);
                    bytes[i++] = pix.R;
                    bytes[i++] = pix.G;
                    bytes[i++] = pix.B;
                }
            }

            for (int j=0; j<i; ++j) 
            {
                _comm.Write(_writeBuffer, j, 1);
            }
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
