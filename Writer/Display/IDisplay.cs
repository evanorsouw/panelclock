using System.Drawing;

namespace WhiteMagic.PanelClock.Display
{
    public interface IDisplay
    {
        int Width { get; }
        int Height { get; }
        void Show(Bitmap bitmap);
    }
}
