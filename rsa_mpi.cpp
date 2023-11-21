#include <cmath>
#include <cstring>
#include <iostream>

#include "mpi.h"

using namespace std;

const size_t size_of_ciur = 500;
const size_t size_array = 10000005;

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
    uint8_t *ciur = (uint8_t *)malloc(size_of_ciur * sizeof(uint8_t));
    memset(ciur, 1, size_of_ciur * sizeof(uint8_t));

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
   si prime2 -> uint8_t
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
void setkeys(uint64_t *primes, size_t no_primes, uint64_t &public_key,
             uint64_t &private_key, uint64_t &n) {
    uint64_t pos = -1;
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
uint64_t encrypt(uint8_t message, uint64_t public_key, uint64_t n, int rank) {
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
uint8_t decrypt(uint64_t encrpyted_text, uint64_t private_key, uint64_t n) {
    uint64_t d = private_key;
    uint64_t decrypted = 1;
    while (d > 0) {
        decrypted *= encrpyted_text;
        decrypted %= n;
        --d;
    }
    return (uint8_t)decrypted;
}

/**
   Convertirea unui string la char array

   @param str stringul de convertit (trimis ca char*)
   @return numbers – array-ul de numere
*/
uint64_t *stringToNumbersArray(char *str, int sizeOfMessage,
                               uint64_t public_key, uint64_t n, int rank) {
    uint64_t *numbers = (uint64_t *)malloc(sizeOfMessage * sizeof(uint64_t));
    memset(numbers, 0, sizeOfMessage * sizeof(uint64_t));
    for (int i = 0; i < sizeOfMessage; ++i) {
        // if (i == sizeOfMessage / 4) {
        //     printf("Rank %d: 25%% done\n", rank);
        // }
        // if (i == sizeOfMessage / 2) {
        //     printf("Rank %d: 50%% done\n", rank);
        // }
        // if (i == sizeOfMessage * 3 / 4) {
        //     printf("Rank %d: 75%% done\n", rank);
        // }
        numbers[i] = encrypt((uint64_t)(str[i]), public_key, n, rank);
    }
    return numbers;
}

/**
   Convertirea unui char array la string

   @param numbers array-ul de numere -> uint64_t
   @param size marimea array-ului -> size_t
   @return str – textul decriptat -> char*
*/
char *numberArrayToString(uint64_t *numbers, size_t size, uint64_t private_key,
                          uint64_t n) {
    char *str = (char *)malloc((size + 1) * sizeof(char));
    memset(str, 0, (size + 1) * sizeof(char));
    for (size_t i = 0; i < size; ++i) {
        str[i] = decrypt(numbers[i], private_key, n);
    }
    return str;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int sizeOfMessage;
    uint64_t public_key;
    uint64_t private_key;
    uint64_t n;

    char *message = (char *)malloc(size_array * sizeof(char));
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // process id
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // number of processes
    char *rezultat = (char *)malloc(size_array * sizeof(char));
    char *str;
    uint64_t *primes = (uint64_t *)malloc(size_of_ciur * sizeof(uint64_t));
    int send_size;
    if (rank == 0) {
        fgets(message, size_array, stdin);
        sizeOfMessage = strlen(message) + 1;
        srand(time(NULL));
        memset(primes, 0, size_of_ciur * sizeof(uint64_t));
        size_t no_primes = primefiller(primes);
        setkeys(primes, no_primes, public_key, private_key, n);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // mpi broadcast with public key, private key and n
    MPI_Bcast(&public_key, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&private_key, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int start_index = 0;
        int portion_size = sizeOfMessage / (size - 1);
        int remainder = sizeOfMessage % (size - 1);
        for (int i = 1; i < size; i++) {
            send_size = portion_size;
            if (i < remainder) {
                send_size++;
            }
            MPI_Send(message + start_index, send_size, MPI_CHAR, i, 0,
                     MPI_COMM_WORLD);
            start_index += send_size;
        }
    }
    if (rank != 0) {
        int recv_size;
        MPI_Status status;
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &recv_size);
        char *recv_message = (char *)malloc(recv_size * sizeof(char));
        MPI_Recv(recv_message, recv_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        uint64_t *numbers =
            stringToNumbersArray(recv_message, recv_size, public_key, n, rank);
        str = numberArrayToString(numbers, recv_size, private_key, n);
        // Send str back to root
        MPI_Send(str, recv_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        free(str);
    }
    if (rank == 0) {
        // Receive str from other processes
        int start_index = 0;
        int portion_size = sizeOfMessage / (size - 1);
        int remainder = sizeOfMessage % (size - 1);
        for (int i = 1; i < size; i++) {
            send_size = portion_size;
            if (i < remainder) {
                send_size++;
            }
            MPI_Recv(rezultat + start_index, send_size, MPI_CHAR, i, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            start_index += send_size;
        }
        sizeOfMessage = strlen(rezultat);
        printf("Decriptat: %s\n", rezultat);
    }
    MPI_Finalize();
    free(primes);
    free(message);
    free(rezultat);
    return 0;
}