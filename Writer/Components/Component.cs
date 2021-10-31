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
        private float _x;
        private float _y;
        private float _x1;
        private float _y1;
        private float _width;
        private float _height;
        private float _actualWidth;
        private float _actualHeight;
        private Alignment _horizontalAlignment;
        private Alignment _verticalAlignment;

        public Component(string id, ILogger logger) : base(id)
        {
            _logger = logger;
            DirectVisibility = true;
            ExternalVisible = true;

            AddProperty(Create("directvisibility", () => DirectVisibility, (obj) => DirectVisibility = obj));
            AddProperty(Create("visible", () => ExternalVisible, (obj) => ExternalVisible = obj));
            AddProperty(Create("showhidetime", () => ShowOrHideTime, (obj) => ShowOrHideTime = obj));
            AddProperty(Create("showinghiding", () => ShowingOrHiding));

            AddProperty(Create("x", () => X, (obj) => X = obj));
            AddProperty(Create("y", () => Y, (obj) => Y = obj));
            AddProperty(Create("width", () => Width, (obj) => Width = obj));
            AddProperty(Create("height", () => Height, (obj) => Height = obj));
            AddProperty(Create("x1", () => X1));
            AddProperty(Create("y1", () => Y1));
            AddProperty(Create("x2", () => X2));
            AddProperty(Create("y2", () => Y2));
            AddProperty(Create("horizontalalignment", () => HorizontalAlignment, (obj) => HorizontalAlignment = obj.ToEnum<Alignment>()));
            AddProperty(Create("verticalalignment", () => VerticalAlignment, (obj) => VerticalAlignment = obj.ToEnum<Alignment>()));

            HorizontalAlignment = Alignment.TopOrLeft;
            VerticalAlignment = Alignment.TopOrLeft;
            X = 0;
            Y = 0;
            Width = -1;
            Height = -1;
            DirectVisibility = true;

            SetGetter(() => ShowOrHideAnimationElapsed);
        }

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
            get { return _internalVisible; }
            set
            {
                if (_directVisibility)
                {
                    InternalVisible = _externalVisible = value;
                }
                else if (_externalVisible != value)
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

        public float ShowOrHideTime { get; set; } = 1;

        public bool ShowingOrHiding => (InternalVisible && ShowOrHideAnimationElapsed < 1) || (!InternalVisible && ShowOrHideAnimationElapsed > 0);

        protected float ShowOrHideAnimationElapsed
        {
            get
            {
                var elapsed = (float)Math.Min(1.0, DateTime.Now.Subtract(_internalVisibilityTime).TotalSeconds / ShowOrHideTime);
                elapsed = InternalVisible ? elapsed : 1.0f - elapsed;
                return elapsed;
            }
        }

        public float Width 
        { 
            get { return _actualWidth; }
            set
            {
                if (_width != value)
                {
                    _width = value;
                    CoordinatesChanged();
                }
            }
        }
        public float Height
        {
            get { return _actualHeight; }
            set
            {
                if (_height != value)
                {
                    _height = value;
                    CoordinatesChanged();
                }
            }
        }
        public float X
        {
            get { return _x; }
            set
            {
                if (_x != value)
                { 
                    _x = value; 
                    CoordinatesChanged(); 
                }
            }
        }
        public float Y
        {
            get { return _y; }
            set
            {
                if (_y != value)
                {
                    _y = value; 
                    CoordinatesChanged();
                }
            }
        }
        public float X1 => _x1;
        public float Y1 => _y1;
        public float X2 => _x1 + Width;
        public float Y2 => _y1 + Height;
        public Alignment HorizontalAlignment 
        {
            get { return _horizontalAlignment; }
            set
            {
                if (_horizontalAlignment != value)
                {
                    _horizontalAlignment = value;
                    CoordinatesChanged();
                }
            } 
        }
        public Alignment VerticalAlignment
        {
            get { return _verticalAlignment; }
            set
            {
                if (_verticalAlignment != value)
                {
                    _verticalAlignment = value;
                    CoordinatesChanged();
                }
            }
        }
        public RectangleF BackgroundBox { get; private set; }
        public PointF ContentTopLeft { get; private set; }

        #endregion

        protected virtual void GetFullSize(out float width, out float height)
        {
            width = Width;
            height = Height;
        }

        protected virtual void CoordinatesChanged()
        {
            GetFullSize(out var maxwidth, out var maxheight);

            float tx;
            if (_width > -1)
            {
                _x1 = _x;
                _actualWidth = _width;
                tx = _horizontalAlignment switch
                {
                    Alignment.TopOrLeft => _x,
                    Alignment.Center => _x + (_width - maxwidth)/2,
                    Alignment.BottomOrRight => _x + _width - maxwidth
                };
            }
            else
            {
                _actualWidth = maxwidth;
                _x1 = tx = _horizontalAlignment switch
                {
                    Alignment.TopOrLeft => _x,
                    Alignment.Center => _x - maxwidth / 2,
                    Alignment.BottomOrRight => _x - maxwidth
                };
            }

            float ty;
            if (_height > -1)
            {
                _y1 = _y;
                _actualHeight = _height;
                ty = _verticalAlignment switch
                {
                    Alignment.TopOrLeft => _y,
                    Alignment.Center => _y + (_height - maxheight) / 2,
                    Alignment.BottomOrRight => _y + _height - maxheight
                };
            }
            else
            {
                _actualHeight = maxheight;
                _y1 = ty = _verticalAlignment switch
                {
                    Alignment.TopOrLeft => _y,
                    Alignment.Center => _y - maxheight / 2,
                    Alignment.BottomOrRight => _y - maxheight
                };
            }
            BackgroundBox = new RectangleF(X1, Y1, X2-X1, Y2-Y1);
            ContentTopLeft = new PointF(tx, ty);
        }

        protected bool Relative(float value, float min, float max, ref float relative)
        {
            relative = Math.Min(1, Math.Max(0, (value - min) / (max - min)));
            return value < max;
        }
    }
}
