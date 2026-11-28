#include <Bibim/Config.h>
#include <Bibim/SequenceDictionary.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    typedef SequenceDictionary::KeyValuePair Pair;

    SequenceDictionary MakeDictionary(bool optimize)
    {
        SequenceDictionary d;
        d.Add("pear", "3");
        d.Add("apple", "1");
        d.Add("melon", "2");
        if (optimize)
            d.Optimize();
        return d;
    }

    // Add asserts against duplicate keys in Debug, so build them through Swap.
    SequenceDictionary MakeDictionaryWithDuplicates(bool optimize)
    {
        SequenceDictionary::Container items;
        items.push_back(Pair("b", "first"));
        items.push_back(Pair("a", "x"));
        items.push_back(Pair("b", "second"));
        items.push_back(Pair("c", "y"));
        items.push_back(Pair("b", "third"));

        SequenceDictionary d;
        d.Swap(items);
        if (optimize)
            d.Optimize();
        return d;
    }
}

TEST_CASE("SequenceDictionary starts empty and clean")
{
    SequenceDictionary d;
    CHECK(d.GetItems().empty());
    CHECK_FALSE(d.IsDirty());
    CHECK_FALSE(d.Contains("a"));
    CHECK(d.Find("a") == String::Empty);
    CHECK_FALSE(d.Remove("a"));
    CHECK_FALSE(d.Remove("a", "1"));

    SequenceDictionary reserved(16);
    CHECK(reserved.GetItems().empty());
    CHECK(reserved.GetItems().capacity() >= 16);
    CHECK_FALSE(reserved.IsDirty());
}

TEST_CASE("SequenceDictionary::Add appends in order and marks the dictionary dirty")
{
    SequenceDictionary d = MakeDictionary(false);
    CHECK(d.IsDirty());
    REQUIRE(d.GetItems().size() == 3);
    CHECK(d.GetItems()[0] == Pair("pear", "3"));
    CHECK(d.GetItems()[1] == Pair("apple", "1"));
    CHECK(d.GetItems()[2] == Pair("melon", "2"));
}

TEST_CASE("SequenceDictionary::Optimize sorts the items by key and clears the dirty flag")
{
    SequenceDictionary d = MakeDictionary(true);
    CHECK_FALSE(d.IsDirty());
    REQUIRE(d.GetItems().size() == 3);
    CHECK(d.GetItems()[0] == Pair("apple", "1"));
    CHECK(d.GetItems()[1] == Pair("melon", "2"));
    CHECK(d.GetItems()[2] == Pair("pear", "3"));

    d.Optimize();
    CHECK_FALSE(d.IsDirty());
    CHECK(d.GetItems()[0] == Pair("apple", "1"));
}

TEST_CASE("SequenceDictionary::Optimize orders keys by byte value")
{
    SequenceDictionary d;
    d.Add("b", "");
    d.Add("a", "");
    d.Add("B", "");
    d.Add("ab", "");
    d.Add("", "");
    d.Optimize();

    REQUIRE(d.GetItems().size() == 5);
    CHECK(d.GetItems()[0].Key == "");
    CHECK(d.GetItems()[1].Key == "B");
    CHECK(d.GetItems()[2].Key == "a");
    CHECK(d.GetItems()[3].Key == "ab");
    CHECK(d.GetItems()[4].Key == "b");
}

TEST_CASE("SequenceDictionary finds keys before and after Optimize")
{
    for (int optimize = 0; optimize < 2; optimize++)
    {
        CAPTURE(optimize);
        const SequenceDictionary d = MakeDictionary(optimize != 0);

        CHECK(d.Contains("apple"));
        CHECK(d.Contains("melon"));
        CHECK(d.Contains("pear"));
        CHECK(d.Find("apple") == "1");
        CHECK(d.Find("melon") == "2");
        CHECK(d.Find("pear") == "3");

        CHECK_FALSE(d.Contains("Apple"));
        CHECK_FALSE(d.Contains("app"));
        CHECK_FALSE(d.Contains("zebra"));
        CHECK_FALSE(d.Contains(""));
        CHECK(d.Find("aardvark") == String::Empty);
        CHECK(d.Find("zebra") == String::Empty);
    }
}

TEST_CASE("SequenceDictionary::Add after Optimize marks the dictionary dirty again")
{
    SequenceDictionary d = MakeDictionary(true);
    d.Add("aaa", "0");

    CHECK(d.IsDirty());
    CHECK(d.GetItems().back() == Pair("aaa", "0"));
    CHECK(d.Find("aaa") == "0");
    CHECK(d.Find("apple") == "1");
    CHECK(d.Find("pear") == "3");

    d.Optimize();
    CHECK(d.GetItems()[0] == Pair("aaa", "0"));
    CHECK(d.Find("aaa") == "0");
}

TEST_CASE("SequenceDictionary::Remove removes a key before and after Optimize")
{
    for (int optimize = 0; optimize < 2; optimize++)
    {
        CAPTURE(optimize);
        SequenceDictionary d = MakeDictionary(optimize != 0);

        CHECK(d.Remove("melon"));
        CHECK(d.GetItems().size() == 2);
        CHECK_FALSE(d.Contains("melon"));
        CHECK(d.Find("apple") == "1");
        CHECK(d.Find("pear") == "3");
        CHECK(d.IsDirty() == (optimize == 0));
        if (optimize)
        {
            CHECK(d.GetItems()[0] == Pair("apple", "1"));
            CHECK(d.GetItems()[1] == Pair("pear", "3"));
        }

        CHECK_FALSE(d.Remove("melon"));
        CHECK_FALSE(d.Remove("kiwi"));
        CHECK(d.GetItems().size() == 2);
    }
}

TEST_CASE("SequenceDictionary::Remove with a value removes only the matching pair")
{
    for (int optimize = 0; optimize < 2; optimize++)
    {
        CAPTURE(optimize);
        SequenceDictionary d = MakeDictionary(optimize != 0);

        CHECK_FALSE(d.Remove("apple", "2"));
        CHECK_FALSE(d.Remove("kiwi", "1"));
        CHECK(d.GetItems().size() == 3);

        CHECK(d.Remove("apple", "1"));
        CHECK(d.GetItems().size() == 2);
        CHECK_FALSE(d.Contains("apple"));
        CHECK(d.Find("melon") == "2");
        CHECK(d.Find("pear") == "3");
    }
}

TEST_CASE("SequenceDictionary keeps duplicate keys and removes one pair at a time")
{
    for (int optimize = 0; optimize < 2; optimize++)
    {
        CAPTURE(optimize);
        SequenceDictionary d = MakeDictionaryWithDuplicates(optimize != 0);
        REQUIRE(d.GetItems().size() == 5);

        const String& found = d.Find("b");
        CHECK((found == "first" || found == "second" || found == "third"));

        CHECK(d.Remove("b", "third"));
        CHECK_FALSE(d.Remove("b", "third"));
        CHECK(d.Remove("b", "first"));
        CHECK(d.GetItems().size() == 3);
        CHECK(d.Find("b") == "second");
        CHECK(d.Find("a") == "x");
        CHECK(d.Find("c") == "y");

        CHECK(d.Remove("b"));
        CHECK_FALSE(d.Contains("b"));
        CHECK_FALSE(d.Remove("b"));
        CHECK(d.GetItems().size() == 2);
    }
}

TEST_CASE("SequenceDictionary::Remove without a value removes one pair of a duplicate key")
{
    for (int optimize = 0; optimize < 2; optimize++)
    {
        CAPTURE(optimize);
        SequenceDictionary d = MakeDictionaryWithDuplicates(optimize != 0);

        CHECK(d.Remove("b"));
        CHECK(d.Remove("b"));
        CHECK(d.Contains("b"));
        CHECK(d.Remove("b"));
        CHECK_FALSE(d.Contains("b"));
        CHECK(d.GetItems().size() == 2);
    }
}

TEST_CASE("SequenceDictionary::Swap with a container takes its items and marks the dictionary dirty")
{
    SequenceDictionary d = MakeDictionary(true);
    REQUIRE_FALSE(d.IsDirty());

    SequenceDictionary::Container items;
    items.push_back(Pair("z", "26"));
    items.push_back(Pair("y", "25"));
    d.Swap(items);

    CHECK(d.IsDirty());
    REQUIRE(d.GetItems().size() == 2);
    CHECK(d.GetItems()[0] == Pair("z", "26"));
    CHECK(d.GetItems()[1] == Pair("y", "25"));
    CHECK(d.Find("y") == "25");

    REQUIRE(items.size() == 3);
    CHECK(items[0] == Pair("apple", "1"));
}

TEST_CASE("SequenceDictionary::Swap with another dictionary exchanges items and dirty flags")
{
    SequenceDictionary a = MakeDictionary(true);
    SequenceDictionary b;
    b.Add("k", "v");

    a.Swap(b);

    CHECK(a.IsDirty());
    REQUIRE(a.GetItems().size() == 1);
    CHECK(a.Find("k") == "v");

    CHECK_FALSE(b.IsDirty());
    REQUIRE(b.GetItems().size() == 3);
    CHECK(b.Find("pear") == "3");
}

TEST_CASE("SequenceDictionary copies its items and dirty flag")
{
    const SequenceDictionary original = MakeDictionary(true);

    SequenceDictionary copy(original);
    CHECK(copy == original);
    CHECK_FALSE(copy != original);
    CHECK_FALSE(copy.IsDirty());

    SequenceDictionary assigned;
    assigned = original;
    CHECK(assigned == original);

    copy.Remove("apple");
    CHECK(copy != original);
    CHECK(original.Contains("apple"));

    const SequenceDictionary dirty = MakeDictionary(false);
    CHECK(SequenceDictionary(dirty).IsDirty());
    assigned = dirty;
    CHECK(assigned.IsDirty());
    CHECK(assigned == dirty);
}

TEST_CASE("SequenceDictionary compares items as if both sides were optimized")
{
    const SequenceDictionary optimized = MakeDictionary(true);

    SequenceDictionary sortedButDirty;
    sortedButDirty.Add("apple", "1");
    sortedButDirty.Add("melon", "2");
    sortedButDirty.Add("pear", "3");
    CHECK(sortedButDirty == optimized);
    CHECK(optimized == sortedButDirty);
    CHECK(MakeDictionary(false) == optimized);
    CHECK_FALSE(MakeDictionary(false) != optimized);

    SequenceDictionary reordered;
    reordered.Add("melon", "2");
    reordered.Add("pear", "3");
    reordered.Add("apple", "1");
    CHECK(reordered == MakeDictionary(false));

    SequenceDictionary::Container items;
    items.push_back(Pair("b", "third"));
    items.push_back(Pair("c", "y"));
    items.push_back(Pair("b", "second"));
    items.push_back(Pair("a", "x"));
    items.push_back(Pair("b", "first"));
    SequenceDictionary duplicates;
    duplicates.Swap(items);
    CHECK(duplicates == MakeDictionaryWithDuplicates(false));
    CHECK(duplicates == MakeDictionaryWithDuplicates(true));
    duplicates.Optimize();
    CHECK(duplicates == MakeDictionaryWithDuplicates(true));

    CHECK(SequenceDictionary() == SequenceDictionary());

    CHECK(reordered.IsDirty());
    CHECK(reordered.GetItems()[0] == Pair("melon", "2"));
}

TEST_CASE("SequenceDictionary compares unequal when the items differ")
{
    const SequenceDictionary optimized = MakeDictionary(true);

    SequenceDictionary otherValue;
    otherValue.Add("pear", "3");
    otherValue.Add("apple", "1");
    otherValue.Add("melon", "9");
    CHECK(otherValue != optimized);
    CHECK_FALSE(otherValue == optimized);

    SequenceDictionary otherKey;
    otherKey.Add("pear", "3");
    otherKey.Add("apple", "1");
    otherKey.Add("lemon", "2");
    CHECK(otherKey != optimized);

    SequenceDictionary fewer;
    fewer.Add("pear", "3");
    fewer.Add("apple", "1");
    CHECK(fewer != optimized);
    CHECK(optimized != fewer);

    SequenceDictionary::Container items;
    items.push_back(Pair("b", "first"));
    items.push_back(Pair("a", "x"));
    items.push_back(Pair("b", "first"));
    items.push_back(Pair("c", "y"));
    items.push_back(Pair("b", "third"));
    SequenceDictionary duplicates;
    duplicates.Swap(items);
    CHECK(duplicates != MakeDictionaryWithDuplicates(false));
    CHECK(duplicates != MakeDictionaryWithDuplicates(true));
}

TEST_CASE("SequenceDictionary::KeyValuePair compares both key and value")
{
    const Pair empty;
    CHECK(empty.Key == String::Empty);
    CHECK(empty.Value == String::Empty);

    const Pair pair("k", "v");
    Pair copy(pair);
    CHECK(copy == pair);

    Pair assigned;
    assigned = pair;
    CHECK(assigned == pair);

    CHECK(Pair("k", "w") != pair);
    CHECK(Pair("j", "v") != pair);
}
