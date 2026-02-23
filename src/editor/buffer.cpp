#include "./text.h"

void TextBuffer::setText(const std::string& text)
{
    lines.clear();

    std::string cur;
    cur.reserve(text.size());

    for (const char ch : text)
    {
        if (ch == '\r') continue;
        if (ch == '\n')
        {
            lines.push_back(cur);
            cur.clear();
        }
        else cur.push_back(ch);
    }

    lines.push_back(cur);
}

std::string TextBuffer::getText() const
{
    std::string result;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        result += lines[i];
        if (i < lines.size() - 1) result += '\n';
    }

    return result;
}

const std::vector<std::string>& TextBuffer::getLines() const { return lines; }
std::vector<std::string>& TextBuffer::getLines() { return lines; }
size_t TextBuffer::lineCount() const { return lines.empty() ? 1 : lines.size(); }

size_t TextBuffer::lineLength(const size_t line) const
{
    return lines.empty() ? 0 : lines[std::min(line, lines.size() - 1)].size();
}
