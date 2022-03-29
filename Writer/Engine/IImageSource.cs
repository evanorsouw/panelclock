using System.Threading.Tasks;
using WhiteMagic.PanelClock.DomainTypes;

namespace WhiteMagic.PanelClock.Engine
{
    public interface IImageSource
    {
        string Name { get; }

        Task<IImageInfo> GetRandomImage();
    }
}
