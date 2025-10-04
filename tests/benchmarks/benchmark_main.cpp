/**
 * ASFMLogger Performance Benchmarks - Main Entry Point
 */

#include <benchmark/benchmark.h>

// Register individual benchmark files
extern void RegisterLogMessageBenchmarks();
extern void RegisterLoggerInstanceBenchmarks();
extern void RegisterImportanceMapperBenchmarks();
extern void RegisterSmartMessageQueueBenchmarks();
extern void RegisterDatabaseLoggerBenchmarks();
extern void RegisterComprehensiveBenchmarks();

int main(int argc, char** argv) {
    // Initialize benchmark
    ::benchmark::Initialize(&argc, argv);

    // Register all component benchmarks
    RegisterLogMessageBenchmarks();
    RegisterLoggerInstanceBenchmarks();
    RegisterImportanceMapperBenchmarks();
    RegisterSmartMessageQueueBenchmarks();
    RegisterDatabaseLoggerBenchmarks();
    RegisterComprehensiveBenchmarks();

    // Run benchmarks
    ::benchmark::RunSpecifiedBenchmarks();

    return 0;
}
