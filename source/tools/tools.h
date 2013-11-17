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

#ifndef F2CC_SOURCE_TOOLS_DATE_H_
#define F2CC_SOURCE_TOOLS_DATE_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Contains various tools used by the classes.
 *
 * This file contains functions which are used by various classes and thus
 * do not belong to any particular class. Typically these include functions
 * which operate on dates, times and strings.
 */

#include "../exceptions/filenotfoundexception.h"
#include "../exceptions/ioexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <vector>

namespace f2cc {

/**
 * @brief Contains various tools used across many classes.
 */
namespace tools {

/**
 * Gets the current system timestamp in the form of "YYYY-MM-DD hh:mm:ss".
 *
 * @returns Current timestamp.
 */
std::string getCurrentTimestamp() throw();

/**
 * Searched a string for another string and replaces it with a third.
 *
 * @param str
 *        Search-in string
 * @param search
 *        Search-for string.
 * @param replace
 *        Replacement string.
 * @returns Modified string.
 */
std::string& searchReplace(std::string& str, const std::string& search,
                           const std::string& replace) throw();

/**
 * Trims front and end of a string from whitespace.
 *
 * @param str
 *        String to trim.
 * @returns Modified string.
 */
std::string& trim(std::string& str) throw();

/**
 * Converts a string to lower case.
 *
 * @param str
 *        String to convert.
 * @returns Modified string.
 */
std::string& toLowerCase(std::string& str) throw();

/**
 * Converts a string to upper case.
 *
 * @param str
 *        String to convert.
 * @returns Modified string.
 */
std::string& toUpperCase(std::string& str) throw();

/**
 * Converts an element of any type (or at least most) into a string.
 *
 * @tparam T
 *         Element type.
 * @param e
 *        Element to convert.
 * @returns String representation.
 */
template <typename T>
std::string toString(const T& e) throw() {
    std::stringstream ss;
    ss << e;
    return ss.str();
}

/**
 * Checks whether a string is numeric.
 *
 * @param str
 *        String to check.
 * @returns \c true if the string consists only of numbers.
 */
bool isNumeric(const std::string& str) throw();

/**
 * Converts a string into an int.
 * 
 * @param str
 *        Numeric string to convert.
 * @returns Int.
 * @throws InvalidArgumentException
 *         When \c str is not an int.
 */
int toInt(const std::string& str) throw(InvalidArgumentException);

/**
 * Checks if a file exists.
 *
 * @param file
 *        File.
 * @returns \c true if the file exists.
 */
bool existsFile(const std::string& file) throw();

/**
 * Reads the content of a file into a string. The string is given by reference
 * to avoid excessive copying.
 *
 * @param file
 *        File to read.
 * @param data
 *        String where to store the read data.
 * @throws FileNotFoundException
 *         When the file cannot be found.
 * @throws IOException
 *         When an I/O error occurs while reading.
 */
void readFile(const std::string& file, std::string& data)
    throw(FileNotFoundException, IOException);

/**
 * Writes a string of data to a file. If the file does not exist, it will be
 * created. If it does exist, the old file will be overwritten.
 *
 * @param file
 *        Destination file.
 * @param data
 *        String to write.
 * @throws IOException
 *         When an I/O error occurs while writing.
 */
void writeFile(const std::string& file, const std::string& data)
    throw(IOException);

/**
 * Gets the file name from a file path. If no file name is found, an empty
 * string is returned.
 *
 * @param file
 *        File path.
 * @returns File name.
 */
std::string getFileName(std::string file) throw();

/**
 * Copies and appends the content of one list to the end of another.
 *
 * @tparam T
 *         List element class.
 * @param to
 *        List to which to append the elements.
 * @param from
 *        List from which to copy the elements.
 */
template <typename T>
void append(std::list<T>& to, const std::list<T>& from) throw() {
    to.insert(to.end(), from.begin(), from.end());
}

/**
 * Prints the content of a list to std::cout, separated by a comma (\c ,).
 *
 * @tparam T
 *         List element class.
 * @param l
 *        List to print
 */
template <typename T>
void print(std::list<T>& l) throw() {
    bool first = true;
    typename std::list<T>::iterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        if (first) first = false;
        else       std::cout << ", ";
        std::cout << *it << std::flush;
    }
    std::cout << std::endl;
}

/**
 * Splits a string into a vector at a certain delimiter.
 *
 * @param str
 *        String to split.
 * @param delim
 *        Delimiter character to split at.
 * @returns Vector of split strings.
 */
std::vector<std::string> split(const std::string& str, char delim) throw();

/**
 * Breaks lines in a string which exceed the specified maximum length at word
 * positions. The remaining part of the line will be indented by a specified
 * amount.
 *
 * @param str
 *        String to break.
 * @param maximum_length
 *        Maximum line length before breaking.
 * @param indent_length
 *        Number of spaces to include following a broken line.
 */
void breakLongLines(std::string& str, size_t maximum_length,
                    size_t indent_length)
    throw();

}
}

#endif
