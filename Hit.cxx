#include "RVersion.h"
#include "TRandom.h"
#include "TDirectory.h"
#include "TProcessID.h"
#include <iostream>

#include "Hit.h"

ClassImp(Hit)

Hit::Hit() {}

Hit::Hit(Hit* ht)
{

  _ChID = ht->GetChID();
  _BoardID = ht->GetBoardID();
  _Nsamples = ht->GetNsamples();
  _Samples = ht->GetSamples();
}

Hit::Hit(Int_t board, Int_t ch, Int_t nsamples, vector<int> sample)
{

  _BoardID = board;
  _ChID = ch;
  _Nsamples = nsamples;
  _Samples = sample;

};


Hit::~Hit() {}

void Hit::SetBoardID(Int_t bid)
{
  _BoardID = bid;
}

void Hit::SetChID(Int_t chid)
{
  _ChID = chid;
}

void Hit::SetNsamples(Int_t samp)
{
  _Nsamples = samp;
}

void Hit::SetSamples(vector<int> smp)
{
  _Samples = smp;
}

Int_t Hit::GetChID()
{
  return _ChID;
}

Int_t Hit::GetBoardID()
{
  return _BoardID;
}

Int_t Hit::GetNsamples()
{
  return _Nsamples;
}

vector<int> Hit::GetSamples()
{
  return _Samples;
}
