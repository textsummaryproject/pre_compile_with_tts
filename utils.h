#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

char* UTF8ToANSI(const char* pszCode);
std::string toLowerCase(const std::string& str);
std::vector<std::string> split(const std::string& str);

#endif // UTILS_H
