#include <filesystem>
//#include <glm/vec3.hpp>  // glm::vec3
#include <iomanip>
#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <string_view>
#include <type_traits>
#include <unordered_map>
namespace fs = std::filesystem;

#include "App.hpp"
#include "common.hpp"
//#include "geometry.hpp"
#include "paramset.hpp"
#include "parser.hpp"
#include "../../msg_system/error.hpp"
#include "../../lib_tinyxml2/tinyxml2.h"

/// Generic convertion function.
/*!
 * This function receives a string,`attr_content`, that may contain one or more instances of type
 * `T`, and tries to convert this string into a list of actual elements of type `T`.
 *
 * In case there is only one instance of T, that instance is stored in the ParamSet object `ps`,
 * passed as output parameter. If there are more than one instance of T, the function extracts them
 * into a `std::vector<T>` and stores the vector in `ps`.
 *
 * Therefore, the output of this function is the ParamSet object `ps` that contains either a
 * single instance of T or a vector<T> with multiples instances of T, found in `attr_content`.
 *
 * Example: convert("h_res", "1920", ps), here the attribute "h_res" should become an integer.
 * The function inserts ps->m_map["h_res"]=1920 (`int`), in the ParamSet object `ps`.
 *
 * @param attr_name The attribute name that will be associated with the value in the ParamSet.
 * @param attr_content The string attribute value (or values) we wish to convert.
 * @param[out] ps The output ParamSet object (dictionary).
 *
 * @return true if convertion worked, false otherwise.
 */

template <typename T>
bool convert(const std::string& attr_name, const std::string& attr_content, ryt::ParamSet* ps) {
  assert(ps);
  std::istringstream iss{ attr_content };      // Make `attr_content` a stream to extract data from.
  T single_value{};                            // Stores a single value of type T...
  std::vector<T> multiple_values;              // ... or use this to try to store multiple values.
  bool input_string_still_has_values{ true };  // Assume we have values to read from.
  // [1]: Try to read several T-values from the input string `attr_content`.
  while (input_string_still_has_values and not iss.eof()) {
    // Slightly different treatment if T is bool
    if constexpr (std::is_same_v<T, bool>) {
      iss >> std::boolalpha >> single_value;  // Use std::boolalpha to read "true" ou "false"
    } else {
      iss >> single_value;  // Regular extraction.
    }
    // Failed while trying to extract value this time?
    if (iss.fail()) {
      if (multiple_values.empty()) {  // Is it completely empty?
        return false;                 // Failed! The input was empty all along.
      }
      // If we got here, at least one value was successfully extracted from `attr_content`
      input_string_still_has_values = false;
      break;
    }
    //std::cout << "   ----> Value extracted is " << single_value << '\n';
    // Store the single value into a vector & look for more.
    multiple_values.push_back(single_value);
  }
  // [2]: If we found only one value in the vector, we get rid of the vector and store this single
  // value in the ParamSet. Otherwise, we store the entire vector.
  if (multiple_values.size() == 1) {
    single_value = multiple_values[0];
    ps->assign(attr_name, single_value);
  } else {
    ps->assign(attr_name, multiple_values);
  }
  return true;
}


/*!
 * This function will extract all the instances of **composite elements** present in the input
 * string `attr_content`. A composite element is a homogeneous n-tuple of 2, 3 or 4 values of the
 * same type, such as Point3f, Vector2i, or Normal3f, for instance.
 *
 * @note
 * The composite type T **must have** the operator[]() implemented for this function to work.
 *
 * @note
 * If the function finds only one instance of the composite element, then we store a single
 * value directly in the ParamSet, rather then a vector with just one instance.
 *
 * @tparam T The basic type of the composit element.
 * @tparam N How many individual values each composite element has.
 * @param attr_name The attribute name.
 * @param attr_content The string attribute value we wish to convert.
 * @param[out] ps The output ryt::ParamSet object.
 * @return A vector with all the composite elements extracted or no-value if none is available.
 *
 * Example: A `Vector3f` has 3 elements of type `float`.
 * bl="255 255 51" will be stored as ps->m_map["bl"]=Color24(255,255,51) in the `ParamSet` `ps`.
 */
template <typename T, std::uint8_t N>
bool convert(const std::string& attr_name, const std::string& attr_content, ryt::ParamSet* ps) {
  assert(ps);
  std::istringstream iss{ attr_content };
  std::vector<T> multiple_composite_values;
  T single_composite_value{};
  bool input_string_still_has_values{ true };
  // [1] Keep reading groups of N values from the input string.
  while (input_string_still_has_values and not iss.eof()) {
    // Try to extract a N-tuple from the string.
    for (std::uint8_t idx{ 0 }; idx < N; ++idx) {
      iss >> single_composite_value[idx];  // Try to extract a value.
      // Failed while extracting value this time around?
      if (iss.fail()) {
        if (multiple_composite_values.empty()) {  // Completely empty?
          return false;                           // then, there is nothing to return.
        }
        input_string_still_has_values = false;
        break;  // There is something in the vector.
      }
    }
    // Add the newly extracted composite item to the result vector.
    multiple_composite_values.push_back(single_composite_value);
  }
  // [2] If we found only one value in the vector, we get rid of the vector and store this single
  // value in the ParamSet. Otherwise, we store the entire vector.
  if (multiple_composite_values.size() == 1) {
    single_composite_value = multiple_composite_values[0];
    ps->assign(attr_name, single_composite_value);
  } else {
    ps->assign(attr_name, multiple_composite_values);
  }
  return true;
}


/// This is the list of all supported tags and their corresponding attributes/type.
std::unordered_map<std::string, std::vector<std::string>> tag_catalog{
  {
    "camera",
    {
      "type",
      "screen_window",
      "fovy",
    },
  },
  {
    "lookat",
    {
      "look_from",
      "look_at",
      "up",
    },
  },
  {
    "background",
    {
      "type",
      "filename",
      "mapping",
      "color",
      "tl",
      "tr",
      "bl",
      "br",
    },
  },
  {
    "film",
    {
      "type",
      "filename",
      "img_type",
      "x_res",
      "y_res",
      "w_res",
      "h_res",
      "crop_window",
      "gamma_corrected",
    },
  },
  {
    "world_begin",
    { "" },  // no attributes
  },
  {
    "world_end",
    { "" },  // no attributes
  },
  {
    "material",
    {
      "type",
      "color",
    },
  },
  {
    "object",
    {
      "type",
      "radius",
      "center",
    },
  },
  {
    "integrator",
    {
      "type"
    },
  },
};


/// Maps the tag name to its corresponding API function.
std::unordered_map<std::string, std::function<void(const ryt::ParamSet&)>> api_functions{
  { "camera", ryt::App::camera },
  { "lookat", ryt::App::look_at },
  { "background", ryt::App::background },
  { "world_begin", ryt::App::world_begin },
  { "world_end", ryt::App::world_end },
  { "film", ryt::App::film },
  { "material", ryt::App::material },
  { "integrator", ryt::App::integrator },
  { "object", ryt::App::object },
};


/// Maps convertion function to an attribute name.
std::unordered_map<std::string, ConverterFunction> converters{
  { "type", convert<std::string> },  // "type" must be a string.
  { "name", convert<std::string> },  // "name" must be a string.
  //
  { "color", convert<ryt::RGBColor, 3> },  // "color" is a RGBColor with 3 fields.
  { "flip", convert<bool> },
  // Background attributes.
  { "mapping", convert<std::string> },
  { "bl", convert<ryt::RGBColor, 3> },
  { "tl", convert<ryt::RGBColor, 3> },
  { "tr", convert<ryt::RGBColor, 3> },
  { "br", convert<ryt::RGBColor, 3> },
  // Image attributes
  { "x_res", convert<int> },
  { "y_res", convert<int> },
  { "h_res", convert<int> },
  { "w_res", convert<int> },
  { "filename", convert<std::string> },
  { "img_type", convert<std::string> },
  { "gamma_corrected", convert<bool> },
  // Camera attributes
  { "screen_window", [](const std::string& attr_name, const std::string& attr_content, ryt::ParamSet* ps) { ps->assign(attr_name, attr_content); return true; } },  // store entire string
  { "fovy", convert<ryt::real_type> },
  // Lookat attributes
  { "look_from", convert<ryt::Point3f, 3> },
  { "look_at", convert<ryt::Point3f, 3> },
  { "up", convert<ryt::Vector3f, 3> },
  // Object attributes
  { "radius", convert<ryt::real_type> },
  { "center", convert<ryt::Point3f, 3> },
};


/*!
 * This function checks if the tag received is valid.
 * @param tag_name The tag name we want to validate.
 */
bool is_valid_tag(std::string_view tag_name) {
  // Check if we have a valid registered tag name.
  auto tag_query{ tag_catalog.find((std::string)tag_name) };
  return tag_query != tag_catalog.end();
}


/*!
 * This function checks if the attribute name belongs to a given tag name.
 * @note The precondition is that tag_name is valid.
 * @param tag_name A valid tag name.
 * @attribute_name The attribute name we want to validate.
 */
bool is_valid_attribute(std::string_view tag_name, std::string_view attribute_name) {
  // Get the attribute list associated with `tag_name`.
  auto attribute_list{ tag_catalog[(std::string)tag_name] };
  auto attr_query = std::find(attribute_list.begin(), attribute_list.end(), attribute_name);
  return attr_query != attribute_list.end();
}


/*!
 * This function invokes a converter function that translates the attribute content (as a string)
 * into the expected type and store it into the ryt::ParamSet object received as input argument.
 * @param attr_name The attribute name.
 * @param attr_content The attribute value as a string.
 * @param ps A reference to the current ryt::ParamSet object we are filling in.
 */
void parse_attribute(const std::string& attr_name /* IN value */,
                     const std::string& attr_content /* IN value */,
                     ryt::ParamSet* ps /* OUT value*/) {
  std::ostringstream oss;
  // Find the proper convertion function.
  auto converter_func = converters[attr_name];
  if (converter_func) {  // Do we have one defined?
    if (converter_func(attr_name, attr_content, ps)) {
      oss << " ⁺ Successfuly converted attribute " << std::quoted(attr_name);
      MESSAGE(oss.str());
    } else {
      oss << " - Convertion of " << std::quoted(attr_name) << " failed!";
      MESSAGE(oss.str());
    }
  } else {
    oss << " - Could not find a convertion function for the tag " << std::quoted(attr_name)
        << ". Skipping...";
    WARNING(oss.str());
  }
}


/*!
 * This is the entry point where the parsing of the scene file begins.
 */
void parse_scene_file(const char* filename) {
  // Load document.
  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(filename) != tinyxml2::XML_SUCCESS) {
    std::cerr << "Error loading the XML file!" << '\n';
    return;
  }

  // Get the Root node
  tinyxml2::XMLElement* root = doc.RootElement();
  if (root == nullptr) {
    std::cerr << "Root node of the XML tree was not found!" << '\n';
    return;
  }
  
  // Iterate over every child elements, i.e. over every tag.
  for (tinyxml2::XMLElement* child_node = root->FirstChildElement(); child_node != nullptr;
    child_node = child_node->NextSiblingElement()) {
    // ================================================================================
    // Validate the current tag name.
    // --------------------------------------------------------------------------------
    std::string tag_name = ryt::str_to_lower(child_node->Name());
    if (not is_valid_tag(tag_name)) {
      std::ostringstream oss;
      oss << "The tag " << std::quoted(tag_name) << " is not valid!";
      WARNING(oss.str());
      continue;  // Skip to the next tag in the scene file.
    }
    std::ostringstream oss;
    oss << ">>>>> Started parsing tag " << std::quoted(tag_name) << ".";
    MESSAGE(oss.str());
    // ================================================================================
    // At this point we have a valid tag name. Now we need to validate its attributes.
    // --------------------------------------------------------------------------------
    // Create the empty ryt::ParamSet object to store the attributes we will process next.
    ryt::ParamSet ps;
    // Iterate over this tag's attributes
    for (const tinyxml2::XMLAttribute* attr = child_node->FirstAttribute(); 
         attr != nullptr; attr = attr->Next()) {
      // Validate the current attribute name.
      std::string attribute_name{ ryt::str_to_lower(attr->Name()) };
      if (not is_valid_attribute(tag_name, attribute_name)) {
        std::ostringstream oss;
        oss << "The tag " << std::quoted(tag_name) << " does not have an attribute "
            << std::quoted(attribute_name) << ". Ignoring...";
        WARNING(oss.str());
        continue;  // Skip to the next attribute inside this tag.
      }
      // Material "color" uses float 0-1 Spectrum, not integer RGBColor.
      std::string attribute_value{ ryt::str_to_lower(attr->Value()) };
      if (tag_name == "material" && attribute_name == "color") {
        convert<ryt::Spectrum, 3>(attribute_name, attribute_value, &ps);
      } else {
        parse_attribute(attribute_name, attribute_value, &ps);
      }
    }

    // ================================================================================
    // Now we have gc::ParamSet object filled in and ready to be passed along to the API.
    // ================================================================================
    // ============================================================================
    /// HACK: If the tag is `include` we call `parse_scene_file()` recursively.
    // ----------------------------------------------------------------------------
    if (tag_name == "include") {
      auto filename = ps.retrieve<std::string>("filename", "");
      if (filename.empty()) {
        WARNING("Missing attribute \"filename\" in tag \"include\"");
        continue;
      }
      if (not fs::exists(fs::path{ filename.c_str() })) {
        std::ostringstream oss;
        oss << "Included file " << std::quoted(filename) << " does not exist.";
        ERROR(oss.str());
      }
      // Recursive call to process subfile.
      parse_scene_file(filename.c_str());
      continue;  // This tag doesn't have an API function associated with; get next tag.
    }
    // ============================================================================

    // Check whether this tag_name has a proper API function.
    if (api_functions.count(tag_name) == 0) {
      std::ostringstream oss;
      oss << "The tag " << std::quoted(tag_name)
          << " does not have a corresponding API function associated with. Ignoring...";
      WARNING(oss.str());
      continue;
    }

    {
      std::ostringstream oss;
      oss << "<<<<< Calling API function for the tag " << std::quoted(tag_name) << ".\n";
      MESSAGE(oss.str());
    }
    // Call the api function associated with the tag name.
    api_functions[tag_name](ps);
  }
}
