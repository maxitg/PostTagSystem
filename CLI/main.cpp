#include <cinttypes>
#include <iostream>
#include <utility>

#include "PostTagHistory.hpp"
#include "PostTagSearcher.hpp"
#include "TagState.hpp"
#include "arguments.hpp"
#include "boost/format.hpp"
#include "files/PostTagCribFile.hpp"
#include "files/PostTagInitFile.hpp"
#include "files/PostTagResultFile.hpp"

using boost::program_options::variables_map;
using PostTagSystem::PostTagHistory, PostTagSystem::TagState, PostTagSystem::PostTagSearcher;

std::vector<bool> integer_bits(uint64_t n, uint32_t bit_count) {
  std::vector<bool> bits(bit_count, false);

  for (size_t i = (bit_count - 1); i > 0; i--) {
    bits[i] = n & 1;
    n >>= 1;
  }

  return bits;
}

void print_bits(const std::vector<bool>& bits) {
  for (auto i = bits.begin(); i != bits.end(); i++) {
    std::cout << *i;
  }
}

PostTagSearcher::EvaluationParameters get_eval_parameters(const variables_map& args) {
  PostTagSearcher::EvaluationParameters eval_params;

  auto max_size = args["maxsize"].as<uint64_t>();
  if (max_size > 0) {
    eval_params.maxTapeLength = max_size;
    std::cout << boost::format("Maximum tape size: %u seconds\n") % max_size;
  } else {
    std::cout << "Maximum tape size: unlimited\n";
  }

  auto max_steps = args["maxsteps"].as<uint64_t>();
  if (max_steps > 0) {
    eval_params.maxEventCount = max_steps;
    std::cout << boost::format("Maximum step count: %u seconds\n") % max_steps;
  } else {
    std::cout << "Maximum step count: unlimited\n";
  }

  auto timeout = args["timeout"].as<uint64_t>();
  if (timeout > 0) {
    eval_params.groupTimeConstraintNs = timeout * 1e9;  // seconds to ns
    std::cout << boost::format("Total evaluation time limit: %u seconds\n") % timeout;
  } else {
    std::cout << "Total evaluation time limit: unlimited\n";
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

int run_mode_chase(const variables_map& args) {
  auto tape_length = args["initsize"].as<uint8_t>();
  auto start = args["initstart"].as<uint64_t>();
  auto count = args["initcount"].as<uint64_t>();
  auto offset = args["initoffset"].as<uint64_t>();

  // allows several jobs of the same size to be run
  // at different offsets from the starting point
  start += count * offset;

  PostTagSearcher searcher;

  auto eval_params = get_eval_parameters(args);
  std::cout << "\n";

  std::cout << boost::format("Evaluating %u initial conditions, starting at %u...\n") % count % start;
  std::cout << "----------------\n";

  std::vector<PostTagSearcher::EvaluationResult> results =
      searcher.evaluateRange(tape_length, start, start + count, eval_params);
  //std::cout << "----------------\n";

  std::cout << boost::format("Evaluation finished with %i results\n") % results.size();

  auto result_file_path = args["outfile"].as<std::string>();
  std::cout << boost::format("Writing results to '%s'\n") % result_file_path;

  PostTagResultFileWriter result_file_writer(result_file_path, std::ios::binary);
  if (!result_file_writer.is_open()) {
    throw std::runtime_error("Failed to open output file '" + result_file_path + "' for writing");
  }

  PostTagResultFile result_file(Version1, results);

  result_file_writer.write_file(result_file);

  return 0;
}

int run_mode_pounce(const variables_map& args) {
  auto init_file_path = args["initfile"].as<std::string>();
  PostTagInitFileReader init_file_reader(init_file_path, std::ios::binary);
  if (!init_file_reader.is_open()) {
    throw std::runtime_error("Failed to open init file '" + init_file_path + "' for reading");
  }

  PostTagInitFile init_file = init_file_reader.read_file();

  printf("Read %" PRIu64 " initial conditions\n", init_file.state_count);

  for (TagState& state : init_file.states) {
    print_bits(state.tape);
    printf(" - %u\n", state.headState);
  }

  return 0;
}

int main(int argc, char** argv) {
  variables_map args;
  try {
    args = parse_arguments(argc, argv);
  } catch (const std::exception& err) {
    printf("Input error: %s\n", err.what());
    return 1;
  }

  if (args["help"].as<bool>() || args["version"].as<bool>()) {
    return 0;
  }

  int mode_result;
  try {
    if (args["chase"].as<bool>()) {
      mode_result = run_mode_chase(args);
    } else if (args["pounce"].as<bool>()) {
      mode_result = run_mode_pounce(args);
    } else {
      throw std::logic_error("No valid mode specified");
    }
  } catch (const std::exception& err) {
    printf("Evaluation error: %s\n", err.what());
    return 1;
  }

  return mode_result;
}
