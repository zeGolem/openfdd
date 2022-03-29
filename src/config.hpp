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

	config_manager(std::string path) : m_path(path)
	{
		if (!std::filesystem::exists(path))
			throw std::runtime_error("File not found: " + path);
		std::fstream file(path);
		m_config = nlohmann::json::parse(file);
		file.close();
	}

	bool device_has_config(std::string device_id) const noexcept;

	nlohmann::json get_device_config(std::string device_id) const;

	void sync_to_disk() const;

  private:
	std::string    m_path;
	nlohmann::json m_config;
};
