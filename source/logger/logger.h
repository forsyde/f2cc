/*
 * fanoutright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

#ifndef F2CC_SOURCE_LOGGER_LOGGER_H_
#define F2CC_SOURCE_LOGGER_LOGGER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the \c Logger class.
 */

#include "../exceptions/ioexception.h"
#include "../exceptions/illegalcallexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <fstream>
#include <string>

namespace f2cc {

/**
 * @brief Manages the log file.
 *
 * The \c Logger class manages the log file. It accepts a log message which is
 * either written to the log file or ignored, depending on the currently set log
 * level. The default log level is set to Logger::INFO.
 *
 * Every log message is also printed to std::cout.
 */
class Logger {
  public:
    /**
     * The log level determines which log messages will be written to file.
     * The levels are hierarchically ordered, where \c DEBUG is the lowest
     * level and \c CRITICAL is the highest level. Log messages with a level
     * equal or higher than that set in the logger will be written to file,
     * whilst log messages with a lower level will not.
     */
    enum LogLevel {
        /**
         * For debugging.
         */
        DEBUG,
        /**
         * For messages which are just informative and can safely be ignored.
         */
        INFO,
        /**
         * For warnings which should be looked at, but does not affect the
         * behaviour of the application if ignored.
         */
        WARNING,
        /**
         * For errors which are caused by invalid input.
         */
        ERROR,
        /**
         * For errors which should just never happen.
         */
        CRITICAL
    };

    /**
     * Determines the maximum length of each line in the log entry. This is
     * set to 80 characters.
     */
    static const size_t kLogEntryLineWidthLimit;

    /**
     * Creates a logger not associated with any log file.
     */
    Logger() throw();

    /**
     * Creates a logger associated with a log file. If the file does not exist,
     * it will be created.
     *
     * @param file
     *        Log file to open.
     * @throws InvalidArgumentException
     *         When \c file is an empty string.
     * @throws IOException
     *         When the log file cannot be opened or created.
     */
    Logger(const std::string& file) throw(InvalidArgumentException, 
                                          IOException);

    /**
     * Closes and destroys this logger.
     */
    ~Logger() throw();
    

    /**
     * Checks if this logger is open and associated with a log file.
     *
     * @returns \c true if open and associated.
     */
    bool isOpen() const throw();

    /**
     * Associates this logger with a log file. If the logger is already
     * associated with another log file, it will be closed. If the log file does
     * not exist, it will be created.
     *
     * @param file
     *        Log file to open.
     * @throws IOException
     *         When the log file cannot be opened or created.
     */
    void open(const std::string& file) throw(f2cc::IOException);

    /**
     * Closes this logger and associated log file. Any pending log messages are
     * written to file. If the logger has already been closed then this method
     * has no effect.
     */
    void close() throw();

    /**
     * Sets a new log level of this logger.
     *
     * @param level
     *        Log level.
     * @see LogLevel
     */
    void setLogLevel(const LogLevel level) throw();

    /**
     * Gets the current log level of this logger. The default log level is set
     * to Logger::INFO.
     *
     * @returns Log level.
     */
    LogLevel getLogLevel() const throw();

    /**
     * Logs a message with the current time stamp of the system. Whether the
     * message is written to the log file depends on its log level and the
     * current log level of the logger. The entry will be immediately flushed
     * to file as the importance of an accurate trace is higher than its impact
     * on performance.
     *
     * The log entry will be formatted such that each line in the log file is no
     * longer than 80 characters. Hence, line breaks may be inserted. The
     * message itself may also contain linebreaks, if desired. Surrounding
     * whitespace will be trimmed.
     *
     * @param level
     *        Message's log level.
     * @param message
     *        Log Message.
     * @throws IOException
     *         When an I/O error occurs.
     * @throws IllegalCallException
     *         When this method is invoked on a closed logger.
     *
     * @see LogLevel
     */
    void logMessage(LogLevel level, const std::string& message)
        throw(f2cc::IOException, f2cc::IllegalCallException);

    /**
     * Same as logMessage(LogLevel, const std::string&) using Logger::DEBUG
     * as argument.
     *
     * @param message
     *        Log Message.
     * @throws IOException
     *         When an I/O error occurs.
     * @throws IllegalCallException
     *         When this method is invoked on a closed logger.
     *
     * @see logMessage(LogLevel, const std::string&)
     */
    void logDebugMessage(const std::string& message)
        throw(f2cc::IOException, f2cc::IllegalCallException);

    /**
     * Same as logMessage(LogLevel, const std::string&) using Logger::INFO
     * as argument.
     *
     * @param message
     *        Log Message.
     * @throws IOException
     *         When an I/O error occurs.
     * @throws IllegalCallException
     *         When this method is invoked on a closed logger.
     *
     * @see logMessage(LogLevel, const std::string&)
     */
    void logInfoMessage(const std::string& message)
        throw(f2cc::IOException, f2cc::IllegalCallException);

    /**
     * Same as logMessage(LogLevel, const std::string&) using Logger::WARNING
     * as argument.
     *
     * @param message
     *        Log Message.
     * @throws IOException
     *         When an I/O error occurs.
     * @throws IllegalCallException
     *         When this method is invoked on a closed logger.
     *
     * @see logMessage(LogLevel, const std::string&)
     */
    void logWarningMessage(const std::string& message)
        throw(f2cc::IOException, f2cc::IllegalCallException);

    /**
     * Same as logMessage(LogLevel, const std::string&) using Logger::ERROR
     * as argument.
     *
     * @param message
     *        Log Message.
     * @throws IOException
     *         When an I/O error occurs.
     * @throws IllegalCallException
     *         When this method is invoked on a closed logger.
     *
     * @see logMessage(LogLevel, const std::string&)
     */
    void logErrorMessage(const std::string& message)
        throw(f2cc::IOException, f2cc::IllegalCallException);

    /**
     * Same as logMessage(LogLevel, const std::string&) using Logger::CRITICAL
     * as argument.
     *
     * @param message
     *        Log Message.
     * @throws IOException
     *         When an I/O error occurs.
     * @throws IllegalCallException
     *         When this method is invoked on a closed logger.
     *
     * @see logMessage(LogLevel, const std::string&)
     */
    void logCriticalMessage(const std::string& message)
        throw(f2cc::IOException, f2cc::IllegalCallException);

    /**
     * Converts a log level into a string.
     *
     * @param level
     *        Log level.
     * @returns String representation.
     */
    static std::string logLevelToString(LogLevel level) throw();

    /**
     * Converts a string into a log level.
     *
     * @param str
     *        String.
     * @returns Log level.
     * @throws InvalidArgumentException
     *         When the string cannot be converted into a log level.
     */
    static LogLevel stringToLogLevel(std::string str)
        throw(InvalidArgumentException);

  private:
    /**
     * Prevents this from being auto-implemented by the compiler.
     *
     * @param rhs
     * @returns
     */
    Logger(Logger& rhs) throw();

    /**
     * @copydoc Logger(Logger&)
     *
     * @param rhs
     * @returns
     */
    Logger& operator=(Logger& rhs) throw();

    /**
     * Formats the log entry and makes the message more readable by breaking too
     * long lines and aligning the message across linebreaks. The maximum line
     * length is determined by Logger::kLogEntryLineWidthLimit.
     *
     * @param entry
     *        Log entry.
     * @param indent_length
     *        Length of the indentation in terms of spaces to perform on a
     *        linebreak.
     */
    void formatLogEntry(std::string& entry, int indent_length) const throw();

  private:
    /**
     * Path to log file.
     */
    std::string file_path_;

    /**
     * Determines whether the logger is open.
     */
    bool is_open_;

    /**
     * Handle to log file.
     */
    std::ofstream file_;

    /**
     * Current log level.
     */
    LogLevel level_;
};

}

#endif
