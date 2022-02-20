#include "pch.h"
#include <filesystem>
#include <string>
#include <time.h>
#include <iostream>
#include <regex>
#include "configurables.h"

using namespace std;
namespace fs = filesystem;

static regex log_old ("nslog([0-9]{2}-){2}[0-9]{4}\ [0-9]{2}(-[0-9]{2}){2}.txt");
static regex log_new("nslog[0-9]{4}(-[0-9]{2}){2}\ [0-9]{2}(-[0-9]{2}){2}.txt");
static regex dmp_old("nsdump([0-9]{2}-){2}[0-9]{4}\ [0-9]{2}(-[0-9]{2}){2}.dmp");

const int seconds_in_day = 86400;

time_t getLogTime(const fs::path path)
{
	struct tm timeinfo = {0};
	int year, month, day;
	string filename = path.filename().string();
	// Check if log has a correct name
	if (regex_search(filename, log_old))
	{
		// Log naming before 1.4.0, which released on January 06 2022
		year = stoi(filename.substr(11, 4));
		month = stoi(filename.substr(8, 2));
		day = stoi(filename.substr(5, 2));
	}
	else if (regex_search(filename, dmp_old))
	{
		// Dumps
		year = stoi(filename.substr(12, 4));
		month = stoi(filename.substr(9, 2));
		day = stoi(filename.substr(6, 2));
	}
	else if (regex_search(filename, log_new))
	{
		// Log naming after 1.4.0
		year = stoi(filename.substr(5, 4));
		month = stoi(filename.substr(10, 2));
		day = stoi(filename.substr(13, 2));
	}
	else
	{
		// Can't get log date
		return 0;
	}

	// Only reads days
	timeinfo.tm_year = year - 1900;
	timeinfo.tm_mon = month - 1;
	timeinfo.tm_mday = day;

	return mktime(&timeinfo);
}

void deleteOldLogs(int days)
{
	time_t currenttime;
	time(&currenttime);
	string path = GetNorthstarPrefix() + "/logs";
	struct stat info;
	// Check if we can access log directory
	if (stat(path.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR))
	{
		return;
	}
	for (const auto& entry : fs::directory_iterator(path))
	{
		fs::path link = entry.path();
		string extension = link.extension().string();
		if (extension == ".txt" || extension == ".dmp")
		{
			if (difftime(currenttime, getLogTime(link)) > seconds_in_day * days)
			{
				remove(link);
			}
		}
	}
}