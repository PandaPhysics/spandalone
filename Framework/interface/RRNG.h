#ifndef PANDATREE_RRNG
#define PANDATREE_RRNG

#include "TRandom3.h"
#include "TMath.h"
#include <vector>
#include <utility>

#define RRNG_DEBUG 0


namespace panda {
  class RRNG {
  public:
    RRNG(int maxEntropy = 50, 
         ULong64_t seed = 0, 
         const ULong64_t* seedAddress = nullptr, 
         bool stream=false);
    RRNG(RRNG const& other);
    RRNG(RRNG&& other);
    ~RRNG() {}
    RRNG& operator=(const RRNG& other);
    
    // configuration
    void setSize(int maxEntropy);
    void setFixedSeed(ULong64_t seed) { _seedAddress = nullptr; _editSeed(seed); }
    void setSeedAddress(const ULong64_t* seedAddress) { _seedAddress = seedAddress; }
    void setStreaming(bool b = true) { _streaming = b; if (b) { setSize(0); } }

    // request new data
    void generate(); 
    void generate(ULong64_t seed) { setFixedSeed(seed); generate(); }

    // various distributions
    double uniform(int& idx = _default_idx);
    void normal(double& x1, double& x2, int& idx = _default_idx);
    double normal(int& idx = _default_idx);
    double landau(double mu, double sigma, int& idx = _default_idx);
    double exp(double alpha, int& idx = _default_idx);
    int poisson(double mu, int& idx = _default_idx);

  private:
    static int _default_idx; // used when user does not provide index

    void _editSeed(ULong64_t seed) { _rng.SetSeed(seed); _currentSeed = seed; }
    double _requestNumber(int& idx);
    int _nextAvailable(int start);

    TRandom3 _rng;
    int _maxEntropy;
    const ULong64_t* _seedAddress{nullptr};
    ULong64_t _currentSeed{0};
    std::vector<std::pair<double,bool>> _x;
    bool _streaming;
  };
}


#endif
