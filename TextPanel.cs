using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WhiteMagic.PanelClock
{
    public class TextPanel : IDrawable
    {
        enum TitleState { Unknown, ScrollIn, ScrollOut }
        enum ItemState { Unknown, Invisible, FadeIn, StartScroll, Scroll, EndScroll, FadeOut }
        enum PanelState { Unknown, Start, MoveNext, ScrollUp, Clear, End}
        class TitleData
        {
            private DateTime _stateStart;
            private TitleState _state;

            public string Text = "";
            public float TextWidth;
            public TitleState State { get { return _state; } set { if (value != _state) { _state = value; _stateStart = DateTime.Now; } } }
            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
        };
        class ItemData
        {
            private DateTime _stateStart;
            private ItemState _state;

            public string Prefix = "";
            public string Text;
            public float PrefixWidth;
            public float TextWidth;
            public float Y;
            public float Width { get { return PrefixWidth + TextWidth; } }
            public ItemState State { get { return _state; } set { if (value != _state) { _state = value; _stateStart = DateTime.Now; } } }
            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
        };
        class PanelData
        {
            private DateTime _stateStart;
            private PanelState _state;

            public int iTop = 0;
            public int nDisplayed = 0;
            public PanelState State { get { return _state; } set { _state = value; _stateStart = DateTime.Now; } }
            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
        };

        public class Timing
        {
            public float TitleSpeed { get; set; } = 0.35f;
            public float InitialAppear { get; set; } = 0.2f;
            public float FadeIn { get; set; } = 0.1f;
            public float FadeOut {get;set;} = 0.3f;
            public float AppearNext { get; set; } = 2f;
            public float ScrollUp { get; set; } = 0.25f;
            public float ScrollLeft { get; set; } = 0.01f;
            public float ScrollPause { get; set; } = 1f;
            public float ScrollSpeed { get; set; } = 0.6f;

        }

        private List<ItemData> _items;
        private float _itemHeight;
        private float _titleHeight;
        private Font _fontTitle;
        private Font _fontItem;
        private float _width;
        private float _height;
        private float _x;
        private float _y;
        private TitleData _title;
        private PanelData _panel;

        public TextPanel() : this(0, 0, 64, 64)
        { }
        public TextPanel(int x, int y, int dx, int dy)
        {
            _items = new List<ItemData>();
            _title = new TitleData();
            _panel = new PanelData();
            _panel.State = PanelState.Start;
            Times = new Timing();

            X = x;
            Y = y;
            Width = dx;
            Height = dy;
            TitleHeight = 10;
            ItemHeight = 10;
        }

        public float Width { get { return _width; } set { SetWidth(value); } }
        public float Height { get { return _height; } set { SetHeight(value); } }
        public float X { get { return _x; } set { SetX(value); } }
        public float Y { get { return _y; } set { SetY(value); } }
        public string Title { get { return _title.Text; } set { SetTitle(value); } }
        public float TitleHeight { get { return _titleHeight; } set { SetTitleHeight(value); } }
        public float ItemHeight { get { return _itemHeight; } set { SetItemHeight(value); } }
        public Timing Times { get; private set; }

        public void Draw(Graphics graphics)
        {
            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            graphics.SetClip(new RectangleF(X, Y, Width, Height));

            DrawTitle(graphics);
            DrawItems(graphics);
            AnimatePanel();
        }
        public TextPanel AddItem(string text)
        {
            return AddItem("", text);
        }

        public TextPanel AddItem(string prefix, string text)
        {
            _items.Add(new ItemData
            {
                Prefix = prefix,
                Text = text,
                PrefixWidth = CalculateWidth(prefix, _fontItem),
                TextWidth = CalculateWidth(text, _fontItem),
                State = ItemState.Invisible
            });
            return this;
        }

        private void SetWidth(float w)
        {
            _width = w;
        }

        private void SetHeight(float h)
        {
            _height = h;
        }

        private void SetX(float x)
        {
            _x = x;
        }

        private void SetY(float y)
        {
            _y = y;
        }

        private void SetItemHeight(float height)
        {
            _itemHeight = height;
            _fontItem = new Font("Tahoma", _itemHeight, FontStyle.Regular, GraphicsUnit.Pixel);
            foreach (var item in _items)
            {
                item.PrefixWidth = CalculateWidth(item.Prefix, _fontItem);
                item.TextWidth = CalculateWidth(item.Text, _fontItem);
            }
        }

        private void SetTitleHeight(float height)
        {
            _titleHeight = height;
            _fontTitle = new Font("Tahoma", _titleHeight, FontStyle.Bold, GraphicsUnit.Pixel);
            SetTitle(_title?.Text);
        }

        private void SetTitle(string title)
        {
            _title.Text = title ?? "";
            _title.TextWidth = CalculateWidth(_title.Text, _fontTitle);
        }

        private float CalculateWidth(string text, Font font)
        {
            var graphics = Graphics.FromImage(new Bitmap(1, 1, System.Drawing.Imaging.PixelFormat.Format32bppArgb));
            return graphics.MeasureString(text, font).Width;
        }

        private void DrawItems(Graphics graphics)
        {
            for (int i = 0; i < _items.Count; ++i)
            {
                var item = _items[i];
                var x = X;
                var elapsed = item.Elapsed;
                var prefixColorScale = 1f;
                var textColorScale = 1f;
                var scrolled = 0f;
                switch (item.State)
                {
                    case ItemState.Invisible:
                        continue;

                    case ItemState.FadeIn:
                        prefixColorScale = textColorScale = Math.Min(1f, elapsed * 2);
                        if (prefixColorScale >= 1f)
                        {
                            item.State = ItemState.StartScroll;
                        }
                        break;

                    case ItemState.FadeOut:
                        prefixColorScale = textColorScale = Math.Max(0f, 1f - (elapsed / Times.FadeOut));
                        if (prefixColorScale <= 0f)
                        {
                            item.State = ItemState.Invisible;
                        }
                        break;

                    case ItemState.StartScroll:
                        if (elapsed >= Times.ScrollPause && item.Width > Width)
                        {
                            item.State = ItemState.Scroll;
                        }
                        break;

                    case ItemState.EndScroll:
                        x -= item.Width - Width;
                        prefixColorScale = 0f;
                        if (elapsed >= Times.ScrollPause)
                        {
                            item.State = ItemState.StartScroll;
                        }
                        break;

                    case ItemState.Scroll:
                        if (item.Width > Width)
                        {
                            prefixColorScale = Math.Max(0f, 1f - (elapsed / Times.FadeOut));
                            var scrollwidth = item.Width - Width;
                            var scrolltime = scrollwidth / 64f;
                            scrolled = Math.Min(scrollwidth, scrollwidth * elapsed / scrolltime);
                            x -= scrolled;
                            if (scrolled == scrollwidth)
                            {
                                item.State = ItemState.EndScroll;
                            }
                        }
                        break;
                }
                Color color;
                var format = new StringFormat();
                format.FormatFlags = StringFormatFlags.NoWrap;
                if (!string.IsNullOrWhiteSpace(item.Prefix))
                {
                    color = Color.Yellow.Scale(prefixColorScale);
                    graphics.DrawString(item.Prefix, _fontItem, new SolidBrush(color), 0, item.Y, format);
                    x += item.PrefixWidth;
                }
                color = Color.White.Scale(textColorScale);
                graphics.DrawString(item.Text, _fontItem, new SolidBrush(color), x, item.Y, format);
            }
        }
        private void DrawTitle(Graphics graphics)
        {
            var x = X;
            var width = _title.TextWidth;
            var elapsed = _title.Elapsed;
            var progress = Math.Min(1f, elapsed / (width / 64f * Times.TitleSpeed));

            switch (_title.State)
            {
                case TitleState.ScrollIn:
                    x = -width * (1f - progress);
                    break;
                case TitleState.ScrollOut:
                    x = -width * progress;
                    break;
            }
            graphics.DrawString(Title, _fontTitle, Brushes.Red, new RectangleF(x, Y, Width, Height));
        }
        private void AnimatePanel()
        {
            var y = TitleHeight;
            var elapsed = _panel.Elapsed;
            switch (_panel.State)
            {
                case PanelState.Start:
                    _title.State = TitleState.ScrollIn;
                    _panel.iTop = 0;
                    var showing = (int)(elapsed / Times.InitialAppear);
                    for (int i = 0; i < _items.Count && i < showing; i++)
                    {
                        var item = _items[i];
                        if (item.State == ItemState.Invisible)
                        {
                            item.State = ItemState.FadeIn;
                            item.Y = y;
                            _panel.nDisplayed++;
                        }
                        y += ItemHeight;
                    }
                    if (y >= Height)
                    {
                        _panel.State = PanelState.MoveNext;
                    }
                    if (elapsed > _items.Count * 2 * Times.AppearNext)
                    {
                        _panel.State = PanelState.Clear;
                    }
                    break;

                case PanelState.MoveNext:
                    if (elapsed > Times.AppearNext)
                    {
                        var item = _items[_panel.iTop];
                        item.State = ItemState.FadeOut;
                        _panel.State = PanelState.ScrollUp;
                    }
                    break;

                case PanelState.ScrollUp:
                    var yOffset = ItemHeight * Math.Max(0, 1f - (elapsed / Times.ScrollUp));
                    for (int i = 1; i < _items.Count; ++i)
                    {
                        var item = _items[(i + _panel.iTop) % _items.Count];
                        if (item.State == ItemState.Invisible)
                        {
                            item.State = ItemState.StartScroll;
                        }
                        item.Y = y + yOffset;
                        y += ItemHeight;
                    }
                    if (yOffset == 0)
                    {
                        _panel.iTop = (_panel.iTop + 1) % _items.Count;
                        _panel.State = PanelState.MoveNext;
                        if (_panel.nDisplayed++ == _items.Count * 2)
                        {
                            _panel.State = PanelState.Clear;
                        }
                    }
                    break;

                case PanelState.Clear:
                    _title.State = TitleState.ScrollOut;
                    var removing = elapsed / 0.1f;
                    for (int i = 0; i < _items.Count && i < removing; ++i)
                    {
                        var item = _items[(i + _panel.iTop) % _items.Count];
                        if (item.Y >= Height)
                        {
                            if (item.State == ItemState.Invisible)
                                _panel.State = PanelState.End;
                            break;
                        }
                        if (item.State != ItemState.Invisible)
                        {
                            item.State = ItemState.FadeOut;
                        }
                    }
                    break;

                case PanelState.End:
                    break;
            }

        }
    }
}
