namespace WhiteMagic.PanelClock.Engine
{
    public class SceneItem
    {
        private string _itemId;

        public SceneItem(string itemId)
        {
            _itemId = itemId;
        }

        public string ItemId { get { return _itemId; } }
    }
}