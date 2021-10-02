using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public abstract class Component : ValueSource, IComponent
    {
        private bool _visible;
        private DateTime _animateStartTime;

        public Component(string id) : base(id)
        {
            Visible = true;

            AddProperty(Create("visible", () => Visible, (obj) => Visible = obj));
            AddProperty(Create("animationtime", () => AnimationTime, (obj) => AnimationTime = obj));
            AddProperty(Create("animating", () => Animating));

            SetGetter(() => AnimationElapsed);
        }

        public abstract IComponent Clone(string id);

        public abstract void Draw(Graphics g);

        #region properties

        public bool Visible
        {
            get { return _visible; }
            set
            {
                if (_visible != value)
                {
                    _visible = value; _animateStartTime = DateTime.Now;
                }
            }
        }
        public float AnimationTime { get; set; } = 1f;
        public bool Animating => (Visible && AnimationElapsed < 1) || (!Visible && AnimationElapsed > 0);
        #endregion

        protected double AnimationElapsed
        {
            get
            {
                var elapsed = Math.Min(1.0, DateTime.Now.Subtract(_animateStartTime).TotalSeconds / AnimationTime);
                elapsed = Visible ? elapsed : 1.0 - elapsed;
                return elapsed;
            }
        }
    }
}
