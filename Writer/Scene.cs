using NCrontab;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Scene
    {
        private List<SceneItem> _items;
        private Stock _stock;
        private string _cronspec;
        private CrontabSchedule _cron;

        public Scene(string id, Stock stock)
        {
            Id = id;
            _stock = stock;
            _items = new List<SceneItem>();
        }

        public string Id { get; private set; }

        public string CronSpec { get { return _cronspec; } set { SetCronSpec(value); } }

        public DateTime NextActive { get { return _cron.GetNextOccurrence(DateTime.Now); } }

        public Scene AddItem(SceneItem item)
        {
            _items.Add(item);
            return this;
        }

        public Bitmap Render(int width, int height)
        {
            var bitmap = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            using (Graphics g = Graphics.FromImage(bitmap))
            {
                foreach (var item in _items.Select(item => _stock.GetItem(item.ItemId)))
                {
                    g.ResetTransform();
                    g.ResetClip();
                    item.Draw(g);
                }
            }
            return bitmap;
        }

        private void SetCronSpec(string value)
        {
            _cronspec = value;
            var options = new CrontabSchedule.ParseOptions();
            options.IncludingSeconds = true;
            _cron = CrontabSchedule.Parse(value, options);
        }
    }
}