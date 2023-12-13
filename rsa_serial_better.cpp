#include <cstring>
#include <fstream>

#include "rsa.hpp"

//#include "src/bignumber.h"

using namespace std;

uint64_t public_key;
uint64_t private_key;
uint64_t n;

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

    @param primes care va referentia un array populat cu numere prime -> uint64_t
    @return size_prime – marimea array-ului de numere prime –> size_t
*/
size_t primefiller(uint64_t *primes) {
    // RIP Atkin's sieve (10-12-2023 -> 13-12-2023) 
    // uint64_t lim = sqrt(size_of_ciur);
    // uint64_t res;
    // size_t size_prime = 0;
    // uint8_t *ciur = (uint8_t *) malloc(size_of_ciur * sizeof(uint8_t) + 1);
    // memset(ciur, 0, size_of_ciur * sizeof(uint8_t) + 1);

    // ciur[0] = false;
    // ciur[1] = false;

    // // aplicam ciurul lui Atkin
    // for (uint64_t i = 1; i <= lim; ++i) {
    //     for (uint64_t j = 1; j <= lim; ++j) {
    //         res = 4 * i * i + j * j;
    //         if (res <= (uint64_t) size_of_ciur && (res % 12 == 1 || res % 12 == 5)) {
    //             ciur[res] = !ciur[res];
    //         }

    //         res = 3 * i * i +  j * j;
    //         if (res <= (uint64_t) size_of_ciur && res % 12 == 7) {
    //             ciur[res] = !ciur[res];
    //         }

    //         res = 3 * i * i - j * j;
    //         if (i > j && res <= (uint64_t) size_of_ciur && res % 12 == 11) {
    //             ciur[res] = !ciur[res];
    //         }
    //     }
    // }

    // // eliminam patratele perfecte
    // for (uint64_t i = 5; i <= lim; ++i) {
    //     if (ciur[i]) {
    //         uint64_t k = i * i;
    //         for (uint64_t j = k; j <= (uint64_t) size_of_ciur; j += k) {
    //             ciur[j] = false;
    //         }
    //     }
    // }
    // for (size_t i = 0; i < size_of_ciur; i++) {
    //     if (ciur[i]) {
    //         primes[size_prime] = i;
    //         ++size_prime;
    //     }
    // }
    // free(ciur);
    // return size_prime;

    size_t size_prime = 0;
    uint8_t *ciur = (uint8_t *) malloc(size_of_ciur * sizeof(uint8_t) + 1);
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
    @param pos retine pozitia anterioara pentru a pastra diferenta intre prime1 si prime2 -> uint64_t
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
    uint64_t result = 1;
    uint64_t copy_message = (uint64_t) message;
    while (e > 0) {
        if (e % 2 == 1) {
            result = (result * copy_message) % n;
        }
        e = e >> 1;
        copy_message = (copy_message * copy_message) % n;
    }
    return result % n;
}

/**
    Functie de decriptare a unui numar

    @param encrypted_text caracterul ce trebuie decriptat -> uint64_t
    @return decrypted – caracterul decriptat -> uint8_t
*/
uint8_t decrypt(uint64_t encrpyted_text) {
    uint64_t copy_private_key = private_key;
    uint64_t result = 1;
    while (copy_private_key > 0) {
        if (copy_private_key % 2 == 1) {
            result = (result * encrpyted_text) % n;
        }
        copy_private_key = copy_private_key >> 1;
        encrpyted_text = (encrpyted_text * encrpyted_text) % n;
    }
    return (uint8_t) result % n;
}

/**
   Convertirea unui string la char array

   @param str stringul de convertit (trimis ca char*)
   @return numbers – array-ul de numere
*/
uint64_t *stringToNumbersArray(char *str) {
    uint64_t *numbers = (uint64_t *)malloc(size_array * sizeof(uint64_t) + 1);
    memset(numbers, 0, size_array * sizeof(uint64_t) + 1);

    for (size_t i = 0; i < strlen(str); ++i) {
        numbers[i] = encrypt((uint64_t)(str[i]));
    }
    return numbers;
}

/**
   Convertirea unui char array la string

   @param numbers array-ul de numere -> uint64_t
   @param size marimea array-ului -> size_t
   @return str – textul decriptat -> char*
*/
char *numberArrayToString(uint64_t *numbers, size_t size) {
    char *str = (char *)malloc(size * sizeof(char) + 1);
    memset(str, 0, size * sizeof(char) + 1);

    for (size_t i = 0; i < size; ++i) {
        str[i] = decrypt(numbers[i]);
    }
    return str;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    char *file_in = (char *)malloc(100 * sizeof(char));
    strcpy(file_in, input);
    strcat(file_in, argv[1]);
    printf("File in: %s\n", file_in);
    uint64_t *primes = (uint64_t*) malloc(size_of_ciur * sizeof(uint64_t) + 1);
    memset(primes, 0, size_of_ciur * sizeof(uint64_t) + 1);
    
    size_t no_primes = primefiller(primes);
    
    setkeys(primes, no_primes);

    char *message = (char *)malloc(size_array * sizeof(char));
    FILE *fin = fopen(file_in, "r");
    fgets(message, size_array, fin);
    fclose(fin);
    int sizeOfMessage = strlen(message);

    uint64_t *numbers = stringToNumbersArray(message);
    char *file_out = (char *)malloc(100 * sizeof(char));
    strcpy(file_out, output);
    strcat(file_out, argv[1]);
    printf("File out: %s\n", file_out);
    FILE *fout = fopen(file_out, "w");
    
    fputs("Criptat: ", fout);
    for (int i = 0; i < sizeOfMessage; i++) {
        fprintf(fout, "%lu ", numbers[i]);
    }
    fputs("\n", fout);

    char *str = numberArrayToString(numbers, sizeOfMessage);
    
    fputs("Decriptat: ", fout);
    fputs(str, fout);
    fputs("\n", fout);
    
    fclose(fout);
    free(primes);
    free(numbers);
    free(str);

    return 0;
}
