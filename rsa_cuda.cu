#include <cuda.h>
#include <cuda_runtime_api.h>

#include <iostream>

using namespace std;

uint64_t public_key;
uint64_t private_key;
uint64_t n;

int num_blocks = 65535;
const int num_threads = 1024;

const size_t size_of_ciur = 500;
const uint64_t size_array = 10000005;

// All headers for functions from bellow

uint64_t gcd(uint64_t a, uint64_t h);
size_t primefiller(uint64_t *primes);
uint64_t pickrandomprime(uint64_t *primes, size_t no_primes, int64_t *pos);
void setkeys(uint64_t *primes, size_t no_primes);
uint64_t encrypt(uint8_t message);
uint8_t decrypt(uint64_t encrpyted_text);
uint64_t *stringToNumbersArray(char *str);
char *numberArrayToString(uint64_t *numbers, size_t size);

/**
    Functie care calculeaza cel mai mare divizor comun
    pentru 2 numere intregi

    @param a primul paremetrul -> uint64_t
    @param h al doilea parametru -> uint64_t
    @return h – cel mai mare divizor comun -> uint64_t
*/
uint64_t gcd(uint64_t a, uint64_t h) {
    uint64_t temp;
    while (1) {
        temp = a % h;
        if (temp == 0) return h;
        a = h;
        h = temp;
    }
}

/**
    Functia care construieste ciurul lui Eratosthenes

    @param primes care va referentia un array populat cu numere prime ->
   uint64_t
    @return size_prime – marimea array-ului de numere prime –> size_t
*/
size_t primefiller(uint64_t *primes) {
    size_t size_prime = 0;
    uint8_t *ciur = (uint8_t *)malloc(size_of_ciur * sizeof(uint8_t) + 1);
    memset(ciur, 1, size_of_ciur * sizeof(uint8_t) + 1);

    ciur[0] = false;
    ciur[1] = false;
    for (size_t i = 2; i < size_of_ciur; i++) {
        for (size_t j = i * 2; j < size_of_ciur; j += i) {
            ciur[j] = false;
        }
    }
    for (size_t i = 0; i < size_of_ciur; i++) {
        if (ciur[i]) {
            primes[size_prime] = i;
            ++size_prime;
        }
    }
    free(ciur);
    return size_prime;
}

/**
    Functie care alege un numar random prim

    @param primes array-ul de numere prime -> uint64_t
    @param no_primes numarul de numere prime -> size_t
    @param pos retine pozitia anterioara pentru a pastra diferenta intre prime1
   si prime2 -> uint64_t
    @return primes[k] – numarul prim de la pozitia k -> uint64_t
*/
uint64_t pickrandomprime(uint64_t *primes, size_t no_primes, uint64_t *pos) {
    uint64_t k = rand() % no_primes;
    while (k == *pos) {
        k = rand() % no_primes;
    }
    *pos = k;
    return primes[k];
}

/**
    Functie care construieste cheia publica si
    cheia privata pornind de la 2 numere prime
    generate folosind functia `pickrandomprime`

    @param primes array-ul de numere prime -> uint64_t
    @param no_primes numarul de numere prime -> size_t
*/
void setkeys(uint64_t *primes, size_t no_primes) {
    uint64_t pos = 0;
    uint64_t prime1 = pickrandomprime(primes, no_primes, &pos);  // 17291
    uint64_t prime2 = pickrandomprime(primes, no_primes, &pos);  // 64817

    n = prime1 * prime2;

    uint64_t phi = (prime1 - 1) * (prime2 - 1);

    uint64_t e = 2;

    while (1) {
        if (gcd(e, phi) == 1) {
            break;
        }
        e++;
    }

    public_key = e;
    int d = 2;

    while (1) {
        if ((d * e) % phi == 1) {
            break;
        }
        d++;
    }

    private_key = d;

    cout << "Public key: " << public_key << endl;
    cout << "Private key: " << private_key << endl;
}

/**
    Functie de encriptare a unui caracter

    @param message caracterul ce trebuie encriptat -> uint8_t
    @return encrpyted_text – caracterul encriptat -> uint64_t
*/
uint64_t encrypt(uint8_t message) {
    uint64_t e = public_key;
    uint64_t encrpyted_text = 1;
    while (e > 0) {
        encrpyted_text *= message;
        encrpyted_text %= n;
        --e;
    }
    return encrpyted_text;
}

/**
    Functie de decriptare a unui numar

    @param encrypted_text caracterul ce trebuie decriptat -> uint64_t
    @return decrypted – caracterul decriptat -> uint8_t
*/
uint8_t decrypt(uint64_t encrpyted_text) {
    uint64_t d = private_key;
    uint64_t decrypted = 1;

    while (d > 0) {
        decrypted *= encrpyted_text;
        decrypted %= n;
        --d;
    }
    return (uint8_t)decrypted;
}

__global__ void parallel_decrypt(char *d_str, uint64_t *d_numbers, size_t size,
                                 uint64_t *d_private_key, uint64_t *d_n) {
    unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index > size) {
        // Indice mai mare decat lungimea vectorului de inserat
        return;
    }
    // d_str[index] = decrypt(d_numbers[index]);
    uint64_t encrpyted_text = d_numbers[index];

    uint64_t d = *d_private_key;
    // printf("Cheie priv: %ld\n", d);
    uint64_t decrypted = 1;
    while (d > 0) {
        decrypted *= encrpyted_text;
        decrypted %= *d_n;
        --d;
    }
    d_str[index] = decrypted;
}

__global__ void parallel_encrypt(uint64_t *d_numbers, char *d_str, size_t size,
                                 uint64_t *d_public_key, uint64_t *d_n) {
    unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index > size) {
        // Indice mai mare decat lungimea vectorului de inserat
        return;
    }
    uint64_t e = *d_public_key;
    uint64_t encrpyted_text = 1;
    while (e > 0) {
        encrpyted_text *= d_str[index];
        encrpyted_text %= *d_n;
        --e;
    }
    d_numbers[index] = encrpyted_text;
}

/**
   Convertirea unui string la char array

   @param str stringul de convertit (trimis ca char*)
   @return numbers – array-ul de numere
*/
uint64_t *stringToNumbersArray(char **h_str) {
    char *d_str;
    uint64_t size = strlen(*h_str) + 1;
    cudaMalloc((void **)&d_str, size * sizeof(char));
    cudaMemset(d_str, 0, size * sizeof(char));
    cudaMemcpy(d_str, *h_str, size * sizeof(char), cudaMemcpyHostToDevice);

    uint64_t *d_numbers;
    cudaMalloc((void **)&d_numbers, size * sizeof(uint64_t));
    cudaMemset(d_numbers, 0, size * sizeof(uint64_t));

    /*for (size_t i = 0; i < strlen(*h_str); ++i) {
        numbers[i] = encrypt((uint64_t)(*h_str[i]));
    }*/
    uint64_t *d_public_key;
    uint64_t *d_n;
    cudaMalloc((void **)&d_public_key, sizeof(uint64_t));
    cudaMalloc((void **)&d_n, sizeof(uint64_t));
    cudaMemcpy(d_public_key, &public_key, sizeof(uint64_t),
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_n, &n, sizeof(uint64_t), cudaMemcpyHostToDevice);

    // Execute on gpu encrypt
    parallel_encrypt<<<num_blocks, num_threads>>>(
        d_numbers, d_str, strlen(*h_str), d_public_key, d_n);
    cudaDeviceSynchronize();

    uint64_t *h_numbers = (uint64_t *)malloc(size * sizeof(uint64_t));
    memset(h_numbers, 0, size * sizeof(uint64_t));
    cudaMemcpy(h_numbers, d_numbers, size * sizeof(uint64_t),
               cudaMemcpyDeviceToHost);
    cudaFree(d_numbers);
    cudaFree(d_str);
    return h_numbers;
}

/**
   Convertirea unui char array la string

   @param numbers array-ul de numere -> uint64_t
   @param size marimea array-ului -> size_t
   @return str – textul decriptat -> char*
*/
char *numberArrayToString(uint64_t **h_numbers, size_t size) {
    // Copy numbers to GPU
    uint64_t *d_numbers;
    cudaMalloc((void **)&d_numbers, size * sizeof(uint64_t));
    cudaMemset(d_numbers, 0, size * sizeof(uint64_t));
    cudaMemcpy(d_numbers, *h_numbers, size * sizeof(uint64_t),
               cudaMemcpyHostToDevice);

    // Initialise d_str
    char *d_str;
    cudaMalloc((void **)&d_str, size * sizeof(char));
    cudaMemset(d_str, 0, size * sizeof(char));

    /*for (size_t i = 0; i < size; ++i) {
        h_str[i] = decrypt(*h_numbers[i]);
    }*/

    // Copy data for decrypt
    uint64_t *d_private_key;
    uint64_t *d_n;
    cudaMalloc((void **)&d_private_key, sizeof(uint64_t));
    cudaMalloc((void **)&d_n, sizeof(uint64_t));

    cudaMemcpy(d_private_key, &private_key, sizeof(uint64_t),
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_n, &n, sizeof(uint64_t), cudaMemcpyHostToDevice);

    // Execute on gpu decrypt
    parallel_decrypt<<<num_blocks, num_threads>>>(d_str, d_numbers, size,
                                                  d_private_key, d_n);
    cudaDeviceSynchronize();

    // Copy str from device to host
    char *h_str = (char *)malloc(size * sizeof(char));
    memset(h_str, 0, size * sizeof(char));
    cudaMemcpy(h_str, d_str, size * sizeof(char), cudaMemcpyDeviceToHost);
    cudaFree(d_str);
    cudaFree(d_numbers);
    return h_str;
}

int main() {
    fflush(stdin);
    fflush(stdout);
    
    srand(time(NULL));

    uint64_t *primes = (uint64_t *)malloc(size_of_ciur * sizeof(uint64_t));
    memset(primes, 0, size_of_ciur * sizeof(uint64_t));

    size_t no_primes = primefiller(primes);

    setkeys(primes, no_primes);

    char *message = (char *)malloc(size_array * sizeof(char));
    char *p = fgets(message, size_array, stdin);

    if (p == NULL) {
        return -1;
    }

    int sizeOfMessage = strlen(message) + 1;

    uint64_t *numbers = stringToNumbersArray(&message);
    printf("Criptat: ");
    for (int i = 0; i < sizeOfMessage; i++) {
        printf("%lu ", numbers[i]);
    }
    printf("\n");

    char *str = numberArrayToString(&numbers, sizeOfMessage);

    printf("Decriptat: %s\n", str);

    free(primes);
    free(numbers);
    free(str);

    return 0;
}
