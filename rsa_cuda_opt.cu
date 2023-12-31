#include <cuda.h>
#include <cuda_runtime_api.h>

#include "rsa.hpp"

using namespace std;

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
    uint64_t prime1 = pickrandomprime(primes, no_primes, &pos);
    uint64_t prime2 = pickrandomprime(primes, no_primes, &pos);

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
    Functie de decriptare paralela a unui numar

    @param encrypted_text caracterul ce trebuie decriptat -> uint64_t
    @return decrypted – caracterul decriptat -> uint8_t
*/
__global__ void parallel_decrypt(char *d_str, uint64_t *d_numbers, size_t size,
                                 uint64_t *d_private_key, uint64_t *d_n) {
    unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index > size) {
        // Indice mai mare decat lungimea vectorului de inserat
        return;
    }
    uint64_t encrpyted_text = d_numbers[index];

    uint64_t copy_private_key = *d_private_key;
    uint64_t result = 1;
    while (copy_private_key > 0) {
        if (copy_private_key & 1) {
            result = (result * encrpyted_text) % *d_n;
        }
        copy_private_key = copy_private_key >> 1;
        encrpyted_text = (encrpyted_text * encrpyted_text) % *d_n;
    }
    d_str[index] = (uint8_t)result % *d_n;
}

/**
    Functie de encriptare a unui caracter

    @param message caracterul ce trebuie encriptat -> uint8_t
    @return encrpyted_text – caracterul encriptat -> uint64_t
*/
__global__ void parallel_encrypt(uint64_t *d_numbers, char *d_str, size_t size,
                                 uint64_t *d_public_key, uint64_t *d_n) {
    unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index > size) {
        // Indice mai mare decat lungimea vectorului de inserat
        return;
    }
    uint64_t e = *d_public_key;

    uint64_t result = 1;
    uint64_t copy_message = (uint64_t)d_str[index];
    while (e > 0) {
        if (e & 1) {
            result = (result * copy_message) % *d_n;
        }
        e = e >> 1;
        copy_message = (copy_message * copy_message) % *d_n;
    }
    d_numbers[index] = result % *d_n;
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

    /*
    Old function:
    for (size_t i = 0; i < strlen(*h_str); ++i) {
        numbers[i] = encrypt((uint64_t)(*h_str[i]));
    }
    */
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

    /*
    Old function:
    for (size_t i = 0; i < size; ++i) {
        h_str[i] = decrypt(*h_numbers[i]);
    }
    */

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

int main(int argc, char *argv[]) {
    fflush(stdin);
    fflush(stdout);
    char *file_in = (char *)malloc(100 * sizeof(char));
    strcpy(file_in, input);
    strcat(file_in, argv[1]);
    // printf("File in: %s\n", file_in);

    srand(seed);

    uint64_t *primes = (uint64_t *)malloc(size_of_ciur * sizeof(uint64_t));
    memset(primes, 0, size_of_ciur * sizeof(uint64_t));

    size_t no_primes = primefiller(primes);

    setkeys(primes, no_primes);

    char *message = (char *)malloc(size_array * sizeof(char));

    FILE *fin = fopen(file_in, "r");

    char *ret = fgets(message, size_array, fin);
    if (ret == NULL) {
        printf("Error reading file\n");
        return -1;
    }

    fclose(fin);

    int sizeOfMessage = strlen(message) + 1;

    uint64_t *numbers = stringToNumbersArray(&message);
    char *file_out = (char *)malloc(100 * sizeof(char));
    strcpy(file_out, output);
    strcat(file_out, argv[1]);
    // printf("File out: %s\n", file_out);
    FILE *fout = fopen(file_out, "w");

    fputs("Criptat: ", fout);
    for (int i = 0; i < sizeOfMessage; i++) {
        fprintf(fout, "%lu ", numbers[i]);
    }
    fputs("\n", fout);

    char *str = numberArrayToString(&numbers, sizeOfMessage);

    fputs("Decriptat: ", fout);
    fputs(str, fout);
    fputs("\n", fout);

    fclose(fout);
    free(primes);
    free(numbers);
    free(str);

    return 0;
}
