#ifndef HASH_FUNCTION_HPP
#define HASH_FUNCTION_HPP
#include <cmath>
#include <cstring>

const int N = 1e6;

//Sort for prime number
bool prime[N];

//Check if the input is prime
bool isprime(int N) {
    if (N < 2) {
        return false;
    }
    for (int i = 2; i * i <= N; i++) {
        if (N % i == 0) {
            return false;
        }
    }
    return true;
}
//Find the biggest prime number that <= n
int prevPrime(int n) {
    // bro will this work 🐧
    int i = n;
    do {
        --i;
    } while (prime[i] == 0 && i >= 2);
    return i;
}

// prime number sieve which precheck prime numbers 🐧
void sieve() {
    std::memset(prime, 1, sizeof(prime));
    prime[0] = 0;
    prime[1] = 0;
    for (int i = 2; (long long)i * i < N; ++i) {
        if (prime[i] == 1) {
            for (int j = i * i; j < N; j += i) {
                prime[j] = false;
            }
        }
    }
}

int divisionHash(int key, int SIZE) {
    return key % prevPrime(SIZE);
}
#endif