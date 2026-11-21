using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using Bibim.Asset;
using Bibim.Asset.Pipeline;
using Bibim.Asset.Pipeline.Recipes;
using Bibim.Graphics;
using NUnit.Framework;
using Image = Bibim.Graphics.Image;

namespace Bibim.Tests.Asset.Pipeline
{
    /// <summary>
    /// Generated textures go to a "gen" directory next to the recipe, and the URIs in the cooked
    /// assets use "/". Before #150, Linux created files named "gen\Name_TEXTURE0.png" instead.
    /// </summary>
    [TestFixture]
    public class TextureOutputPathTests
    {
        [Test]
        public void DirectoryImageSetRecipeWritesTexturesUnderGen()
        {
            using (var directory = new TemporaryDirectory())
            {
                // Asset/Sprites.asset with the built-in "dirimageset" recipe, which packs Asset/Sprites/*.png.
                Directory.CreateDirectory(directory.Combine("Asset", "Sprites"));
                File.WriteAllText(directory.Combine("Asset", "Sprites.asset"), "{ \"usage\" : \"dirimageset\" }");
                SaveBitmap(directory.Combine("Asset", "Sprites", "Red.png"), Color.Red);
                SaveBitmap(directory.Combine("Asset", "Sprites", "Blue.png"), Color.Blue);

                var kitchen = new GameAssetKitchen(new GameAssetStorage());
                GameAssetKitchen.CookingReport report = kitchen.Cook(directory.Path, "Asset/Sprites.asset");

                var imageSet = (ImageSet)report.Asset;
                Assert.That(imageSet.Images.Keys, Is.EquivalentTo(new[] { "Red", "Blue" }));
                AssertTexturesUnderGen(directory, imageSet.Images.Values, "Sprites");
            }
        }

        [Test]
        public void BackslashOutputPathOfOlderRecipesStillWritesUnderGen()
        {
            // Recipe files written before #150 (Maple, touhoubreak) used "gen\\...".
            using (var directory = new TemporaryDirectory())
            {
                Directory.CreateDirectory(directory.Combine("Asset"));
                var kitchen = new GameAssetKitchen(new GameAssetStorage());
                var context = new CookingContext(kitchen, directory.Path, "Asset/Sprites.asset", null);
                context.SetVariable("AssetName", "Sprites");

                var images = new Dictionary<string, Image>();
                images.Add("Red", CreateImage(Color.Red));
                images.Add("Blue", CreateImage(Color.Blue));
                ExportImageSet.Export(context, new ImageSet(images),
                                      new Size(1024, 1024), new Size(16, 16),
                                      "gen\\$(AssetName)_TEXTURE{0}");

                AssertTexturesUnderGen(directory, images.Values, "Sprites");
            }
        }

        private static void AssertTexturesUnderGen(TemporaryDirectory directory, IEnumerable<Image> images, string assetName)
        {
            string texture = directory.Combine("Asset", "gen", assetName + "_TEXTURE0.png");
            Assert.That(File.Exists(texture), Is.True, texture);
            Assert.That(Directory.GetFiles(directory.Combine("Asset"), "*\\*"), Is.Empty,
                        "a file name contains a backslash");

            foreach (Image item in images)
                Assert.That(item.TextureURI, Is.EqualTo("Asset/gen/" + assetName + "_TEXTURE0"));
        }

        private static Image CreateImage(Color color)
        {
            return new Image(string.Empty, Rectangle.Empty)
            {
                Tag = new ImageCookingTag(CreateBitmap(color)),
            };
        }

        private static void SaveBitmap(string path, Color color)
        {
            using (Bitmap bitmap = CreateBitmap(color))
                bitmap.Save(path, ImageFormat.Png);
        }

        private static Bitmap CreateBitmap(Color color)
        {
            var result = new Bitmap(8, 8, PixelFormat.Format32bppArgb);
            using (var g = System.Drawing.Graphics.FromImage(result))
                g.Clear(color);

            return result;
        }
    }
}
