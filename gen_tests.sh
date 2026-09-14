#!/usr/bin/env bash
set -e

rm -rf tests/sec2 tests/sec3
mkdir -p tests/sec2 tests/sec3

echo ">>> Dang tao bo test Section 2 (du 2 dong First occurrence va Comparisons)..."

# Test 01: Sample test tu de
cat << 'EOF' > tests/sec2/test01_sample.in
8
2 4 4 4 6 8 10 12
4
EOF
cat << 'EOF' > tests/sec2/test01_sample.ans
First occurrence at index 1
Comparisons: 3
EOF

# Test 02: Mang rong (N = 0)
cat << 'EOF' > tests/sec2/test02_empty.in
0

5
EOF
cat << 'EOF' > tests/sec2/test02_empty.ans
Not Found
Comparisons: 0
EOF

# Test 03: Mang 1 phan tu (tim thay)
cat << 'EOF' > tests/sec2/test03_single_found.in
1
7
7
EOF
cat << 'EOF' > tests/sec2/test03_single_found.ans
First occurrence at index 0
Comparisons: 1
EOF

# Test 04: Mang 1 phan tu (khong tim thay)
cat << 'EOF' > tests/sec2/test04_single_notfound.in
1
7
3
EOF
cat << 'EOF' > tests/sec2/test04_single_notfound.ans
Not Found
Comparisons: 1
EOF

# Test 05: Key nho hon bien trai (arr[mid] luon > key -> thu hep ve trai)
cat << 'EOF' > tests/sec2/test05_out_left.in
5
10 20 30 40 50
5
EOF
cat << 'EOF' > tests/sec2/test05_out_left.ans
Not Found
Comparisons: 2
EOF

# Test 06: Key lon hon bien phai
cat << 'EOF' > tests/sec2/test06_out_right.in
5
10 20 30 40 50
60
EOF
cat << 'EOF' > tests/sec2/test06_out_right.ans
Not Found
Comparisons: 3
EOF

# Test 07: Key o giua cac gia tri nhung khong ton tai
cat << 'EOF' > tests/sec2/test07_between_notfound.in
6
2 5 8 12 16 23
10
EOF
cat << 'EOF' > tests/sec2/test07_between_notfound.ans
Not Found
Comparisons: 3
EOF

# Test 08: Key nam o dau mang (index 0)
cat << 'EOF' > tests/sec2/test08_first_elem.in
6
10 20 30 40 50 60
10
EOF
cat << 'EOF' > tests/sec2/test08_first_elem.ans
First occurrence at index 0
Comparisons: 2
EOF

# Test 09: Key nam o cuoi mang (index N-1)
cat << 'EOF' > tests/sec2/test09_last_elem.in
6
10 20 30 40 50 60
60
EOF
cat << 'EOF' > tests/sec2/test09_last_elem.ans
First occurrence at index 5
Comparisons: 3
EOF

# Test 10: Toan bo mang chua cung 1 gia tri
cat << 'EOF' > tests/sec2/test10_all_duplicates.in
7
5 5 5 5 5 5 5
5
EOF
cat << 'EOF' > tests/sec2/test10_all_duplicates.ans
First occurrence at index 0
Comparisons: 3
EOF

# Test 11: Mang co so am va so 0
cat << 'EOF' > tests/sec2/test11_negative.in
8
-20 -15 -5 -5 0 3 7 9
-5
EOF
cat << 'EOF' > tests/sec2/test11_negative.ans
First occurrence at index 2
Comparisons: 3
EOF

# Test 12: Stress test N = 100,000 phan tu
python3 - << 'EOF'
N = 100000
with open("tests/sec2/test12_large_100k.in", "w") as f:
    f.write(f"{N}\n")
    f.write(" ".join(str(i * 2) for i in range(N)) + "\n")
    f.write("150000\n")

# Voi N = 100,000 va key = 150000 (tai index 75000), so buoc binary search la 17
with open("tests/sec2/test12_large_100k.ans", "w") as f:
    f.write("First occurrence at index 75000\n")
    f.write("Comparisons: 17\n")
EOF


echo ">>> Dang tao bo test Section 3..."

# Test 01
cat << 'EOF' > tests/sec3/test01_sample.in
7
12 23 12 34 45 23 12
EOF
cat << 'EOF' > tests/sec3/test01_sample.ans
Slot [1]: 12 (Count: 3)
Slot [2]: 23 (Count: 2)
Slot [3]: 34 (Count: 1)
Slot [4]: 45 (Count: 1)
EOF

# Test 02
cat << 'EOF' > tests/sec3/test02_chain_collision.in
5
1 12 23 34 1
EOF
cat << 'EOF' > tests/sec3/test02_chain_collision.ans
Slot [1]: 1 (Count: 2)
Slot [2]: 12 (Count: 1)
Slot [3]: 23 (Count: 1)
Slot [4]: 34 (Count: 1)
EOF

# Test 03
cat << 'EOF' > tests/sec3/test03_wraparound.in
3
10 21 0
EOF
cat << 'EOF' > tests/sec3/test03_wraparound.ans
Slot [0]: 21 (Count: 1)
Slot [1]: 0 (Count: 1)
Slot [10]: 10 (Count: 1)
EOF

# Test 04
cat << 'EOF' > tests/sec3/test04_negative_keys.in
4
-1 -12 -1 5
EOF
cat << 'EOF' > tests/sec3/test04_negative_keys.ans
Slot [0]: -12 (Count: 1)
Slot [5]: 5 (Count: 1)
Slot [10]: -1 (Count: 2)
EOF

# Test 05
cat << 'EOF' > tests/sec3/test05_single_key_many_times.in
6
99 99 99 99 99 99
EOF
cat << 'EOF' > tests/sec3/test05_single_key_many_times.ans
Slot [0]: 99 (Count: 6)
EOF

# Test 06
cat << 'EOF' > tests/sec3/test06_dense_table.in
10
0 1 2 3 4 5 6 7 8 9
EOF
cat << 'EOF' > tests/sec3/test06_dense_table.ans
Slot [0]: 0 (Count: 1)
Slot [1]: 1 (Count: 1)
Slot [2]: 2 (Count: 1)
Slot [3]: 3 (Count: 1)
Slot [4]: 4 (Count: 1)
Slot [5]: 5 (Count: 1)
Slot [6]: 6 (Count: 1)
Slot [7]: 7 (Count: 1)
Slot [8]: 8 (Count: 1)
Slot [9]: 9 (Count: 1)
EOF

echo "==> Hoan thanh! Da cap nhat toan bo test voi format day du 2 dong."
