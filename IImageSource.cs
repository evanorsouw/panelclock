namespace WhiteMagic.PanelClock
{
    public interface IImageSource
    {
        string Name { get; }

        IImageInfo GetRandomImage();
    }
}
