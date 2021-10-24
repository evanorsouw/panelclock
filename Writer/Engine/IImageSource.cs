using System.Threading.Tasks;

namespace WhiteMagic.PanelClock
{
    public interface IImageSource
    {
        string Name { get; }

        Task<IImageInfo> GetRandomImage();
    }
}
