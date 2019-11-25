#include <variti/util/config.hpp>

#include <iostream>
#include <cassert>

int main(int argc, char* argv[])
{
  using namespace variti;
  using namespace variti::util;
  assert(argc = 2);
  config conf(
    [](const config_setting& st) {
      if (!st.visited())
        std::cerr << "config not visited: " << st.path() << "\n";
    });
  conf.load(argv[1]);
  auto root = conf.root();
  root["module"]["name"].to_string();
  return 0;
}
