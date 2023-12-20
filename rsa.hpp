// Description: Header file for all RSA files

#include <stdio.h>
#include <stdlib.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

const size_t size_of_ciur = 30000;
const uint64_t size_array = 40000005;

const char *input = "tests/input/";
const char *output = "tests/output/";

uint64_t public_key;
uint64_t private_key;
uint64_t n;

// For CUDA
int num_blocks = 65535;
const int num_threads = 1024;

const int seed = 0;

// All headers for functions from bellow
uint64_t gcd(uint64_t a, uint64_t h);
size_t primefiller(uint64_t *primes);
uint64_t pickrandomprime(uint64_t *primes, size_t no_primes, int64_t *pos);
void setkeys(uint64_t *primes, size_t no_primes);
uint64_t encrypt(uint8_t message);
uint8_t decrypt(uint64_t encrpyted_text);
uint64_t *stringToNumbersArray(char *str);
char *numberArrayToString(uint64_t *numbers, size_t size);
