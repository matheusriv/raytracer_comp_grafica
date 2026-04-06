#ifndef PARSER_HPP
#define PARSER_HPP

#include <algorithm>
#include <string>

#include <functional>

#include "paramset.hpp"

// === Support functions

/// Map that binds a convertion function to an attribute name.
using ConverterFunction
  = std::function<bool(const std::string&, const std::string&, ryt::ParamSet*)>;

// === parsing functions.
void parse_scene_file(const char*);
bool is_valid_tag(std::string_view tag_name);
bool is_valid_attribute(std::string_view tag_name, std::string_view attribute_name);
void parse_attribute(const std::string& attr_name, const std::string& attr_content,
                     ryt::ParamSet* ps);

#endif  // PARSER_HPP
