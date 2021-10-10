using Microsoft.Extensions.Logging;
using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public abstract class Component : ValueSource, IComponent
    {
        private ILogger _logger;
        private bool _visible;
        private DateTime _showOrHideStartTime;

        public Component(string id, ILogger logger) : base(id)
        {
            _logger = logger;
            Visible = true;

            AddProperty(Create("visible", () => Visible, (obj) => Visible = obj));
            AddProperty(Create("showhidetime", () => ShowOrHideTime, (obj) => ShowOrHideTime = obj));
            AddProperty(Create("showinghiding", () => ShowingOrHiding));

            SetGetter(() => ShowOrHideAnimationElapsed);
        }

        public abstract IComponent Clone(string id);

        public abstract void Draw(Graphics g);

        protected ILogger Logger => _logger;

        #region properties

        public bool Visible
        {
            get { return _visible; }
            set
            {
                if (_visible != value)
                {
                    _visible = value; _showOrHideStartTime = DateTime.Now;
                }
            }
        }

        public double ShowOrHideTime { get; set; } = 1;

        public bool ShowingOrHiding => (Visible && ShowOrHideAnimationElapsed < 1) || (!Visible && ShowOrHideAnimationElapsed > 0);

        protected double ShowOrHideAnimationElapsed
        {
            get
            {
                var elapsed = Math.Min(1.0, DateTime.Now.Subtract(_showOrHideStartTime).TotalSeconds / ShowOrHideTime);
                elapsed = Visible ? elapsed : 1.0 - elapsed;
                return elapsed;
            }
        }

        protected bool Relative(double value, double min, double max, ref double relative)
        {
            relative = Math.Min(1, Math.Max(0, (value - min) / (max - min)));
            return value < max;
        }

        #endregion
    }
}
