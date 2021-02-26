#ifndef CLI_ARGUMENTS_HPP_
#define CLI_ARGUMENTS_HPP_

#include <boost/program_options.hpp>

boost::program_options::variables_map parse_arguments(int argc, char** argv);

#endif  // CLI_ARGUMENTS_HPP_
