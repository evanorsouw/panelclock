using NCrontab;
using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Scene
    {
        private List<string> _items;
        private Stock _stock;
        private string _cronspec;
        private CrontabSchedule _cron;

        public Scene(string id, Stock stock)
        {
            Id = id;
            _stock = stock;
            _items = new List<string>();
        }

        public string Id { get; private set; }

        public string CronSpec { get { return _cronspec; } set { SetCronSpec(value); } }

        public DateTime NextActive { get { return _cron.GetNextOccurrence(DateTime.Now); } }

        public Scene AddItem(string itemId)
        {
            _items.Add(itemId);
            return this;
        }

        public IEnumerable<Component> GetItems()
        {
            return _items.Select(itemId => _stock.GetItem(itemId));
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