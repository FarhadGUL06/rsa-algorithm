# Week 1:

Am facut research pe tema aleasa
Am adaugat solutia seriala pentru aceasta
Am creat un checker pentru a verifica faptul ca implementarea seriala si viitoarele implementari functioneaza

# Week 2:

Am adaugat implementarea cu pthread si openMP
Am adaugat o suita de teste pentru a determina timpii de executare al fiecarui program pe inputuri de marimi diferite

For ./serial:
input00.txt: OK -> 0.78
input01.txt: OK -> 2.43
input02.txt: OK -> 2.60
input03.txt: OK -> 9.96
input04.txt: OK -> 22.26
input05.txt: OK -> 12.20
input06.txt: OK -> 65.00
input07.txt: OK -> 86.34
input08.txt: OK -> 131.88
input09.txt: OK -> 145.31
input10.txt: OK -> 338.98

For ./openmp:
input00.txt: OK -> 0.43
input01.txt: OK -> 0.58
input02.txt: OK -> 0.79
input03.txt: OK -> 0.83
input04.txt: OK -> 1.04
input05.txt: OK -> 1.66
input06.txt: OK -> 2.49
input07.txt: OK -> 4.68
input08.txt: OK -> 5.85
input09.txt: OK -> 21.59
input10.txt: OK -> 30.90

For run_pthread:
input00.txt: OK -> 0.91
input01.txt: OK -> 1.06
input02.txt: OK -> 2.39
input03.txt: OK -> 1.49
input04.txt: OK -> 0.27
input05.txt: OK -> 2.71
input06.txt: OK -> 2.15
input07.txt: OK -> 0.47
input08.txt: OK -> 10.99
input09.txt: OK -> 19.11
input10.txt: OK -> 59.82

# Week 3:

Am adaugat implementarea cu MPI 
Am rezolvat buguri legate de alocari de memorie (care incetineau timpii)
Am masurat timpul de executare al solutiei cu MPI:

input00.txt: OK -> 0.30 
input01.txt: OK -> 0.26 
input02.txt: OK -> 0.31 
input03.txt: OK -> 0.26 
input04.txt: OK -> 0.26 
input05.txt: OK -> 0.32 
input06.txt: OK -> 0.26 
input07.txt: OK -> 0.26 
input08.txt: OK -> 0.27 
input09.txt: OK -> 0.33 
input10.txt: OK -> 0.49 
Testing huge input input_huge.txt: OK -> 236.27

# Week 4:

De schimbat citirea de la stdin in citire din fisiere
Am adaugat implementarea cu OpenMP + MPI si CUDA
Am masurat timpul de executare al celor 2 solutii:
TO DO: de testat modificarea Ciurului lui Eratosterne -> Ciurul lui Atkin

# Week 5:
Dupa mai multe testari, am ajuns la concluzia ca implementarea ce foloseste 
ciurul lui Atkin [1] nu reprezinta o implementare mai eficienta decat cea ce 
foloseste ciurul lui Eratostene generarea numerelor prime, astfel am revenit
la rezolvarea initiala. Am venit totusi cu o optimizare pe functiile de 
encriptare si decriptare folosind o metoda de calculare a exponentului modular
in timp logaritmic. [2]


# Referinte

[1] https://en.wikipedia.org/wiki/Sieve_of_Atkin
[2] https://www.geeksforgeeks.org/modular-exponentiation-power-in-modular-arithmetic/