using System;

namespace CnC
{
    public static class ColorConsole
    {
        private static object sync;

        static ColorConsole()
        {
            ColorConsole.sync = new object();
        }

        public static void WriteLine(string value)
        {
            lock (sync)
            {
                Console.WriteLine(value);
            }
        }

        public static void Write(ConsoleColor textColor, string value, params object[] args)
        {
            string str = string.Format(value, args);
            Write(textColor, str);
        }


        public static void WriteLine(ConsoleColor textColor, string value, params object[] args)
        {
            string str = string.Format(value, args);
            WriteLine(textColor, str);
        }

        public static void WriteLine(ConsoleColor textColor, ConsoleColor backColor, string value, params object[] args)
        {
            string str = string.Format(value, args);
            WriteLine(textColor, backColor, str);
        }

        public static void Write(ConsoleColor textColor, ConsoleColor backColor, string value, params object[] args)
        {
            string str = string.Format(value, args);
            Write(textColor, backColor, str);
        }

        public static void WriteLine(ConsoleColor textColor, string value)
        {
            lock (sync)
            {
                ConsoleColor old_color = Console.ForegroundColor;
                Console.ForegroundColor = textColor;
                Console.WriteLine(value);
                Console.ForegroundColor = old_color;
            }
        }

        public static void Write(ConsoleColor textColor, string value)
        {
            lock (sync)
            {
                ConsoleColor old_color = Console.ForegroundColor;
                Console.ForegroundColor = textColor;
                Console.Write(value);
                Console.ForegroundColor = old_color;
            }
        }



        public static void WriteLine(ConsoleColor textColor, ConsoleColor backColor, string value)
        {
            lock (sync)
            {
                ConsoleColor old_text_color = Console.ForegroundColor;
                ConsoleColor old_back_color = Console.BackgroundColor;
                Console.ForegroundColor = textColor;
                Console.BackgroundColor = backColor;
                Console.WriteLine(value);
                Console.ForegroundColor = old_text_color;
                Console.BackgroundColor = old_back_color;

            }
        }
    }
}