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
    /// Runs the Bibim.Baaaaaab console programs ([ConsoleEntryPoint]) in a terminal, without WPF.
    /// The command line is the same as the console mode of Bibim.Baaaaaab.exe:
    ///   Bibim.Baaaaaab.Cli.exe console /class:AssetServer [/Key:Value ...]
    /// Each line on standard input is sent to the running program as a command (for example exit).
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

            // When standard input closes or Ctrl+C is pressed, send the same stop commands as closing the WPF window.
            // (Mono installs the Ctrl+C handler only when standard input is a terminal; any other SIGINT terminates at once.)
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
                // The first Ctrl+C requests a graceful stop; the second terminates at once.
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
        /// Loads the assemblies in the Plugin folder of the current directory, like Bibim.Baaaaaab (App.OnStartup).
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
        /// Parses "/Key:Value" and "/Flag" command-line arguments (same rules as App.OnStartup).
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
        /// Converts the command-line arguments to the console program's parameters (same rules as ConsoleWindow).
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
