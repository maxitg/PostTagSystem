#include "arguments.hpp"

#include <iostream>
#include <string>

namespace po = boost::program_options;

auto validator_uint_greater_equal(const char* const option_name, uint64_t min) {
  return [option_name, min](auto n) {
    if (n < min) {
      throw po::validation_error(
          po::validation_error::invalid_option_value, option_name, std::to_string(static_cast<uint64_t>(n)));
    }
  };
}

auto validator_uint_between(const char* const option_name, uint64_t min, uint64_t max) {
  return [option_name, min, max](auto n) {
    if (n < min || n > max) {
      throw po::validation_error(
          po::validation_error::invalid_option_value, option_name, std::to_string(static_cast<uint64_t>(n)));
    }
  };
}

void validate_option_existence(const po::variables_map& args, const char* const option_name) {
  if (args.count(option_name) == 0) {
    // TODO(jessef): better exception
    throw po::validation_error(po::validation_error::at_least_one_value_required, option_name);
  }
}

void mode_validate_chase(const po::variables_map& args) {
  validate_option_existence(args, "initsize");
  validate_option_existence(args, "initstart");
  validate_option_existence(args, "initcount");
}

void mode_validate_pounce(const po::variables_map& args) { validate_option_existence(args, "initfile"); }

po::variables_map parse_arguments(int argc, char** argv) {
  po::options_description general_options("General options");
  po::options_description chase_options("Chase-mode options");
  po::options_description pounce_options("Pounce-mode options");

  // clang-format off
  general_options.add_options()
    ("help,h",        po::bool_switch(),
      "Print help text")
    ("version,v",     po::bool_switch(),
      "Print program version")
    ("chase,c",       po::bool_switch(),
      "Chase mode (range search)")
    ("pounce,p",      po::bool_switch(),
      "Pounce mode (file search)")
    ("outfile,o",     po::value<std::string>()->default_value("./output.postresult")->value_name("path.postresult"),
      "Path to output file")
    ("maxsize,x",     po::value<uint64_t>()->default_value(static_cast<uint64_t>(1e9), "10^9")->value_name("size"),
      "Maximum tape length to evaluate each initial condition to (0 = no limit)")
    ("maxoutsize,z",  po::value<uint64_t>()->value_name("size"),
      "Maximum tape length to include in output file entries (0 = never write final tapes; omit for no limit)")
    ("maxsteps,m",    po::value<uint64_t>()->default_value(static_cast<uint64_t>(1e10), "10^10")->value_name("steps"),
      "Maximum number of steps to evaluate each initial condition to (0 = no limit)")
    ("timeout,t",     po::value<uint32_t>()->default_value(0)->value_name("secs"),
      "Total execution time constraint (seconds) (0 = no limit)");

  chase_options.add_options()
    ("cribfile,f",    po::value<std::string>()->value_name("path.postcrib"),
      "Path to crib file (list of known sequences)")
    ("initsize,l",    po::value<uint16_t>()->value_name("size")
                          ->notifier(validator_uint_between("initsize", 1, 64)),
      "Size of initial condition tapes")
    ("initstart,s",   po::value<uint64_t>()->value_name("start"),
      "Starting initial condition tape (as decimal integer)")
    ("initcount,n",   po::value<uint64_t>()->default_value(1)->value_name("count")
                          ->notifier(validator_uint_greater_equal("initcount", 1)),
      "Number of initial condition tapes to evaluate")
    ("initoffset,e",  po::value<uint64_t>()->default_value(0)->value_name("offset"),
      "Shifts starting condition by offset * count (for use with zero-indexed array jobs)");

  pounce_options.add_options()
    ("initfile,i",    po::value<std::string>()->value_name("path.postinit"),
      "Path to file of initial conditions");
  // clang-format on

  po::options_description all_options;
  all_options.add(general_options).add(chase_options).add(pounce_options);

  po::variables_map args;
  po::store(po::command_line_parser(argc, argv).options(all_options).run(), args);
  po::notify(args);

  if (args["help"].as<bool>()) {
    std::cout << all_options << "\n";
    return args;
  }

  if (args["version"].as<bool>()) {
#if defined(POST_TAG_VERSION_MAJOR) && defined(POST_TAG_VERSION_MINOR) && defined(POST_TAG_VERSION_PATCH)
    printf("wolfram-postproject v%u.%u.%u =^._.^=\n",
           POST_TAG_VERSION_MAJOR,
           POST_TAG_VERSION_MINOR,
           POST_TAG_VERSION_PATCH);
#else
    printf("wolfram-postproject unknown version =^._.^=\n");
#endif

    return args;
  }

  if (args["chase"].as<bool>() && args["pounce"].as<bool>()) {
    throw std::runtime_error("Only one of --chase or --pounce may be specified");
  } else if (args["chase"].as<bool>()) {
    mode_validate_chase(args);
  } else if (args["pounce"].as<bool>()) {
    mode_validate_pounce(args);
  } else {
    throw std::runtime_error("One of --chase or --pounce must be specified");
  }

  return args;
}
