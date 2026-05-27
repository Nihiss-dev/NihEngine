#include "Logger.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

Logger& Logger::Get()
{
	static Logger instance;
	return instance;
}

void Logger::SetOutputDebugString(bool enabled)
{
	m_OutputDebugString = enabled;
}

void Logger::SetConsoleOutput(bool enabled)
{
	m_ConsoleOutput = enabled;
}

void Logger::SetFileOutput(bool enabled, const std::string& filename)
{
	m_FileOutput = enabled;
	if (m_FileOutput && !m_LogFile.is_open())
	{
		m_LogFile.open(filename, std::ios::out | std::ios::app);
	}
	else
	{
		if (m_LogFile.is_open())
		{
			m_LogFile.close();
		}
	}
}

void Logger::Log(LogLevel level, LogCategory category, const std::string& message)
{
	const auto now = std::chrono::system_clock::now();
	const std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm{};
	localtime_s(&tm, &in_time_t);

	std::ostringstream oss;
	oss << "["
		<< std::setw(2) << std::setfill('0') << tm.tm_hour << ":"
		<< std::setw(2) << std::setfill('0') << tm.tm_min << ":"
		<< std::setw(2) << std::setfill('0') << tm.tm_sec << "]"
		<< "[" << LogLevelToString(level) << "]"
		<< "[" << LogCategoryToString(category) << "] "
		<< message;

	WriteToAll(oss.str(), level);
}

void Logger::WriteToAll(const std::string& line, LogLevel level)
{
	if (m_OutputDebugString)
	{
		OutputDebugStringA(line.c_str());
		OutputDebugStringA("\n");
	}
	if (m_ConsoleOutput)
	{
		if (level == LogLevel::Error)
			std::cerr << line << std::endl;
		else
			std::cout << line << std::endl;
	}
	if (m_FileOutput && m_LogFile.is_open())
	{
		m_LogFile << line << std::endl;
		m_LogFile.flush();
	}
}

Logger::~Logger()
{
	if (m_LogFile.is_open())
	{
		m_LogFile.close();
	}
}

const char* Logger::LogLevelToString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Info:
		return "Info";
	case LogLevel::Warning:
		return "Warning";
	case LogLevel::Error:
		return "Error";
	default:
		return "Unknown";
	}
}

const char* Logger::LogCategoryToString(LogCategory category)
{
	switch (category)
	{
	case LogCategory::General:
		return "General";
	case LogCategory::Renderer:
		return "Renderer";
	case LogCategory::TaskManager:
		return "TaskManager";
	case LogCategory::Window:
		return "Window";
	case LogCategory::Engine:
		return "Engine";
	default:
		return "Unknown";
	}
}