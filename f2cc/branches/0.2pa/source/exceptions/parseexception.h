/*
 * Copyright (c) 2011-2013 Gabriel Hjort Blindell <ghb@kth.se>
 *                          George Ungureanu <ugeorge@kth.se>
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

#ifndef F2CC_SOURCE_EXCEPTIONS_PARSEEXCEPTION_H_
#define F2CC_SOURCE_EXCEPTIONS_PARSEEXCEPTION_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines exception for parsing errors.
 */

#include "exception.h"
#include <string>

namespace f2cc {

/**
 * @brief Used when a parse method fails.
 */
class ParseException : public Exception {
  public:
    /**
     * Creates an exception.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line in the source file from where the exception was thrown.
     * @param file
     *        Parsed file.
     * @param message
     *        Error message.
     */
    ParseException(const std::string& source_file, int source_line,
                   const std::string& file, const std::string& message) throw();

    /**
     * Creates an exception.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        Parsed file.
     * @param line
     *        Line  where the parsing failed.
     * @param message
     *        Error message.
     */
    ParseException(const std::string& source_file, int source_line,
                   const std::string& file, int line,
                   const std::string& message) throw();

    /**
     * Creates an exception.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        Parsed file.
     * @param line
     *        Line where the parsing failed.
     * @param column
     *        Column  where the parsing failed.
     * @param message
     *        Error message.
     */
    ParseException(const std::string& source_file, int source_line,
                   const std::string& file, int line, int column,
                   const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~ParseException() throw();

    /**
     * @copydoc Exception::getMessage()
     */
    virtual std::string getMessage() const throw();

    /**
     * Gets the parsed file.
     *
     * @returns Parsed file.
     */
    std::string getFile() const throw();

    /**
     * Gets the line  where the parsing failed. If no line  is
     * available \c -1 is returned.
     *
     * @returns Line , if available; otherwise \c -1.
     */
    int getLine() const throw();

    /**
     * Gets the column  where the parsing failed. If no column  is
     * available \c -1 is returned.
     *
     * @returns Column , if available; otherwise \c -1.
     */
    int getColumn() const throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Parsed file.
     */
    const std::string file_;

    /**
     * Line where the parsing failed.
     */
    int line_;

    /**
     * Column where the parsing failed.
     */
    int column_;
};

}

#endif
