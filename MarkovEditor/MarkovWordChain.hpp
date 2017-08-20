//
//  MarkovWordChain.hpp
//  example_aubioDemo
//
//  Created by Jonathan B Gallagher on 5/6/17.
//
//

#ifndef MarkovWordChain_hpp
#define MarkovWordChain_hpp

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <cstring>
#include <locale>
#include "chaosMap.h"
#include <iostream>
#include <istream>
#include <fstream>
#include <regex>
#include <random>
#include <chrono>


using namespace std;

typedef map<string,float> wordMap;
typedef map<float,string> rWordMap;
typedef map<int,wordMap> levelMap;
typedef map<int,rWordMap> rLevelMap;
typedef map<string,wordMap> markovChain;
typedef map<string,rWordMap> markovRChain;
typedef map<string,levelMap> markovChainNLevel;
typedef map<string,rLevelMap> markovProbChainNLevel;

enum MarkovMode {BY_WORD = 0 , BY_CHAR};
enum MarkovDist {RANDOM = 0, LOGISTIC_MAP, EXP_MAP, MOUSE_MAP, NORMAL, BINOMIAL, EXPONENTIAL, GEOMETRIC, LOGNORMAL, POISSON};

class MarkovWordChain
{
public:
    MarkovWordChain();
    MarkovWordChain(string fname, string seedWord, int numLevels,float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,int mt);
    void CreateMarkovWordChain();
    void CreateMarkovCharChain();
    
    //compute probabilities is used for both by char and by cumulative words modes
    void ComputeProbabilities();
    
    void LoadChaosMap(float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,mapType mpType);
    void LoadChaosMap();
    void DeleteChaosMap();
    
    void setMyMtype(int mt);
    void LoadTextIntoVector(string fname);
    void LoadTextIntoVectorByChar(string fname);
    void LoadTextFromWordsToLetters();
    vector<string> splitString(const std::string& text, const std::string& delims);
    string GetFoundWord();
    string GetFirstWord();
    
    
    bool GetNextProbChain();
    void GetNextWordInProbChain();
    bool GetNextProbChainByChar();
    void GetNextStrInProbChainByChar();
    
    void MakeDistribution(int num, float f1, float f2);
    void NormalizeDistribution();
    float GetProbability();
    
    void ClearMarkovData();
    void ClearMarkovChain();
    void SetupMarkovChain();
    void SetupCharMarkovChain();
    
    void SetKeyWord();
    void SetNextWord();
    
    float GetNextWordCount(string bwrd, string wrd);
    float GetNextWordPerc(string bwrd, string wrd);
    float GetBaseWordSum(string wrd);
    
    void SetWordMapPointer(string word);
    //inlines
    inline void setUseChaosMap(bool uch) { useChaosMap = uch;}
    inline bool getUseChaosMap() { return useChaosMap;}
    inline void setParBegin(float pb) { parBegin = pb;}
    inline float getParBegin() { return parBegin; }
    inline void setParEnd(float pb) { parEnd = pb;}
    inline float getParEnd() { return parEnd; }
    inline void setSteps(int st) { steps = st;}
    inline int getSteps() { return steps;};
    inline void setItr(int it) { itr = it; }
    inline int getItr() { return itr;}
    inline void setKit(int kt) { kit = kt;}
    inline int getKit() { return kit;}
    inline void setMpar(float mp) { mpar = mp;}
    inline float getMpar() { return mpar;}
    inline void setX(float xin) { x = xin;}
    inline float getX() { return x;}

    inline void setMyMapType(mapType mt) { myMtype = mt; }
    inline mapType getMyMapType() { return myMtype;}

    inline void setSeedWord(string sdw) { firstWord = sdw; }
    inline string getSeedWord() { return firstWord; }
    inline void setNumLevels(int nl) { nlevels = nl; }
    inline int getNumLevels() { return nlevels; }
    inline bool getChainIsReady() { return chainIsReady;}
    inline void setMarkovMode(MarkovMode mode) { mmode = mode; }
    inline int getMarkovMode() { return mmode;}
    inline void setFirstRun(bool fr) { firstRun = fr;}
    inline bool getFirstRun() { return firstRun;}
    inline void setFirstWordInChain(bool fr) { firstWordInChain = fr;}
    inline bool getFirstWordInChain() { return firstWordInChain;}

    inline void setRemovePunc(bool rmpnc) { removePunc = rmpnc;}
    inline bool getRemovePunc() { return removePunc;}
    inline void setRemoveQuot(bool rmqt) { removeQuot = rmqt; }
    inline bool getRemoveQuot() { return removeQuot;}
    
    inline void setMarkovDist(MarkovDist dist) { myDist = dist;}
    MarkovDist getMarkovDist() { return myDist;}
    
    wordMap *wMapPointer;
private:
    vector<string> Words;
    vector<string> Letters;
    vector<float> Distribution;
    
    //by words cumulative string or by char
    markovChain myMarkovChain;
    markovChain myMarkovChainCopy;
    markovRChain myProbChain;
    
    int nlevels;
    markovChain::iterator itMrk;
    markovChain::iterator itMrkCp;
    markovRChain::iterator itRMrk;
    markovChain::iterator itMrkActive;
    
    //chaos stuff
    ChaosMap *myMap;
    mapType  myMtype;
    float parBegin,parEnd,x,mpar;
    int steps,itr,kit,mtype;
    bool useChaosMap,firstRun,firstWordInChain;
    logMap::iterator parItr;
    vector<double>::iterator probItr;
    
    //found word
    string seedWord;
    string foundWord;
    string firstWord;
    string lastFoundWord;
    
    string keyWord;
    string nextWord;
    bool chainIsReady;
    bool removePunc;
    bool removeQuot;
    
    //markov mode
    //cumulative token or N words away
    MarkovMode mmode;
    MarkovDist myDist;
    int distribCount;
    
    
    
  
    
};
#endif /* MarkovWordChain_hpp */
