// Checks that a BBMain game links on Windows.
#include <Bibim/GameFramework.h>
#include <Bibim/Startup.h>

class LinkGame : public Bibim::GameFramework
{
};

BBMain(LinkGame);
