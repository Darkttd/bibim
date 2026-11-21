using NUnitLite;

namespace Bibim.Tests
{
    /// <summary>
    /// Runs the tests in this assembly with NUnitLite, so no separate test runner is needed:
    /// "mono Bibim.Toolkit.Tests.exe" on Linux, "Bibim.Toolkit.Tests.exe" on Windows.
    /// The exit code is the number of failed tests. NUnitLite options such as --where work too.
    /// </summary>
    public static class Program
    {
        public static int Main(string[] args)
        {
            return new AutoRun().Execute(args);
        }
    }
}
