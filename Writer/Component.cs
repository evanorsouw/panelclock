using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public abstract class Component : ValueSource, IComponent
    {
        public Component (string id) : base(id)
        {
        }

        public abstract IComponent Clone(string id);

        public abstract void Draw(Graphics g);
    }
}
