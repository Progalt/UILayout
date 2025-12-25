
#ifndef LATTE_LOG_H
#define LATTE_LOG_H

#include <format>
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>

namespace latte
{
	/*
		Provides logging functionality
	*/
	class Log
	{
	public:

		enum class Severity
		{
			Info, 
			Warning,
			Error, 
			Fatal
		};

		static void setSeverityFilter(Severity sev)
		{
			m_SeverityFilter = sev;
		}

		template<class... Args>
		static void log(Severity sev, const std::string& msg, Args&&... args)
		{
			// Don't log if less than filter
			if ((int)sev < (int)m_SeverityFilter)
				return;

			std::string formattedString = std::vformat(msg, std::make_format_args(args...));

			std::string output = "";
			if (!m_TimeFormat.empty()) 
			{
				auto now = std::chrono::system_clock::now();
				auto time_t = std::chrono::system_clock::to_time_t(now);
				std::stringstream ss;
				ss << std::put_time(std::localtime(&time_t), m_TimeFormat.c_str());
				output = "[" + ss.str() + "]" + output;
			}

			switch (sev)
			{
			case Severity::Info:
				output += "[\033[32mInfo\033[39m]";
				break;
			case Severity::Warning:
				output += "[\033[33mWarning\033[39m]";
				break;
			case Severity::Error:
				output += "[\033[31mError\033[39m]";
				break;
			case Severity::Fatal:
				output += "[\033[31mFatal\033[39m]";
				break;
			}

			output += " -> " + formattedString;

			// For now just do it to stdout
			std::cout << output << std::endl;
		}

	private:

		static Severity m_SeverityFilter;
		static std::string m_TimeFormat;
	};
}

#endif // LATTE_LOG_H