#include <cmath>
#include <cstring>
#include <iostream>

using namespace std;

uint64_t public_key;
uint64_t private_key;
uint64_t n;



const size_t size_of_ciur = 500;
const uint64_t size_array = 1000000000;

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
    size_t size_prime = 0;
    uint8_t *ciur = (uint8_t *) malloc(size_of_ciur * sizeof(uint8_t) + 1);
    memset(ciur, 1, size_of_ciur * sizeof(uint8_t));

    ciur[0] = false;
    ciur[1] = false;
    
    size_t i, j;
    #pragma omp parallel for private(i)
    for (i = 2; i < size_of_ciur; i++) {
        #pragma omp parallel for private(j)
        for (j = i * 2; j < size_of_ciur; j += i) {
            ciur[j] = false;
        }
    }
    
    #pragma omp parallel for private(i) 
    for (i = 0; i < size_of_ciur; i++) {
        if (ciur[i]) {
            primes[size_prime] = i;
            ++size_prime;
        }
    }
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
    return (uint8_t) decrypted;
}

/**
   Convertirea unui string la char array

   @param str stringul de convertit (trimis ca char*)
   @return numbers – array-ul de numere
*/
uint64_t *stringToNumbersArray(char *str) {
    size_t size = strlen(str);
    uint64_t *numbers = (uint64_t *)malloc(size * sizeof(uint64_t) + 1);
    memset(numbers, 0, size * sizeof(uint64_t) + 1);
    size_t i;
    #pragma omp parallel for private(i) shared(numbers, str)
    for (i = 0; i < strlen(str); ++i) {
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
    size_t i;
    #pragma omp parallel for private(i) shared(numbers, str)
    for (i = 0; i < size; ++i) {
        str[i] = decrypt(numbers[i]);
    }
    return str;
}

int main() {
    srand(time(NULL));
    
    uint64_t *primes = (uint64_t*) malloc(size_array * sizeof(uint64_t));
    memset(primes, 0, size_of_ciur * sizeof(uint64_t));
    
    size_t no_primes;

    char *message = (char *)malloc(size_array * sizeof(char));
    int sizeOfMessage;
    
    #pragma omp parallel 
    {
        # pragma omp single
        {
            #pragma omp task
            {
                no_primes = primefiller(primes);
                setkeys(primes, no_primes);
            }

            #pragma omp task
            {
                fgets(message, size_array, stdin);
                sizeOfMessage = strlen(message);
            }
        }
        
    }
    uint64_t *numbers = stringToNumbersArray(message);

    printf("Criptat: ");
    for (int i = 0; i < sizeOfMessage; i++) {
        printf("%lu ", numbers[i]);
    }
    printf("\n");

    char *str = numberArrayToString(numbers, sizeOfMessage);
    printf("Decriptat: %s\n", str);
    
    return 0;
}
