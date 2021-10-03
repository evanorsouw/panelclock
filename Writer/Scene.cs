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

        public IEnumerable<Component> GetItems()
        {
            return _items.Select(item => _stock.GetItem(item.ItemId));
        }

        private void SetCronSpec(string value)
        {
            _cronspec = value;
            var options = new CrontabSchedule.ParseOptions();
            options.IncludingSeconds = true;
            _cron = CrontabSchedule.Parse(value, options);
        }

        internal void MakeAssignments()
        {
            foreach(var assignment in _stock.GetAssignments())
            {
                assignment.Make();
            }
        }
    }
}