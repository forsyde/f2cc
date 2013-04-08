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

#ifndef F2CC_SOURCE_EXCEPTIONS_IOEXCEPTION_H_
#define F2CC_SOURCE_EXCEPTIONS_IOEXCEPTION_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines exception for I/O errors.
 */

#include "runtimeexception.h"
#include <string>

namespace f2cc {

/**
 * @brief Used to indicate that an I/O error has occurred when operating on
 *        a file.
 */
class IOException : public RuntimeException {
  public:
    /**
     * Creates an exception with no message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        File where the I/O error occurred.
     */
    IOException(const std::string& source_file, int source_line,
                const std::string& file) throw();

    /**
     * Same as IOException(const std::string&) but with a specified error
     * message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        File path where the I/O error occurred.
     * @param message
     *        Error message.
     */
    IOException(const std::string& source_file, int source_line,
                const std::string& file, const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~IOException() throw();

    /**
     * Gets the file of this exception.
     *
     * @returns File path.
     */
    std::string getFile() const throw();

    /**
     * See Exception::getMessage().
     *
     * @returns Error message.
     */
    virtual std::string getMessage() const throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * File where the I/O error occurred.
     */
    const std::string file_;
};

}

#endif
