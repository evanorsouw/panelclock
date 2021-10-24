using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Animator
    {
        private Stock _stock;
        private Scene _activeScene;
        private List<Component> _activeComponents;
        private List<Component> _removingComponents;

        public Animator(Stock stock)
        {
            _stock = stock;
        }

        public void Render(Bitmap bitmap, DateTime now)
        {
            var scene = _stock.GetScene(now);
            scene.MakeAssignments();

            if (scene != _activeScene)
            {
                _activeComponents = scene.GetItems().ToList();
                _removingComponents = new List<Component>();
                if (_activeScene != null)
                {
                    _removingComponents.AddRange(_activeScene.GetItems().Where(c1 => scene.GetItems().All(c2 => c1.Id != c2.Id)));
                }
                _activeComponents.Select(c => c.InternalVisible = true).ToList();
                _removingComponents.Select(c => c.InternalVisible = false).ToList();
                _activeScene = scene;
            }

            /// een scene switch is pas compleet als alle removing items uit ge-fade zijn.
            /// tijdens faden worden visible assignments van removing items niet geaccepteerd.

            using (Graphics graphics = Graphics.FromImage(bitmap))
            {
                var removing = false;
                foreach (var component in _removingComponents)
                {
                    RenderComponent(component, graphics);
                    removing |= component.ShowingOrHiding;
                }
                foreach (var component in _activeComponents)
                {
                    RenderComponent(component, graphics);
                }

                //if (removing)
                //{
                //    graphics.ResetClip();
                //    graphics.ResetTransform();
                //    graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
                //    graphics.FillEllipse(new SolidBrush(Color.Yellow), -2.5f, -2.5f, 5, 5);
                //}
            }
        }

        private void RenderComponent(IDrawable item, Graphics graphics)
        {
            graphics.ResetClip();
            graphics.ResetTransform();
            item.Draw(graphics);
        }
    }
}
