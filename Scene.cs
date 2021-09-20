using System;
using System.Collections.Generic;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class Scene
    {
        private int _id;
        private List<IDrawable> _items;

        public Scene(int id)
        {
            _id = id;
            _items = new List<IDrawable>();
        }

        public Scene AddItem(IDrawable item)
        {
            _items.Add(item);
            return this;
        }

        public Bitmap Render(int width, int height)
        {
            var bitmap = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            using (Graphics g = Graphics.FromImage(bitmap))
            {
                foreach (var item in _items)
                {
                    g.ResetTransform();
                    g.ResetClip();
                    item.Draw(g);
                }
            }

            return bitmap;
        }
    }
}