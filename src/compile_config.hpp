#pragma once

// Compile time configuration
namespace compile_config
{

// DAEMON
#ifdef NO_DAEMON
auto constexpr use_daemon = false;
#else
auto constexpr use_daemon = true;
#endif
// ---

} // namespace compile_config
