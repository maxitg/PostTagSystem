#ifndef POSTTAGSYSTEM_CLI_ARGUMENTS_HPP_
#define POSTTAGSYSTEM_CLI_ARGUMENTS_HPP_

#include <boost/program_options.hpp>

boost::program_options::variables_map parse_arguments(int argc, char** argv);

#endif  // POSTTAGSYSTEM_CLI_ARGUMENTS_HPP_
