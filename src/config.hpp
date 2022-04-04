#pragma once

#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <stdexcept>
#include <string>

#include "3rd_party/json.hpp"

class config_manager
{
  public:
	static std::string get_default_filepath(
	    std::optional<std::string> fallback = {});

	config_manager(std::string path);
	bool           device_has_config(std::string config_id) const noexcept;
	nlohmann::json create_config_for(std::string config_id) const;

	nlohmann::json get_device_config(std::string config_id) const;

	void update_config(std::string           config_id,
	                   nlohmann::json const& config) const;

  private:
	std::string m_path;
};
