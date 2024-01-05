#ifndef ROOT_Hit
#define ROOT_Hit

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
#include <vector>

using namespace std;

class Hit : public TObject {

private:
  
  Int_t _BoardID;
  Int_t _ChID;
  Int_t _Nsamples;
  vector <int> _Samples;

public:

   Hit();
   Hit(Hit *ht);
   Hit(Int_t board, Int_t ch, Int_t nsamples, vector<int> sample);
   virtual ~Hit();

   void SetBoardID(Int_t bid);
   void SetChID(Int_t chid);
   void SetNsamples(Int_t samp);
   void SetSamples(vector<int> smp);
   
    Int_t GetBoardID();
    Int_t GetChID();
    Int_t GetNsamples();
    vector<int>  GetSamples();
   
   ClassDef(Hit,1)  //Event structure
};



#endif
