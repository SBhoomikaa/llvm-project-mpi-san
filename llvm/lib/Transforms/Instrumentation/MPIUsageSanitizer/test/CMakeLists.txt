# CMake configuration for MPI Usage Sanitizer Pass tests

# Configure lit tests
if(LLVM_INCLUDE_TESTS AND LLVM_ENABLE_MPI_SANITIZER)
  # Configure the lit site configuration
  configure_lit_site_cfg(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
    MAIN_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  )

  # Add the test suite
  add_lit_testsuite(check-mpi-sanitizer "Running MPI Usage Sanitizer tests"
    ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS opt FileCheck count not llvm-dis llvm-as
  )

  # Add to the main LLVM test suite
  add_dependencies(check-llvm check-mpi-sanitizer)
  
  # Set test properties
  set_target_properties(check-mpi-sanitizer PROPERTIES FOLDER "Tests")
  
  # Add test timeout
  if(DEFINED LIT_TEST_TIMEOUT)
    set_property(TEST check-mpi-sanitizer PROPERTY TIMEOUT ${LIT_TEST_TIMEOUT})
  else()
    set_property(TEST check-mpi-sanitizer PROPERTY TIMEOUT 300)
  endif()
  
  # Test categories for selective running
  add_lit_testsuite(check-mpi-sanitizer-basic "Running basic MPI sanitizer tests"
    ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS opt FileCheck
    ARGS --filter="basic-.*"
  )
  
  add_lit_testsuite(check-mpi-sanitizer-optimization "Running optimization tests"
    ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS opt FileCheck
    ARGS --filter="optimization-.*"
  )
  
  if(LLVM_MPI_SANITIZER_ENABLE_PERFORMANCE_TESTS)
    add_lit_testsuite(check-mpi-sanitizer-performance "Running performance tests"
      ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS opt FileCheck
      ARGS --filter="performance-.*"
    )
  endif()
  
  # Property-based testing integration
  if(LLVM_ENABLE_PROPERTY_BASED_TESTING)
    add_lit_testsuite(check-mpi-sanitizer-properties "Running property-based tests"
      ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS opt FileCheck
      ARGS --filter="property-.*"
    )
  endif()
endif()

# Install test files for external testing
if(LLVM_INSTALL_UTILS AND LLVM_ENABLE_MPI_SANITIZER)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION test/Transforms/Instrumentation/MPIUsageSanitizer
    COMPONENT llvm-tests
    FILES_MATCHING
    PATTERN "*.ll"
    PATTERN "*.c"
    PATTERN "*.cpp"
    PATTERN "*.f90"
    PATTERN "lit.cfg.py"
    PATTERN "CMakeLists.txt" EXCLUDE
  )
  
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
    DESTINATION test/Transforms/Instrumentation/MPIUsageSanitizer
    COMPONENT llvm-tests
  )
endif()