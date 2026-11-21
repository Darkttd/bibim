using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Threading;
using Bibim.Asset;
using Bibim.Asset.Pipeline;
using NUnit.Framework;

namespace Bibim.Tests.Asset.Pipeline
{
    /// <summary>
    /// The asset server caches cooked assets and drops a cache entry when one of its source files
    /// changes (hot reload). Before #153, Linux never set up the file watchers, so the cache stayed.
    /// </summary>
    [TestFixture]
    public class GameAssetServerTests
    {
        private const string AssetPath = "Asset/Sprites.asset";
        private static readonly TimeSpan Timeout = TimeSpan.FromSeconds(30);

        private sealed class TestServer : GameAssetServer
        {
            public TestServer(GameAssetKitchen kitchen)
                : base(kitchen)
            {
            }
        }

        [Test]
        public void ChangingASourceFileClearsTheCache()
        {
            using (var directory = new TemporaryDirectory())
            {
                string recipe = directory.Combine("Asset", "Sprites.asset");
                Directory.CreateDirectory(directory.Combine("Asset", "Sprites"));
                File.WriteAllText(recipe, "{ \"usage\" : \"dirimageset\" }");
                using (var bitmap = new Bitmap(8, 8, PixelFormat.Format32bppArgb))
                    bitmap.Save(directory.Combine("Asset", "Sprites", "Red.png"), ImageFormat.Png);

                var server = new TestServer(new GameAssetKitchen(new GameAssetStorage()));
                try
                {
                    Cook(server, directory.Path);
                    Assert.That(WaitUntil(() => IsCached(server, directory.Path)), Is.True,
                                "the cooked asset was not cached");

                    // The server sets up its file watchers after it stores the cache entry, so keep
                    // changing the recipe until the entry goes away.
                    bool cleared = WaitUntil(() =>
                    {
                        File.AppendAllText(recipe, " ");
                        Thread.Sleep(200);
                        return IsCached(server, directory.Path) == false;
                    });
                    Assert.That(cleared, Is.True, "changing the recipe did not clear the cache");
                }
                finally
                {
                    server.Alive = false; // Stops the cooking thread and the file watchers.
                }
            }
        }

        private static void Cook(GameAssetServer server, string directory)
        {
            // Not disposed: when the server's work after the callback throws, it also calls the
            // fallback, possibly after this method has returned.
            var done = new ManualResetEvent(false);
            bool cooked = false;
            server.Precook(directory, AssetPath,
                           (buffer, index, count) => { cooked = true; done.Set(); },
                           () => done.Set());

            Assert.That(done.WaitOne(Timeout), Is.True, "cooking did not finish");
            Assert.That(cooked, Is.True, "cooking failed");
        }

        /// <summary>
        /// A cached asset is passed to the callback before Precook returns, on the calling thread.
        /// Otherwise Precook queues a cooking task, which calls back on the cooking thread.
        /// </summary>
        private static bool IsCached(GameAssetServer server, string directory)
        {
            int callingThread = Thread.CurrentThread.ManagedThreadId;
            bool cached = false;
            server.Precook(directory, AssetPath,
                           (buffer, index, count) =>
                           {
                               if (Thread.CurrentThread.ManagedThreadId == callingThread)
                                   cached = true;
                           },
                           null);
            return cached;
        }

        private static bool WaitUntil(Func<bool> condition)
        {
            Stopwatch stopwatch = Stopwatch.StartNew();
            while (stopwatch.Elapsed < Timeout)
            {
                if (condition())
                    return true;

                Thread.Sleep(100);
            }

            return false;
        }
    }
}
