#pragma once

#include <string>
#include <fstream>

#include <Core/NonCopyable.h>

enum class LogLevel
{
	Info,
	Warning,
	Error
};

enum class LogCategory
{
	General,
	Renderer,
	TaskManager,
	Window,
	Engine
};

class Logger : public NonCopyable
{
public:
	static Logger& Get();

	void SetOutputDebugString(bool enabled);
	void SetConsoleOutput(bool enabled);
	void SetFileOutput(bool enabled, const std::string& filename = "NihEngine.log");

	void Log(LogLevel level, LogCategory category, const std::string& message);

	void WriteToAll(const std::string& line, LogLevel level);

	static const char* LogLevelToString(LogLevel level);
	static const char* LogCategoryToString(LogCategory category);

private:
	Logger() = default;
	~Logger();

private:
	std::ofstream m_LogFile;

	bool m_OutputDebugString{ true };
	bool m_ConsoleOutput{ true };
	bool m_FileOutput{ true };
};

#define NIH_LOG(level, category, message) Logger::Get().Log(level, category, message)

#define NIH_LOG_INFO(category, message) NIH_LOG(LogLevel::Info, category, message)
#define NIH_LOG_WARNING(category, message) NIH_LOG(LogLevel::Warning, category, message)
#define NIH_LOG_ERROR(category, message) NIH_LOG(LogLevel::Error, category, message)