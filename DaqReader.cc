#include "DaqReader.h"

#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TMultiGraph.h"

#include <iostream>
#include <cstddef>
#include <fstream>

// Costruttore del reader, memorizzo il path e apro il file
DaqReader::DaqReader(std::string filePath, int numberOfEvents) :
	m_filePath{ filePath },
	m_binaryFile{ std::fopen(filePath.c_str(), "r") },
	m_events{ numberOfEvents }
{
	// Controllo se l'apertura ha avuto successo e che non ho un nullptr
	if (!m_binaryFile)
	{
		std::cerr << "Errore in apertura del file.\n";
		std::exit(1);
	}
	std::cout << "DAQReader::DAQReader()		DAQREADER CREATED" << '\n';
}

// Funzione che serve per pulire le risorse che utilizzo
void DaqReader::cleanup()
{
	if (m_binaryFile)
		std::fclose(m_binaryFile);
}

// Pulisco le risorse
DaqReader::~DaqReader()
{
	cleanup();
}

// Helper function per controllare che il primo header (quello inserito ad hoc
// dalla collaborazione Argo) sia corretto
// Dopo che ho verificato il header, la funzione restituisce il numero di parole
// dei miei dati
int DaqReader::checkFirstHeader(const int* const firstHeader)
{
	// Controllo se la prima "parola magica" è corretta
	constexpr int firstCheckWord{ 0x17081996 };
	if (firstHeader[2] != firstCheckWord)
	{
		std::cerr << "Errore! Non ho trovato la parola magica ma ho trovato: "
			<< firstHeader[2] << '\n';
		std::exit(1);
	}

	// Qui ho una seconda parola magica che controllo
	constexpr int secondCheckWord{ 0xA0EF };
	int extractedCheckWord{ (firstHeader[13] >> 16) & 0xFFFF };
	if (extractedCheckWord != secondCheckWord)
	{
		std::cout << "Errore! Non ho trovato la parola magica della V1720.\n";
		std::exit(1);
	}

	// Nel firstHeader[0] c'è l'informazione su tutte le schede dell'esperimento
	// Argo, da cui viene fuori questa espressione
	int dataSize{ (firstHeader[0] - 28 - 44) / 4 };
	m_boards = firstHeader[5];
	m_eventCount = firstHeader[3];
	if (g_debug)
	{
		std::cout << "Reading event #" << m_eventCount << '\n';
		std::cout << "Number of 1720 Cards in DAQ: " << m_boards << '\n';
		std::cout << "d1720: " << std::hex << firstHeader[13] << '\n';
		std::cout << "LDATA is: " << std::dec << dataSize << '\n';
		std::cout << "Check 1720: " << std::hex << extractedCheckWord << std::dec << '\n';
	}
	return dataSize;
}

// Funzione che serve a contare il numero di canali dato il channel mask
static int computeChannels(int channelMask)
{
	constexpr int V1720channels{ 8 };
	int channels{ 0 };
	for (int bit{ 0 }; bit < V1720channels; bit++)
	{
		if (((channelMask >> bit) & 0x1) == 1)
			channels++;
	}
	return channels;
}

// Funzione che si occupa del grosso dell'estrazione dei dati
void DaqReader::processEventData(const std::size_t dataSize)
{
	// Rimuovo i dati dell'evento precedente siccome voglio immagazzinare quelli nuovi
	m_ADC00_CH0.clear();
	m_ADC00_CH1.clear();
	m_ADC00_CH2.clear();

	int boardData[g_maxBufferSize];
	// Leggiamo tutti i dati per questo evento
	const std::size_t boardDataSize{ std::fread(boardData, g_dataDimension, dataSize, m_binaryFile) };

	// Vediamo se il numero di parole che abbiamo letto è lo stesso di quelle 
	// che ci aspettiamo
	if (boardDataSize != dataSize)
	{
		std::cerr << "Errore! Le data size nei due header sono diverse.\n"
			<< "Primo header: " << dataSize << '\n'
			<< "Secondo header: " << boardDataSize << '\n';
		std::exit(1);
	}
	if (g_debug)
		std::cout << "Found V1720 data block!";

	// Questo ciclo è stato scritto in caso ci fossero più di una board	
	int index{ 0 };
	for (int board{ 0 }; board < m_boards; board++)
	{
		if (g_debug)
			std::cout << "Reading BOARD number: " << board << '\n';

		// Prendiamo la checkword che può essere trovata anche nel manuale
		// della V1720, ovvero: 0b1010 = 0xa.
		const int boardCheckWord{ (boardData[index] >> 28) & 0xf };
		if (boardCheckWord != 0xa)
		{
			std::cout << "Second checkWord failed, data not found in place!\n";
			std::exit(1);
		}

		// Vediamo il numero di canali attivi
		const int channelMask{ boardData[index + 1] & 0xff };
		const int channels{ computeChannels(channelMask) };

		// Vediamo se il numero di evento tra i due header è lo stesso
		const int boardEventCount{ boardData[index + 2] & 0xffffff };
		if (boardEventCount != m_eventCount - 1)
		{
			std::cout << "Errore! I dati non sono allineati!\n";
			std::exit(1);
		}

		// Qui inizia la vera e propria fase di sbittaggio
		const int boardWords{ boardData[index] & 0xfffffff };


		constexpr int headerWords{ 4 };
		const int wordsPerChannel{ (static_cast<int>(boardDataSize) - headerWords) / channels };

		// Facciamo il loop su ogni canale della board
		for (int ichan{ 0 }; ichan < channels; ichan++)
		{
			for (int word{ 0 }; word < wordsPerChannel; ++word)
			{
				int wordContent{ boardData[index + headerWords + word + ichan * wordsPerChannel] };
				// Come si può leggere dal manuuale della scheda V1720, in ogni
				// word sono salvati i dati di due sample, ognuno da 12bit
				int firstSample{ wordContent & 0xfff };
				int secondSample{ (wordContent >> 16) & 0xfff };

				if (board == 0 && ichan == 0)
				{
					m_ADC00_CH0.push_back(firstSample);
					m_ADC00_CH0.push_back(secondSample);
				}

				if (board == 0 && ichan == 1)
				{
					m_ADC00_CH1.push_back(firstSample);
					m_ADC00_CH1.push_back(secondSample);
				}

				if (board == 0 && ichan == 2)
				{
					m_ADC00_CH2.push_back(firstSample);
					m_ADC00_CH2.push_back(secondSample);
				}
			} // end word
		} // end channel
		// Sposto l'indice del numero di word che ci sono per ogni board
		index += boardWords;
	} // end board

	// Codice di controllo a fine evento, ulteriore controllo per vedere se
	// la parte 
	int dump[4];
	std::fread(dump, g_dataDimension, 4, m_binaryFile);
	if ((((dump[0] >> 16) & 0xFFFF) == 0xA1EF) &&
		(((dump[1] >> 16) & 0xFFFF) == 0xA2EF) &&
		(((dump[2] >> 16) & 0xFFFF) == 0xA3E0) &&
		(((dump[3] >> 16) & 0xFFFF) == 0xA4EF))
	{
		if (g_debug)
			printf("Sto alla fine dell'evento\n\n\n");
	}
}

// Nel caso non ci siano più dati da processare la funzione restituisce falso
// nel caso siano presenti altri dati, la funzione elabora i dati e li salva
// nelle variabili dei vari canali (m_ADC00CH0/1/2)
bool DaqReader::processNextEvent()
{
	// Creo l'array per il primo header e lo salvo
	int firstHeader[14];
	std::size_t objectsRead{ std::fread(firstHeader, g_dataDimension, g_firstHeaderWords, m_binaryFile) };
	// Controllo che il numero di word sia giusto, ovvero 14
	if (objectsRead != 14 || m_currentEvent >= m_events)
		return false;

	// Questa funzione controlla il primo header e vede se i dati non siano corrotti
	// Successivamente restituisce la dimensione dei dati
	const int dataSize{ checkFirstHeader(firstHeader) };

	processEventData(dataSize);
	m_currentEvent++;
	return true;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					Qui finisce la parte dello sbittaggio
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

// Definisco la funzione che fa la derivata numerica simmetrica al quarto ordine
// Questo permette di abbassare il rumore
std::vector<int> derivate(const std::vector<int>& data)
{
	std::vector<int> result{};
	// Per avere la stessa dimensione tra dati e derivate mettiamo le prime due
	// entrate pari a 0
	result.push_back(0);
	result.push_back(0);
	for (std::size_t i{ 2 }; i < data.size() - 2; i++)
		result.push_back((-data[i + 2] + 8 * data[i + 1] - 8 * data[i - 1] + data[i - 2]) / 12);
	// Per lo stesso motivo di sopra anche le ultime due entries devono essere nulle
	result.push_back(0);
	result.push_back(0);

	return result;
}

// Definisco la funzione che individua gli indici dei picchi con un algoritmo
Peaks findPeak(const std::vector<int>& dataY)
{
	std::vector<int> dataYDer{ derivate(dataY) };

	// Soglia ottenuta a tentativi analizzando i dati
	constexpr double derivativeThreshold{ -11 };
	bool signalFound{ false };
	bool signalAscending{ false };
	std::size_t peakStart{};
	std::size_t peakEnd{};
	std::size_t peakMinimum{};

	Peaks result{};

	for (std::size_t i{ 0 }; i < dataY.size(); i++)
	{
		// Ho trovato il segnale, è finita la risalita, salvo gli estremi del picco
		if (signalFound && signalAscending && dataYDer[i] < 0)
		{
			peakEnd = i;
			result.amount += 1;
			result.peakStart.push_back(peakStart);
			result.peakEnd.push_back(peakEnd);
			result.peakMinimum.push_back(peakMinimum);

			peakEnd = 0;
			peakStart = 0;
			peakMinimum = 0;
			signalFound = false;
			signalAscending = false;
		}

		// Ho trovato il segnale ma non ho ancora superato il minimo (non sto risalendo)
		if (signalFound && !signalAscending && dataYDer[i] > 0)
			signalAscending = true;

		// Non ho ancora trovato il segnale
		if (dataYDer[i] < derivativeThreshold && !signalFound)
		{
			signalFound = true;
			peakStart = i;
			peakMinimum = i;
		}

		// Ho trovato il segnale, cerco il minimo
		if (signalFound && dataY[i] < dataY[peakMinimum])
			peakMinimum = i;
	}
	return result;
}


double countToV(double counts)
{
	// Offset ottenuto dall'analisi delle ampiezze
	constexpr int offset{ 2110 };
	// il 2 al numeratore è dovuto dal fatto che il range del fADC è di 2Vpp
	// 2^12 siccome è un convertitore a 12 bit.
	const double conversionScaleADC{ 2 / (std::pow(2, 12) - 1) };
	return (counts - offset) * conversionScaleADC;
}

int sampleToNs(int sample)
{
	// Parametro estratto dal manuale della V1720
	constexpr int nsPerSample{ 4 };
	return sample * nsPerSample;
}

double integrateSpectrum(std::size_t start, std::size_t end, const std::vector<int>& data)
{
	// Resistenza 
	constexpr int resistance{ 50 };
	constexpr double time{ 4e-9 };
	// Voglio che i dati siano esplicitamente in nano
	constexpr double converstionToNs{ 1e9 };
	double result{ 0 };
	for (size_t i{ start }; i < end; i++)
		result += std::abs(countToV((data[i] + data[i + 1]) / 2.));

	return result / resistance * time * converstionToNs;
}

// Questa è la funzione che è stata scritta per l'elaborazione dei dati
int DaqReader::generateRootFile()
{
	std::string rootPath{ m_filePath + ".root" };
	TFile rootFile(rootPath.c_str(), "RECREATE");

	TH1D timeHistogram("h_TimeDifference", "Distribuzione tempi di decadimento;Tempo [ns];Eventi", 500, 0, 10000);
	TH1D electronSpectrum("h_AreaElettrone", "Spettro elettrone;Carica [nC];Eventi", 1250, 0, 1.25);
	TH1D muonSpectrum("h_AreaMuone", "Spettro muone;Carica [nC];Eventi", 1250, 0, 1.25);

	Peaks peaks{};
	std::vector<int> data{};

	// Loop principale per l'accesso ai dati
	while (processNextEvent())
	{
		data = GetCH1();
		peaks = findPeak(data);

		// Se non ho almeno due picchi ho un problema con l'evento
		if (peaks.amount < 2)
		{
			std::cerr << "Ho un problema di picchi nell'evento " << GetCurrentEvent() << " lo salto.\n";
			continue;
		}

		// Devo controllare di avere almeno due picchi per poter definire timeDifference
		int timeDifference{ sampleToNs(static_cast<double>(peaks.peakStart[1] - peaks.peakEnd[0])) };

		// Trovo l'area del muone così posso vedere se supera la soglia
		double muonIntegral{ integrateSpectrum(peaks.peakStart[0], peaks.peakEnd[0], data) };

		// Imposto dei limiti sull'evento per pulire il rumore e migliorare la qualità dei dati
		constexpr int minimumTimeDifference{ 20 };
		constexpr double minimumCharge{ 0.2 };
		if (peaks.amount == 2 &&
			timeDifference > minimumTimeDifference &&
			muonIntegral > minimumCharge)
		{
			timeHistogram.Fill(timeDifference);
			electronSpectrum.Fill(integrateSpectrum(peaks.peakStart[1], peaks.peakEnd[1], data));
			muonSpectrum.Fill(muonIntegral);
		}

		// Genero i grafici per vedere se l'algoritmo trova picchi funziona in maniera corretta 
		if (GetCurrentEvent() < 2000 && g_debug)
		{
			TMultiGraph finalGraph("peakTest", "Test algoritmo picchi; Time [ns]; Amplitude [V]");
			TGraph dataPoints;
			for (std::size_t i{ 0 }; i < data.size(); ++i)
			{
				dataPoints.AddPoint(sampleToNs(i), countToV(data[i]));
			}
			finalGraph.Add(&dataPoints);

			TGraph startPeakPoints;
			startPeakPoints.SetMarkerColor(kRed);
			startPeakPoints.SetMarkerStyle(kFullDotLarge);
			startPeakPoints.SetLineWidth(0);

			TGraph endPeakPoints;
			endPeakPoints.SetMarkerColor(kBlue);
			endPeakPoints.SetMarkerStyle(kFullDotLarge);
			endPeakPoints.SetLineWidth(0);

			TGraph minimumPeakPoints;
			minimumPeakPoints.SetMarkerColor(kGreen);
			minimumPeakPoints.SetMarkerStyle(kFullDotLarge);
			minimumPeakPoints.SetLineWidth(0);

			for (std::size_t i{ 0 }; i < peaks.amount; i++)
			{
				startPeakPoints.AddPoint(sampleToNs(peaks.peakStart[i]), countToV(data[peaks.peakStart[i]]));
				endPeakPoints.AddPoint(sampleToNs(peaks.peakEnd[i]), countToV(data[peaks.peakEnd[i]]));
				minimumPeakPoints.AddPoint(sampleToNs(peaks.peakMinimum[i]), countToV(data[peaks.peakMinimum[i]]));

				finalGraph.Add(&startPeakPoints);
				finalGraph.Add(&endPeakPoints);
				finalGraph.Add(&minimumPeakPoints);
			}

			finalGraph.Write();
		}
	}

	// Salvo i grafici sul file root
	muonSpectrum.Write();
	electronSpectrum.Write();
	timeHistogram.Write();
	rootFile.Close();

	return m_currentEvent;
}