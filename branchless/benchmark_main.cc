#include <cstdlib>
#include <iostream>

#include "benchmark/benchmark.h"

// Runs the benchmark based on branching. The function `choose_decision_value`
// is used to determine the values of the decision vector.
void run_branching_benchmark(benchmark::State& state,
                             int (*choose_decision_value)()) {
  std::srand(1);
  const unsigned int n = state.range(0);
  std::vector<unsigned long> v1(n, 0);
  std::vector<unsigned long> v2(n, 0);
  std::vector<int> decision_vector(n, 0);
  for (int i = 0; i < n; i++) {
    v1[i] = std::rand();
    v2[i] = std::rand();
    decision_vector[i] = choose_decision_value();
  }

  for (auto _ : state) {
    unsigned long calc = 0;
    for (size_t i = 0; i < n; ++i) {
      if (decision_vector[i]) {
        calc += v1[i];
      } else {
        calc *= v2[i];
      }
    }
    benchmark::DoNotOptimize(calc);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(n * state.iterations());
}

void run_branchless_benchmark(benchmark::State& state,
                              int (*choose_decision_value)()) {
  std::srand(1);
  const unsigned int n = state.range(0);
  std::vector<unsigned long> v1(n, 0);
  std::vector<unsigned long> v2(n, 0);
  std::vector<int> decision_vector(n, 0);
  for (int i = 0; i < n; i++) {
    v1[i] = std::rand();
    v2[i] = std::rand();
    decision_vector[i] = choose_decision_value();
  }

  for (auto _ : state) {
    unsigned long calc = 0;
    for (size_t i = 0; i < n; ++i) {
      unsigned long arr[2] = {calc * v2[i], calc + v1[i]};
      calc = arr[bool(decision_vector[i])];
    }
    benchmark::DoNotOptimize(calc);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(n * state.iterations());
}

static void BM_branch_prediction_random_vals(benchmark::State& state) {
  // The decision vector is filled with random values. We expect that the
  // hardware branch predictor will only predict correctly 50% of the time.
  run_branching_benchmark(state, []() -> int { return std::rand() & 0x1; });
}
BENCHMARK(BM_branch_prediction_random_vals)->Arg(1 << 22);

static void BM_branch_prediction_30_percent_true(benchmark::State& state) {
  // The decision vector is filled with values that are true 30% of the time.
  // We expect that the hardware branch predictor will predict correctly 70% of
  // the time.
  run_branching_benchmark(
      state, []() -> int { return std::rand() * 1.0 / RAND_MAX > 0.7; });
}
BENCHMARK(BM_branch_prediction_30_percent_true)->Arg(1 << 22);

static void BM_branch_prediction_70_percent_true(benchmark::State& state) {
  // The decision vector is filled with values that are true 70% of the time.
  // We expect that the hardware branch predictor will still predict correctly
  // 70% of the time.
  run_branching_benchmark(
      state, []() -> int { return std::rand() * 1.0 / RAND_MAX > (1 - 0.7); });
}
BENCHMARK(BM_branch_prediction_70_percent_true)->Arg(1 << 22);

static void BM_branch_prediction_almost_always_true(benchmark::State& state) {
  // The decision vector is filled with values that are almost always true. We
  // expect that the hardware branch predictor will predict correctly almost all
  // of the time.
  run_branching_benchmark(state, []() -> int { return std::rand() > 0; });
}
BENCHMARK(BM_branch_prediction_almost_always_true)->Arg(1 << 22);

static void BM_branch_prediction_always_true(benchmark::State& state) {
  // The decision vector is filled with values that are always true. We expect
  // that the hardware branch predictor will predict correctly almost all of the
  // time (as it might take some time to learn the pattern).
  run_branching_benchmark(state, []() -> int { return true; });
}
BENCHMARK(BM_branch_prediction_always_true)->Arg(1 << 22);

static void BM_branchless_random_vals(benchmark::State& state) {
  // The decision vector is filled with random values.
  run_branchless_benchmark(state, []() -> int { return std::rand() & 0x1; });
}
BENCHMARK(BM_branchless_random_vals)->Arg(1 << 22);

static void BM_branchless_always_true(benchmark::State& state) {
  // The decision vector is filled with random values.
  run_branchless_benchmark(state, []() -> int { return true; });
}
BENCHMARK(BM_branchless_always_true)->Arg(1 << 22);

BENCHMARK_MAIN();