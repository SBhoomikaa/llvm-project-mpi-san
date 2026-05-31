# MPI Usage Sanitizer Test Harness Summary Report

**Execution Date:** 2026-05-31 13:23:15
**Platform:** win32

## Suite Statistics
| Metric | Value |
| --- | --- |
| **Total Test Programs Run** | 19 |
| **Passed Cases** | 15 |
| **Failed Cases** | 4 |
| **Pass Rate** | 78.9% |

## Test Execution Details

| Test Name | Category | Expected Behavior | Outcome | Details |
| --- | --- | --- | --- | --- |
| `type_mismatch_int_double` | `error_cases` | Detect seeded error | ❌ **FAIL** | Executed to completion without any error signals or warnings flagged |
| `type_mismatch_float_char` | `error_cases` | Detect seeded error | ✅ **PASS** | Sanitizer warnings/errors found in logs |
| `count_mismatch` | `error_cases` | Detect seeded error | ✅ **PASS** | Sanitizer warnings/errors found in logs |
| `buffer_aliasing_overlap` | `error_cases` | Detect seeded error | ❌ **FAIL** | Executed to completion without any error signals or warnings flagged |
| `buffer_aliasing_same` | `error_cases` | Detect seeded error | ✅ **PASS** | Sanitizer warnings/errors found in logs |
| `collective_mismatch_bcast_barrier` | `error_cases` | Detect seeded error | ✅ **PASS** | Detected deadlock/timeout (Sanitizer triggered or circular wait active) |
| `collective_mismatch_reduce_allreduce` | `error_cases` | Detect seeded error | ✅ **PASS** | Detected deadlock/timeout (Sanitizer triggered or circular wait active) |
| `collective_param_mismatch_root` | `error_cases` | Detect seeded error | ❌ **FAIL** | Executed to completion without any error signals or warnings flagged |
| `collective_param_mismatch_count` | `error_cases` | Detect seeded error | ✅ **PASS** | Sanitizer warnings/errors found in logs |
| `deadlock_circular_send` | `error_cases` | Detect seeded error | ✅ **PASS** | Detected deadlock/timeout (Sanitizer triggered or circular wait active) |
| `deadlock_circular_recv` | `error_cases` | Detect seeded error | ✅ **PASS** | Detected deadlock/timeout (Sanitizer triggered or circular wait active) |
| `deadlock_send_recv_cycle` | `error_cases` | Detect seeded error | ✅ **PASS** | Detected deadlock/timeout (Sanitizer triggered or circular wait active) |
| `nonblocking_type_mismatch` | `error_cases` | Detect seeded error | ✅ **PASS** | Sanitizer warnings/errors found in logs |
| `sendrecv_type_mismatch` | `error_cases` | Detect seeded error | ❌ **FAIL** | Executed to completion without any error signals or warnings flagged |
| `derived_type_mismatch` | `error_cases` | Detect seeded error | ✅ **PASS** | Sanitizer warnings/errors found in logs |
| `correct_point_to_point` | `correct` | Zero false positive | ✅ **PASS** | Executed successfully with zero false positives |
| `correct_collectives` | `correct` | Zero false positive | ✅ **PASS** | Executed successfully with zero false positives |
| `correct_nonblocking` | `correct` | Zero false positive | ✅ **PASS** | Executed successfully with zero false positives |
| `correct_derived_types` | `correct` | Zero false positive | ✅ **PASS** | Executed successfully with zero false positives |
