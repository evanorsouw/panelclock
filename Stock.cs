using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Stock
    {
        private Dictionary<string, Scene> _scenes;
        private Dictionary<string, IDrawable> _items;

        public Stock()
        {
            _scenes = new Dictionary<string, Scene>();
            _items = new Dictionary<string, IDrawable>();
        }

        public Stock AddScene(Scene scene)
        {
            _scenes[scene.Id] = scene;
            return this;
        }

        public Stock AddItem(IDrawable item)
        {
            _items[item.Id] = item;
            return this;
        }

        public Scene GetScene(DateTime now)
        {
            return _scenes.First().Value;
        }

        public IDrawable GetItem(string id)
        {
            return _items[id];
        }
    }
}