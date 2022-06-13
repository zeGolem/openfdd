# Understanding and writing a OpenFDD driver

OpenFDD is fully written in C++, and drivers are too. Drivers are, for now,
built inside OpenFDD only. As such, you'll need to add your driver to the
OpenFDD tree if you wish to make one.

## 1. Structure

Each driver is split into two files, a C++ header, and a C++ source. In code, each
driver is a class inheriting from the base `driver` class. All drivers are located
in the `src/drivers/` folder. Drivers are organized by manufacturer, so for
example, all SteelSeries drivers are in `src/drivers/steelseries/`.

The C++ header is the device's name, in snake_case, with the `.hpp` extension, and
the C++ source is the same, but with the `.cpp` extension.

Drivers communicate with the underlying hardware through a custom built set of
C++ abstractions for libusb. Each driver instance has an associated usb::device
instance, which is used for sending packets.

Drivers expose functionality to the outside world using actions. Each action
optionally has parameters, and is used to, for example, update a mouse's DPI,
a keyboard's pulling rate, or just to save data to the onboard memory.

Drivers can save data persistently using the config system. I won't go into too
much details here, because it's still very experimental, doesn't work well, and
is prone to changes soon™.

## 2. A sample driver

Right now, the best driver in terms of completeness is the SteelSeries Aerox
3 Wireless, found in `src/drivers/steelseries/aerox_3_wireless.cpp`, and
should be used as a reference when building new drivers.

Though, this guide will also give more information about the things you need
to have a working OpenFDD driver.

The following is a good starting point for a driver header:

```cpp
#pragma once
#include "drivers/driver.hpp"
#include "usb/device.hpp"
#include <cstdint>
#include <vector>

namespace drivers
{
namespace brand_name // TODO: Brand name
{

// TODO: Device name
class device_name final : public driver
{
  public:
	device_name(std::shared_ptr<usb::device>    dev,
	           std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
		deserialize_config(config->get_device_config(config_id()));
		create_actions();
	}

	static bool is_compatible(std::shared_ptr<usb::device>);

	std::string config_id() const noexcept final
	{
		// TODO
		return "brand_name:device_name";
	}

	const std::string name() const noexcept final
	{
		// TODO
		return "Canonical Name of the Product";
	};

	// TODO: Add your own functions that implement the driver's fuctionality
	//       here :D
	//
	//       Examples:
	//        void save() const;
	//        void set_dpi(std::uint16_t new_dpi) const;

  protected:
	nlohmann::json serialize_current_config() const noexcept override final;
	void           deserialize_config(
	              nlohmann::json const& config_on_disk) override final;

	virtual void create_actions() noexcept override final;

  private:
	struct {
		// TODO: Config data: Put here anything that you'll need to be
		//       written on disk.
		//       
		//       Examples:
		//        std::uint32_t sleep_timeout = 0;
	} m_config;
};

}
}
```

Same here for the source:

```cpp
#include "drivers/brand_name/device_name.hpp"
#include "drivers/driver.hpp"
#include "steelseries.hpp"
#include "usb/device.hpp"
#include "utils.hpp"
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace drivers
{
namespace brand_name
{

bool device_name::is_compatible(std::shared_ptr<usb::device> dev)
{
	auto id = dev->get_descriptor_id();
	if (/* Check if the ID matches the device the driver is for */)
		return true;
	return false;
}

void device_name::create_actions() noexcept
{
	// This is where actions are registered and implemented.
	// The macros bellow make it easier and faster to write the actions. There
	// is no real need to understand how they work, just how they are used.

#define CREATE_ACTION_HANDLER(action_name)                                     \
	auto action_name##_handler = [this](                                       \
	    std::vector<std::string> const& parameters)

#define REGISTER_ACTION(action_name)                                           \
	register_action(#action_name, action_name, action_name##_handler)

#define CHECK_PARAMS_SIZE(minimum_size, action_name)                           \
	if (parameters.size() < minimum_size)                                      \
		throw std::runtime_error("Missing arguements for " #action_name);
	
	// TODO: Implement your driver's actions

	action exmaple{ // Here we define the action's "layout", giving it a name
	                // and parameters
	    .name        = "An example action",
	    .description = "This is only to be used for demonstration purposes",
	    .parameters  = {{ // This is the list of parameters
			 // You can look at existing drivers for more example of parameters
			 // and how different types are used. More documentation
			 // coming soon™
	         .type        = parameter::type::uint,
	         .type_info   = {.uint = {.min = 1, .max = 5}},
	         .name        = "parameter",
	         .description = "An example parameter",
        }},
	};

	// Now we register the handler for the action, this is the code that
	// will get run when the action is executed.
	CREATE_ACTION_HANDLER(example)
	{
		// We can check how many parameters were provided with the handy macro
		// The action name needs to be specified as it will show up in the
		// error message.
		CHECK_PARAMS_SIZE(1, example);

		std::uint8_t exmaple_param = // For now, we need to parse manually
		                             // the parameters. We use the stoi_safe
									 // util to make sure the number is in
									 // range, and throw an error if not
		    utils::stoi_safe(parameters[0], {.min = 1, .max = 5}, "Example");

		// TODO: Do something with the action
		save_config();
	};

	// Register our action
	REGISTER_ACTION(example);

	// TODO: MOAR ACTIONS!!

#undef CREATE_ACTION_HANDLER
#undef REGISTER_ACTION
#undef CHECK_PARAMS_SIZE
}

// TODO: Implement your custom functions for driver functionality here :D

nlohmann::json device_name::serialize_current_config() const noexcept
{
	// TODO: Config things, look at other drivers for examples, this is
	//       unstable so won't be documented for now :/ sorry
}

void device_name::deserialize_config(nlohmann::json const& config_on_disk)
{
	// TODO: Config things, look at other drivers for examples, this is
	//       unstable so won't be documented for now :/ sorry
}

}
}
```

## 2.1 Note on how to use C++

This isn't meant as a C++ tutorial, but to make this easier to follow, here are
a few good practices/convention to better understand the code, and make yours nicer.

1. Use `auto` as a variable type wherever possible. Modern IDE can resolve the type
   for you, no need to clutter your code with it
1. Use postfix notation, so a `const` variable should be defined as `auto const`,
   not `const auto`. This makes it more consistent with function declaration,
   where the `const` has to be at the end.
1. `const` should be used on everything you can.  
   If you have a function that doesn't edit any of its class' members, it should be
   marked as `const`.
   If a variable is only written to only once, it should be `const`.
1. `noexcept` should be used on everything you can. If a function doesn't throw, 
   it should always be marked `noexcept`.
1. Use references as much as possible. That will avoid copying memory.  
   When storing a value from a function, use `auto const& value = function();`
   When defining a function use `void func(std::string const& param)`
1. Avoid using "C-isms". If you need to use a C function, you should write
   a small wrapper around it in `src/utils.{cpp,hpp}`

## 3. Writing your first driver

You should copy the example from above as a starting point, and fill in all
the TODOs. Then, register your driver in `src/drivers/manager.cpp`:

In `manager::create_driver_if_available`, add a new `CHECK_DRIVER`.
You will need to `#include "..."` your driver's header.

At this point, if your `is_compatible` function works, recompiling and restarting
OpenFDD should show your device in the list.

You should now [reverse-engineer](./reversing.md) your device to implement some
actions.

You should write one function per packet type you reverse engineer.
