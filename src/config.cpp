#include "config.hpp"
#include "3rd_party/json.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>

// TODO: Do this properly.
std::string config_manager::get_default_filepath(std::optional<std::string>)
{
	auto const path = "/etc/openfdd.d/";
	// TODO: Should this be somewhere else?
	if (!std::filesystem::exists(path))
		std::filesystem::create_directories(path);
	return path;
}

config_manager::config_manager(std::string path) : m_path(path)
{
	if (!std::filesystem::exists(path))
		throw std::runtime_error("File not found: " + path);
}

bool config_manager::device_has_config(std::string config_id) const noexcept
{
	return std::filesystem::exists(m_path + config_id + ".json");
}

nlohmann::json config_manager::create_config_for(std::string config_id) const
{
	if (device_has_config(config_id)) return get_device_config(config_id);

	nlohmann::json new_config(nlohmann::json::value_t::object);

	std::ofstream config_output(m_path + config_id + ".json");
	config_output << new_config;
	return new_config;
}

nlohmann::json config_manager::get_device_config(std::string config_id) const
{
	if (!device_has_config(config_id)) return create_config_for(config_id);

	std::ifstream config(m_path + config_id + ".json");
	return nlohmann::json::parse(config);
}

void config_manager::update_config(std::string           config_id,
                                   nlohmann::json const& config) const
{
	std::ofstream config_output(m_path + config_id + ".json");
	config_output << config;
}
