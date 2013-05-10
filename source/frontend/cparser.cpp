/*
 * Copyright (c) 2011-2013
 *     Gabriel Hjort Blindell <ghb@kth.se>
 *     George Ungureanu <ugeorge@kth.se>
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

#include "cparser.h"
#include "../tools/tools.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/castexception.h"
#include <list>
#include <new>
#include <iostream>
#include <sstream>
#include <utility>

using namespace f2cc;
using std::string;
using std::stringstream;
using std::list;
using std::pair;
using std::vector;
using std::bad_alloc;

CParser::CParser(Logger& logger, int indent) throw() :
		level_(indent), file_(NULL), cdata_(NULL), logger_(logger){}

CParser::~CParser() throw() {}

CFunction* CParser::parseCFunction(const string& file, const string& name)
    throw(InvalidArgumentException, IOException, ParseException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    if (!tools::existsFile(file)) {
        THROW_EXCEPTION(IOException, string("File \"")
        		        + file + "\" does not exist");
    }

    file_ = file;
    tools::readFile(file,cdata_);
    if (cdata_.length() == 0) {
        THROW_EXCEPTION(IOException, file_, "file contains no data ");
    }

    CFunction* function = new CFunction(name, file);

	logger_.logMessage(Logger::DEBUG,
	        		string(tools::indent(level_)
	                      + "Parsing the function declaration..."));
    parseDeclaration(function);

	logger_.logMessage(Logger::DEBUG,
	        		string(tools::indent(level_)
	                      + "Extracting the function body..."));
    extractBody(function);

	logger_.logMessage(Logger::DEBUG,
	        		string(tools::indent(level_)
	                      + "Renaming wrapped variables..."));
	renameWrappedVariables(function);

    return function;
}

void CParser::parseDeclaration(CFunction* function)
    throw(InvalidArgumentException, ParseException) {
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
                        "string");
    }

    std::string declaration = "";
    string name = function->getName();

    //getting the declaration stream
    stringstream tempcode(cdata_);
    std::string line;
    unsigned current_line = 0;
    bool is_declaration = false;
    while (std::getline(tempcode, line)) {
    	if((string::npos != line.find(name)) &&
    			(string::npos != line.find("void"))){
    		is_declaration = true;
    		declaration += line;
    	}
    	else if((is_declaration) && (string::npos != line.find("{"))){
			declaration += line;
			cdata_.erase(cdata_.find(line), line.size());
			break;
		}
    	else if (is_declaration){
    		declaration += line;
    	}
    	cdata_.erase(cdata_.find(line), line.size() + 1);
    	current_line++;
    }
    if (!is_declaration){
		THROW_EXCEPTION(ParseException, file_,
				        current_line,
						string("Function \"")
						+ name + "\" does not have a declaration.");
	}
    tools::searchReplace(declaration, "\t", "");
    tools::searchReplace(declaration, "void", "");
    tools::searchReplace(declaration, name, "");
    tools::trim(declaration);

    bool is_output = true;
    unsigned previous = 0;
    unsigned found = declaration.find_first_of(",<{");
    while (found!=std::string::npos){
    	string part_before = declaration.substr(previous, found - previous);
    	if (declaration[found] == ','){
    		createFunctionParameter(function, part_before, is_output);
    		is_output = false;
    	}
    	if (declaration[found] == '<'){
    		string * data_type = new string();
    		string part_after = declaration.substr(found + 1,
    				declaration.size() - found - 1);
    		if(string::npos != part_before.find("array")){
    			found = found + getArrayDataType(part_after, data_type) + 1;
    			unsigned var_pos = declaration.find_first_of(",)", found);
    			string var_name = declaration.substr(found, var_pos - found);
    			createFunctionParameter(function, *data_type + var_name, is_output);
    			is_output = false;
    			found = var_pos + 1;
    		}
    		else{
    			found = found + getTemplateBaseDataType(part_after, data_type) + 1;
    			unsigned var_pos = declaration.find_first_of(",)", found);
    			string var_name = declaration.substr(found, var_pos - found);
    			createFunctionParameter(function, *data_type + var_name, is_output);
    			is_output = false;
    			found = var_pos + 1;
    		}
    		delete data_type;
    	}
    	if (declaration[found] == '{') break;
    	previous = found;
    	found = declaration.find_first_of(",<{", found+1);
    }
}

void CParser::extractBody(CFunction* function)
    throw(InvalidArgumentException, IOException){
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
                        "string");
    }
    std::string body = "";

    //getting the declaration stream
    stringstream tempcode(cdata_);
    std::string line;
    bool is_body = false;
    while (std::getline(tempcode, line)) {
    	if(string::npos != line.find("#pragma ForSyDe begin")){
    		is_body = true;
    	}
    	else if((is_body) && (string::npos != line.find("#pragma ForSyDe end"))){
			break;
		}
    	else if (is_body){
    		body += line;
    		cdata_.erase(cdata_.find(line), line.size() + 1);
    	}
    }
    if (!is_body){
		THROW_EXCEPTION(IOException,
						string("The function in file \"")
						+ file_ + "\" has no body.");
	}
}

void CParser::renameWrappedVariables(CFunction* function)
        throw(InvalidArgumentException, ParseException){
	if (!function) {
		THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
						"string");
	}
    stringstream tempcode(cdata_);
    std::string line;

    list<CVariable*> var_list =function->getInputParameters();
    var_list.push_back(function->getOutputParameter());

    list<pair<string, string> > name_dict;

    while (std::getline(tempcode, line)) {
    	if(string::npos != line.find("=")){

			unsigned equal_pos = line.find_first_of("=");

			//get lhs;
			string lhs = line.substr(0, equal_pos);
			tools::trim(lhs);
			if(string::npos != lhs.find(" ")){
				unsigned decl_end_pos = lhs.find_last_of(" ");
				lhs = lhs.substr(decl_end_pos, lhs.size() - decl_end_pos);
				tools::trim(lhs);
			}
			//get rhs
			string rhs = line.substr(equal_pos, line.size() - equal_pos);
			unsigned first_par_pos = rhs.find_last_of("(");
			rhs = rhs.substr(first_par_pos + 1, rhs.size() - first_par_pos - 1);
			rhs = rhs.substr(0, rhs.find_first_of("),"));

			if ((lhs.length() == 0) || (rhs.length() == 0)){
				THROW_EXCEPTION(ParseException, file_,
								string("Could not find rhs or lhs in:\n")
								+ line);
			}
			name_dict.push_back(std::make_pair(lhs,rhs));
    	}
    }

    list<CVariable*>::iterator var_it;
    list<pair<string, string> >::iterator dict_it;
    for (var_it = var_list.begin(); var_it != var_list.end(); ++var_it){
    	string var_name = (*var_it)->getReferenceString();
    	for (dict_it = name_dict.begin(); dict_it != name_dict.end(); ++dict_it){
    		if (var_name == (*dict_it).first) {
    			(*var_it)->changeReferenceString((*dict_it).second);
    			logger_.logMessage(Logger::DEBUG,
    			        		string(tools::indent(level_)
    			                      + "Renamed variable \""
    			                      + var_name
    			                      +"\" to \""
    			                      + (*dict_it).second
    			                      + "\" to function \""
    			                      + file_
    			                      +  "\"..."));
    			name_dict.erase(dict_it);
    			var_it = var_list.begin();
    			break;
    		}
    		if (var_name == (*dict_it).second) {
				(*var_it)->changeReferenceString((*dict_it).first);
				logger_.logMessage(Logger::DEBUG,
								string(tools::indent(level_)
									  + "Renamed variable \""
									  + var_name
									  +"\" to \""
									  + (*dict_it).first
									  + "\" to function \""
									  + file_
									  +  "\"..."));
				name_dict.erase(dict_it);
				var_it = var_list.begin();
				break;
			}
    	}
    }

    if (name_dict.size() != 0){
    	THROW_EXCEPTION(ParseException, file_,
						string("Parameter renaming is incomplete.")
						+ " Remaining are: \n"
						+ (*name_dict.begin()).first + " : "
						+ (*name_dict.begin()).second);
    }

}

void CParser::createFunctionParameter(CFunction* function, string analysis_string,
		bool is_output)
     throw(InvalidArgumentException, ParseException, OutOfMemoryException){
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
                        "string");
    }
	int separator = analysis_string.find_last_of(" ");

	CDataType* c_data_type;
	bool is_array = false;
	string data_type_string = analysis_string.substr(0, separator);
	if (string::npos != data_type_string.find("*")){
		is_array=true;
		tools::searchReplace(data_type_string, "*", "");
	}
	tools::searchReplace(data_type_string, "&", "");
	tools::trim(data_type_string);
	c_data_type = new CDataType(CDataType::stringToType(data_type_string),
			is_array, false, 0, false, false);

	CVariable* c_variable;
	string name_string = analysis_string.substr(separator + 1,
			analysis_string.length() - separator - 1);
	tools::searchReplace(name_string, "&", "");
	tools::trim(name_string);
	c_variable = new CVariable(name_string, *c_data_type);

	bool is_added;
	try {
		if (is_output) is_added = function->setOutputParameter(*c_variable);
		else is_added = function->addInputParameter(*c_variable);
		if (is_added) {
			logger_.logMessage(Logger::DEBUG,
			        		string(tools::indent(level_)
			                      + "Added variable \""
			                      + c_variable->getDataType()->toString()
			                      +" "
			                      + c_variable->getReferenceString()
			                      + "\" to function \""
			                      + file_
			                      +  "\"..."));
		}
	} catch (bad_alloc&) {
		THROW_EXCEPTION(OutOfMemoryException);
	}
	delete c_data_type;
	delete c_variable;
}

unsigned CParser::getArrayDataType(const string& analysis_string, string* data_type)
     throw(InvalidArgumentException, ParseException){
    if (analysis_string.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"analysis_string\" must not be empty "
                        "string");
    }

    unsigned found_data_type = analysis_string.find_first_of(",");
    *data_type = analysis_string.substr(0, found_data_type);
    tools::trim(*data_type);
	*data_type+="*";

	unsigned found_template_end = analysis_string.find_first_of(">");

    return found_template_end + 1;

}

unsigned CParser::getTemplateBaseDataType(const string& analysis_string, string* data_type)
     throw(InvalidArgumentException, ParseException){
    if (analysis_string.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"analysis_string\" must not be empty "
                        "string");
    }

    unsigned template_end = analysis_string.find_first_of(">");
    unsigned template_start = analysis_string.find_first_of("<",1);
    unsigned out_pos;

    bool found_nested_template = (template_end!=string::npos) &&
    		(template_start!=string::npos) && (template_start < template_end);

	if (found_nested_template){
		string part_before = analysis_string.substr(0, template_start);
		string part_after = analysis_string.substr(template_start + 1,
				analysis_string.size() - template_start - 1);
		if(string::npos != part_before.find("array")){
			out_pos = getArrayDataType(part_after, data_type);
			out_pos = out_pos +template_start + 1;
			template_end = analysis_string.find_first_of(">",out_pos);
			template_start = analysis_string.find_first_of("<",out_pos);
			if((template_end!=string::npos) && (template_start!=string::npos) &&
					(template_start < template_end)){
				THROW_EXCEPTION(ParseException, file_,
								string("Declaration has complex templates \n")
								+ analysis_string
								+"\n in file \""
								+ file_ + "\"");
			}
		}
		else{
			out_pos = getTemplateBaseDataType(part_after, data_type);
			out_pos = out_pos +template_start + 1;
			template_end = analysis_string.find_first_of(">",out_pos);
			template_start = analysis_string.find_first_of("<",out_pos);
			if((template_end!=string::npos) && (template_start!=string::npos) &&
					(template_start < template_end)){
				THROW_EXCEPTION(ParseException, file_,
								string("Declaration has complex templates \n")
								+ analysis_string
								+"\n in file \""
								+ file_ + "\"");
			}
		}
	}
	else if (template_end!=string::npos){
	    *data_type = analysis_string.substr(0, template_end);
	    tools::searchReplace(*data_type, "<", "");
	    tools::searchReplace(*data_type, ">", "");
	    tools::trim(*data_type);

	    out_pos = template_end;
	}
	else {
		THROW_EXCEPTION(ParseException, file_,
						string("Declaration template is not closed \n")
						+ analysis_string
						+"\n in file \""
						+ file_ + "\"");
	}

	return out_pos + 1;
}

