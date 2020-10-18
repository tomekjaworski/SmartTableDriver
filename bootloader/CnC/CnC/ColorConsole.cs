using System;

namespace CnC
{
    public static class ColorConsole
    {
        private static object sync;

        public static object Synchronization => sync;

        static ColorConsole()
        {
            ColorConsole.sync = new object();
        }

        #region Write/WriteLine
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

        #endregion

        public static void PressAnyKey(string messge = "Press any key...", bool clearInput = true)
        {
            while (clearInput && Console.KeyAvailable)
                Console.ReadKey(false);

            if (!string.IsNullOrEmpty(messge))
                Console.Write(messge);

            Console.ReadKey(true);
            Console.WriteLine();
        }

        internal static void WriteXY(int left, int top, string str)
        {
            lock(ColorConsole.sync)
            {
                int old_left = Console.CursorLeft;
                int old_top = Console.CursorTop;
                Console.SetCursorPosition(left, top);


                Console.Write(str);


                Console.SetCursorPosition(old_left, old_top);

            }
        }

        internal static void WriteXY(int left, int top, ConsoleColor textColor, string str)
        {
            lock (ColorConsole.sync)
            {
                int old_left = Console.CursorLeft;
                int old_top = Console.CursorTop;
                ConsoleColor old_text = Console.ForegroundColor;

                Console.SetCursorPosition(left, top);
                Console.ForegroundColor = textColor;
                Console.Write(str);

                Console.SetCursorPosition(old_left, old_top);
                Console.ForegroundColor = old_text;
            }
        }

        internal static void WriteXY(int left, int top, char ch)
        {
            lock (ColorConsole.sync)
            {
                int old_left = Console.CursorLeft;
                int old_top = Console.CursorTop;
                Console.SetCursorPosition(left, top);
                Console.Write(ch);
                Console.SetCursorPosition(old_left, old_top);
            }
        }
    }
}