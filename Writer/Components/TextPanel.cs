//using System;
//using System.Collections.Generic;
//using System.Drawing;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;

//namespace WhiteMagic.PanelClock
//{
//    public class TextPanel : IDrawable
//    {
//        enum TitleState { Unknown, ScrollIn, ScrollOut }
//        enum ItemState { Unknown, Invisible, FadeIn, StartScroll, Scroll, EndScroll, FadeOut, Idle }
//        enum PanelState { Unknown, Start, MoveNext, ScrollUp, Clear, End }
//        class TitleData
//        {
//            private DateTime _stateStart;
//            private TitleState _state;

//            public string Text = "";
//            public float TextWidth;
//            public TitleState State { get { return _state; } set { if (value != _state) { _state = value; _stateStart = DateTime.Now; } } }
//            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
//        };
//        class ItemData
//        {
//            private DateTime _stateStart;
//            private ItemState _state;

//            public string Prefix = "";
//            public string Text;
//            public float PrefixWidth;
//            public float TextWidth;
//            public float Y;
//            public float LastX;
//            public float Width { get { return PrefixWidth + TextWidth; } }
//            public ItemState State { get { return _state; } set { if (value != _state) { _state = value; _stateStart = DateTime.Now; } } }
//            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
//        };
//        class PanelData
//        {
//            private DateTime _stateStart;
//            private PanelState _state;

//            public int iTop = 0;
//            public int nDisplayed = 0;
//            public PanelState State { get { return _state; } set { _state = value; _stateStart = DateTime.Now; } }
//            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
//        };

//        public class TimingInfo
//        {
//            public float TitleSpeed { get; set; } = 0.35f;
//            public float InitialAppear { get; set; } = 0.2f;
//            public float FadeIn { get; set; } = 0.1f;
//            public float FadeOut { get; set; } = 0.3f;
//            public float AppearNext { get; set; } = 2f;
//            public float ScrollUpSpeed { get; set; } = 0.4f;
//            public float StartScrollPause { get; set; } = 1.5f;
//            public float EndScrollPause { get; set; } = 1f;
//            public float ScrollSpeed { get; set; } = 0.4f;
//        }

//        public class FontInfo
//        {
//            private Action _whenChanged;
//            private string _typeface = "Tahoma";
//            private float _size = 11;
//            private Font _font;
//            public FontInfo(Action whenChanged)
//            {
//                _whenChanged = whenChanged;
//                CreateFont(false);
//            }
//            public string Typeface { get { return _typeface; } set { _typeface = value; CreateFont(true); } }
//            public float Height { get { return _size; } set { _size = value; CreateFont(true); } }

//            public Font Fnt {get{return _font;} }

//            private void CreateFont(bool notify)
//            {
//                _font = new Font(_typeface, _size, FontStyle.Regular, GraphicsUnit.Pixel);
//                if (notify)
//                {
//                    _whenChanged();
//                }
//            }
//        }

//        private List<ItemData> _items;
//        private float _itemHeight;
//        private float _titleHeight;
//        private float _width;
//        private float _height;
//        private float _x;
//        private float _y;
//        private TitleData _title;
//        private PanelData _panel;

//        public TextPanel() : this(0, 0, 64, 64)
//        { }
//        public TextPanel(int x, int y, int dx, int dy)
//        {
//            _items = new List<ItemData>();
//            _title = new TitleData();
//            _panel = new PanelData();
//            _panel.State = PanelState.Start;
//            Times = new TimingInfo();
//            TitleFont = new FontInfo(() => FontsChanged());
//            ItemFont = new FontInfo(() => FontsChanged());
//            PrefixFont = new FontInfo(() => FontsChanged());
//            AllFonts = new FontInfo(() =>
//            {
//                TitleFont.Typeface = AllFonts.Typeface;
//                PrefixFont.Typeface = AllFonts.Typeface;
//                ItemFont.Typeface = AllFonts.Typeface;
//                TitleFont.Height = AllFonts.Height;
//                PrefixFont.Height = AllFonts.Height;
//                ItemFont.Height = AllFonts.Height;
//            });

//            X = x;
//            Y = y;
//            Width = dx;
//            Height = dy;
//        }

//        public float Width { get { return _width; } set { SetWidth(value); } }
//        public float Height { get { return _height; } set { SetHeight(value); } }
//        public float X { get { return _x; } set { SetX(value); } }
//        public float Y { get { return _y; } set { SetY(value); } }
//        public string Title { get { return _title.Text; } set { SetTitle(value); } }
//        public TimingInfo Times { get; private set; }
//        public FontInfo AllFonts { get; private set; }
//        public FontInfo TitleFont { get; private set; }
//        public FontInfo PrefixFont { get; private set; }
//        public FontInfo ItemFont { get; private set; }

//        public void Draw(Graphics graphics)
//        {
//            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
//            graphics.SetClip(new RectangleF(X, Y, Width, Height));

//            DrawTitle(graphics);
//            DrawItems(graphics);
//            AnimatePanel();
//        }
//        public TextPanel AddItem(string text)
//        {
//            return AddItem("", text);
//        }

//        public TextPanel AddItem(string prefix, string text)
//        {
//            _items.Add(new ItemData
//            {
//                Prefix = prefix,
//                Text = text,
//                PrefixWidth = CalculateWidth(prefix, PrefixFont.Fnt),
//                TextWidth = CalculateWidth(text, ItemFont.Fnt),
//                State = ItemState.Invisible
//            });
//            return this;
//        }

//        private void SetWidth(float w)
//        {
//            _width = w;
//        }

//        private void SetHeight(float h)
//        {
//            _height = h;
//        }

//        private void SetX(float x)
//        {
//            _x = x;
//        }

//        private void SetY(float y)
//        {
//            _y = y;
//        }

//        private void FontsChanged()
//        {
//            foreach (var item in _items)
//            {
//                item.PrefixWidth = CalculateWidth(item.Prefix, PrefixFont.Fnt);
//                item.TextWidth = CalculateWidth(item.Text, ItemFont.Fnt);
//            }
//        }

//        private void SetTitle(string title)
//        {
//            _title.Text = title ?? "";
//            _title.TextWidth = CalculateWidth(_title.Text, TitleFont.Fnt);
//        }

//        private float CalculateWidth(string text, Font font)
//        {
//            var graphics = Graphics.FromImage(new Bitmap(1, 1, System.Drawing.Imaging.PixelFormat.Format32bppArgb));
//            return graphics.MeasureString(text, font).Width;
//        }

//        private void DrawItems(Graphics graphics)
//        {
//            for (int i = 0; i < _items.Count; ++i)
//            {
//                var item = _items[i];
//                var x = X;
//                var elapsed = item.Elapsed;
//                var prefixColorScale = 1f;
//                var textColorScale = 1f;
//                switch (item.State)
//                {
//                    case ItemState.Invisible:
//                        continue;

//                    case ItemState.FadeIn:
//                        prefixColorScale = textColorScale = Math.Min(1f, elapsed * 2);
//                        if (prefixColorScale >= 1f)
//                        {
//                            item.State = ItemState.StartScroll;
//                        }
//                        break;

//                    case ItemState.FadeOut:
//                        if (item.Width > Width)
//                        {
//                            x -= item.PrefixWidth + Math.Max(0f, item.TextWidth - Width);
//                        }
//                        textColorScale = Math.Max(0f, 1f - (elapsed / Times.FadeOut));
//                        prefixColorScale = (item.Width > Width) ? 0f : textColorScale;
//                        if (textColorScale <= 0f)
//                        {
//                            item.State = ItemState.Invisible;
//                        }
//                        break;

//                    case ItemState.StartScroll:
//                        if (elapsed >= Times.StartScrollPause)
//                        {
//                            if (item.Width > Width)
//                            {
//                                item.State = ItemState.Scroll;
//                            }
//                            else
//                            {
//                                item.State = ItemState.Idle;
//                            }
//                        }
//                        break;

//                    case ItemState.Scroll:
//                        if (item.Width > Width)
//                        {
//                            prefixColorScale = Math.Max(0f, 1f - (elapsed / Times.FadeOut));
//                            var scrollwidth = item.PrefixWidth + Math.Max(0f, item.TextWidth - Width);
//                            var scrolltime = scrollwidth / 64f / Times.ScrollSpeed;
//                            var scrolled = Math.Min(scrollwidth, scrollwidth * elapsed / scrolltime);
//                            x -= scrolled;
//                            if (scrolled == scrollwidth)
//                            {
//                                item.State = ItemState.EndScroll;
//                            }
//                        }
//                        break;

//                    case ItemState.EndScroll:
//                        x -= item.PrefixWidth + Math.Max(0f, item.TextWidth - Width);
//                        prefixColorScale = 0f;
//                        if (elapsed >= Times.EndScrollPause)
//                        {
//                            item.State = ItemState.Idle;
//                        }
//                        break;

//                    case ItemState.Idle:
//                        break;
//                }
//                Color color;
//                var format = new StringFormat();
//                format.FormatFlags = StringFormatFlags.NoWrap;
//                item.LastX = x;
//                if (!string.IsNullOrWhiteSpace(item.Prefix))
//                {
//                    color = Color.Yellow.Scale(prefixColorScale);
//                    graphics.DrawString(item.Prefix, PrefixFont.Fnt, new SolidBrush(color), 0, item.Y, format);
//                    x += item.PrefixWidth;
//                }
//                color = Color.White.Scale(textColorScale);
//                graphics.DrawString(item.Text, ItemFont.Fnt, new SolidBrush(color), x, item.Y, format);
//            }
//        }
//        private void DrawTitle(Graphics graphics)
//        {
//            var x = X;
//            var width = _title.TextWidth;
//            var elapsed = _title.Elapsed;
//            var progress = Math.Min(1f, elapsed / (width / 64f * Times.TitleSpeed));

//            switch (_title.State)
//            {
//                case TitleState.ScrollIn:
//                    x = -width * (1f - progress);
//                    break;
//                case TitleState.ScrollOut:
//                    x = -width * progress;
//                    break;
//            }
//            graphics.DrawString(Title, TitleFont.Fnt, Brushes.Red, new RectangleF(x, Y, Width, Height));
//        }
//        private void AnimatePanel()
//        {
//            var y = TitleFont.Height;
//            var elapsed = _panel.Elapsed;
//            ItemData item;
//            switch (_panel.State)
//            {
//                case PanelState.Start:
//                    _title.State = TitleState.ScrollIn;
//                    _panel.iTop = 0;
//                    var showing = (int)(elapsed / Times.InitialAppear);
//                    for (int i = 0; i < _items.Count && i < showing; i++)
//                    {
//                        item = _items[i];
//                        if (item.State == ItemState.Invisible)
//                        {
//                            item.State = ItemState.FadeIn;
//                            item.Y = y;
//                            _panel.nDisplayed++;
//                        }
//                        else if (item.State == ItemState.Idle)
//                        {
//                            item.State = ItemState.StartScroll;
//                        }
//                        y += ItemFont.Height;
//                    }
//                    if (y >= Height)
//                    {
//                        _panel.State = PanelState.MoveNext;
//                    }
//                    if (elapsed > _items.Count * 2 * Times.AppearNext)
//                    {
//                        //_panel.State = PanelState.Clear;
//                    }
//                    break;

//                case PanelState.MoveNext:
//                    item = _items[_panel.iTop];
//                    if (elapsed > Times.AppearNext && item.State == ItemState.Idle)
//                    {
//                        item.State = ItemState.FadeOut;
//                        _panel.State = PanelState.ScrollUp;
//                    }
//                    foreach (var i in _items)
//                    {
//                        if (i.State == ItemState.Idle)
//                            i.State = ItemState.StartScroll;
//                    }
//                    break;

//                case PanelState.ScrollUp:
//                    var yOffset = ItemFont.Height * Math.Max(0, 1f - (elapsed / Times.ScrollUpSpeed));
//                    for (int i = 1; i < _items.Count; ++i)
//                    {
//                        item = _items[(i + _panel.iTop) % _items.Count];
//                        if (item.State == ItemState.Invisible || item.State == ItemState.Idle)
//                        {
//                            item.State = ItemState.StartScroll;
//                        }
//                        item.Y = y + yOffset;
//                        y += ItemFont.Height;
//                    }
//                    if (yOffset == 0)
//                    {
//                        _panel.iTop = (_panel.iTop + 1) % _items.Count;
//                        _panel.State = PanelState.MoveNext;
//                        if (_panel.nDisplayed++ == _items.Count * 2)
//                        {
//                            //_panel.State = PanelState.Clear;
//                        }
//                    }
//                    break;

//                case PanelState.Clear:
//                    _title.State = TitleState.ScrollOut;
//                    var removing = elapsed / 0.1f;
//                    for (int i = 0; i < _items.Count && i < removing; ++i)
//                    {
//                        item = _items[(i + _panel.iTop) % _items.Count];
//                        if (item.Y >= Height)
//                        {
//                            if (item.State == ItemState.Invisible)
//                                _panel.State = PanelState.End;
//                            break;
//                        }
//                        if (item.State != ItemState.Invisible)
//                        {
//                            item.State = ItemState.FadeOut;
//                        }
//                    }
//                    break;

//                case PanelState.End:
//                    break;
//            }

//        }
//    }
//}
