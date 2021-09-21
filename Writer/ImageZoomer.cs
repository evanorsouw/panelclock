using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public interface IImageZoomer
    {
        public RectangleF GetRectangle(float progress); 
    }
}
