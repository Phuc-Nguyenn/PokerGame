#pragma once

#include "Common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <span>
#include <spdlog/spdlog.h>
#include <sstream>
#include <utility>
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
  
  inline std::optional<std::string> Serialize(common::Action a)
  {
    switch(a.command)
    {
    case common::Command::Fold:
      return "F";
    case common::Command::CheckCall:
      return "C";
    case common::Command::Raise:
      return "R" + std::to_string(a.amount);
    }
    std::ostringstream oss;
    oss << a;
    SPDLOG_CRITICAL("Failed to serialize action=[{}]", oss.view());
    return std::nullopt;
  }

  inline std::optional<std::string> Serialize(Suit s) {
    switch (s) {
      case Suit::Spades:
        return "1";
      case Suit::Clubs:
        return "2";
      case Suit::Diamond:
        return "3";
      case Suit::Hearts:
        return "4";
      case Suit::Unassigned:
        return "Unassigned";
    }
    std::ostringstream oss;
    oss << s;
    SPDLOG_CRITICAL("Failed to serialize suit=[{}] [{}]", static_cast<int>(s), oss.view());
    return std::nullopt;
  }

  inline std::optional<char> Serialize(Rank r) {
    switch (r) {
      case Rank::Ace:
        return 'A';
      case Rank::Ten:
        return 'T';
      case Rank::Jack:
        return 'J';
      case Rank::Queen:
        return 'Q';
      case Rank::King:
        return 'K';
      case Rank::Two:
      case Rank::Three:
      case Rank::Four:
      case Rank::Five:
      case Rank::Six:
      case Rank::Seven:
      case Rank::Eight:
      case Rank::Nine:
        return '0' + static_cast<int>(r);
    }
    std::ostringstream oss;
    oss << r;
    SPDLOG_CRITICAL("Failed to serialize rank=[{}] [{}]", static_cast<int>(r), oss.view());
    return std::nullopt;
  }
  
  inline std::optional<std::string> Serialize(Card c) { 
    auto rank = Serialize(c.rank);
    auto suit = Serialize(c.suit);
    if (!rank || !suit) {
      std::ostringstream oss;
      oss << c;
      SPDLOG_CRITICAL("Failed to serialize card=", oss.view());
      return std::nullopt;
    };
    return std::string{*rank} + std::string{*suit};
  }
   
  class JsonSerializer : public ISerializer {
    public:
      std::vector<std::byte>
      Serialize(const poker::common::RoundState &state) override {
        SPDLOG_DEBUG("Serializing state");
        nlohmann::json json;
        json["board"] = {};
        json["history"] = {};
        for (const auto &card : state.communityCards) {
          auto result = json::Serialize(card);
          if (!result)
          {
            SPDLOG_CRITICAL("Failed to serialize cards while serializing state");
            return {};
          } 
          json["board"].emplace_back(std::move(*result));
        }

        for (const auto &action : state.prevActions) {
          auto result = json::Serialize(action);
          if (!result)
          {
            SPDLOG_CRITICAL("Failed to serialize previous actions while serializing state");
            return {};
          }
          json["history"].emplace_back(std::move(*result));
        }

        SPDLOG_DEBUG("Serialized state=[{}]", json.dump());

        auto binary = json.dump();
        auto ptr = reinterpret_cast<std::byte *>(binary.data());
        return std::vector<std::byte>(ptr, ptr + binary.size());
      };

      std::optional<poker::common::Action>
      Deserialize(std::span<std::byte> payload) override {
        SPDLOG_DEBUG("Deserializing payload");
        nlohmann::json json = nlohmann::json::parse(payload);
        SPDLOG_DEBUG("Json parsed payload into=[{}]", json.dump());
        std::string str = json.get<std::string>();
        if (str.empty())
        {
          return std::nullopt;
        }
        try {
          switch(str.at(0))
          {
            case 'C':
              return poker::common::Action{poker::common::Command::CheckCall};
            case 'F':
              return poker::common::Action{poker::common::Command::Fold};
            case 'R':
            {
              auto amount = std::stoul(std::string{str.begin() + 1, str.end()});
              return poker::common::Action{poker::common::Command::Raise, amount};
            }
            case 'A':
            {
              auto amount = std::stoul(std::string{str.begin() + 1, str.end()});
              return poker::common::Action{poker::common::Command::Raise, amount};
            } 
          }
        } catch(const std::exception& e) {
          SPDLOG_CRITICAL("unhandled exception occured when deserializing: {}", e.what());
          return std::nullopt;
        };
        return std::nullopt;
      };

    private:
};


} // namespace json
}; // namespace poker::serializer
