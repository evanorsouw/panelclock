using System.Collections.Generic;
using System.Linq;

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

        public IImageInfo GetRandomImage()
        {
            return GetRandomSource().GetRandomImage();
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
