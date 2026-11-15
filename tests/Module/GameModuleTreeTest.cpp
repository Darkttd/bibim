// Tests for GameModule / GameModuleNode / GameModuleTree.
//
// Ownership: modules and nodes created with new are not deleted here. Once
// attached, the tree owns them: ~GameModuleTree deletes the root node, each
// ~GameModuleNode deletes its children and its module, and DetachChild deletes
// the detached node. A node built with new GameModuleNode(...) has no owner
// until it is attached.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/GameModule.h>
#include <Bibim/GameModuleNode.h>
#include <Bibim/GameModuleTree.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    class TestModule : public GameModule
    {
        BBModuleClass(TestModule, GameModule, 'T', 'S', 'T', 'A');
        public:
            static int Destroyed;

            TestModule() { }
            explicit TestModule(int id) : GameModule(id) { }
            virtual ~TestModule() { Destroyed++; }
    };

    int TestModule::Destroyed = 0;

    class OtherModule : public GameModule
    {
        BBModuleClass(OtherModule, GameModule, 'T', 'S', 'T', 'B');
        public:
            OtherModule() { }
            explicit OtherModule(int id) : GameModule(id) { }
    };
}

TEST_CASE("GameModuleTree finds attached modules by ID")
{
    GameModuleTree tree;
    TestModule* a = new TestModule(1);
    TestModule* b = new TestModule(2);
    GameModuleNode* aNode = tree.GetRoot()->AttachChild(a);
    aNode->AttachChild(b);

    CHECK(tree.Find(1) == a);
    CHECK(tree.Find(2) == b);
    CHECK(tree.Find(3) == nullptr);
    CHECK(tree.GetRoot()->FindChild(2) == b);
}

TEST_CASE("GameModuleTree finds modules by class, including nested ones")
{
    GameModuleTree tree;
    TestModule* a = new TestModule();
    OtherModule* nested = new OtherModule();
    tree.GetRoot()->AttachChild(a)->AttachChild(nested);

    CHECK(tree.FindModule<TestModule>() == a);
    CHECK(tree.FindModule<OtherModule>() == nested);
}

TEST_CASE("Modules attached to a detached subtree are indexed when the subtree joins the tree")
{
    GameModuleTree tree;
    TestModule* parent = new TestModule(10);
    TestModule* child = new TestModule(11);

    GameModuleNode* parentNode = new GameModuleNode(parent);
    parentNode->AttachChild(child);
    CHECK(tree.Find(11) == nullptr);

    tree.GetRoot()->AttachChild(parentNode);
    CHECK(tree.Find(10) == parent);
    CHECK(tree.Find(11) == child);
    CHECK(parentNode->GetTree() == &tree);
}

TEST_CASE("DetachChild destroys the node, its module and its descendants, and unindexes them")
{
    GameModuleTree tree;
    TestModule* parent = new TestModule(20);
    GameModuleNode* parentNode = tree.GetRoot()->AttachChild(parent);
    parentNode->AttachChild(new TestModule(21));

    TestModule::Destroyed = 0;
    CHECK(tree.GetRoot()->DetachChild(parentNode));
    CHECK(TestModule::Destroyed == 2);
    CHECK(tree.Find(20) == nullptr);
    CHECK(tree.Find(21) == nullptr);
}

TEST_CASE("Destroying the tree destroys every module in it")
{
    TestModule::Destroyed = 0;
    {
        GameModuleTree tree;
        tree.GetRoot()->AttachChild(new TestModule())->AttachChild(new TestModule());
        tree.GetRoot()->AttachChild(new TestModule());
    }
    CHECK(TestModule::Destroyed == 3);
}

TEST_CASE("GameModule status combines alive and active")
{
    GameModuleTree tree;
    TestModule* m = new TestModule();
    tree.GetRoot()->AttachChild(m);

    CHECK(m->GetStatus() == GameModule::ActiveStatus);
    m->SetActive(false);
    CHECK(m->GetStatus() == GameModule::AliveStatus);
    m->SetAlive(false);
    CHECK(m->GetStatus() == GameModule::DeadStatus);
    m->SetActive(true);
    CHECK(m->GetStatus() == GameModule::DeadStatus);
}

TEST_CASE("Killing a module kills its descendants; reviving one revives its ancestors")
{
    GameModuleTree tree;
    TestModule* top = new TestModule();
    TestModule* middle = new TestModule();
    TestModule* leaf = new TestModule();
    TestModule* sibling = new TestModule();
    GameModuleNode* middleNode = tree.GetRoot()->AttachChild(top)->AttachChild(middle);
    middleNode->AttachChild(leaf);
    middleNode->AttachChild(sibling);

    middle->SetAlive(false);
    CHECK(top->GetAlive());
    CHECK_FALSE(leaf->GetAlive());
    CHECK_FALSE(sibling->GetAlive());

    leaf->SetAlive(true);
    CHECK(middle->GetAlive());
    // Current behavior: the revived parent revives all of its children, so the
    // sibling comes back too.
    CHECK(sibling->GetAlive());
}

TEST_CASE("A direct child of the root can be killed and revived")
{
    GameModuleTree tree;
    TestModule* top = new TestModule();
    TestModule* child = new TestModule();
    tree.GetRoot()->AttachChild(top)->AttachChild(child);

    top->SetAlive(false);
    CHECK_FALSE(child->GetAlive());

    // The root has no module, so there is no ancestor to revive.
    top->SetAlive(true);
    CHECK(top->GetAlive());
    CHECK(child->GetAlive());
}

TEST_CASE("Deactivating a module does not deactivate its children")
{
    GameModuleTree tree;
    TestModule* parent = new TestModule();
    TestModule* child = new TestModule();
    tree.GetRoot()->AttachChild(parent)->AttachChild(child);

    parent->SetActive(false);
    CHECK(child->GetActive());
}

TEST_CASE("GameModule::SetID re-indexes the module in its tree")
{
    GameModuleTree tree;
    TestModule* m = new TestModule(30);
    tree.GetRoot()->AttachChild(m);

    m->SetID(31);
    CHECK(m->GetID() == 31);
    CHECK(tree.Find(31) == m);
    CHECK(tree.Find(30) == nullptr);

    m->SetID(GameModule::UnspecifiedID);
    CHECK(tree.Find(31) == nullptr);
}

TEST_CASE("GameModule::SetID works on a module outside any tree")
{
    TestModule m(40);
    m.SetID(41);
    CHECK(m.GetID() == 41);
}

TEST_CASE("GameModuleNode::IsRoot is true only for the tree root")
{
    GameModuleTree tree;
    GameModuleNode* child = tree.GetRoot()->AttachChild(new TestModule());

    CHECK(tree.GetRoot()->IsRoot());
    CHECK_FALSE(child->IsRoot());

    // A node outside any tree has no parent either, but is not a root.
    GameModuleNode detached(new TestModule());
    CHECK_FALSE(detached.IsRoot());
}
