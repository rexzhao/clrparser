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

            double v1;
            v1 = 10.0 % 6.0; // is 4(+10 div + 6 = 1)
            Console.WriteLine("v1 " + v1);
            v1 = 10.0 % -6.0; // is 4(+10 div - 6 = -1)
            Console.WriteLine("v2 " + v1);
            v1 = -10.0 % 6.0; // is -4(-10 div + 6 = -1)
            Console.WriteLine("v3 " + v1);
            v1 = -10.0 % -6.0; // is -4(-10 div - 6 = 1)
            Console.WriteLine("v4 " + v1);

            int v = Fab(1);
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

