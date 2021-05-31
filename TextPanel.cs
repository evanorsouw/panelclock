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
        enum ScrollState {  Start, Scroll, End }
        class Item
        {
            private DateTime _stateStart;
            private ScrollState _state;

            public string Prefix = "";
            public string Text;
            public float PrefixWidth;
            public float TextWidth;
            public float Width { get { return PrefixWidth + TextWidth; } }
            public ScrollState State { get { return _state; } set { _state = value; _stateStart = DateTime.Now; } }
            public float Elapsed { get { return (float)DateTime.Now.Subtract(_stateStart).TotalSeconds; } }
        };

        private string _title;
        private List<Item> _items;
        private float _itemHeight;
        private float _titleHeight;
        private Font _fontTitle;
        private Font _fontItem;
        private float _width;
        private float _height;
        private float _x;
        private float _y;
        private DateTime _lastDrawTime;

        public TextPanel() : this(0, 0, 64, 64)
        { }
        public TextPanel(int x, int y, int dx, int dy)
        {
            _items = new List<Item>();
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
        public string Title { get { return _title; } set { SetTitle(value); } }
        public float TitleHeight { get { return _titleHeight; } set { SetTitleHeight(value); } }
        public float ItemHeight { get { return _itemHeight; } set { SetItemHeight(value); } }

        public void Draw(Graphics graphics)
        {
            var y = Y;

            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            var format = new StringFormat();
            format.Alignment = StringAlignment.Near;
            format.FormatFlags = StringFormatFlags.NoWrap | StringFormatFlags.NoClip;

            graphics.DrawString(Title, _fontTitle, Brushes.Red, new RectangleF(X,Y,Width,Height), format);
            graphics.SetClip(new RectangleF(X, Y, Width, Height));
            y += TitleHeight + 2;

            for(int i=0; i<_items.Count; ++i)
            {
                var item = _items[i];
                var x = X;
                var elapsed = item.Elapsed;
                var colorPrefix = Color.Yellow;
                var colorText = Color.White;
                switch (item.State)
                {
                    case ScrollState.Start:
                        if (elapsed >= 1f && item.Width > Width)
                            item.State = ScrollState.Scroll;
                        break;
                    case ScrollState.Scroll:
                        var scrollwidth = item.Width - Width + 30;
                        var scrolltime = scrollwidth / 30f;
                        x = X - scrollwidth / scrolltime * elapsed;
                        if (elapsed > scrolltime)
                        {
                            var factor = Math.Max(0f, 1f - (elapsed-scrolltime));
                            colorPrefix = Color.FromArgb((byte)(colorPrefix.R * factor), (byte)(colorPrefix.G * factor), (byte)(colorPrefix.R * factor));
                            colorText = Color.FromArgb((byte)(colorText.R * factor), (byte)(colorText.G * factor), (byte)(colorText.R * factor));
                            if (factor == 0f)
                                item.State = ScrollState.Start;
                        }
                        break;
                }
                if (!string.IsNullOrWhiteSpace(item.Prefix))
                {
                    graphics.DrawString(item.Prefix, _fontItem, new SolidBrush(colorPrefix), new RectangleF(x, y, 200, 64), format);
                    x += item.PrefixWidth;
                }
                graphics.DrawString(item.Text, _fontItem, new SolidBrush(colorText), new RectangleF(x, y, 200, 64), format);
                y += ItemHeight;
            }
        }

        public TextPanel SetWidth(float w)
        {
            _width = w;
            return this;
        }

        public TextPanel SetHeight(float h)
        {
            _height = h;
            return this;
        }

        public TextPanel SetX(float x)
        {
            _x = x;
            return this;
        }

        private TextPanel SetY(float y)
        {
            _y = y;
            return this;
        }

        public TextPanel SetItemHeight(float height)
        {
            _itemHeight = height;
            _fontItem = new Font("Tahoma", _itemHeight, FontStyle.Regular, GraphicsUnit.Pixel);
            foreach (var item in _items)
            {
                item.PrefixWidth = CalculateWidth(item.Prefix, _fontItem);
                item.TextWidth = CalculateWidth(item.Text, _fontItem);
            }
            return this;
        }

        public TextPanel SetTitleHeight(float height)
        {
            _titleHeight = height;
            _fontTitle = new Font("Tahoma", _titleHeight, FontStyle.Bold, GraphicsUnit.Pixel);
            return this;
        }

        public TextPanel SetTitle(string title)
        {
            _title = title;
            return this;
        }

        public TextPanel AddItem(string text)
        {
            _items.Add(new Item
            {
                Text = text,
                TextWidth = CalculateWidth(text, _fontItem)
            });
            return this;
        }

        public TextPanel AddItem(string prefix, string text)
        {
            _items.Add(new Item
            {
                Prefix = prefix,
                Text = text,
                PrefixWidth = CalculateWidth(prefix, _fontItem),
                TextWidth = CalculateWidth(text, _fontItem)
            });
            return this;
        }

        private float CalculateWidth(string text, Font font)
        {
            var graphics = Graphics.FromImage(new Bitmap(1, 1, System.Drawing.Imaging.PixelFormat.Format32bppArgb));
            return graphics.MeasureString(text, font).Width;
        }
    }
}
