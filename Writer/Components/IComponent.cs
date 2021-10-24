namespace WhiteMagic.PanelClock
{
    public interface IComponent : IDrawable, IValueSource
    { 
        /// <summary>Clone into a identical but separate instance</summary>
        /// <param name="id">the identification of the newly create instance</param>
        /// <returns>cloned copy</returns>
        IComponent Clone(string id);
    }
}
