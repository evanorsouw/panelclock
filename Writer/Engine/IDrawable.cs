using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public interface IDrawable
    {
        /// <summary>Draw this item using the provided graphics instance.</summary>
        /// <param name="g">graphics item to draw</param>
        void Draw(Graphics g);
    }
}
