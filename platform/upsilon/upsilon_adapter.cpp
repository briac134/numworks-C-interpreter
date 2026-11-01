#include "../adapter.hpp"

namespace nwcc::upsilon {
// Device build: bind this adapter to Upsilon's external-app API and archive
// storage. Keeping this boundary separate prevents Upsilon headers from
// leaking into the portable compiler/runtime.
class Adapter final : public platform::Adapter {
public:
  void show_text(const std::string &) override {}
  std::string read_source() override { return {}; }
};
} // namespace nwcc::upsilon
