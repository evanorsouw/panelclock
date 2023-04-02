using System;
using System.Drawing;

namespace WhiteMagic.PanelClock.DomainTypes
{
    public interface IImageInfo
    {
        Image Image { get; }
        DateTime When { get; }
        string Source { get; }
        string Path { get; }
    }
}
