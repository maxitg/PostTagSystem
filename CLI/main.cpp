#include <cinttypes>
#include <iostream>
#include <utility>

#include "PostTagSearcher.hpp"
#include "arguments.hpp"
#include "boost/format.hpp"
#include "files/PostTagCribFile.hpp"
#include "files/PostTagInitFile.hpp"
#include "files/PostTagResultFile.hpp"

using boost::program_options::variables_map;
using PostTagSystem::PostTagSearcher;

PostTagSearcher::EvaluationParameters get_eval_parameters(const variables_map& args) {
  PostTagSearcher::EvaluationParameters eval_params;

  auto max_size = args["maxsize"].as<uint64_t>();
  if (max_size > 0) {
    eval_params.maxTapeLength = max_size;
    std::cout << boost::format("Maximum tape size: %.6g\n") % static_cast<double>(max_size);
  } else {
    std::cout << "Maximum tape size: unlimited\n";
  }

  auto max_steps = args["maxsteps"].as<uint64_t>();
  if (max_steps > 0) {
    eval_params.maxEventCount = max_steps;
    std::cout << boost::format("Maximum step count: %.6g\n") % static_cast<double>(max_steps);
  } else {
    std::cout << "Maximum step count: unlimited\n";
  }

  auto timeout = args["timeout"].as<uint32_t>();
  if (timeout > 0) {
    eval_params.groupTimeConstraintNs = timeout * static_cast<uint64_t>(1e9);  // seconds to ns
    std::cout << boost::format("Total evaluation time limit: %u seconds\n") % timeout;
  } else {
    std::cout << "Total evaluation time limit: unlimited\n";
  }

  if (args["allstates"].as<bool>()) {
    eval_params.includeUnevaluatedStates = true;
    eval_params.includeMergedStates = true;
  }

  if (args.count("cribfile")) {
    auto crib_file_path = args["cribfile"].as<std::string>();
    PostTagCribFileReader crib_file_reader(crib_file_path, std::ios::binary);
    if (!crib_file_reader.is_open()) {
      throw std::runtime_error("Failed to open crib file '" + crib_file_path + "' for reading");
    }

    PostTagCribFile crib_file = crib_file_reader.read_file();

    eval_params.checkpoints = std::move(crib_file.checkpoints);

    std::cout << boost::format("Loaded %u checkpoint(s) from crib file '%s'\n") % crib_file.checkpoint_count %
                     crib_file_path;
  } else {
    std::cout << "No crib file specified; not loading checkpoints\n";
  }

  // TODO(jessef): check that outfile can be opened prior to running the evaluation

  return eval_params;
}

PostTagInitFile get_initial_states_from_file(const variables_map& args) {
  auto init_file_path = args["initfile"].as<std::string>();
  PostTagInitFileReader init_file_reader(init_file_path, std::ios::binary);
  if (!init_file_reader.is_open()) {
    throw std::runtime_error("Failed to open init file '" + init_file_path + "' for reading");
  }

  PostTagInitFile init_file = init_file_reader.read_file();

  std::cout << boost::format("Loaded %u initial conditions from file '%s'\n") % init_file.state_count % init_file_path;

  return init_file;
}

int main(int argc, char** argv) {
  variables_map args;
  try {
    args = parse_arguments(argc, argv);
  } catch (const std::exception& err) {
    std::cout << "Input error: " << err.what() << "\n";
    return 1;
  }

  if (args["help"].as<bool>() || args["version"].as<bool>()) {
    return 0;
  }

  auto chaseMode = args["chase"].as<bool>();
  auto pounceMode = args["pounce"].as<bool>();
  if (!(chaseMode || pounceMode)) {
    std::cout << "Input error: No valid mode specified\n";
  }

  PostTagSearcher searcher;
  PostTagSearcher::EvaluationParameters eval_params;

  try {
    eval_params = get_eval_parameters(args);
    std::cout << "\n";
  } catch (const std::exception& err) {
    std::cout << "Input error: " << err.what() << "\n";
    return 1;
  }

  std::vector<PostTagSearcher::EvaluationResult> results;

  if (chaseMode) {
    auto tape_length = static_cast<uint8_t>(args["initsize"].as<uint16_t>());
    auto start = args["initstart"].as<uint64_t>();
    auto count = args["initcount"].as<uint64_t>();
    auto offset = args["initoffset"].as<uint64_t>();

    // allows several jobs of the same size to be run
    // at different offsets from the starting point
    start += count * offset;

    std::cout << boost::format("Evaluating %u initial condition tapes, starting at %u...\n") % count % start;
    std::cout << "----------------\n";

    try {
      results = searcher.evaluateRange(tape_length, start, start + count, eval_params);
    } catch (const std::exception& err) {
      std::cout << "Evaluation error: " << err.what() << "\n";
      return 1;
    }

  } else if (pounceMode) {
    PostTagInitFile init_file;
    try {
      init_file = get_initial_states_from_file(args);
    } catch (const std::exception& err) {
      std::cout << "Input error: " << err.what() << "\n";
      return 1;
    }

    std::cout << boost::format("Evaluating %u initial condition tapes...\n") % init_file.state_count;
    std::cout << "----------------\n";

    try {
      results = searcher.evaluateGroup(init_file.states, eval_params);
    } catch (const std::exception& err) {
      std::cout << "Evaluation error: " << err.what() << "\n";
      return 1;
    }
  }

  std::cout << boost::format("Evaluation finished with %u results\n") % results.size();

  auto result_file_path = args["outfile"].as<std::string>();

  PostTagResultFileWriter result_file_writer(result_file_path, std::ios::binary);
  if (!result_file_writer.is_open()) {
    std::cout << boost::format("Output error: Failed to open output file '%s' for writing\n") % result_file_path;
    return 1;
  }

  PostTagResultFile result_file(Version1, results);
  result_file.biggest_tape_to_write = args["maxoutsize"].as<uint64_t>();

  result_file_writer.write_file(result_file);

  std::cout << boost::format("Wrote results to '%s'\n") % result_file_path;

  return 0;
}
