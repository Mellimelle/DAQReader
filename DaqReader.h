#ifndef DAQREADER_H
#define DAQREADER_H

#include "TTree.h"

#include <vector>
#include <string>
#include <cstdio>

// Definizione di costanti globali
// Dimensione delle word in bytes, in questo caso sono parole da 32bit
constexpr int g_dataDimension{ 4 };
// Numero di word nel primo header
constexpr int g_firstHeaderWords{ 14 };
// Variabile per attivare il print di debug, cambiare in true per avere molte più scritte
constexpr bool g_debug{ false };
// Dimensione del buffer dell'evento
constexpr int g_maxBufferSize{ 0x100000 };
// Dimensione del sample utilizzando circa 16 us per ogni buffer
constexpr int g_maxSamples{ 4096 };

// Creo un oggetto per immagazzinare gli indici di tutti i picchi
struct Peaks
{
    std::size_t amount{ 0 };
    std::vector<std::size_t> peakStart{};
    std::vector<std::size_t> peakEnd{};
    std::vector<std::size_t> peakMinimum{};
};
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                Alcune forward declaration per funzioni
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Funzione che esegue la derivata
std::vector<int> derivate(const std::vector<int>& data);
// Funzione trova picchi
Peaks findPeak(const std::vector<int>& dataY);
// Conversione tra conteggi fADC e volt
double countToV(double);
// Conversione tra numero del sample e nanosecondi
int sampleToNs(int time);
// Integrazione con la regola del trapezio
double integrateSpectrum(std::size_t start, std::size_t end, const std::vector<int>& data);


// Oggetto che si occupa della corretta gestione del codice binario e dei vari check.
// Una volta fatte le verifiche necessarie garantisce un facile accesso ai dati
// Attraverso le funzini GetCH...
class DaqReader
{
public:
    // Costruttore
    DaqReader(std::string pathFile, int numberOfEventsToRead);
    // Distruttore
    ~DaqReader();

    // Funzione per l'esecuzione del loop di lettura dati
    bool processNextEvent();

    // Member function per la generazione del file .root con tutta l'annessa 
    int generateRootFile();

    // Funzione per l'accesso ai dati
    std::vector<int> GetCH0() { return m_ADC00_CH0; }
    std::vector<int> GetCH1() { return m_ADC00_CH1; }
    std::vector<int> GetCH2() { return m_ADC00_CH2; }
    int GetCurrentEvent() { return m_currentEvent; }

private:
    // Output canali
    std::vector<int> m_ADC00_CH0{};
    std::vector<int> m_ADC00_CH1{};
    std::vector<int> m_ADC00_CH2{};

    // Member variables per apertura del file binario
    std::string m_filePath{};
    std::FILE* m_binaryFile{ nullptr };

    // Member variables per l'elaborazione del codice binario
    int m_events{};
    int m_eventCount{};
    int m_currentEvent{ 0 };
    int m_boards{};


    // Helper member function, non voglio chiamarla
    int checkFirstHeader(const int* const);
    void processEventData(std::size_t);

    // Funzione per pulizia della classe
    void cleanup();
};
#endif
