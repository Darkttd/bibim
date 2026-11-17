using System;
using System.Collections.Concurrent;

namespace Bibim.Bab.Consoles
{
    /// <summary>
    /// Passes the commands the user types to the running console program.
    /// Both the WPF ConsoleWindow and the CLI host (Bibim.Baaaaaab.Cli) use it, so it must not depend on any UI.
    /// </summary>
    public static class ConsoleCommandQueue
    {
        public static readonly ConcurrentQueue<string> Commands = new ConcurrentQueue<string>();
    }
}
