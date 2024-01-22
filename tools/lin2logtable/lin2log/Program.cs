using System;

namespace lin2log
{
    class Program
    {
        static void Main(string[] args)
        {
            for (int lin=0; lin<256; ++lin)
            {
                //Excel: (POWER(10, 1 + B3 / 255) - POWER(10, 1)) / 90 * 255
                var log = (int)((Math.Pow(10, 1 + lin / 255.0) - Math.Pow(10, 1)) / 90.0 * 255.0);
                if (lin %8 ==0)
                    Console.WriteLine();
                Console.Write($"X\"{log:X2}\", ");
            }
        }
    }
}
