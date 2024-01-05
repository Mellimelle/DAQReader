# Introduzione
Quest'anno si è voluto riscrivere il codice che analizzava ed elaborava i dati per la relazione di laboratorio del secondo anno in fisica magistrale presso l'Università Roma Tre.
Per rendere il codice più accessibile, è stato reso "open-source" e publicato su internet.

Per chi fosse interessato, il codice originale è conservato nella *farm* del laboratorio, nella directory:
```
/home/daq/muon_lifetime_exp/Decodifica/DAQReader
```

# Compatibilità
Il codice è stato riscritto in C++, mantenendo il linguaggio originale ma 
adottando uno stile più moderno, utilizzando lo standard C++17 e diverse delle sue 
funzionalità.

Il programma può essere compilato solo su sistemi Unix (non su Windows); 
è stato specificamente testato su Ubuntu 22.04. Nel caso si utilizzi un 
sistema Windows, è possibile seguire [questa guida](https://learn.microsoft.com/it-it/windows/wsl/install) per istruzioni su come 
installare una virtualizzazione Linux sul proprio sistema.

# Requisiti
Per compilare ed eseguire correttamente il programma sono necessari i seguenti requisiti:
- G++ con versione compatibile per lo standard C++17;
- ROOT versione 6.30/2 o successive;
- Sistema basato su Unix (non Windows).

# Come compilare ed eseguire il programma
Di seguito si fornisce un elenco dei vari comandi che è necessario eseguire per

## Compilazione
Per compilare il programma, è necessario eseguire i seguenti comandi dal 
terminale all'interno della cartella con il codice sorgente:
```bash
$ make clean
$ make all
```
Verrà creato l'eseguibile chiamato `Reader.bin` nella cartella esterna al codice sorgente, insieme a una cartella `lib` contenente la libreria denominata `libEvent.so`.

## Esecuzione
Una volta ottenuto l'eseguibile, seguire questi passaggi:
1. fornire il percorso al file binario con i dati;
2. indicare il numero massimo di eventi da decodificare.

Se si specifica un numero di eventi maggiore di quelli presenti nel file, il programma terminerà automaticamente dopo aver letto tutti gli eventi disponibili.

Supponendo di avere il file di dati `dati.dat`, e si desidera leggere 10000 dati, il comando da eseguire sarà:
```bash
$ ./Reader.bin dati.dat 10000
```

Questo genererà un file `ROOT` con il nome `dati.dat.root`. In generale, verrà creato un file con lo stesso nome, aggiungendo l'estensione ".root" alla fine.

# Modifiche al programma
Il programma è stato scritto cercando di rendere l'espansione e la creazione di proprie funzioni in maniera agevole.

Per utilizzare le funzioni di elaborazione dei dati, è necessario creare un oggetto`DaqReader`. Successivamente è disponibile la member function `processNextEvent()`, che si occupa di processare l'evento successivo. Questa funzione restituisce un booleano se riesce a leggere i dati. È quindi facilmente utilizzabile all'interno di un ciclo `while`. Per accedere ai dati, sono disponibili le funzioni `Get`:
- `GetCH0`;
- `GetCH1`;
- `GetCH2`.

Di seguito è fornito un esempio di funzione che legge i dati e li utilizza:
```C++
void prova()
{
    DaqReader reader("dati.dat", 10000);
    while(reader.processNextEvent())
        {
            handleData(reader.GetCH1);
            saveData();
        }
}
```

Se si desidera un esempio più pratico, si consiglia di consultare il codice della funzione `generateRootFile()`.

## Funzioni di debug
All'interno del file `DaqReader.h`, è stata definita la variabile `g_debug` che può essere impostata su `true` o `false`. Quando impostata su `true`, durante l'elaborazione dei dati verranno stampate informazioni sulla decodifica. Nel file `.root` generato, saranno anche inclusi dei grafici di test per valutare le prestazioni dell'algoritmo dei picchi ([vedi sezione sull'algoritmo di ricerca dei picchi](#algoritmo-cerca-picchi)).

# Sbittaggio
Di seguito verranno forniti ulteriori dettagli sul funzionamento dello sbittaggio, sia per i curiosi che per avere una documentazione facilmente accessibile.

Il codice binario dei dati contiene due "header". Il primo è composto da 14 parole da 32 bit ed è stato scritto dall'esperimento ARGO, da cui l'Università Roma3 ha ereditato la strumentazione delle schede V1720. Dopo queste 14 parole, è presente l'header della V1720 e i dati veri e propri (consultare il [manuale offerto dalla CAEN](https://www.caen.it/products/v1720/) per il formato standard degli eventi).

La struttura del primo header è descritta di seguito utilizzando la numerazione degli array:

0. dimensione dell’evento totale, considerando anche la presenza di ulteriori schede, moltiplicato per dei opportuni numeri;
1. *check word* con valore 1;
2. *check word* con valore esadecimale 0x17081996;
3. numero corrispondente all’evento. Nel caso della lettura del primo
evento nel file, si attende un valore pari a 1, e così via;
4. *check word* con valore 1;
5. numero di schede V1720 coinvolte nella presa dati;
6. *check word* con valore 0;
7. ridondanza del valore che si trova alla posizione 0;
8. informazione sconosciuta;
9. ridondanza del numero di evento;
10. *check word* con valore 0;
11. informazione sconosciuta;
12. informazione sconosciuta;
13. eseguendo un’operazione di bitwise right shift di 16 bit e mascherando
(bitwise and ) il risultato con il numero 0xFFFF, si ottiene la check
word corrispondente al valore 0xA0EF.

Purtroppo, di alcune parole non è stato possibile ricavare il significato in quanto non utilizzate nemmeno nel codice originale. Per farlo sarebbe necessario consultare il programma che produce il file binario dei dati.

# Algoritmo cerca picchi
Qui è brevemente spiegato il funzionamento dell'algoritmo utilizzato per la ricerca dei picchi.

1. Analisi sequenziale dei dati: l’algoritmo inizia leggendo sequenzial-
mente la lista dei dati.
2. Individuazione dell’inizio del segnale: Trova il primo punto della
lista in cui la derivata è minore della soglia prestabilita, salvando la
posizione del campione poiché corrisponde all’inizio del segnale.
3. Individuazione del minimo del segnale: Prosegue nella lettura
della lista fino a quando la derivata diventa positiva, segnalando il
punto di minimo e registrandone la posizione.
4. Risalita del segnale: Dopo il minimo, la derivata diventa positiva in
quanto il segnale risale.
5. Fine del segnale: Identifica la fine del segnale quando la derivata
diventa nuovamente negativa, salvando la posizione del sample corri-
spondente.
6. Reiterazione: L’algoritmo continua la ricerca dei picchi, eseguendo
nuovamente i passaggi descritti finché non si arriva alla fine dei dati.
