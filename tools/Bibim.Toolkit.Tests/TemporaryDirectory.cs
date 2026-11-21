using System;
using System.IO;

namespace Bibim.Tests
{
    /// <summary>
    /// A new empty directory under the system temporary directory, deleted on Dispose.
    /// </summary>
    public sealed class TemporaryDirectory : IDisposable
    {
        public string Path
        {
            get;
            private set;
        }

        public TemporaryDirectory()
        {
            Path = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "bibim-tests-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(Path);
        }

        public string Combine(params string[] parts)
        {
            string result = Path;
            foreach (string item in parts)
                result = System.IO.Path.Combine(result, item);

            return result;
        }

        public void Dispose()
        {
            try
            {
                if (Directory.Exists(Path))
                    Directory.Delete(Path, true);
            }
            catch (IOException)
            {
                // A FileSystemWatcher or a pending write may still hold a file; leave it to the OS.
            }
        }
    }
}
