#ifndef ROOT_Event
#define ROOT_Event

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Event                                                                //
//                                                                      //
// Description of the event and track parameters                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TObject.h"
#include "TClonesArray.h"
#include "TRefArray.h"
#include "TRef.h"
#include "TH1.h"
#include "TBits.h"
#include "TMath.h"
#include "TVector.h"
#include "Hit.h"
#include <vector>

using namespace std;

class TDirectory;


class Event : public TObject {

private:
    
    Int_t _nhit;
   TClonesArray *_Hitlist;

public:
   
   Event();
   virtual ~Event();
   void AddHit(Hit *ht);
   void AddHit(Int_t board, Int_t ch, Int_t nsamples, vector<int> sample);
   void Reset();

   ClassDef(Event,1)  //Event structure
};



#endif
