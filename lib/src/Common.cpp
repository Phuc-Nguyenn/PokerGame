#include "Common.hpp"

namespace poker::common {

static Config readConfig()
{
	return Config{};
}

const Config &GetConfig() {
  static Config instance = readConfig();
  return instance;
}

} // namespace poker::common