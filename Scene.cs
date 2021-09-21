using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Scene
    {
        private List<SceneItem> _items;
        private Stock _stock;

        public Scene(string id, Stock stock)
        {
            Id = id;
            _stock = stock;
            _items = new List<SceneItem>();
        }

        public string Id { get; private set; }

        public Scene AddItem(SceneItem item)
        {
            _items.Add(item);
            return this;
        }

        public Bitmap Render(int width, int height)
        {
            var bitmap = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            using (Graphics g = Graphics.FromImage(bitmap))
            {
                foreach (var item in _items.Select(item => _stock.GetItem(item.ItemId)))
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