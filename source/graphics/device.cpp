#include "device.hpp"
#include "../input.hpp"
#include <memory>

namespace Gfx {
    std::shared_ptr<Input::InputData> Device::fetch_incoming_input_data_pointer() { return Input::get_ptr_incoming(); }
} // namespace Gfx
