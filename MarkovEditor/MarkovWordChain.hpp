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

class MarkovWordChain
{
public:
    MarkovWordChain();
    MarkovWordChain(string fname, string seedWord, int numLevels,float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,int mt);
    void CreateMarkovWordChain();
    void ComputeProbabilities();
    void LoadChaosMap(float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,mapType mpType);
    void setMyMtype(int mt);
    void LoadTextIntoVector(string fname);
    vector<string> splitString(const std::string& text, const std::string& delims);
    string GetFoundWord();
    string GetFirstWord();
    
    
    void GetNextProbChain();
    void GetNextWordInProbChain();
    
    void ClearMarkovChain();
    void SetupMarkovChain();
    //inlines
    inline void setUseChaosMap(bool uch) { useChaosMap = uch;}
    inline bool getUseChaosMap() { return useChaosMap;}
    inline void setSeedWord(string sdw) { firstWord = sdw; }
    inline string getSeedWord() { return firstWord; }
    inline void setNumLevels(int nl) { nlevels = nl; }
    inline int getNumLevels() { return nlevels; }
    inline bool getChainIsReady() { return chainIsReady;}
    
    
private:
    typedef map<string,float> wordMap;
    typedef map<float,string> rWordMap;
    typedef map<int,wordMap> levelMap;
    typedef map<int,rWordMap> rLevelMap;
    typedef map<string,wordMap> markovChain;
    typedef map<string,rWordMap> markovRChain;
    
    vector<string> Words;
    
    markovChain myMarkovChain;
    markovRChain myProbChain;
    
    int nlevels;
    markovChain::iterator itMrk;
    markovRChain::iterator itRMrk;
    markovChain::iterator itMrkActive;
    //chaos stuff
    ChaosMap *myMap;
    mapType  myMtype;
    float parBegin,parEnd,x,mpar;
    int steps,itr,kit,mtype;
    bool useChaosMap,firstRun;
    logMap::iterator parItr;
    vector<double>::iterator probItr;
    
    //found word
    string foundWord;
    string firstWord;
    
    bool chainIsReady;
  
    
};
#endif /* MarkovWordChain_hpp */
