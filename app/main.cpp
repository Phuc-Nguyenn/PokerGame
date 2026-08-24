
#include "PokerGame.hpp"
#include "Common.hpp"

#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>


using namespace poker::common::functional;

int main(int argc, char** argv)
{
	spdlog::set_pattern("[%H:%M:%S %z] [%^-%l-%$] [%s:%#]\t\t%v");
	
	#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
	#endif

	boost::filesystem::path configPath;
	if (argc == 1) {
		configPath = boost::filesystem::current_path()/std::string("configs.json");
	} else {
		std::string configFilename = argv[1];
		configPath = boost::filesystem::current_path()/configFilename;
	}
	
	poker::PokerGame sim{};
	auto result = sim.Init(configPath).and_then([&](Void){ return sim.Start(); });

	if (!result) {
		SPDLOG_CRITICAL("Error occured while running PokerSim [{}]", result.error().message());
	}

	return 0;
}