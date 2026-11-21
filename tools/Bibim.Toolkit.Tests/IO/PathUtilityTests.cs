using Bibim.IO;
using NUnit.Framework;

namespace Bibim.Tests.IO
{
    [TestFixture]
    public class PathUtilityTests
    {
        [TestCase(@"gen\titleUI_TEXTURE0", "gen/titleUI_TEXTURE0")]
        [TestCase(@"Asset\UI\gen/titleUI", "Asset/UI/gen/titleUI")]
        [TestCase(@"C:\Game\Asset\UI\titleUI.asset", "C:/Game/Asset/UI/titleUI.asset")]
        [TestCase("Asset/UI/titleUI", "Asset/UI/titleUI")]
        [TestCase("titleUI", "titleUI")]
        public void NormalizeReplacesBackslashesWithSlashes(string path, string expected)
        {
            Assert.That(PathUtility.Normalize(path), Is.EqualTo(expected));
        }

        [Test]
        public void NormalizeKeepsNullAndEmpty()
        {
            Assert.That(PathUtility.Normalize(null), Is.Null);
            Assert.That(PathUtility.Normalize(string.Empty), Is.Empty);
        }
    }
}
