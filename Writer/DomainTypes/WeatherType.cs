using System.ComponentModel;

namespace WhiteMagic.PanelClock
{
    public enum WeatherType
    {
        [Description("zonnig")]
        sunny,
        [Description("bliksem")]
        lightning,
        [Description("regen")]
        rain,
        [Description("buien")]
        showers,
        [Description("hagel")]
        hail,
        [Description("mist")]
        fog,
        [Description("sneeuw")]
        snow,
        [Description("bewolkt")]
        clouded,
        [Description("lichtbewolkt")]
        cloudy,
        [Description("halfbewolkt")]
        partlycloudy,
        [Description("halfbewolkt_regen")]
        cloudyrain,
        [Description("zwaarbewolkt")]
        heavyclouds,
        [Description("nachtmist")]
        nightfog,
        [Description("helderenacht")]
        clearnight,
        [Description("wolkennacht")]
        cloudednight
    }
}
