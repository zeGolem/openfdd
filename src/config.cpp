#include "config.hpp"
#include "3rd_party/json.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>

// TODO: Do this properly.
std::string config_manager::get_default_filepath(std::optional<std::string>)
{
	std::string const& home_dir     = std::getenv("HOME");
	auto const& default_config_path = home_dir + "/.config/openfdd/config.json";
	return default_config_path;
}

bool config_manager::device_has_config(std::string device_id) const noexcept
{
	return m_config.contains(device_id);
}

nlohmann::json config_manager::get_device_config(std::string device_id) const
{
	return m_config[device_id];
}

void config_manager::sync_to_disk() const
{
	std::ofstream stream(m_path);
	stream << m_config.dump().c_str();
	stream.close();
}
