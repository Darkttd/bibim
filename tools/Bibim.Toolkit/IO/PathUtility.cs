using System;

namespace Bibim.IO
{
    /// <summary>
    /// The asset tools write paths with "/" only. .NET accepts "/" on Windows too, but Path.Combine,
    /// Path.GetDirectoryName and FileSystemWatcher return "\" there, and older recipes contain "\".
    /// Pass such paths through Normalize before they are stored, compared or used as file names.
    /// </summary>
    public static class PathUtility
    {
        public static string Normalize(string path)
        {
            if (string.IsNullOrEmpty(path))
                return path;

            return path.Replace('\\', '/');
        }
    }
}
