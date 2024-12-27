using System.Text;

namespace fontparsers
{
    internal class Program
    {
        enum ParseType { Header, Code, Bitmap, NextCode };
        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine("Usage: fontcompiler <textual-fontfile> <output-wmf-file>\n");
                return;
            }
            var lines = File.ReadAllLines(args[0]).ToList();
            lines.Add("\n");
            var description = "";
            var ascend = 0u;
            var descend = 0u;
            var linespacing = 0u;
            var tracking = 0u;
            var code = 0u;
            var baseline = 0;
            var width = 0;
            var scanlines = new List<List<bool>>();
            var wmffile = new List<byte>();
            var glyphcount = 0;

            var it = lines.GetEnumerator();
            var lineno = 0;
            var parsing = ParseType.Header;

            while (it.MoveNext())
            {
                lineno++;
                var line = it.Current;

                if (parsing == ParseType.Header)
                {
                    if (line.Trim() == "")
                        continue;

                    if (line.StartsWith("description:"))
                    {
                        description = line.Substring(12).Trim();
                    }
                    else if (line.StartsWith("ascend:"))
                    {
                        ascend = uint.Parse(line.Substring(7).Trim());
                    }
                    else if (line.StartsWith("descend:"))
                    {
                        descend = uint.Parse(line.Substring(8).Trim());
                    }
                    else if (line.StartsWith("linespacing:"))
                    {
                        linespacing = uint.Parse(line.Substring(12).Trim());
                    }
                    else if (line.StartsWith("tracking:"))
                    {
                        tracking = uint.Parse(line.Substring(9).Trim());
                    }
                    else if (line.StartsWith("code:"))
                    {
                        var descriptionBytes = Encoding.UTF8.GetBytes(description);
                        var offsetGlyphs = 12 + descriptionBytes.Length;
                        wmffile.Add(1);
                        wmffile.Add(0);
                        wmffile.Add((byte)ascend);
                        wmffile.Add((byte)descend);
                        wmffile.Add((byte)linespacing);
                        wmffile.Add((byte)tracking);
                        wmffile.Add(0);
                        wmffile.Add(0);
                        wmffile.Add((byte)(offsetGlyphs / 256));
                        wmffile.Add((byte)(offsetGlyphs & 0xFF));
                        wmffile.Add(0);
                        wmffile.Add(12);
                        wmffile.AddRange(descriptionBytes);

                        parsing = ParseType.Code;
                        code = ParseCodepoint(line.Substring(5).Trim());
                        baseline = 0;
                        glyphcount++;
                    }
                    else
                    {
                        Console.WriteLine($"syntax error on line {lineno}");
                        return;
                    }
                }
                else if (parsing == ParseType.Code)
                {
                    if (line.StartsWith("baseline:"))
                    {
                        baseline = int.Parse(line.Substring(9).Trim());
                    }
                    else if (line.StartsWith("bitmap:"))
                    {
                        parsing = ParseType.Bitmap;
                        width = 0;
                        scanlines = new List<List<bool>>();
                    }
                    else
                    {
                        Console.WriteLine($"syntax error on line {lineno}");
                        return;
                    }
                }
                else if (parsing == ParseType.NextCode)
                {
                    if (line.StartsWith("code:"))
                    {
                        glyphcount++;
                        code = ParseCodepoint(line.Substring(5).Trim());
                        baseline = 0;
                        parsing = ParseType.Code;
                    }
                    else if (line.Trim() != "")
                    {
                        Console.WriteLine($"syntax error on line {lineno}");
                        return;
                    }
                }
                else if (parsing == ParseType.Bitmap)
                {
                    if (line.Trim() == "" || line.StartsWith("code:"))
                    {
                        wmffile.Add((byte)(code / 256));
                        wmffile.Add((byte)(code % 255));
                        wmffile.Add((byte)width);
                        wmffile.Add((byte)scanlines.Count);
                        wmffile.Add((byte)baseline);
                        foreach (var scanline in scanlines)
                        {
                            scanline.AddRange(Enumerable.Range(0, 5).Select(_ => false));
                            byte mask = 0x80;
                            byte bits = 0x00;
                            foreach (var bit in scanline.Take(width))
                            {
                                if (bit)
                                    bits |= mask;
                                mask >>= 1;
                                if (mask == 0)
                                {
                                    wmffile.Add(bits);
                                    mask = 0x80;
                                    bits = 0;
                                }
                            }
                            if (mask != 0x80)
                                wmffile.Add(bits);
                        }
                        if (line.Trim() == "")
                        {
                            parsing = ParseType.NextCode;
                        }
                        else
                        {
                            code = ParseCodepoint(line.Substring(5).Trim());
                            baseline = 0;
                            glyphcount++;
                            parsing = ParseType.Code;
                        }
                    }
                    else
                    {
                        var scanline = new List<bool>();
                        for (var x = 0; x < line.Length; ++x)
                        {
                            if (line[x] == 'X' || line[x]=='x')
                            {
                                scanline.Add(true);
                                width = Math.Max(x + 1, width);
                            }
                            else if (line[x] == ' ')
                            {
                                scanline.Add(false);
                            }
                            else if (line[x] == '-')
                            {
                                scanline.Add(false);
                                width = Math.Max(x + 1, width);
                            }
                            else
                            {
                                Console.WriteLine($"syntax error on line {lineno}");
                                return;
                            }
                        }
                        scanlines.Add(scanline);
                    }
                }
            }
            wmffile[6] = (byte)(glyphcount / 256);
            wmffile[7] = (byte)(glyphcount % 255);
            File.Delete(args[1]);
            File.WriteAllBytes(args[1], wmffile.ToArray());
        }

        static uint ParseCodepoint(string value)
        {
            if (value.StartsWith("+"))
                return uint.Parse(value.Substring(1), System.Globalization.NumberStyles.HexNumber);
            if (value.StartsWith("U+"))
                return uint.Parse(value.Substring(2), System.Globalization.NumberStyles.HexNumber);
            return uint.Parse(value);
        }
    }
}
