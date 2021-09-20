using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Movie
    {
        private List<Scene> _scenes;

        public Movie()
        {
            _scenes = new List<Scene>();
        }

        public Movie AddScene(Scene scene)
        {
            _scenes.Add(scene);
            return this;
        }

        public Scene GetScene(DateTime now)
        {
            return _scenes.First();
        }
    }
}