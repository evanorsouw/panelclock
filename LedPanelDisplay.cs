using System;
using System.Drawing;
using System.IO.Ports;
using WhiteMagic.PanelClock;

namespace WhiteMagic.PanelClock
{
    public class LedPanelDisplay : IDisplay
    {
        private string _port;
        private SerialPort _comm;
        private int _panelWidth;
        private int _panelHeight;
        private byte[] _writeBuffer;

        public LedPanelDisplay(string port, int dx, int dy)
        {
            _port = port;
            _panelWidth = dx;
            _panelHeight = dy;
            _writeBuffer = new byte[(_panelWidth * 3 + 5) * _panelHeight];
        }

        public int Width => _panelWidth;

        public int Height => _panelHeight;

        public void Show(Bitmap bitmap)
        {
            var bytes = _writeBuffer;
            var i = 0;
            var width = Math.Min(bitmap.Width, _panelWidth);
            var height = Math.Min(bitmap.Height, _panelHeight);
            for (int y = 0; y < height; ++y)
            {
                var addr = y * _panelWidth;
                bytes[i++] = 1;
                bytes[i++] = (byte)(addr >> 8);
                bytes[i++] = (byte)addr;
                bytes[i++] = 2;
                bytes[i++] = (byte)width;
                for (int x = 0; x < bitmap.Width && x < _panelWidth; ++x)
                {
                    var pix = bitmap.GetPixel(x, y);
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

        private void Open()
        {
            Close();
            try
            {
                _comm = new SerialPort(_port, 3750000, Parity.None, 8, StopBits.One);
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
