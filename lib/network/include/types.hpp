#include "config/basic_types.hpp"
#include "header.hpp"
#include "error.hpp"

#include <variant>

namespace network {
class HeadFlit;
class BodyFlit;
class TailFlit;
class NopeFlit;
class NetwrorkError;

using Flit = std::variant<HeadFlit, BodyFlit, TailFlit, NopeFlit>;
}