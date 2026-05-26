#!/bin/bash

# Performance comparison script for MPI Usage Sanitizer
# Compares execution time and overhead between instrumented and uninstrumented versions

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$(dirname "$SCRIPT_DIR")/build"
INSTRUMENTED_DIR="$BUILD_DIR/instrumented"
UNINSTRUMENTED_DIR="$BUILD_DIR/uninstrumented"
RESULTS_DIR="$BUILD_DIR/performance_results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print colored output
print_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN} MPI Usage Sanitizer Performance Test${NC}"
    echo -e "${CYAN}========================================${NC}"
}

print_status() {
    echo -e "${BLUE}[PERF]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if program exists
check_program() {
    local program=$1
    
    if [ ! -f "$UNINSTRUMENTED_DIR/$program" ]; then
        print_error "Uninstrumented program not found: $program"
        return 1
    fi
    
    if [ ! -f "$INSTRUMENTED_DIR/$program" ]; then
        print_error "Instrumented program not found: $program"
        return 1
    fi
    
    return 0
}

# Function to run performance test
run_performance_test() {
    local program=$1
    local mpi_args="$2"
    local iterations=${3:-5}
    
    print_status "Running performance test for: $program"
    print_status "MPI arguments: $mpi_args"
    print_status "Iterations: $iterations"
    
    # Create results directory
    mkdir -p "$RESULTS_DIR"
    
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local result_file="$RESULTS_DIR/${program//\//_}_${timestamp}.txt"
    
    echo "Performance Test Results" > "$result_file"
    echo "Program: $program" >> "$result_file"
    echo "MPI Args: $mpi_args" >> "$result_file"
    echo "Iterations: $iterations" >> "$result_file"
    echo "Timestamp: $(date)" >> "$result_file"
    echo "========================================" >> "$result_file"
    
    # Arrays to store timing results
    declare -a uninstrumented_times
    declare -a instrumented_times
    
    print_status "Running uninstrumented version..."
    
    # Run uninstrumented version
    for i in $(seq 1 $iterations); do
        print_status "  Iteration $i/$iterations (uninstrumented)"
        
        local start_time=$(date +%s.%N)
        
        # Capture both stdout and stderr, but suppress output for timing
        if mpirun $mpi_args "$UNINSTRUMENTED_DIR/$program" > /tmp/mpi_output_$$ 2>&1; then
            local end_time=$(date +%s.%N)
            local elapsed=$(echo "$end_time - $start_time" | bc -l)
            uninstrumented_times+=($elapsed)
            
            echo "Uninstrumented iteration $i: ${elapsed}s" >> "$result_file"
        else
            print_error "Uninstrumented run $i failed"
            echo "Uninstrumented iteration $i: FAILED" >> "$result_file"
        fi
        
        rm -f /tmp/mpi_output_$$
    done
    
    print_status "Running instrumented version..."
    
    # Run instrumented version
    for i in $(seq 1 $iterations); do
        print_status "  Iteration $i/$iterations (instrumented)"
        
        local start_time=$(date +%s.%N)
        
        # Capture output including sanitizer messages
        if mpirun $mpi_args "$INSTRUMENTED_DIR/$program" > /tmp/mpi_instrumented_$$ 2>&1; then
            local end_time=$(date +%s.%N)
            local elapsed=$(echo "$end_time - $start_time" | bc -l)
            instrumented_times+=($elapsed)
            
            echo "Instrumented iteration $i: ${elapsed}s" >> "$result_file"
            
            # Save sanitizer output from first iteration
            if [ $i -eq 1 ]; then
                echo "" >> "$result_file"
                echo "Sample Sanitizer Output (Iteration 1):" >> "$result_file"
                echo "----------------------------------------" >> "$result_file"
                cat /tmp/mpi_instrumented_$$ >> "$result_file"
                echo "----------------------------------------" >> "$result_file"
            fi
        else
            print_error "Instrumented run $i failed"
            echo "Instrumented iteration $i: FAILED" >> "$result_file"
        fi
        
        rm -f /tmp/mpi_instrumented_$$
    done
    
    # Calculate statistics
    calculate_statistics "$result_file" uninstrumented_times instrumented_times
    
    print_success "Performance test completed. Results saved to: $result_file"
}

# Function to calculate and display statistics
calculate_statistics() {
    local result_file=$1
    shift
    local -n uninst_times=$1
    shift
    local -n inst_times=$1
    
    if [ ${#uninst_times[@]} -eq 0 ] || [ ${#inst_times[@]} -eq 0 ]; then
        print_error "No valid timing data collected"
        return 1
    fi
    
    # Calculate averages
    local uninst_sum=0
    local inst_sum=0
    
    for time in "${uninst_times[@]}"; do
        uninst_sum=$(echo "$uninst_sum + $time" | bc -l)
    done
    
    for time in "${inst_times[@]}"; do
        inst_sum=$(echo "$inst_sum + $time" | bc -l)
    done
    
    local uninst_avg=$(echo "scale=6; $uninst_sum / ${#uninst_times[@]}" | bc -l)
    local inst_avg=$(echo "scale=6; $inst_sum / ${#inst_times[@]}" | bc -l)
    
    # Calculate min/max
    local uninst_min=${uninst_times[0]}
    local uninst_max=${uninst_times[0]}
    local inst_min=${inst_times[0]}
    local inst_max=${inst_times[0]}
    
    for time in "${uninst_times[@]}"; do
        if (( $(echo "$time < $uninst_min" | bc -l) )); then
            uninst_min=$time
        fi
        if (( $(echo "$time > $uninst_max" | bc -l) )); then
            uninst_max=$time
        fi
    done
    
    for time in "${inst_times[@]}"; do
        if (( $(echo "$time < $inst_min" | bc -l) )); then
            inst_min=$time
        fi
        if (( $(echo "$time > $inst_max" | bc -l) )); then
            inst_max=$time
        fi
    done
    
    # Calculate overhead
    local overhead=$(echo "scale=2; ($inst_avg - $uninst_avg) / $uninst_avg * 100" | bc -l)
    local overhead_abs=$(echo "scale=6; $inst_avg - $uninst_avg" | bc -l)
    
    # Write statistics to file
    echo "" >> "$result_file"
    echo "Performance Statistics:" >> "$result_file"
    echo "======================" >> "$result_file"
    printf "Uninstrumented - Avg: %.6fs, Min: %.6fs, Max: %.6fs\n" $uninst_avg $uninst_min $uninst_max >> "$result_file"
    printf "Instrumented   - Avg: %.6fs, Min: %.6fs, Max: %.6fs\n" $inst_avg $inst_min $inst_max >> "$result_file"
    printf "Overhead       - Absolute: %.6fs, Relative: %.2f%%\n" $overhead_abs $overhead >> "$result_file"
    
    # Display results on console
    echo ""
    print_status "Performance Results:"
    echo -e "  ${YELLOW}Uninstrumented${NC} - Avg: ${uninst_avg}s, Min: ${uninst_min}s, Max: ${uninst_max}s"
    echo -e "  ${YELLOW}Instrumented${NC}   - Avg: ${inst_avg}s, Min: ${inst_min}s, Max: ${inst_max}s"
    
    # Color-code overhead based on magnitude
    if (( $(echo "$overhead < 5" | bc -l) )); then
        local overhead_color=$GREEN
    elif (( $(echo "$overhead < 20" | bc -l) )); then
        local overhead_color=$YELLOW
    else
        local overhead_color=$RED
    fi
    
    echo -e "  ${overhead_color}Overhead${NC}       - Absolute: ${overhead_abs}s, Relative: ${overhead}%"
    
    # Performance assessment
    echo ""
    if (( $(echo "$overhead < 5" | bc -l) )); then
        print_success "Low overhead - Excellent for production use"
    elif (( $(echo "$overhead < 20" | bc -l) )); then
        print_warning "Moderate overhead - Acceptable for development/testing"
    else
        print_warning "High overhead - Consider selective instrumentation"
    fi
}

# Function to run comprehensive benchmark suite
run_benchmark_suite() {
    local mpi_args="$1"
    
    print_header
    print_status "Running comprehensive benchmark suite"
    print_status "MPI arguments: $mpi_args"
    
    # List of programs to benchmark
    local programs=(
        "basic/hello_world"
        "basic/send_recv"
        "basic/broadcast"
        "collective/allreduce_patterns"
        "point_to_point/nonblocking_comm"
        "performance/bandwidth_test"
    )
    
    local total_programs=${#programs[@]}
    local completed=0
    
    for program in "${programs[@]}"; do
        ((completed++))
        print_status "[$completed/$total_programs] Testing $program"
        
        if check_program "$program"; then
            run_performance_test "$program" "$mpi_args" 3
        else
            print_warning "Skipping $program (not found)"
        fi
        
        echo ""
    done
    
    # Generate summary report
    generate_summary_report
}

# Function to generate summary report
generate_summary_report() {
    local summary_file="$RESULTS_DIR/summary_$(date +%Y%m%d_%H%M%S).txt"
    
    print_status "Generating summary report..."
    
    echo "MPI Usage Sanitizer Performance Summary" > "$summary_file"
    echo "Generated: $(date)" >> "$summary_file"
    echo "========================================" >> "$summary_file"
    echo "" >> "$summary_file"
    
    # Find all recent result files
    local result_files=($(find "$RESULTS_DIR" -name "*.txt" -not -name "summary_*" -mmin -60 | sort))
    
    if [ ${#result_files[@]} -eq 0 ]; then
        echo "No recent performance results found." >> "$summary_file"
        return
    fi
    
    echo "Program Performance Overview:" >> "$summary_file"
    echo "----------------------------" >> "$summary_file"
    
    for result_file in "${result_files[@]}"; do
        local program=$(grep "^Program:" "$result_file" | cut -d' ' -f2-)
        local overhead=$(grep "Overhead.*Relative:" "$result_file" | grep -o '[0-9.-]*%' | tail -1)
        
        if [ -n "$program" ] && [ -n "$overhead" ]; then
            printf "%-30s %s\n" "$program" "$overhead" >> "$summary_file"
        fi
    done
    
    echo "" >> "$summary_file"
    echo "Detailed results available in individual files:" >> "$summary_file"
    for result_file in "${result_files[@]}"; do
        echo "  $(basename "$result_file")" >> "$summary_file"
    done
    
    print_success "Summary report generated: $summary_file"
}

# Function to display usage
show_usage() {
    echo "Usage: $0 [OPTIONS] <program> [mpi_args...]"
    echo ""
    echo "OPTIONS:"
    echo "  -h, --help              Show this help message"
    echo "  -i, --iterations N      Number of test iterations (default: 5)"
    echo "  -s, --suite             Run comprehensive benchmark suite"
    echo ""
    echo "EXAMPLES:"
    echo "  $0 basic/hello_world -np 4"
    echo "  $0 -i 10 performance/bandwidth_test -np 2"
    echo "  $0 --suite -np 4"
    echo ""
    echo "AVAILABLE PROGRAMS:"
    if [ -d "$UNINSTRUMENTED_DIR" ]; then
        find "$UNINSTRUMENTED_DIR" -type f -executable | sed "s|$UNINSTRUMENTED_DIR/||" | sort | sed 's/^/  /'
    else
        echo "  (Run build_all.sh first to compile examples)"
    fi
}

# Main function
main() {
    local iterations=5
    local run_suite=false
    local program=""
    local mpi_args=""
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -i|--iterations)
                iterations="$2"
                shift 2
                ;;
            -s|--suite)
                run_suite=true
                shift
                ;;
            -*)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
            *)
                if [ -z "$program" ]; then
                    program="$1"
                else
                    mpi_args="$mpi_args $1"
                fi
                shift
                ;;
        esac
    done
    
    # Check prerequisites
    if ! command -v mpirun &> /dev/null; then
        print_error "mpirun not found. Please install MPI runtime."
        exit 1
    fi
    
    if ! command -v bc &> /dev/null; then
        print_error "bc (calculator) not found. Please install bc package."
        exit 1
    fi
    
    # Check if build directory exists
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Run build_all.sh first."
        exit 1
    fi
    
    # Run benchmark suite or single program test
    if [ "$run_suite" = true ]; then
        run_benchmark_suite "$mpi_args"
    elif [ -n "$program" ]; then
        if check_program "$program"; then
            print_header
            run_performance_test "$program" "$mpi_args" "$iterations"
        else
            exit 1
        fi
    else
        print_error "No program specified"
        show_usage
        exit 1
    fi
}

# Run main function
main "$@"