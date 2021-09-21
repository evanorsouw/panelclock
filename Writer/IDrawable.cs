using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public interface IDrawable
    {
        /// <summary>
        /// The unique identification of this item
        /// </summary>
        public string Id { get; }

        /// <summary>Clone into a identical but separate instance</summary>
        /// <param name="id">the identification of the newly create instance</param>
        /// <returns>cloned copy</returns>
        IDrawable Clone(string id);

        /// <summary>Draw this item using the provided graphics instance.</summary>
        /// <param name="g">graphics item to draw</param>
        void Draw(Graphics g);
    }
}
