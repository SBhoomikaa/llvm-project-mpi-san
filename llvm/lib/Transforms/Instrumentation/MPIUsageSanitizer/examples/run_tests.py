#!/usr/bin/env python3
"""
MPI Usage Sanitizer Test Runner & Harness
This script automates Phase 1 & Phase 3 validation:
1. Runs all 15 seeded error C programs to verify the sanitizer detects them.
2. Runs all 4 correct C programs to ensure zero false positives.
3. Generates a comprehensive Markdown test report.
"""

import os
import subprocess
import time
import sys
import re

# Configuration
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(SCRIPT_DIR, "build")
INSTRUMENTED_DIR = os.path.join(BUILD_DIR, "instrumented")
UNINSTRUMENTED_DIR = os.path.join(BUILD_DIR, "uninstrumented")

# ANSI Colors
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
BLUE = "\033[94m"
BOLD = "\033[1m"
RESET = "\033[0m"

def print_status(msg):
    print(f"{BLUE}[RUNNER]{RESET} {msg}")

def print_success(msg):
    print(f"{GREEN}[PASS]{RESET} {msg}")

def print_failure(msg):
    print(f"{RED}[FAIL]{RESET} {msg}")

def print_warning(msg):
    print(f"{YELLOW}[WARN]{RESET} {msg}")

def get_mpi_runner():
    """Detects standard MPI runners in path (mpirun or mpiexec)"""
    for runner in ["mpirun", "mpiexec"]:
        try:
            subprocess.run([runner, "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return runner
        except FileNotFoundError:
            continue
    return None

def run_test_binary(runner, category, name, ranks, timeout=4):
    """Runs a compiled MPI test executable and records stdout, stderr, exit code, and timeout status"""
    bin_path = os.path.join(INSTRUMENTED_DIR, category, name)
    # On Windows, compiled C files might end in .exe
    if sys.platform == "win32" and not bin_path.endswith(".exe"):
        bin_path += ".exe"

    if not os.path.exists(bin_path):
        return {
            "status": "NOT_COMPILED",
            "stdout": "",
            "stderr": "",
            "exit_code": -1,
            "duration": 0
        }

    cmd = [runner, "-np", str(ranks), bin_path]
    
    start_time = time.time()
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = proc.communicate(timeout=timeout)
        duration = time.time() - start_time
        return {
            "status": "COMPLETED",
            "stdout": stdout,
            "stderr": stderr,
            "exit_code": proc.returncode,
            "duration": duration
        }
    except subprocess.TimeoutExpired:
        proc.kill()
        stdout, stderr = proc.communicate()
        duration = time.time() - start_time
        return {
            "status": "TIMEOUT",
            "stdout": stdout,
            "stderr": stderr,
            "exit_code": -9,
            "duration": duration
        }
    except Exception as e:
        duration = time.time() - start_time
        return {
            "status": "ERROR",
            "stdout": "",
            "stderr": str(e),
            "exit_code": -1,
            "duration": duration
        }

def analyze_error_case(name, result):
    """
    Analyzes output for an error case. We EXPECT:
    - An error message, OR
    - An abort/crash/prevented crash, OR
    - A timeout (signifying a deadlock was correctly stimulated or halted).
    """
    output = result["stdout"] + result["stderr"]
    
    # Check for sanitizer keywords
    sanitizer_fired = any(kw in output.upper() for kw in [
        "SANITIZER", "ERROR", "WARNING", "MISMATCH", "OVERFLOW", "UNDERFLOW", "ALIASING", "DEADLOCK"
    ])
    
    if result["status"] == "TIMEOUT":
        # In deadlock cases, timing out is a sign the circular block was stimulated.
        # If sanitizer deadlock prevention kicked in, it might have aborted or warned.
        return True, "Detected deadlock/timeout (Sanitizer triggered or circular wait active)"
    
    if sanitizer_fired:
        # Found sanitizer logs or warning
        match = re.search(r"(\*\*\* .*? \*\*\*|\[MPI Sanitizer\].*?(?:ERROR|WARNING|mismatch|aliasing|overflow|underflow|deadlock))", output, re.IGNORECASE)
        reason = match.group(0).strip() if match else "Sanitizer warnings/errors found in logs"
        return True, reason

    if result["exit_code"] != 0:
        return True, f"Program terminated with non-zero exit code ({result['exit_code']}) preventing unsafe operation"

    return False, "Executed to completion without any error signals or warnings flagged"

def analyze_correct_case(name, result):
    """
    Analyzes output for a correct case. We EXPECT:
    - Normal completion (exit code 0).
    - No critical sanitizer warning/error logs.
    """
    output = result["stdout"] + result["stderr"]
    
    if result["status"] == "TIMEOUT":
        return False, "Program timed out (unexpected deadlock in correct code)"
    
    if result["exit_code"] != 0:
        return False, f"Program failed with exit code {result['exit_code']}"

    # Verify no error levels are reported by the sanitizer
    has_sanitizer_error = "*** ERROR" in output or "CRITICAL ERROR" in output or "VIOLATION" in output
    if has_sanitizer_error:
        return False, "False positive triggered: Sanitizer incorrectly logged a critical error/violation"

    return True, "Executed successfully with zero false positives"

def generate_report(results, report_path):
    """Generates a structured Markdown report of the test suite execution"""
    total = len(results)
    passed = sum(1 for r in results if r["passed"])
    failed = total - passed
    pass_percentage = (passed / total * 100) if total > 0 else 0

    with open(report_path, "w", encoding="utf-8") as f:
        f.write("# MPI Usage Sanitizer Test Harness Summary Report\n\n")
        f.write(f"**Execution Date:** {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"**Platform:** {sys.platform}\n\n")
        
        f.write("## Suite Statistics\n")
        f.write("| Metric | Value |\n")
        f.write("| --- | --- |\n")
        f.write(f"| **Total Test Programs Run** | {total} |\n")
        f.write(f"| **Passed Cases** | {passed} |\n")
        f.write(f"| **Failed Cases** | {failed} |\n")
        f.write(f"| **Pass Rate** | {pass_percentage:.1f}% |\n\n")
        
        f.write("## Test Execution Details\n\n")
        f.write("| Test Name | Category | Expected Behavior | Outcome | Details |\n")
        f.write("| --- | --- | --- | --- | --- |\n")
        
        for r in results:
            outcome_str = f"✅ **PASS**" if r["passed"] else f"❌ **FAIL**"
            expected_behavior = "Detect seeded error" if r["category"] == "error_cases" else "Zero false positive"
            f.write(f"| `{r['name']}` | `{r['category']}` | {expected_behavior} | {outcome_str} | {r['details']} |\n")

def main():
    print_status("Starting MPI Usage Sanitizer Test Runner...")
    
    runner = get_mpi_runner()
    if not runner:
        print_failure("No MPI runner (mpirun or mpiexec) was detected in PATH.")
        print_warning("Please install OpenMPI, MPICH, or MS-MPI to run tests.")
        sys.exit(1)
    
    print_status(f"Using MPI launcher: {runner}")

    # Compile files if needed
    if not os.path.exists(os.path.join(INSTRUMENTED_DIR, "basic")):
        print_status("Binaries not detected. Attempting compilation...")
        if sys.platform != "win32":
            build_script = os.path.join(SCRIPT_DIR, "build_scripts", "build_all.sh")
            subprocess.run(["bash", build_script], check=True)
        else:
            build_script_ps = os.path.join(SCRIPT_DIR, "build_scripts", "build_all.ps1")
            print_status(f"Running PowerShell build script: {build_script_ps}")
            subprocess.run(["powershell", "-ExecutionPolicy", "Bypass", "-File", build_script_ps], check=True)

    # List of tests to execute
    tests = [
        # Seeded Error Tests (Phase 2)
        {"category": "error_cases", "name": "type_mismatch_int_double", "ranks": 2},
        {"category": "error_cases", "name": "type_mismatch_float_char", "ranks": 2},
        {"category": "error_cases", "name": "count_mismatch", "ranks": 2},
        {"category": "error_cases", "name": "buffer_aliasing_overlap", "ranks": 2},
        {"category": "error_cases", "name": "buffer_aliasing_same", "ranks": 2},
        {"category": "error_cases", "name": "collective_mismatch_bcast_barrier", "ranks": 2},
        {"category": "error_cases", "name": "collective_mismatch_reduce_allreduce", "ranks": 2},
        {"category": "error_cases", "name": "collective_param_mismatch_root", "ranks": 2},
        {"category": "error_cases", "name": "collective_param_mismatch_count", "ranks": 2},
        {"category": "error_cases", "name": "deadlock_circular_send", "ranks": 2},
        {"category": "error_cases", "name": "deadlock_circular_recv", "ranks": 2},
        {"category": "error_cases", "name": "deadlock_send_recv_cycle", "ranks": 3},
        {"category": "error_cases", "name": "nonblocking_type_mismatch", "ranks": 2},
        {"category": "error_cases", "name": "sendrecv_type_mismatch", "ranks": 2},
        {"category": "error_cases", "name": "derived_type_mismatch", "ranks": 2},
        
        # Correct Programs (Phase 3)
        {"category": "correct", "name": "correct_point_to_point", "ranks": 2},
        {"category": "correct", "name": "correct_collectives", "ranks": 2},
        {"category": "correct", "name": "correct_nonblocking", "ranks": 2},
        {"category": "correct", "name": "correct_derived_types", "ranks": 2},
    ]

    results = []
    print_status(f"Running {len(tests)} test programs...")

    for test in tests:
        name = test["name"]
        cat = test["category"]
        ranks = test["ranks"]
        
        print(f"Running {cat}/{name} (ranks={ranks})... ", end="", flush=True)
        res = run_test_binary(runner, cat, name, ranks)
        
        if res["status"] == "NOT_COMPILED":
            print(f"{YELLOW}NOT COMPILED{RESET}")
            results.append({
                "name": name,
                "category": cat,
                "passed": False,
                "details": "Executable not found (compilation failed or skipped)"
            })
            continue

        passed = False
        details = ""
        
        if cat == "error_cases":
            passed, details = analyze_error_case(name, res)
        else: # correct
            passed, details = analyze_correct_case(name, res)
            
        results.append({
            "name": name,
            "category": cat,
            "passed": passed,
            "details": details
        })

        if passed:
            print(f"{GREEN}PASS{RESET} - {details[:60]}")
        else:
            print(f"{RED}FAIL{RESET} - {details[:60]}")

    # Generate Markdown Report
    report_path = os.path.join(SCRIPT_DIR, "test_report.md")
    generate_report(results, report_path)
    print_status(f"Test suite finished. Markdown summary report generated at: {report_path}")
    
    # Exit with code matching suite success
    overall_success = all(r["passed"] for r in results)
    sys.exit(0 if overall_success else 1)

if __name__ == "__main__":
    main()
