#include <cinttypes>
#include <iostream>

#include "PostTagHistory.hpp"
#include "TagState.hpp"
#include "arguments.hpp"
#include "files/PostTagCribFile.hpp"
#include "files/PostTagInitFile.hpp"
#include "files/PostTagResultFile.hpp"

using boost::program_options::variables_map;
using PostTagSystem::PostTagHistory, PostTagSystem::TagState;

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

int run_mode_chase(variables_map args) {
  auto size = args["initsize"].as<uint64_t>();
  auto start = args["initstart"].as<uint64_t>();
  auto count = args["initcount"].as<uint64_t>();
  auto offset = args["initoffset"].as<uint64_t>();
  auto max_steps = args["maxsteps"].as<uint64_t>();

  // allows several jobs of the same size to be run
  // at different offsets from the starting point
  start += count * offset;

  PostTagHistory::CheckpointSpec checkpoint_spec;

  if (args.count("cribfile")) {
    auto crib_file_path = args["cribfile"].as<std::string>();
    PostTagCribFileReader crib_file_reader(crib_file_path, std::ios::binary);
    if (!crib_file_reader.is_open()) {
      throw std::runtime_error("Failed to open crib file '" + crib_file_path + "' for reading");
    }

    PostTagCribFile crib_file = crib_file_reader.read_file();

    checkpoint_spec.states = crib_file.checkpoints;

    for (size_t i = 0; i < checkpoint_spec.states.size(); i++) {
      auto checkpoint = checkpoint_spec.states[i];
      print_bits(checkpoint.tape);
      printf(" - %u\n", checkpoint.headState);
    }
    return 0;
  }

  TagState init_state;
  PostTagHistory system;

  std::vector<PostTagHistory::EvaluationResult> results(count);

  printf("Chasing...\n");
  printf("--------------\n");

  for (size_t i = 0; i < count; i++) {
    uint64_t init = start + i;

    init_state.headState = 0;
    init_state.tape = integer_bits(init, size);

    printf("Initial condition: %" PRIu64 " (", init);
    print_bits(init_state.tape);
    printf(") - %u\n", init_state.headState);

    results[i] = system.evaluate(PostTagHistory::NamedRule::Post, init_state, max_steps, checkpoint_spec);

    printf("Event count: %" PRIu64 "\n", results[i].eventCount);
    printf("Max tape length: %" PRIu64 "\n", results[i].maxTapeLength);
    printf("Final condition: ");
    print_bits(results[i].finalState.tape);
    printf(" - %u\n", results[i].finalState.headState);
    printf("--------------\n");
  }

  printf("Done chasing!\n");

  if (args.count("outfile")) {
    auto result_file_path = args["outfile"].as<std::string>();
    PostTagResultFileWriter result_file_writer(result_file_path, std::ios::binary);
    if (!result_file_writer.is_open()) {
      throw std::runtime_error("Failed to open output file '" + result_file_path + "' for writing");
    }

    PostTagResultFile result_file(Version1, results);

    result_file_writer.write_file(result_file);
  }

  return 0;
}

int run_mode_pounce(variables_map args) {
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
