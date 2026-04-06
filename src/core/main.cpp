#include <filesystem>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "App.hpp"
#include "common.hpp"

void usage(std::string_view msg = "") {
  if (not msg.empty()) {
    std::cout << msg << "\n\n";
  }
  std::cout
    << "Usage: raytmath [<options>] <input_scene_file>\n"
    << "  Rendering simulation options:\n"
    << "    --help                     Print this help text.\n"
    << "    --verbose or -v            Run in verbose mode.\n"
    << "    --cropwindow x0 x1 y0 y1   Specify an image crop window; values must be between 0 and "
       "1.\n"
    << "    --quick                    Reduces quality parameters to render image quickly.\n"
    << "    --outfile <filename>       Write the rendered image to <filename>.\n\n";
  exit(msg.empty() ? 1 : 0);
}

void validate_cropwindow(std::array<std::string_view, 4> arguments,
                         ryt::RunningOptions& run_opt_out) {
  // Validate crop values.
  try {
    unsigned short idx{ 0 };
    for (const auto& value : arguments) {
      run_opt_out.crop_window[idx++] = std::stof((std::string)value);
    }
  } catch (const std::invalid_argument& e) {
    usage("\n Could not convert --cropwindow's arguments into real number.");
  } catch (const std::out_of_range& e) {
    usage("\n At least one of the --cropwindow values are out of range.");
  }
  float& x0{ run_opt_out.crop_window[0] };
  float& x1{ run_opt_out.crop_window[1] };
  float& y0{ run_opt_out.crop_window[2] };
  float& y1{ run_opt_out.crop_window[3] };
  // Check whether the values are in the expected range (in [0,1])...
  if (x0 < 0.F or x0 > 1.F or x1 < 0.F or x1 > 1.F or y0 < 0.F or y0 > 1.F or y1 < 0.F
      or y1 > 1.F) {
    usage("\n At least one of the --cropwindow values are out of range.");
  }
  // ... and if they obey x0<x1 and y0<y1.
  if (x0 > x1 or y0 > y1) {
    usage("\n The --cropwindow values doest not comply with x0<x1 and y0<y1.");
  }
  // Finally, turn on flag to indicate we got a crop window via CLI.
  run_opt_out.crop_window_provided = true;
}

void validate_arguments(int argc, char* argv[], ryt::RunningOptions& run_options) {
  if (argc == 1) {  // Missing arguments.
    usage();        // Send default message to the user, and exit.
  }
  // Prepare to parse input argumnts.
  std::ostringstream oss;
  bool has_scenefile{ false };  // use must provide a scene file.
  for (int i{ 1 }; i < argc; ++i) {
    std::string option{ ryt::str_lowercase(argv[i]) };
    // Parsing cropwindow values.
    if (option == "--cropwindow" or option == "-cropwindow" or option == "--crop"
        or option == "-crop") {
      if (i + 4 >= argc) {  // The option's argument is missing.
        usage("\n Expected at least 4 values after --cropwindow argument.");
      }
      std::array<std::string_view, 4> arguments{ argv[++i], argv[++i], argv[++i], argv[++i] };
      validate_cropwindow(arguments, run_options);
    } else if (option == "--outfile" or option == "-outfile" or option == "-o") {
      if (i + 1 == argc) {  // The option's argument is missing.
        usage("\n Missing filename after --outfile argument.");
      }
      // Get output image file name.
      run_options.outfile = argv[++i];
    } else if (option == "--quickrender" or option == "-quickrender" or option == "-q"
               or option == "--quick" or option == "-quick") {
      run_options.quick_render = true;
    } else if (option == "--verbose" or option == "-v") {
      run_options.verbose = true;
    } else if (option == "--help" or option == "-help" or option == "-h") {
      usage();
    } else {
      // Required: scene file.
      if (not std::filesystem::exists(std::filesystem::path(option))) {
        usage("\n Scene file provided does not exist.");
      }
      run_options.filename = option;
      has_scenefile = true;
    }
  }  // for to traverse the argument list.
  if (not has_scenefile) {
    usage("\n Missing scene file, cannot proceed.");
  }
}

std::string to_string(const ryt::RunningOptions& ro) {
  std::ostringstream oss;
  oss << "- cropwindow: [" << ro.crop_window[0] << " " << ro.crop_window[1] << " "
      << ro.crop_window[2] << " " << ro.crop_window[3] << "]\n";
  oss << "- quick rendering: " << std::boolalpha << ro.quick_render << '\n';
  oss << "- verbose: " << std::boolalpha << ro.verbose << '\n';
  oss << "- outfile: " << std::quoted(ro.outfile) << '\n';
  oss << "- scene file: " << std::quoted(ro.filename);
  return oss.str();
}


int main(int argc, char* argv[]) {
  ryt::RunningOptions run_options;  // stores incoming arguments.

  // ================================================
  // (1) Validate command line arguments.
  // ================================================
  validate_arguments(argc, argv, run_options);

  if (run_options.verbose) {  // Show options set by user if in "verbose" mode.
    constexpr short line_length{ 80 };
    std::cout << std::setw(line_length) << std::setfill('-') << "\n";
    std::cout << ">>> Running options are:\n" << to_string(run_options) << '\n';
    std::cout << std::setw(line_length) << std::setfill('-') << "\n\n";
  }

  // ================================================
  // (2) Welcome message
  // ================================================
  std::cout << "Ray Tracer -- raym, v1.0 Matheus R.\n";

  // ================================================
  // (3) Initialize the renderer engine and load a scene.
  // ================================================
  ryt::App::init_engine(run_options);
  ryt::App::run();

  std::cout << "\n  --> Thanks for using! <--\n";

  return EXIT_SUCCESS;
}
