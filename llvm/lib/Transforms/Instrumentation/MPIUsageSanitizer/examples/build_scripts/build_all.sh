#!/bin/bash

# Build script for MPI Usage Sanitizer examples
# This script compiles all example programs with and without instrumentation

set -e  # Exit on any error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$EXAMPLES_DIR/build"
INSTRUMENTED_DIR="$BUILD_DIR/instrumented"
UNINSTRUMENTED_DIR="$BUILD_DIR/uninstrumented"

# Compiler settings
MPICC=${MPICC:-mpicc}
MPICXX=${MPICXX:-mpicxx}
MPIFORT=${MPIFORT:-mpifort}

# LLVM settings for instrumentation
LLVM_DIR=${LLVM_DIR:-"$EXAMPLES_DIR/../../../.."}
CLANG=${CLANG:-"$LLVM_DIR/bin/clang"}
CLANGXX=${CLANGXX:-"$LLVM_DIR/bin/clang++"}
FLANG=${FLANG:-"$LLVM_DIR/bin/flang-new"}

# MPI Usage Sanitizer pass
MPI_SANITIZER_PASS="-fpass-plugin=$LLVM_DIR/lib/MPIUsageSanitizer.so"
MPI_SANITIZER_FLAGS="-fmpi-sanitizer -fmpi-sanitizer-level=full"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[BUILD]${NC} $1"
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

# Function to create build directories
setup_build_dirs() {
    print_status "Setting up build directories..."
    
    rm -rf "$BUILD_DIR"
    mkdir -p "$INSTRUMENTED_DIR"
    mkdir -p "$UNINSTRUMENTED_DIR"
    
    # Create subdirectories for different example categories
    for category in basic collective point_to_point error_cases correct performance multi_language; do
        mkdir -p "$INSTRUMENTED_DIR/$category"
        mkdir -p "$UNINSTRUMENTED_DIR/$category"
    done
    
    # Special handling for multi-language subdirectories
    mkdir -p "$INSTRUMENTED_DIR/multi_language/fortran_bindings"
    mkdir -p "$INSTRUMENTED_DIR/multi_language/cpp_bindings"
    mkdir -p "$UNINSTRUMENTED_DIR/multi_language/fortran_bindings"
    mkdir -p "$UNINSTRUMENTED_DIR/multi_language/cpp_bindings"
    
    print_success "Build directories created"
}

# Function to compile C examples
compile_c_examples() {
    local category=$1
    local source_dir="$EXAMPLES_DIR/$category"
    
    if [ ! -d "$source_dir" ]; then
        print_warning "Directory $source_dir not found, skipping"
        return
    fi
    
    print_status "Compiling C examples in $category..."
    
    for c_file in "$source_dir"/*.c; do
        if [ -f "$c_file" ]; then
            local basename=$(basename "$c_file" .c)
            
            # Compile without instrumentation
            print_status "  Compiling $basename (uninstrumented)..."
            if $MPICC -O2 -g -Wall "$c_file" -o "$UNINSTRUMENTED_DIR/$category/$basename"; then
                print_success "    $basename (uninstrumented) compiled successfully"
            else
                print_error "    Failed to compile $basename (uninstrumented)"
            fi
            
            # Compile with instrumentation
            print_status "  Compiling $basename (instrumented)..."
            if $CLANG -O2 -g -Wall $MPI_SANITIZER_PASS $MPI_SANITIZER_FLAGS \
               -I$(mpicc --showme:incdirs | tr ' ' '\n' | head -1) \
               $(mpicc --showme:libs | sed 's/-l/ -l/g') \
               "$c_file" -o "$INSTRUMENTED_DIR/$category/$basename"; then
                print_success "    $basename (instrumented) compiled successfully"
            else
                print_warning "    Failed to compile $basename (instrumented) - sanitizer may not be available"
                # Copy uninstrumented version as fallback
                cp "$UNINSTRUMENTED_DIR/$category/$basename" "$INSTRUMENTED_DIR/$category/$basename" 2>/dev/null || true
            fi
        fi
    done
}

# Function to compile C++ examples
compile_cpp_examples() {
    local category=$1
    local source_dir="$EXAMPLES_DIR/$category"
    
    if [ ! -d "$source_dir" ]; then
        print_warning "Directory $source_dir not found, skipping"
        return
    fi
    
    print_status "Compiling C++ examples in $category..."
    
    for cpp_file in "$source_dir"/*.cpp; do
        if [ -f "$cpp_file" ]; then
            local basename=$(basename "$cpp_file" .cpp)
            
            # Compile without instrumentation
            print_status "  Compiling $basename (uninstrumented)..."
            if $MPICXX -O2 -g -Wall -std=c++17 "$cpp_file" -o "$UNINSTRUMENTED_DIR/$category/$basename"; then
                print_success "    $basename (uninstrumented) compiled successfully"
            else
                print_error "    Failed to compile $basename (uninstrumented)"
            fi
            
            # Compile with instrumentation
            print_status "  Compiling $basename (instrumented)..."
            if $CLANGXX -O2 -g -Wall -std=c++17 $MPI_SANITIZER_PASS $MPI_SANITIZER_FLAGS \
               -I$(mpicxx --showme:incdirs | tr ' ' '\n' | head -1) \
               $(mpicxx --showme:libs | sed 's/-l/ -l/g') \
               "$cpp_file" -o "$INSTRUMENTED_DIR/$category/$basename"; then
                print_success "    $basename (instrumented) compiled successfully"
            else
                print_warning "    Failed to compile $basename (instrumented) - sanitizer may not be available"
                # Copy uninstrumented version as fallback
                cp "$UNINSTRUMENTED_DIR/$category/$basename" "$INSTRUMENTED_DIR/$category/$basename" 2>/dev/null || true
            fi
        fi
    done
}

# Function to compile Fortran examples
compile_fortran_examples() {
    local category=$1
    local source_dir="$EXAMPLES_DIR/$category"
    
    if [ ! -d "$source_dir" ]; then
        print_warning "Directory $source_dir not found, skipping"
        return
    fi
    
    print_status "Compiling Fortran examples in $category..."
    
    for f90_file in "$source_dir"/*.f90; do
        if [ -f "$f90_file" ]; then
            local basename=$(basename "$f90_file" .f90)
            
            # Compile without instrumentation
            print_status "  Compiling $basename (uninstrumented)..."
            if $MPIFORT -O2 -g "$f90_file" -o "$UNINSTRUMENTED_DIR/$category/$basename"; then
                print_success "    $basename (uninstrumented) compiled successfully"
            else
                print_error "    Failed to compile $basename (uninstrumented)"
            fi
            
            # Compile with instrumentation (if Flang supports the sanitizer)
            print_status "  Compiling $basename (instrumented)..."
            if [ -x "$FLANG" ]; then
                if $FLANG -O2 -g $MPI_SANITIZER_PASS $MPI_SANITIZER_FLAGS \
                   -I$(mpifort --showme:incdirs | tr ' ' '\n' | head -1) \
                   $(mpifort --showme:libs | sed 's/-l/ -l/g') \
                   "$f90_file" -o "$INSTRUMENTED_DIR/$category/$basename"; then
                    print_success "    $basename (instrumented) compiled successfully"
                else
                    print_warning "    Failed to compile $basename (instrumented) with Flang"
                    # Copy uninstrumented version as fallback
                    cp "$UNINSTRUMENTED_DIR/$category/$basename" "$INSTRUMENTED_DIR/$category/$basename" 2>/dev/null || true
                fi
            else
                print_warning "    Flang not available, copying uninstrumented version"
                cp "$UNINSTRUMENTED_DIR/$category/$basename" "$INSTRUMENTED_DIR/$category/$basename" 2>/dev/null || true
            fi
        fi
    done
}

# Function to create run scripts
create_run_scripts() {
    print_status "Creating run scripts..."
    
    # Create script to run uninstrumented examples
    cat > "$BUILD_DIR/run_uninstrumented.sh" << 'EOF'
#!/bin/bash
# Script to run uninstrumented MPI examples

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UNINSTRUMENTED_DIR="$SCRIPT_DIR/uninstrumented"

if [ $# -eq 0 ]; then
    echo "Usage: $0 <category/program> [mpi_args...]"
    echo "Example: $0 basic/hello_world -np 4"
    echo ""
    echo "Available programs:"
    find "$UNINSTRUMENTED_DIR" -type f -executable | sed "s|$UNINSTRUMENTED_DIR/||" | sort
    exit 1
fi

PROGRAM="$1"
shift

if [ ! -f "$UNINSTRUMENTED_DIR/$PROGRAM" ]; then
    echo "Error: Program $PROGRAM not found"
    exit 1
fi

echo "Running uninstrumented: $PROGRAM"
mpirun "$@" "$UNINSTRUMENTED_DIR/$PROGRAM"
EOF

    # Create script to run instrumented examples
    cat > "$BUILD_DIR/run_instrumented.sh" << 'EOF'
#!/bin/bash
# Script to run instrumented MPI examples

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTRUMENTED_DIR="$SCRIPT_DIR/instrumented"

if [ $# -eq 0 ]; then
    echo "Usage: $0 <category/program> [mpi_args...]"
    echo "Example: $0 basic/hello_world -np 4"
    echo ""
    echo "Available programs:"
    find "$INSTRUMENTED_DIR" -type f -executable | sed "s|$INSTRUMENTED_DIR/||" | sort
    exit 1
fi

PROGRAM="$1"
shift

if [ ! -f "$INSTRUMENTED_DIR/$PROGRAM" ]; then
    echo "Error: Program $PROGRAM not found"
    exit 1
fi

echo "Running instrumented: $PROGRAM"
echo "MPI Sanitizer output will be displayed below:"
echo "================================================"
mpirun "$@" "$INSTRUMENTED_DIR/$PROGRAM"
EOF

    chmod +x "$BUILD_DIR/run_uninstrumented.sh"
    chmod +x "$BUILD_DIR/run_instrumented.sh"
    
    print_success "Run scripts created"
}

# Function to display build summary
display_summary() {
    print_status "Build Summary:"
    
    echo ""
    echo "Uninstrumented binaries:"
    find "$UNINSTRUMENTED_DIR" -type f -executable | wc -l | xargs echo "  Total programs:"
    
    echo ""
    echo "Instrumented binaries:"
    find "$INSTRUMENTED_DIR" -type f -executable | wc -l | xargs echo "  Total programs:"
    
    echo ""
    echo "Usage:"
    echo "  Run uninstrumented: $BUILD_DIR/run_uninstrumented.sh <program> [mpi_args]"
    echo "  Run instrumented:   $BUILD_DIR/run_instrumented.sh <program> [mpi_args]"
    echo ""
    echo "Examples:"
    echo "  $BUILD_DIR/run_uninstrumented.sh basic/hello_world -np 4"
    echo "  $BUILD_DIR/run_instrumented.sh basic/hello_world -np 4"
    echo "  $BUILD_DIR/run_instrumented.sh performance/bandwidth_test -np 2"
    echo ""
    
    if [ -x "$BUILD_DIR/performance_comparison.sh" ]; then
        echo "Performance comparison:"
        echo "  $BUILD_DIR/performance_comparison.sh <program> [mpi_args]"
    fi
}

# Main build process
main() {
    print_status "Starting MPI Usage Sanitizer examples build..."
    
    # Check prerequisites
    if ! command -v mpicc &> /dev/null; then
        print_error "mpicc not found. Please install MPI development packages."
        exit 1
    fi
    
    # Setup build environment
    setup_build_dirs
    
    # Compile examples by category
    compile_c_examples "basic"
    compile_c_examples "collective" 
    compile_c_examples "point_to_point"
    compile_c_examples "error_cases"
    compile_c_examples "correct"
    compile_c_examples "performance"
    
    # Multi-language examples
    compile_cpp_examples "multi_language/cpp_bindings"
    compile_fortran_examples "multi_language/fortran_bindings"
    
    # Create utility scripts
    create_run_scripts
    
    # Copy performance comparison script if it exists
    if [ -f "$SCRIPT_DIR/performance_comparison.sh" ]; then
        cp "$SCRIPT_DIR/performance_comparison.sh" "$BUILD_DIR/"
        chmod +x "$BUILD_DIR/performance_comparison.sh"
    fi
    
    # Display summary
    display_summary
    
    print_success "Build completed successfully!"
}

# Run main function
main "$@"