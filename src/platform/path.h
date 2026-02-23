#pragma once

#include <string>
#include <cctype>

inline std::string sanitize(std::string name, std::string* outError = nullptr)
{
    while (!name.empty() && std::isspace(static_cast<unsigned char>(name.front())) != 0) name.erase(name.begin());
    while (!name.empty() && std::isspace(static_cast<unsigned char>(name.back())) != 0) name.pop_back();
    while (!name.empty() && (name.back() == '/' || name.back() == '\\')) name.pop_back();

    if (name.empty())
    {
        if (outError) *outError = "Name cannot be empty.";
        return "";
    }

    if (name == "." || name == "..")
    {
        if (outError) *outError = "Name cannot be . or ..";
        return "";
    }

    for (const char c : name)
        if (static_cast<unsigned char>(c) < 32 || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"'
            || c == '<' || c == '>' || c == '|')
        {
            if (outError) *outError = "Name contains invalid character(s).";
            return "";
        }

    while (!name.empty() && (name.back() == '.' || std::isspace(static_cast<unsigned char>(name.back())) != 0))
        name.pop_back();
    if (name.empty())
    {
        if (outError) *outError = "Name cannot be empty.";
        return "";
    }

    if (name.size() > 120)
    {
        if (outError) *outError = "Name is too long.";
        return "";
    }

    return name;
}
