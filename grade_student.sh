#!/usr/bin/env bash

STUDENT_DIR=$1

if [ -z "$STUDENT_DIR" ]; then
    echo "Usage: ./grade_student.sh <path_to_student_submission_dir>"
    echo "Example: ./grade_student.sh submissions/B21DCCN045"
    exit 1
fi

STUDENT_ID=$(basename "$STUDENT_DIR")
EVIDENCE_DIR="grading/evidence/${STUDENT_ID}"
mkdir -p "$EVIDENCE_DIR"
LOG_FILE="${EVIDENCE_DIR}/compile_run_log.txt"

exec > >(tee "$LOG_FILE") 2>&1

echo "=========================================================="
echo "GRADING LOG FOR: $STUDENT_ID"
echo "Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"
echo "=========================================================="

# ----------------------------------------------------------
# CHẤM SECTION 2: BINARY SEARCH (KIỂM TRA CẢ DÒNG 1 VÀ DÒNG 2)
# ----------------------------------------------------------
echo ""
echo "=========================================================="
echo ">>> [SECTION 2] Compiling and Testing Binary Search..."
echo "=========================================================="
SEC2_SRC="${STUDENT_DIR}/sec2.cpp"

if [ ! -f "$SEC2_SRC" ]; then
    echo "[-] FAIL: Khong tim thay file sec2.cpp"
else
    g++ -std=c++17 -Wall "$SEC2_SRC" -o sec2_bin 2> sec2_compile.log
    if [ $? -ne 0 ]; then
        echo "[-] FAIL: Loi bien dich sec2.cpp:"
        cat sec2_compile.log
    else
        echo "[+] Bien dich thanh cong (g++ -std=c++17 -Wall)"
        if [ -s sec2_compile.log ]; then
            echo "[!] Canh bao bien dich (Warnings):"
            cat sec2_compile.log
        fi

        SEC2_PASS=0
        SEC2_TOTAL=0

        for infile in tests/sec2/*.in; do
            SEC2_TOTAL=$((SEC2_TOTAL + 1))
            testname=$(basename "$infile" .in)
            ansfile="tests/sec2/${testname}.ans"
            
            set +e
            timeout 1s ./sec2_bin < "$infile" > "${testname}.out" 2>&1
            EXIT_CODE=$?
            set -e

            if [ $EXIT_CODE -eq 124 ]; then
                echo "  [FAIL] $testname: TIMEOUT (Qua 1s)"
                continue
            elif [ $EXIT_CODE -ne 0 ]; then
                echo "  [FAIL] $testname: RUNTIME ERROR (Exit code $EXIT_CODE)"
                continue
            fi

            # So khop ca 2 dong (First occurrence va Comparisons)
            if diff -w -B --strip-trailing-cr "${testname}.out" "$ansfile" > /dev/null; then
                echo "  [PASS] $testname"
                SEC2_PASS=$((SEC2_PASS + 1))
            else
                echo "  [FAIL] $testname"
                echo "      --- Ky vong ---"
                cat "$ansfile"
                echo "      --- Thuc te ---"
                cat "${testname}.out"
            fi
            rm -f "${testname}.out"
        done
        echo "=> Ket qua Section 2: $SEC2_PASS / $SEC2_TOTAL tests passed."
        rm -f sec2_bin sec2_compile.log
    fi
fi

# ----------------------------------------------------------
# CHẤM SECTION 3: HASH TABLE LINEAR PROBING
# ----------------------------------------------------------
echo ""
echo "=========================================================="
echo ">>> [SECTION 3] Compiling and Testing Hash Table..."
echo "=========================================================="
SEC3_SRC="${STUDENT_DIR}/sec3.cpp"

if [ ! -f "$SEC3_SRC" ]; then
    echo "[-] FAIL: Khong tim thay file sec3.cpp"
else
    g++ -std=c++17 -Wall "$SEC3_SRC" -o sec3_bin 2> sec3_compile.log
    if [ $? -ne 0 ]; then
        echo "[-] FAIL: Loi bien dich sec3.cpp:"
        cat sec3_compile.log
    else
        echo "[+] Bien dich thanh cong (g++ -std=c++17 -Wall)"
        if [ -s sec3_compile.log ]; then
            echo "[!] Canh bao bien dich (Warnings):"
            cat sec3_compile.log
        fi

        SEC3_PASS=0
        SEC3_TOTAL=0

        for infile in tests/sec3/*.in; do
            SEC3_TOTAL=$((SEC3_TOTAL + 1))
            testname=$(basename "$infile" .in)
            ansfile="tests/sec3/${testname}.ans"

            set +e
            timeout 1s ./sec3_bin < "$infile" > "${testname}.out" 2>&1
            EXIT_CODE=$?
            set -e

            if [ $EXIT_CODE -eq 124 ]; then
                echo "  [FAIL] $testname: TIMEOUT (Qua 1s)"
                continue
            elif [ $EXIT_CODE -ne 0 ]; then
                echo "  [FAIL] $testname: RUNTIME ERROR (Exit code $EXIT_CODE)"
                continue
            fi

            if diff -w -B --strip-trailing-cr "${testname}.out" "$ansfile" > /dev/null; then
                echo "  [PASS] $testname"
                SEC3_PASS=$((SEC3_PASS + 1))
            else
                echo "  [FAIL] $testname"
                echo "      --- Ky vong ---"
                cat "$ansfile"
                echo "      --- Thuc te ---"
                cat "${testname}.out"
            fi
            rm -f "${testname}.out"
        done
        echo "=> Ket qua Section 3: $SEC3_PASS / $SEC3_TOTAL tests passed."
        rm -f sec3_bin sec3_compile.log
    fi
fi

echo ""
echo "=========================================================="
echo "Log luu tai: $LOG_FILE"
echo "=========================================================="
