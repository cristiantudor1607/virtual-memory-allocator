**Nume: Tudor Cristian-Andrei**

**Grupa: 311CAa**

## Virtual Memory Allocator - Tema 1

> ### Functionalitatea programului
> La rulare, se introduc de la tastatura string-uri, pana la apasarea enter-ului, care sunt prelucrate pentru a gasi un **keyword** , ce reprezinta comanda. Daca keyword-ul nu reprezinta nicio comanda valida, atunci se printeaza un mesaj pentru fiecare string din intreaga comanda (considerand ca sirurile sunt separate prin spatii), si nu se intampla nimic la nivelul memoriei. Prima comanda ce trebuie data este **ALLOC_ARENA**, care rezerva **virtual** un numar de octeti din memoria calculatorului. Dupa alocarea arenei se pot face diferite operatii, precum **ALLOC_BLOCK**, **FREE_BLOCK**, **READ**, **WRITE**, etc. Prin comanda **ALLOC_BLOCK**, se aloca efectiv memoria reala, iar prin comanda **FREE_BLOCK**, se elibereaza aceea memorie. Programul se ocupa de gestionarea blocurilor de memorie, astfel incat sa respecte regulile din cerinta, adica organizarea lor in structura de lista in lista. Cu **WRITE** si **READ** sunt folosite efectiv zonele, putand sa scrii sau sa citesti intr-o sau dintr-o zona alocata. <br>
Comanda aditionala **MPROTECT** schimba permisiunile unui miniblock, care poate avea intre 0 si 3 permisiuni, *READ*, *WRITE* si *EXECUTE*.

<br>

### Impartirea codului in fisiere

Am impartit codul in mai multe fisiere sursa, care poate sunt greu de urmarit, dar fiecare are propriul scop:

1. <font color=#e8702a> main.c </font> <br> main.c este foarte sumar, contine doar main-ul si un define cu un mesaj care se afiseaza in cazurile in care alocarea memoriei da fail. Main-ul contine doar un loop din care se iese doar la **DEALLOC_ARENA** sau in cazul oricarui fail de alocare, care se testeaza cu variabila **memups**.
2. <font color=#e8702a> funcs.c / funcs.h </font> <br> Toate functiile din funcs.c sunt functii care extrag paramterii din comanda, necesari pentru a apela functiile din scheletul vma.c, si in majoritatea cazurilor, tot ele fac verificari inainte de apela o functie care face o modificare la nivelul memoriei. 
Practic, continutul acestor functii ar fi putut fi pus direct in main, dar nu ar mai fi fost asa compact.
3. <font color=#e8702a> text_utils.c / text_utils.h </font> <br> Acest header contine toate functiile de parsare a textului, precum functia **read_string**, care citeste un string caracter cu caracter si il aloca dinamic, functia **get_nth_arg** care returneaza un argument sub forma de string (sunt transformate in ce tip de date am nevoie in funcs.c), functia **check_validity**, care verifica daca numarul de argumente este corect, etc (am adaugat la fiecare functie comentarii in cod).
4. <font color=#e8702a> structures.h </font> <br> structures.h este header-ul cu toate structurile folosite in program, inclusiv cele din schelet, precum si cele definite de mine, **address_t** (structura care retine o pereche de 2 noduri (block, miniblock), specifice unei adrese date), structura **permission_t** (care este folosita pentru a separa permisiunile date ca un int8_t), cat si structurile **node_t** si **list_t**. Cu explicatii despre un camp "ciudat" din structura de arena adaugat, revin la ultima parte din acest document.
5. <font color=#e8702a> DLL.c / DLL.h </font> <br> (DLL vine de la Doubly Linked List, sper sa fiu iertat pentru acesta prescurtare :))) ) <br> DLL.c contine implementare functiilor pentru listele dublu inlantuite. Merita sa precizez ca e un mix intre functiile de la laborator si ceva idei de ale mele. Pe langa functiile clasice, am mai adaugat o functie **mergelists** care, asa cum zice numele, ~~face merge intre 2 liste~~ ~~lipeste 2 liste~~ leaga a doua lista la finalul primei. Tot aici am declarat si cele 2 functii specifice de free.
6. <font color=#e87021> vma.c / vma.h </font> <br> Este "fisierul principal" care contine functiile date in schelet cat si toate functiile care tin de orice inseamna **memorie**, **modificarea listei de blockuri sau a celor de miniblockuri**, **cautarea unei adrese prin lista pentru free/mprotect sau read/write**, **scrierea in buffer**, **citirea din buffer**, etc.

### Explicatii asupra codului

> observatie: in aceasta rubrica nu voi vorbi deloc despre programarea defensiva, insa voi reveni la acest aspect mai jos 

> **ALLOC_ARENA** <br> Principiul este unul simplu, la apelarea functiei, se aloca arena, se initializeaza campurile ei, si se creaza lista de miniblock-uri cu functia de create din DLL.c, asemanatoare cu cea de la laborator, modificata astfel incat sa primeasca si un pointer la o functie de free specifica.

> **DEALLOC_ARENA** <br> Pentru ca fiecare lista are functia specifica de free, este de ajuns sa dau **free_list(&arena->alloc_list)**, iar functia **free_list** se va ocupa de tot. A doua linie din urmatorul for, apeleaza pentru fiecare block, functia care elibereaza un block, care la randul ei, o sa apeleze **free_list**, pentru ca data-ul dintr-un nod care contine un block, va fi tot o lista. Abia la "nivelul cel mai de jos", se apeleaza functia de free pentru un miniblock, care sterge rw_buffer-ul si miniblock-ul.
```c
for (uint64_t i = 0; i < n; i++) { 
		curr = remove_nthnode(*list, 0);
		(*list)->free_func(curr->data);
		free(curr);
	}
```

> **ALLOC_BLOCK** <br> Pentru alocarea unui block am 4 cazuri: <br> &emsp; 1. Cand noul block nu are alte blockuri adiacente, aloc nu nou block, si creez lista de miniblock-uri in care pun un singur miniblock. Caut in lista din arena unde trebuie adaugat, si il pun in lista. <br> &emsp; 2. Cand noul block are adiacent la dreapta. Aloc un nou miniblock si il adaug in lista de miniblockuri pe pozitia 0. <br> &emsp; 3. Cand noul block are adiacent la stanga. Aloc un nou miniblock si il adaug in lista de miniblockuri pe ultima pozitie. <br> &emsp; 4. Cand noul block are in ambele parti blocuri adiacente. Aloc un nou block, il adaug pe prima pozitia in block-ul din dreapta, lipesc lista blockului din dreapta la finalul listei blockului din stanga, si dau free la block-ul din dreapta, fara a da free la lista.

> **FREE_BLOCK** <br> La fel ca la adaugare, am mai multe cazuri: <br> &emsp; 1. Cand e la inceputul unui block, eliberez doar primul miniblock din lista. Daca in urma acestei operatii size-ul blockului este egal cu 0, sterg si blockul, deoarece era unicul miniblock. <br> &emsp; 2. Cand e la sfarsitul unui block, sterg doar miniblock-ul de pe ultima pozitie. Daca era unicul miniblock, sterg tot blockul, facand acceasi verificare de mai sus. <br> &emsp; 3. Cand e la mijlocul unui block, creez un nou block, care va fi adaugat la dreapta blockului din care sterg, deoarece salvez in el a doua parte a listei din care sterg, respectiv cea care contine tail-ul original.

> **READ / WRITE** <br> Comenzile functioneaza asemanator, buffer-ul se parcurge caracter cu caracter cu un pointer. In cazul lui read se printeaza fiecare caracter in parte, iar in cazul lui write, se parcurg si datele scrise in data cu alt pointer si se copiaza unul cate unul. Cand ajunge la final de miniblock, pointer-ul se muta pe urmatorul block.

> **PMAP** <br> Pentru PMAP, parcurg toata lista din arena, si pentru fiecare block, parcurg lista de miniblock-uri, printand informatiile necesare. (Aceasta comanda consider ca nu necesita asa multe explicatii, sper ca se intelege din cod).

> **MPROTECT** <br> Dificultatea de la mprotect, daca pot spune asa, nu vine de la comanda in sine, deoarece doar transform string-uri intr-un numar, si setez miniblock->perm la acea valoare, ci vine de la verificari in plus care trebuie facute la **READ / WRITE**, daca toate blocurile in care scriu sau din care citesc au acea permisiune (nu e asa greu, tho').

## Alte comentarii asupra temei

### Programarea defensiva

> Cand am inceput lucrul la tema, nu stiam ca pot folosi doar macro-ul DIE (obisnuit de la PCLP 1, sa eliberez manual resursele in caz de fail). Ca sa nu modific antetele functiilor din schelet, am adaugat acel parametru in plus la arena, **arena->alloc_fail**, ca sa testez aceste cazuri. <br> Programarea defensiva se bazeaza pe faptul ca, in urma unui fail, nu schimb nimic din ce era in arena, nu adaug noduri, nu scot noduri, nu modific paramterii arenei pana nu stiu ca s-a reusit alocarea. Astfel, pot pur si simplu sa apelez functia de **dealloc_arena**, deoarece va elibera arena ca si cum ar fi eliberat-o la un pas inapoi (suna ciudat, dar are sens).
>> Spre exemplu, daca se dau comenzile <br> ALLOC_ARENA 5000 <br> ALLOC_BLOCK 0 100 <br> ALLOC_BLOCK 400 100 <br> Daca ultimul ALLOC_BLOCK da fail, este ca si cum as fi dat DEALLOC_ARENA imediat dupa primul ALLOC_BLOCK, pentru ca nu am facut nicio modificare intre legaturile listei, sau la parametrii arenei.

>Disclaimer: nu prea am avut cum sa testez fail-urile de alocare, asa ca s-ar putea sa gresesc cu logica asta.

<br>

### Ce am invatat din realizarea temei

Dupa aceasta prima tema la SDA, am acelasi sentiment ca dupa a 2-a tema la PCLP 1. Atunci simteam ca am inteles in sfarsit pointerii, dupa zeci de videoclipuri cu indieni, iar acum simt ca am inteles pe deplin listele (fara videoclipuri cu indieni). <br> La inceput nici nu intelegeam enuntul, dar dupa realizarea temei, consider ca inteleg de ce trebuie sa existe un Alocator de Memorie Virtuala la nivelul sistemului de operare, si ce face el de fapt.