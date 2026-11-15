using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading;
using Bibim.Bab.Consoles;
using Bibim.Toolkit;

namespace Bibim.Bab
{
    /// <summary>
    /// Bibim.Baaaaaab의 콘솔 프로그램([ConsoleEntryPoint])을 WPF 없이 터미널에서 실행합니다.
    /// 명령행 형식은 Bibim.Baaaaaab.exe의 console 모드와 같습니다:
    ///   Bibim.Baaaaaab.Cli.exe console /class:AssetServer [/Key:Value ...]
    /// 표준 입력의 각 줄은 실행 중인 프로그램에 명령으로 전달됩니다(예: exit).
    /// </summary>
    public static class Program
    {
        private static readonly string[] stopCommands = { "close", "exit", "quit" };

        public static int Main(string[] args)
        {
            Trace.Listeners.Add(new ConsoleTraceListener());

            LoadPlugins();

            SortedList<string, string> options = ParseOptions(args);
            string className;
            if (args.Length == 0 || IsConsoleMode(args[0]) == false || options.TryGetValue("class", out className) == false)
            {
                Console.Error.WriteLine("Usage: Bibim.Baaaaaab.Cli.exe console /class:<Name> [/Key:Value ...]");
                return 2;
            }

            MethodInfo programEntry = ConsoleEntryPointAttribute.Find(className);
            if (programEntry == null)
            {
                Console.Error.WriteLine("Console program not found: {0}", className);
                return 2;
            }

            object[] programArgs = ConvertArguments(programEntry, options);

            // 표준 입력이 닫히거나 Ctrl+C를 누르면 WPF 창을 닫을 때와 같은 종료 명령을 보냅니다.
            // (Mono는 표준 입력이 터미널일 때만 Ctrl+C 처리기를 설치합니다. 그 밖의 SIGINT는 즉시 종료됩니다.)
            var inputThread = new Thread(() =>
            {
                string line;
                while ((line = Console.ReadLine()) != null)
                    ConsoleCommandQueue.Commands.Enqueue(line);
                RequestStop();
            });
            inputThread.IsBackground = true;
            inputThread.Start();

            int cancelCount = 0;
            Console.CancelKeyPress += (sender, e) =>
            {
                // 첫 Ctrl+C는 정상 종료를 요청하고, 두 번째는 즉시 종료합니다.
                if (Interlocked.Increment(ref cancelCount) == 1)
                {
                    e.Cancel = true;
                    RequestStop();
                }
            };

            try
            {
                programEntry.Invoke(null, programArgs);
                return 0;
            }
            catch (TargetInvocationException ex)
            {
                Trace.WriteLine(ex.InnerException);
                return 1;
            }
        }

        private static bool IsConsoleMode(string mode)
        {
            return string.Compare(mode, "con", true) == 0 ||
                   string.Compare(mode, "console", true) == 0;
        }

        private static void RequestStop()
        {
            foreach (string item in stopCommands)
                ConsoleCommandQueue.Commands.Enqueue(item);
        }

        /// <summary>
        /// Bibim.Baaaaaab(App.OnStartup)처럼 현재 디렉터리의 Plugin 폴더에 있는 어셈블리를 불러옵니다.
        /// </summary>
        private static void LoadPlugins()
        {
            if (Directory.Exists("Plugin") == false)
                return;

            foreach (string item in Directory.GetFiles("Plugin", "*.dll", SearchOption.TopDirectoryOnly))
            {
                try
                {
                    AppDomain.CurrentDomain.Load(AssemblyName.GetAssemblyName(item));
                }
                catch (Exception ex)
                {
                    Trace.TraceWarning("Plugin load failed > {0}: {1}", item, ex.Message);
                }
            }
        }

        /// <summary>
        /// "/Key:Value" 혹은 "/Flag" 형태의 명령행 인자를 분석합니다. (App.OnStartup과 같은 규칙)
        /// </summary>
        private static SortedList<string, string> ParseOptions(string[] args)
        {
            var result = new SortedList<string, string>(args.Length, StringComparer.CurrentCultureIgnoreCase);
            foreach (string item in args)
            {
                Match match = Regex.Match(item, @"(?<name>/\w+):(?<value>.+)");
                if (match.Success)
                {
                    result[match.Groups["name"].Value.Substring(1)] = match.Groups["value"].Value;
                    continue;
                }

                match = Regex.Match(item, @"(?<name>/\w+)");
                if (match.Success)
                    result[match.Groups["name"].Value.Substring(1)] = "yes";
            }

            return result;
        }

        /// <summary>
        /// 명령행 인자를 콘솔 프로그램의 매개변수에 맞게 변환합니다. (ConsoleWindow와 같은 규칙)
        /// </summary>
        private static object[] ConvertArguments(MethodInfo programEntry, SortedList<string, string> options)
        {
            ParameterInfo[] parameters = programEntry.GetParameters();
            object[] result = new object[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                foreach (KeyValuePair<string, string> item in options)
                {
                    if (AliasAttribute.MatchName(parameters[i], item.Key))
                    {
                        try
                        {
                            result[i] = Convert.ChangeType(item.Value, parameters[i].ParameterType);
                        }
                        catch (FormatException) { result[i] = null; }
                        catch (InvalidCastException) { result[i] = null; }

                        break;
                    }
                }
            }

            return result;
        }
    }
}
