using Microsoft.Extensions.Logging;
using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public abstract class Component : ValueSource, IComponent
    {
        private ILogger _logger;
        private bool _directVisibility;
        private bool _externalVisible;
        private bool _internalVisible;
        private DateTime _internalVisibilityTime;

        public Component(string id, ILogger logger) : base(id)
        {
            _logger = logger;
            DirectVisibility = true;
            ExternalVisible = true;

            AddProperty(Create("directvisibility", () => DirectVisibility, (obj) => DirectVisibility = obj));
            AddProperty(Create("visible", () => ExternalVisible, (obj) => ExternalVisible = obj));
            AddProperty(Create("showhidetime", () => ShowOrHideTime, (obj) => ShowOrHideTime = obj));
            AddProperty(Create("showinghiding", () => ShowingOrHiding));

            SetGetter(() => ShowOrHideAnimationElapsed);
        }

        public abstract IComponent Clone(string id);

        public abstract void Draw(Graphics g);

        protected ILogger Logger => _logger;

        #region properties

        public bool DirectVisibility
        {
            get { return _directVisibility; }
            set
            {
                _directVisibility = value;
                if (value)
                {
                    _internalVisible = _externalVisible;
                }
            }
        }

        public bool ExternalVisible
        {
            get { return _externalVisible; }
            set
            {
                if (_externalVisible != value)
                {
                    InternalVisible |= value;
                    _externalVisible = value; 
                }
            }
        }

        public bool InternalVisible
        {
            get { return _internalVisible; }
            set
            {
                if (_internalVisible != value)
                {
                    _internalVisible = value; 
                    _internalVisibilityTime = DateTime.Now;
                }
            }
        }

        public double ShowOrHideTime { get; set; } = 1;

        public bool ShowingOrHiding => (InternalVisible && ShowOrHideAnimationElapsed < 1) || (!InternalVisible && ShowOrHideAnimationElapsed > 0);

        protected double ShowOrHideAnimationElapsed
        {
            get
            {
                var elapsed = Math.Min(1.0, DateTime.Now.Subtract(_internalVisibilityTime).TotalSeconds / ShowOrHideTime);
                elapsed = InternalVisible ? elapsed : 1.0 - elapsed;
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
