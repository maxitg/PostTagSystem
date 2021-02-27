#include <iostream>

#include "PostTagHistory.hpp"
#include "PostTagState.hpp"
#include "arguments.hpp"
#include "files/PostTagCribFile.hpp"

using PostTagSystem::PostTagHistory, PostTagSystem::PostTagState;

namespace po = boost::program_options;

std::vector<bool> integer_bits(uint64_t n, uint32_t bit_count) {
  std::vector<bool> bits(bit_count, false);

  for (size_t i = (bit_count - 1); i > 0; i--) {
    bits[i] = n & 1;
    n >>= 1;
  }

  return bits;
}

void print_bits(std::vector<bool> bits) {
  for (auto i = bits.begin(); i != bits.end(); i++) {
    std::cout << *i;
  }
}

int run_mode_chase(po::variables_map args) {
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
      throw std::runtime_error("Failed to open crib file '" + crib_file_path + "'");
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

  PostTagState init_state;
  PostTagHistory system;
  PostTagHistory::EvaluationResult result;

  printf("Chasing...\n");
  printf("--------------\n");

  for (uint64_t init = start; init < (start + count); init++) {
    init_state.headState = 0;
    init_state.tape = integer_bits(init, size);

    printf("Initial condition: %lu (", init);
    print_bits(init_state.tape);
    printf(") - %u\n", init_state.headState);

    result = system.evaluate(PostTagHistory::NamedRule::Post, init_state, max_steps, checkpoint_spec);

    printf("Event count: %lu\n", result.eventCount);
    printf("Max tape length: %lu\n", result.maxTapeLength);
    printf("Final condition: ");
    print_bits(result.finalState.tape);
    printf(" - %u\n", result.finalState.headState);
    printf("--------------\n");
  }

  printf("Done chasing!\n");

  return 0;
}

int run_mode_pounce(po::variables_map args) {
  throw std::logic_error("Pounce mode not implemented");
  return 1;
}

int main(int argc, char** argv) {
  po::variables_map args;
  try {
    args = parse_arguments(argc, argv);
  } catch (const std::exception& err) {
    printf("Input error: %s\n", err.what());
    return 1;
  }

  if (args["help"].as<bool>()) {
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
