/*
 * Copyright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tools.h"
#include <ctime>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <istream>
#include <cstdlib>

using std::string;
using std::vector;
using std::ofstream;

/**
 * Converts a month name (e.g. "Jan") to its corresponding number (e.g. "01").
 * If the name is not recognized, "??" is returned.
 *
 * @param name
 *        Month name in Mmm format.
 * @returns Corresponding month number.
 */
static string monthName2Number(const string& name) throw() {
    string names[] = {
        "jan",
        "feb",
        "mar",
        "apr",
        "may",
        "jun",
        "jul",
        "aug",
        "sep",
        "oct",
        "nov",
        "dec"
    };

    string lowercase_name(name);
    f2cc::tools::toLowerCase(lowercase_name);
    int month_number;
    for (month_number = 0; month_number < 12; ++month_number) {
        if (lowercase_name.compare(names[month_number]) == 0) {
            ++month_number;
            string str;
            if (month_number < 10) {
                str += "0";
            }
            str += f2cc::tools::toString(month_number);
            return str;
        }
    }

    // Name has not been recognized
    return "??";
}

string f2cc::tools::getCurrentTimestamp() throw() {
    time_t raw_time;
    tm* timeinfo;

    // Get date and time data
    time(&raw_time);
    timeinfo = localtime(&raw_time);
    string raw_date(asctime(timeinfo));

    // Form timestamp
    string timestamp;
    timestamp += raw_date.substr(20, 4); // Year
    timestamp += "-";
    timestamp += monthName2Number(raw_date.substr(4, 3)); // Month
    timestamp += "-";
    string day = raw_date.substr(8, 2);
    if (day[0] == ' ') day[0] = '0';
    timestamp += day;
    timestamp += " ";
    timestamp += raw_date.substr(11, 8); // Time

    return timestamp;
}

string& f2cc::tools::searchReplace(string& str, const string& search,
                                   const string& replace) throw() {
    size_t pos = 0;
    while ((pos = str.find(search, pos)) != string::npos) {
        str.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return str;
}

static string& ltrim(string& str) throw() {
    str.erase(str.begin(),
              std::find_if(str.begin(), str.end(),
                           std::not1(std::ptr_fun<int, int>(std::isspace))
                  )
        );
    return str;
}

static string& rtrim(string& str) throw() {
    str.erase(
        std::find_if(str.rbegin(), str.rend(),
                     std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
        str.end());
    return str;
}

string& f2cc::tools::trim(string& str) throw() {
    return ltrim(rtrim(str));
}

string& f2cc::tools::toLowerCase(string& str) throw() {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

string& f2cc::tools::toUpperCase(string& str) throw() {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

bool f2cc::tools::isNumeric(const string& str) throw() {
    std::istringstream iss(str);
    int num;
    iss >> num;
    return iss && (iss.rdbuf()->in_avail() == 0);
}

int f2cc::tools::toInt(const std::string& str) throw(InvalidArgumentException) {
    if (!isNumeric(str)) {
        THROW_EXCEPTION(InvalidArgumentException, "Not a number");
    }
    return atoi(str.c_str());
}

bool f2cc::tools::existsFile(const string& file) throw() {
    std::ifstream fs(file.c_str());
    return fs.good();
}

void f2cc::tools::readFile(const string& file, string& data)
    throw(FileNotFoundException, IOException) {
    std::ifstream fs(file.c_str());
    if (!fs.good()) {
        THROW_EXCEPTION(FileNotFoundException, file);
    }

    std::stringstream ss;
    if (ss << fs.rdbuf()) {
        data = ss.str();
    }
    else {
        THROW_EXCEPTION(IOException, file);
    }
}

void f2cc::tools::writeFile(const std::string& file, const std::string& data)
    throw(IOException) {
    std::ofstream ofile(file.c_str(), std::ios::out | std::ios::trunc);
    if (!ofile.is_open()) {
        THROW_EXCEPTION(IOException, file, "Failed to open output file");
    }
    ofile.exceptions(ofstream::failbit | ofstream::badbit);

    try {
        ofile << data << std::flush;
    }
    catch (ofstream::failure&) {
        THROW_EXCEPTION(IOException, file);
    }

    ofile.close();
}

string f2cc::tools::getFileName(string file) throw() {
    if (file.length() == 0) return "";

    size_t last_slash_pos = file.find_last_of("/\\");
    int start_pos = last_slash_pos == string::npos ? 0 : last_slash_pos + 1;
    size_t last_dot_pos = file.find_last_of(".");
    int end_pos = last_dot_pos == string::npos ? file.length() : last_dot_pos;
    return file.substr(start_pos, end_pos - start_pos);
}

vector<string> f2cc::tools::split(const string& str, char delim) throw() {
    vector<string> elems;
    std::stringstream ss(str);
    string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

bool isCharCuttingPoint(char chr) throw() {
    const char cutting_points[] = {'\n', ' ', '-', ':', '/', '\\'};
    const int num_cutting_points = sizeof(cutting_points) / sizeof(char);
    for (int index = 0; index < num_cutting_points; ++index) {
        if (chr == cutting_points[index]) return true;
    }
    return false;
}

void f2cc::tools::breakLongLines(string& str, size_t maximum_length,
                                 size_t indent_length) throw() {
    string old_str(str);
    str.clear();
    string buf;
    size_t offset = 0;
    size_t next_cut_pos = 0;
    size_t current_length = 0;
    bool length_limit_hit = false;
    while (offset < old_str.length()) {
        // Reset
        buf.clear();
        if (length_limit_hit) {
            // Insert indents
            buf.insert(0, indent_length, ' ');
            buf.insert(0, 1, '\n');
            current_length = indent_length;
            length_limit_hit = false;
        }
        else {
            current_length = 0;
        }
        next_cut_pos = offset;

        // Copy words until either line break, length limit or end of string is 
        // hit
        for (size_t i = offset; ; ++i) {
            ++current_length;
            bool is_at_cutting_point = isCharCuttingPoint(old_str[i])
                || i >= old_str.length();
            if (is_at_cutting_point) {
                // Append previous words to buffer
                buf += old_str.substr(next_cut_pos, i - next_cut_pos + 1);
                next_cut_pos = i + 1;
                if (old_str[i] == '\n' || i >= old_str.length()) {
                    break;
                }
            }

            if (current_length >= maximum_length) {
                // Remove trailing space, if present
                if (buf[buf.length() - 1] == ' ') {
                    buf.erase(buf.length() - 1);
                }
                length_limit_hit = true;
                break;
            }
        }

        // If no cuts have been done, do a forcive cut
        if (next_cut_pos == offset) {
            buf += old_str.substr(offset, maximum_length);
            offset += maximum_length;
        }
        else {
            offset = next_cut_pos;
        }

        str += buf;
    }
}
