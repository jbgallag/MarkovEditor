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


using namespace std;

enum MarkovMode {BY_WORD = 0 , BY_NWORD, BY_CHAR};

class MarkovWordChain
{
public:
    MarkovWordChain();
    MarkovWordChain(string fname, string seedWord, int numLevels,float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,int mt);
    void CreateMarkovWordChain();
    //compute probabilities is used for both by char and by cumulative words modes
    void ComputeProbabilities();
    void CreateNLevelMarkovChain();
    void ComputeNLevelProbabilities();
    void CreateMarkovCharChain();
    
    void LoadChaosMap(float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,mapType mpType);
    void setMyMtype(int mt);
    void LoadTextIntoVector(string fname);
    void LoadTextIntoVectorByChar(string fname);
    void LoadTextFromWordsToLetters();
    vector<string> splitString(const std::string& text, const std::string& delims);
    string GetFoundWord();
    string GetFirstWord();
    
    
    void GetNextProbChain();
    void GetNextWordInProbChain();
    void GetNextNLevelProbChain();
    void GetNextWordInNLevelProbChain();
    void GetNextProbChainByChar();
    void GetNextStrInProbChainByChar();
    
    float GetProbability();
    void ClearMarkovChain();
    void SetupMarkovChain();
    void SetupCharMarkovChain();
    void ClearNLevelMarkovChain();
    void SetupNLevelMarkovChain();
    
    void SetKeyWord();
    void SetNextWord();
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
    
    
private:
    typedef map<string,float> wordMap;
    typedef map<float,string> rWordMap;
    typedef map<int,wordMap> levelMap;
    typedef map<int,rWordMap> rLevelMap;
    typedef map<string,wordMap> markovChain;
    typedef map<string,rWordMap> markovRChain;
    typedef map<string,levelMap> markovChainNLevel;
    typedef map<string,rLevelMap> markovProbChainNLevel;
    vector<string> Words;
    vector<string> Letters;
    
    //by words cumulative string or by char
    markovChain myMarkovChain;
    markovRChain myProbChain;
    
    //by words N-Words-Away
    markovChainNLevel myMarkovChainNLevel;
    markovProbChainNLevel myProbChainNLevel;
    
    int nlevels,curlevel;
    markovChain::iterator itMrk;
    markovRChain::iterator itRMrk;
    markovChain::iterator itMrkActive;
    
    markovChainNLevel::iterator itNMrk;
    markovProbChainNLevel::iterator itNRMrk;
    rLevelMap::iterator itRLvl;
    //chaos stuff
    ChaosMap *myMap;
    mapType  myMtype;
    float parBegin,parEnd,x,mpar;
    int steps,itr,kit,mtype;
    bool useChaosMap,firstRun;
    logMap::iterator parItr;
    vector<double>::iterator probItr;
    
    //found word
    string seedWord;
    string foundWord;
    string firstWord;
    
    string keyWord;
    string nextWord;
    bool chainIsReady;
    
    //markov mode
    //cumulative token or N words away
    MarkovMode mmode;
  
    
};
#endif /* MarkovWordChain_hpp */
