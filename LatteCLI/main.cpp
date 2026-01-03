
#include "latteui.h"
#include "argparse.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sol/sol.hpp>
namespace fs = std::filesystem;

#include <string>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

std::string getExePath() 
{
	char buffer[4096];

#if defined(_WIN32)
	DWORD size = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
	if (size == 0 || size == sizeof(buffer))
		return "";
	return std::string(buffer, size);

#elif defined(__APPLE__)
	uint32_t size = sizeof(buffer);
	if (_NSGetExecutablePath(buffer, &size) == 0) {
		return std::string(buffer);
	}
	// Buffer too small, allocate the needed size
	std::string path(size, '\0');
	if (_NSGetExecutablePath(&path[0], &size) == 0) {
		return std::string(path.c_str());
	}
	return "";

#elif defined(__linux__)
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len == -1)
		return "";
	buffer[len] = '\0';
	return std::string(buffer);

#else
	return ""; // Unsupported platform
#endif
}

void runScript(const std::string& path)
{
	latte::runScript(path);
}

bool runProject()
{
	if (!fs::exists("latteproj.lua")) 
	{
		std::cout << "Directory does not contain a latteproj.lua\n";
		return false;
	}

	try 
	{
		std::string exePath = getExePath();
		fs::path basePath = fs::path(exePath).parent_path();
		basePath = basePath / "luaSrc";

		latte::setLibBasePath(basePath.string() + "/");

		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::package);

		sol::table proj = lua.script_file("latteproj.lua");

		std::string name = proj["name"].get_or(std::string("Unknown Project"));
		std::cout << "Running Project: " << name << "\n";

		std::string entry = proj["entry"].get_or(std::string("src/main.lua"));


		if (!fs::exists(entry)) 
		{
			std::cout << "Entry file does not exist: " << entry << "\n";
			return false;
		}

		runScript(entry);

		return true;
	}
	catch (const sol::error& err) 
	{
		std::cout << "Failed to load latteproj.lua: " << err.what() << "\n";
		return false;
	}
}

bool copyDirectoryRecursively(const fs::path& source, const fs::path& destination) 
{
	std::error_code ec;
	fs::create_directories(destination, ec);
	for (const auto& entry : fs::directory_iterator(source, ec)) 
	{
		const auto& path = entry.path();
		auto dest = destination / path.filename();
		if (fs::is_directory(path)) 
		{
			if (!copyDirectoryRecursively(path, dest)) return false;
		}
		else 
		{
			fs::copy_file(path, dest, fs::copy_options::overwrite_existing, ec);
		}
	}
	return true;
}

/*
	Create a new named Project

	A project has a certain structure: 

		my_app/
			| 
			|- src/ 
			|	|- main.lua
			|	|- ...
			|- assets/ 
			|	|- ... 
			|- latteproj.lua
			|- README.md
	

*/
bool createProject(const std::string& name, const std::string& templateName = "", const std::string argv0 = "")
{
	fs::path projectDir = name;

	// 1. Create project dir
	std::error_code ec;
	if (fs::exists(projectDir)) 
	{
		std::cout << "Directory already exists: " << projectDir << "\n";
		return false;
	}
	if (!fs::create_directory(projectDir, ec)) 
	{
		std::cout << "Failed to create project directory: " << ec.message() << "\n";
		return false;
	}

	// Create subdirectories
	fs::create_directory(projectDir / "src", ec);
	fs::create_directory(projectDir / "assets", ec);

	// Create latteproj.lua
	std::string latteprojLua = R"(-- Project configuration
return {
    name = ")" + name + R"(",
    version = "0.1.0",
    description = "A new Latte UI project.",
	authors = {},
	dependencies = {
		latteui = "^0.1.0"
	},
	assets = {
		-- assets the app uses go here
		-- You can add a folder with a wildcard * 
		"assets/*"
	},
	entry = "src/main.lua"
}
)";
	std::ofstream(projectDir / "latteproj.lua") << latteprojLua;

	// Create README.md
	std::string readme = "# " + name + "\n\nA new Latte UI project.\n";
	std::ofstream(projectDir / "README.md") << readme;

	// Optionally handle templates 
	if (!templateName.empty()) 
	{
		
	}
	else
	{
		// Copy Templates/Basic.lua into src/main.lua
		fs::path engineRoot = "";
		fs::path templatePath = engineRoot / "Templates/Basic";
		fs::path destPath = projectDir / "src";

		if (fs::exists(templatePath)) 
		{
			copyDirectoryRecursively(templatePath, destPath);
		}
		else
		{
			std::cout << "Warning: Basic template directory not found! Only minimal files created.\n";
		}
	}

	std::cout << "Project '" << name << "' created!\n";

	std::cout << "\n\tcd " << name << "\n";
	std::cout << "\tlatte run " << "\n";

	return true;
}

int main(int argc, char* argv[])
{

	argparse::ArgumentParser program("latte");

	program.add_argument("cmd")
		.help("run to Run a Lua script, create to make a project");
	program.add_argument("arg")
		.help("Argument to be passed to cmd")
		.default_value("");

	program.add_argument("--template")
		.help("When passed with create use a specific project template");

	std::string cmd = "";
	try
	{
		program.parse_args(argc, argv);
		cmd = program.get<std::string>("cmd");
	} 
	catch (std::exception& ex)
	{
		std::cout << ex.what() << "\n";
		return -1;
	}

	if (cmd == "run")
	{
		const std::string file = program.get<std::string>("arg");

		if (!file.empty())
		{
			runScript(file);
		}
		else
		{
			runProject();
		}
	}
	else if (cmd == "create")
	{
		const std::string name = program.get<std::string>("arg");

		if (!name.empty())
		{
			createProject(name);
		}
	}

	return 0;
}