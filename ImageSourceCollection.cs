using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace WhiteMagic.PanelClock
{
    public class ImageSourceCollection
    {
        List<IImageSource> _imageSources = new List<IImageSource>();

        public void AddImageSource(IImageSource source)
        {
            lock (_imageSources)
            {
                _imageSources.Add(source);
            }
        }

        public async Task<IImageInfo> GetRandomImage()
        {
            return await GetRandomSource().GetRandomImage();
        }

        private IImageSource GetRandomSource()
        {
            lock (_imageSources)
            {
                return _imageSources.First();
            }
        }
    }
}
