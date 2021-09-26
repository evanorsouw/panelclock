using System;
using System.Collections.Generic;

namespace WhiteMagic.PanelClock
{
    public class Stock
    {
        private Dictionary<string, Scene> _scenes;
        private Dictionary<string, Component> _items;
        private Dictionary<string, ValueSource> _valueSource;
        private List<Assignment> _assignments;

        public Stock()
        {
            _scenes = new Dictionary<string, Scene>();
            _items = new Dictionary<string, Component>();
            _valueSource = new Dictionary<string, ValueSource>();
            _assignments = new List<Assignment>();
        }

        public Stock AddScene(Scene scene)
        {
            _scenes[scene.Id] = scene;
            return this;
        }

        public Stock AddItem(Component item)
        {
            _items[item.Id] = item;
            return this;
        }

        public void AddExpression(ValueSource expression)
        {
            _valueSource[expression.Id] = expression;
        }

        private DateTime _nextCheckTime = DateTime.MinValue;
        private Scene _activeScene;

        public Scene GetScene(DateTime now)
        {
            if (now < _nextCheckTime)
                return _activeScene;

            _nextCheckTime = DateTime.MaxValue;
            foreach (var scene in _scenes.Values)
            {
                var next = scene.NextActive;
                if (next < _nextCheckTime)
                {
                    _nextCheckTime = next;
                    _activeScene = scene;
                }
            }
            return _activeScene;
        }

        public Component GetItem(string id)
        {
            return _items[id];
        }

        internal ValueSource GetValueSource(string identifier)
        {
            if (!_valueSource.TryGetValue(identifier, out var source))
                return null;
            return source;
        }

        public IEnumerable<Assignment> GetAssignments()
        {
            return _assignments;
        }

        public void AddAssignment(Assignment assigner)
        {
            _assignments.Add(assigner);
        }
    }
}