#include "../adapter.hpp"

namespace nwcc::epsilon {
// Device build: implement this adapter with eadk_display, eadk_keyboard,
// and the EADK external-file API. The compiler/runtime core has no EADK
// dependency, so the same source is also usable in the simulator.
class Adapter final : public platform::Adapter {
public:
  void show_text(const std::string &) override {}
  std::string read_source() override { return {}; }
};
} // namespace nwcc::epsilon
