using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using Bibim.Asset.Pipeline.Recipes;
using NUnit.Framework;

namespace Bibim.Tests.Asset.Pipeline
{
    /// <summary>
    /// Texture cooking encodes with FreeImage (FreeImageNET). On Linux this needs the native library
    /// (libfreeimage.so.3) and the dllmap in tools/References/FreeImageNET.dll.config (#149).
    /// </summary>
    [TestFixture]
    public class BitmapToTexture2DTests
    {
        [Test]
        public void PNGKeepsEveryPixelOfAnOpaqueBitmap()
        {
            using (Bitmap source = CreateBitmap(64, 32, (x, y) => Color.FromArgb(255, x * 4, y * 8, 128)))
            {
                byte[] png = BitmapToTexture2D.GeneratePNGBuffer(source);

                Assert.That(png, Is.Not.Empty);
                Assert.That(MaxChannelDifference(source, png, true), Is.EqualTo(0));
            }
        }

        [Test]
        public void PNGKeepsEveryPixelOfATranslucentBitmap()
        {
            using (Bitmap source = CreateBitmap(16, 16, (x, y) => Color.FromArgb(x * 16, 200, y * 16, 50)))
            {
                byte[] png = BitmapToTexture2D.GeneratePNGBuffer(source);

                Assert.That(MaxChannelDifference(source, png, true), Is.EqualTo(0));
            }
        }

        [Test]
        public void JPEGEncodesA32BitBitmap()
        {
            // FreeImage 3.18 refuses to save 32-bit RGBA as JPEG unless it is converted to 24 bits first.
            using (Bitmap source = CreateBitmap(64, 32, (x, y) => Color.FromArgb(255, x * 4, y * 8, 128)))
            {
                byte[] jpeg = BitmapToTexture2D.GenerateJPEGBuffer(source);

                Assert.That(jpeg.Length, Is.GreaterThan(2));
                Assert.That(jpeg[0], Is.EqualTo(0xFF), "JPEG SOI marker");
                Assert.That(jpeg[1], Is.EqualTo(0xD8), "JPEG SOI marker");
                Assert.That(MaxChannelDifference(source, jpeg, false), Is.LessThan(16));
            }
        }

        private static Bitmap CreateBitmap(int width, int height, Func<int, int, Color> color)
        {
            var result = new Bitmap(width, height, PixelFormat.Format32bppArgb);
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                    result.SetPixel(x, y, color(x, y));
            }

            return result;
        }

        private static int MaxChannelDifference(Bitmap source, byte[] encoded, bool compareAlpha)
        {
            using (var stream = new MemoryStream(encoded))
            using (var decoded = new Bitmap(stream))
            {
                Assert.That(decoded.Size, Is.EqualTo(source.Size));

                int result = 0;
                for (int y = 0; y < source.Height; y++)
                {
                    for (int x = 0; x < source.Width; x++)
                    {
                        Color a = source.GetPixel(x, y);
                        Color b = decoded.GetPixel(x, y);
                        result = Math.Max(result, Math.Abs(a.R - b.R));
                        result = Math.Max(result, Math.Abs(a.G - b.G));
                        result = Math.Max(result, Math.Abs(a.B - b.B));
                        if (compareAlpha)
                            result = Math.Max(result, Math.Abs(a.A - b.A));
                    }
                }

                return result;
            }
        }
    }
}
