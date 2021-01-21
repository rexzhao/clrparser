using System;

namespace ConsoleApp1
{
    class Program
    {
        static void Main(string[] args)
        {
            CLRExport.Util.Export("TestExport");

            var watch = System.Diagnostics.Stopwatch.StartNew();
            TestExport.Test.Start();

            Console.WriteLine("time " + watch.ElapsedMilliseconds + " ms");
        }
    }

}


namespace TestExport
{
    public class Test
    {
        public static int Start()
        {
            int v = Fab(15000);
            Console.WriteLine("Hello World! " + v);
            return v;
        }

        public static int Fab(int n)
        {
            if (n > 1)
            {
                return n + Fab(n - 1);
            }
            return n;
        }
    }
}

