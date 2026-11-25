#pragma once

#include "Common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <span>
#include <spdlog/spdlog.h>
#include <vector>

namespace poker::serializer {

class ISerializer {
  public:
  virtual std::vector<std::byte>
  Serialize(const poker::common::RoundState &rs) = 0;
  
  virtual std::optional<poker::common::Action>
  Deserialize(std::span<std::byte> payload) = 0;
};

namespace json {
  
  std::optional<std::string> Serialize(common::Action a);

  std::optional<std::string> Serialize(Suit s);

  std::optional<char> Serialize(Rank r);
  
  std::optional<std::string> Serialize(Card c);
   
  class JsonSerializer : public ISerializer {
    public:
      std::vector<std::byte>
      Serialize(const poker::common::RoundState &state) override;

      std::optional<poker::common::Action>
      Deserialize(std::span<std::byte> payload) override;

    private:
};


} // namespace json
}; // namespace poker::serializer
