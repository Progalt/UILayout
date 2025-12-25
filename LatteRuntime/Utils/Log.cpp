#include "Log.h"

namespace latte
{
	Log::Severity Log::m_SeverityFilter = Log::Severity::Info;
	std::string Log::m_TimeFormat = "%Y-%m-%d %H:%M:%S";
}