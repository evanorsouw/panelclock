using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public interface IDisplay
    {
        int Width { get; }
        int Height { get; }
        void Show(Bitmap bitmap);
    }
}
