using System.ComponentModel;

namespace WhiteMagic.PanelClock.DomainTypes
{
    public enum WeatherType
    {
        [Description("bewolkt")]
        clouded,
        [Description("bliksem")]
        lightning,
        [Description("buien")]
        showers,
        [Description("hagel")]
        hail,
        [Description("halfbewolkt")]
        partlycloudy,
        [Description("halfbewolkt_regen")]
        cloudyrain,
        [Description("helderenacht")]
        clearnight,
        [Description("lichtbewolkt")]
        cloudy,
        [Description("mist")]
        fog,
        [Description("nachtbewolkt")]
        cloudednight,
        [Description("nachtmist")]
        nightfog,
        [Description("regen")]
        rain,
        [Description("sneeuw")]
        snow,
        [Description("zonnig")]
        sunny,
        [Description("zwaarbewolkt")]
        heavyclouds,
    }
}
