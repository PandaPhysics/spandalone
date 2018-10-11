#include "PandaTree/Framework/interface/RRNG.h"
#include "Math/QuantFuncMathCore.h"
#include "TString.h"
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace panda;

int RRNG::_default_idx;

RRNG::RRNG(int maxEntropy, ULong64_t seed, const ULong64_t* seedAddress, bool streaming): 
  _rng(seed),
  _maxEntropy(maxEntropy),
  _seedAddress(seedAddress),
  _currentSeed(seed),
  _streaming(streaming)
{
  _default_idx = -1;
  setSize(maxEntropy);
  generate();
}

RRNG::RRNG(RRNG const& other): 
  _rng(other._rng),
  _maxEntropy(other._maxEntropy),
  _seedAddress(other._seedAddress),
  _currentSeed(other._currentSeed),
  _x(other._x),
  _streaming(other._streaming)
{
}

RRNG::RRNG(RRNG&& other): 
  _rng(other._rng), // still make a copy of the other rng's state
  _maxEntropy(std::move(other._maxEntropy)),
  _seedAddress(std::move(other._seedAddress)),
  _currentSeed(std::move(other._currentSeed)),
  _x(std::move(other._x)),
  _streaming(other._streaming)
{
}

RRNG& RRNG::operator=(RRNG const& other)
{
  _rng = other._rng;
  setSize(other._maxEntropy);
  std::copy(other._x.begin(), other._x.end(), std::back_inserter(_x));
  _currentSeed = other._currentSeed;
  _seedAddress = other._seedAddress;
  _streaming = other._streaming;
  return *this;
}

void RRNG::setSize(int maxEntropy)
{
  _maxEntropy = maxEntropy;
  assert(_maxEntropy >= 0);
  _x.clear(); // clear so we can use same default value
  _x.resize(_maxEntropy, std::pair<double,bool>(0, false));
  if (_maxEntropy > 0)        // if we require more than one number,
    setStreaming(false); // streaming mode cannot be used
}

void RRNG::generate() 
{
  if (_seedAddress != nullptr && _currentSeed != *_seedAddress) {
    _editSeed(*_seedAddress);
  }
  for (auto &p : _x) {
    p.first = _rng.Rndm();
    p.second = true;
  }
}

int RRNG::_nextAvailable(int start) 
{
  while (true) {
    for (auto p = _x.begin() + start; p != _x.end(); ++p) {
      if (p->second)
        return static_cast<int>(p - _x.begin());
    }
#if RRNG_DEBUG
    cerr << "RRNG::_requestNumber forced to re-generate numbers because we ran out!" << endl;
#endif
    generate();
    start = 0;
  }
}

double RRNG::_requestNumber(int& idx) 
{
  if (_streaming) { 
    return _rng.Rndm();
  }
  if (_maxEntropy == 0) {
    throw runtime_error("RRNG::_requestNumber internal size is 0 and streaming mode is off");
  }
  if (idx >= _maxEntropy) {
    throw out_of_range(Form("RRNG::_requestNumber trying to access idx=%i out of %i", idx, _maxEntropy));
  } 
  if (idx >= 0 && !(_x.at(idx).second)) {
    throw runtime_error(Form("RRNG::_requestNumber already accessed idx=%i", idx));
  }
  
  int idx_(_nextAvailable(std::max(0,idx)));

#if RRNG_DEBUG
  cerr << "RRNG::_requestNumber returning index " << idx_ << endl;
#endif
  _x[idx_].second = false;

  if (idx >= 0) {
    // if user specified an index, then tell them 
    // the next one they could access
    idx = _nextAvailable(idx_); 
  }

  return _x[idx_].first;
}

double RRNG::uniform(int& idx) 
{
  return _requestNumber(idx);
}

// Box-Muller
void RRNG::normal(double& x1, double& x2, int& idx)
{
  double u = _requestNumber(idx);
  double v = _requestNumber(idx);

  u = TMath::Sqrt(-2 * TMath::Log(u));
  v = 2 * TMath::Pi() * v;

  x1 = u * TMath::Cos(v);
  x2 = u * TMath::Sin(v);  
}

// note that this requires two RNs even though it returns only one
double RRNG::normal(int& idx) 
{
  double x1, x2;
  normal(x1, x2, idx);
  return x1;
}

double RRNG::landau(double mu, double sigma, int& idx)
{
  if (sigma <= 0) 
    return 0;
  double x = _requestNumber(idx);
  return mu + ROOT::Math::landau_quantile(x, sigma);
}

double RRNG::exp(double alpha, int& idx)
{
  double x = _requestNumber(idx);
  return - TMath::Log(x) / alpha;
}

// this RV may use an arbitrary number of RNs!
int RRNG::poisson(double mu, int& idx)
{
  if (mu <= 0)
    return 0;

  double prod = 1;
  double threshold = TMath::Exp(-mu);
  int N = -1; 

  while (prod >= threshold) {
    prod *= _requestNumber(idx);
    N += 1;
  }
   
  return N; 
}
