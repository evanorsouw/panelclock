using System.Drawing;

internal class Program
{
    private static void Main(string[] args)
    {
        var image = (Bitmap)Image.FromFile("..\\..\\..\\..\\..\\fpga\\icon.png");
        var ox = 0;
        var oy = 0;

        for (var y = 0; y < image.Height; ++y)
        {
            var address = 128 * (y+oy) + ox;
            Console.Write($"X\"01\",X\"{address >> 8:X2}\",X\"{address & 0xFF:X2}\",X\"00\",X\"{image.Width:X2}\", ");
            for (var x = 0; x < image.Width; ++x)
            {
                var pixel = image.GetPixel(x, y);
                Console.Write($"X\"{pixel.R:X2}\",X\"{pixel.G:X2}\",X\"{pixel.B:X2}\", ");
            }
            Console.WriteLine();
        }
    }
}