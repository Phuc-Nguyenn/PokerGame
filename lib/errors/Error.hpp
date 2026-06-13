#include <system_error>

namespace poker {
namespace errors {

enum class SimError {
  NoError = 0,
  BadRequest = 1,
  InvalidMove = 2,
  StateNotHandled = 3,
};

struct SimErrorCategory : std::error_category {
  const char *name() const noexcept override { return "HttpError"; }
  std::string message(int ev) const override;
};

const std::error_category &GetSimErrorCategory();
} // namespace errors
} // namespace poker

namespace std
{
  std::error_code make_error_code(poker::errors::SimError e);
}

template <>
struct std::is_error_code_enum<poker::errors::SimError> : true_type {};