#include "DaqReader.h"

#include "TObject.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TFile.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>

// Implementiamo la derivata. Per essere meno sensibili alle oscillazioni del segnale
// utilizziamo la definizione di derivata numerica simmetrica del quarto ordine

int main(int argc, char* argv[])
{
    /* Controlliamo che l'utente abbia inserito il giusto numero di variabili,
    ovver, uno per il nome del file e il seconda per il numero di eventi da leggere*/
    if (argc <= 2)
    {
        std::cerr << "Errore: numero di argomenti insufficiente!\n";
        std::exit(1);
    }
    const std::string filePath = argv[1];

    // Qui è necessario l'utilizzo di "atoi" in quanto "stoi" non è supportato
    const int numberOfEvents = std::atoi(argv[2]);

    // Instanziamo l'oggetto che ci servità per leggere i dati
    DaqReader reader(filePath, numberOfEvents);

    reader.generateRootFile();

    return 0;
}
