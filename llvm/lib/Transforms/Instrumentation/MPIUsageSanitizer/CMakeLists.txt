# CMakeLists.txt for MPI Usage Sanitizer components

set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  TransformUtils
)

# Enable exceptions for error handling
set(LLVM_REQUIRES_EH 1)

add_llvm_component_library(LLVMMPIUsageSanitizerComponents
  MPISanitizerPass.cpp
  MPICallDetector.cpp
  MetadataExtractor.cpp
  HookInserter.cpp
  MPIFunctionDatabase.cpp
  RuntimeInterface.cpp
  StaticAnalyzer.cpp
  OptimizationEngine.cpp
  ConfigurationManager.cpp
  ErrorHandler.cpp
  RuntimeInterfaceValidator.cpp
  PropertyBasedTestFramework.cpp
  IntegrationTests.cpp
  PerformanceTests.cpp
  PerformanceProfiler.cpp
  PassOptimizer.cpp
  OptimizationOverheadTest.cpp
  ErrorHandlerTest.cpp
  ParameterMarshalingTest.cpp
  RuntimeInterfaceValidatorTest.cpp
  RuntimeLibraryIntegrationTest.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Instrumentation

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  ${LLVM_LINK_COMPONENTS}
)