#include "arguments.hpp"

#include <iostream>
#include <string>

namespace po = boost::program_options;

auto validator_uint_greater_equal(const char* const option_name, uint64_t min) {
  return [option_name, min](auto n) {
    if (n < min) {
      throw po::validation_error(po::validation_error::invalid_option_value, option_name, std::to_string(n));
    }
  };
}

void validate_option_existence(po::variables_map args, const char* const option_name) {
  if (args.count(option_name) == 0) {
    // TODO(jessef): better exception
    throw po::validation_error(po::validation_error::at_least_one_value_required, option_name);
  }
}

void mode_validate_chase(po::variables_map args) {
  validate_option_existence(args, "initsize");
  validate_option_existence(args, "initstart");
  validate_option_existence(args, "initcount");
}

void mode_validate_pounce(po::variables_map args) {
  // TODO(jessef)
}

po::variables_map parse_arguments(int argc, char** argv) {
  po::options_description general_options("General options");
  po::options_description chase_options("Chase-mode options");
  po::options_description pounce_options("Pounce-mode options");

  // clang-format off
  general_options.add_options()
    ("help,h",        po::bool_switch(),
      "Print help text")
    ("chase,c",       po::bool_switch(),
      "Chase mode (breadth search)")
    ("pounce,p",      po::bool_switch(),
      "Pounce mode (depth search)")
    ("outfile,o",     po::value<std::string>()->default_value("./posttag.dat")->value_name("path"),
      "Path to output file")
    ("timeout,t",     po::value<uint64_t>()->default_value(0)->value_name("secs"),
      "Total execution time constraint (seconds)");

  chase_options.add_options()
    ("cribfile,f",    po::value<std::string>()->value_name("path"),
      "Path to crib file (list of known sequences)")
    ("cribsize,b",  po::value<uint32_t>()->value_name("size")
                          ->notifier(validator_uint_greater_equal("cribsize", 2)),
      "Size of sequences in crib file")
    ("initsize,l",  po::value<uint64_t>()->default_value(30)->value_name("size")
                          ->notifier(validator_uint_greater_equal("initsize", 1)),
      "Size of initial condition sequences")
    ("initstart,s",   po::value<uint64_t>()->value_name("start"),
      "Starting initial condition")
    ("initcount,n",   po::value<uint64_t>()->default_value(1)->value_name("count")
                          ->notifier(validator_uint_greater_equal("initcount", 1)),
      "Number of initial conditions")
    ("maxsteps,m",    po::value<uint64_t>()->default_value(1e10, "10^10")->value_name("steps")
                          ->notifier(validator_uint_greater_equal("maxsteps", 1)),
      "Maximum number of steps to evaluate each initial condition to");

  pounce_options.add_options()
    ("initfile,i",    po::value<std::string>()->value_name("path"),
      "Path to file of initial conditions")
    ("maxsize,x",     po::value<unsigned int>()->default_value(1e9, "10^9")->value_name("size")
                          ->notifier(validator_uint_greater_equal("maxsize", 1)),
      "Maximum size to evaluate to");
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
