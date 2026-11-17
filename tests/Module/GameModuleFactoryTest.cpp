// Tests for GameModuleFactory, which creates modules by class ID when loading
// game data.
//
// The factory keeps its entries in one process-wide table, and nothing can
// remove them, so the IDs used here must not be registered anywhere else in the
// test executable.
#include <Bibim/Config.h>
#include <Bibim/GameModule.h>
#include <Bibim/GameModuleFactory.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    class FirstModule : public GameModule
    {
        BBModuleClass(FirstModule, GameModule, 'F', 'T', 'S', 'A');
    };

    class SecondModule : public GameModule
    {
        BBModuleClass(SecondModule, GameModule, 'F', 'T', 'S', 'M');
    };

    class ThirdModule : public GameModule
    {
        BBModuleClass(ThirdModule, GameModule, 'F', 'T', 'S', 'Z');
    };

    // Registers the three modules once, out of ID order, as the engine does at
    // startup before calling SortEntries.
    void RegisterTestModules()
    {
        static bool registered = false;
        if (registered)
            return;

        GameModuleFactory::AddEntry<ThirdModule>();
        GameModuleFactory::AddEntry<FirstModule>();
        GameModuleFactory::AddEntry<SecondModule>();
        GameModuleFactory::SortEntries();
        registered = true;
    }

    int ClassIDOf(GameModule* module)
    {
        const int id = module ? module->GetClassID() : 0;
        delete module;
        return id;
    }
}

TEST_CASE("GameModuleFactory::Create creates every registered module by its class ID")
{
    RegisterTestModules();
    // Copies: CHECK binds its operands by reference, and the in-class ClassID
    // constants have no out-of-class definition to bind to.
    const int first = FirstModule::ClassID;
    const int second = SecondModule::ClassID;
    const int third = ThirdModule::ClassID;

    CHECK(ClassIDOf(GameModuleFactory::Create(first)) == first);
    CHECK(ClassIDOf(GameModuleFactory::Create(second)) == second);
    CHECK(ClassIDOf(GameModuleFactory::Create(third)) == third);
}

TEST_CASE("GameModuleFactory::Create returns null for an unregistered class ID")
{
    RegisterTestModules();
    const int first = FirstModule::ClassID;
    const int third = ThirdModule::ClassID;

    // Below, between and above the registered IDs (their last letters are A, M, Z).
    CHECK(GameModuleFactory::Create(first - 1) == nullptr);
    CHECK(GameModuleFactory::Create(first + 1) == nullptr);
    CHECK(GameModuleFactory::Create(third + 1) == nullptr);
}
