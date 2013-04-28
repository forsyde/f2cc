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

#include "logger.h"
#include "../tools/tools.h"
#include <iostream>

using namespace f2cc;
using std::string;
using std::cout;
using std::endl;
using std::ofstream;

const size_t Logger::kLogEntryLineWidthLimit = 100;

Logger::Logger() throw() : file_path_(""), is_open_(false), level_(INFO) {}

Logger::Logger(const string& file) throw(InvalidArgumentException, IOException) 
        : level_(INFO) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    file_path_ = file;
    file_.exceptions(ofstream::failbit | ofstream::badbit);

    open(file);
}

Logger::~Logger() throw() {
    close();
}

void Logger::setLogLevel(const LogLevel level) throw() {
    level_ = level;
}

bool Logger::isOpen() const throw() {
    return is_open_;
}

void Logger::open(const std::string& file) throw(f2cc::IOException) {
    if (is_open_) close();

    file_.open(file.c_str(), std::ios::out | std::ios::app);
    if (!file_.is_open()) {
        THROW_EXCEPTION(IOException, file, "Failed to open log file");
    }
    is_open_ = true;
}

Logger::LogLevel Logger::getLogLevel() const throw() {
    return level_;
}

void Logger::logMessage(LogLevel level, const string& message)
    throw(IOException, IllegalCallException) {
    if (!is_open_) {
        THROW_EXCEPTION(IllegalCallException, "Logger is closed");
    }

    if (level < level_) {
        // Ignore this message
        return;
    }

    // Generate log entry
    string entry;
    entry += f2cc::tools::getCurrentTimestamp();
    entry += " [";
    entry += logLevelToString(level);
    entry += "] - ";
    int indent_length = entry.length();
    string formatted_message(message);
    f2cc::tools::trim(formatted_message);
    entry += formatted_message;
    formatLogEntry(entry, indent_length);
    entry += '\n';
    try {
        file_ << entry << std::flush;
    }
    catch (ofstream::failure&) {
        THROW_EXCEPTION(IOException, file_path_);
    }

    // Generate console output
    string prompt_output(" * ");
    prompt_output += logLevelToString(level) + ": ";
    indent_length = prompt_output.length();
    prompt_output += message;
    formatLogEntry(prompt_output, indent_length);
    cout << prompt_output << std::endl;
}

void Logger::logDebugMessage(const string& message)
    throw(IOException, IllegalCallException) {
    logMessage(Logger::DEBUG, message);
}

void Logger::logInfoMessage(const string& message)
    throw(IOException, IllegalCallException) {
    logMessage(Logger::INFO, message);
}

void Logger::logWarningMessage(const string& message)
    throw(IOException, IllegalCallException) {
    logMessage(Logger::WARNING, message);
}

void Logger::logErrorMessage(const string& message)
    throw(IOException, IllegalCallException) {
    logMessage(Logger::ERROR, message);
}

void Logger::logCriticalMessage(const string& message)
    throw(IOException, IllegalCallException) {
    logMessage(Logger::CRITICAL, message);
}

string Logger::logLevelToString(LogLevel level) throw() {
    switch (level) {
        case DEBUG: {
            return "DEBUG";
        }
        case INFO: {
            return "INFO";
        }
        case WARNING: {
            return "WARNING";
        }
        case ERROR: {
            return "ERROR";
        }
        case CRITICAL: {
            return "CRITICAL";
        }
        default: {
            // Should never be reached
            return "???";
        }
    }
}

Logger::LogLevel Logger::stringToLogLevel(string str)
    throw(InvalidArgumentException) {
    if (str == "DEBUG") {
        return Logger::DEBUG;
    }
    else if (str == "INFO") {
        return Logger::INFO;
    }
    else if (str == "WARNING") {
        return Logger::WARNING;
    }
    else if (str == "ERROR") {
        return Logger::ERROR;
    }
    else if (str == "CRTICAL") {
        return Logger::CRITICAL;
    }
    else {
        THROW_EXCEPTION(InvalidArgumentException, "Unrecognized log level");
    }
}

void Logger::close() throw() {
    if (is_open_) {
        file_.close();
        is_open_ = false;
    }
}

void Logger::formatLogEntry(string& entry, int indent_length) const throw() {
    // Align the log message over linebreaks
    string new_linebreak(indent_length, ' ');
    new_linebreak.insert(0, 1, '\n');
    f2cc::tools::searchReplace(entry, "\n", new_linebreak);
    f2cc::tools::breakLongLines(entry, kLogEntryLineWidthLimit, indent_length);
}
