#include <fstream>

#include "StarVariant.hpp"
#include "StarLua.hpp"
#include "StarFormat.hpp"

using namespace Star;
using namespace std;

int main(int argc, char** argv) {
  try {
    LuaEnginePtr luaRoot = LuaEngine::create();
    luaRoot->loadScript("hello", 
        R"SCRIPT(
          function doit()
            self.called = self.called + 1
            print("Hello, I am " .. callbacks.name() .. " from " .. self.location .. ", I have " .. self.called .. " matchstick.")
          end
        )SCRIPT");

    luaRoot->loadScript("respond", 
        R"SCRIPT(
          function doit()
            self.called = self.called + 1
            print("Hello " .. callbacks.name() .. " I am from " .. self.location .. ", I have " .. self.called .. " matchstick.")
          end
        )SCRIPT");

    LuaCallbacks callbacks;

    auto helloContext = luaRoot->scriptContext("hello");
    helloContext->addPersistentTable("self", {{"location", "Sweden"}, {"called", 0}});

    auto respondContext = luaRoot->scriptContext("respond");
    respondContext->addPersistentTable("self", {{"location", "Norway"}, {"called", 0}});

    callbacks.registerFunction("name", [](VariantList const& args) -> Variant { return "Hanz"; });
    helloContext->setCallbacks("callbacks", callbacks);
    respondContext->setCallbacks("callbacks", callbacks);

    helloContext->invoke("doit");
    respondContext->invoke("doit");

    callbacks.registerFunction("name", [](VariantList const& args) -> Variant { return "Franz"; });
    helloContext->setCallbacks("callbacks", callbacks);
    respondContext->setCallbacks("callbacks", callbacks);

    helloContext->invoke("doit");
    respondContext->invoke("doit");

  } catch (std::exception const& e) {
    coutf("Exception caught: %s\n", e.what());
  }
}
