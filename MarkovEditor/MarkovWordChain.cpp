//
//  MarkovWordChain.cpp
//  example_aubioDemo
//
//  Created by Jonathan B Gallagher on 5/6/17.
//
//

#include "MarkovWordChain.hpp"
bool mySort2(int i , int j)  { return (i<j); }


void MarkovWordChain::setMyMtype(int mt)
{
    if(mt == 0)
        myMtype = LOG;
    if(mt == 1)
        myMtype = EXP;
    if(mt == 2)
        myMtype = MOUSE;
    if(mt == 3)
        useChaosMap = false;
}

MarkovWordChain::MarkovWordChain()
{
    firstRun = true;
    chainIsReady = false;
    firstWordInChain = true;
    mmode = BY_WORD;
    srand((unsigned int)time(NULL));
}

MarkovWordChain::MarkovWordChain(string fname, string seedWord, int numLevels,float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,int mt)
{
    useChaosMap = true;
    firstRun = true;
    firstWordInChain = true;
    nlevels = numLevels;
    setMyMtype(mt);
    LoadTextIntoVector(fname);
    CreateMarkovWordChain();
    ComputeProbabilities();
    LoadChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    firstWord.append(seedWord);
    printf("MarkovChain: %s %s\n",fname.c_str(),firstWord.c_str());
}

void MarkovWordChain::ClearMarkovData()
{
    ClearMarkovChain();
    if(useChaosMap)
        DeleteChaosMap();
    Letters.clear();
    Words.clear();
    Distribution.clear();
}

void MarkovWordChain::ClearMarkovChain()
{
    for(markovChain::iterator chIt = myMarkovChain.begin(); chIt != myMarkovChain.end(); chIt++) {
        chIt->second.clear();
    }
    myMarkovChain.clear();
    for(markovChain::iterator chIt = myMarkovChainCopy.begin(); chIt != myMarkovChainCopy.end(); chIt++) {
        chIt->second.clear();
    }
    myMarkovChainCopy.clear();

    for(markovRChain::iterator chrIt = myProbChain.begin(); chrIt != myProbChain.end(); chrIt++) {
        chrIt->second.clear();
    }
    myProbChain.clear();
    chainIsReady = false;
}

void MarkovWordChain::SetupMarkovChain()
{
    if(getChainIsReady())
        ClearMarkovChain();
    CreateMarkovWordChain();
    ComputeProbabilities();
    chainIsReady = true;
    
    
}

void MarkovWordChain::SetupCharMarkovChain()
{
    if(getChainIsReady())
        ClearMarkovChain();
    CreateMarkovCharChain();
    ComputeProbabilities();
    chainIsReady = true;
    
    
}


vector<string> MarkovWordChain::splitString(const string &text, const string &delims)
{
    std::vector<std::string> tokens;
    std::size_t start = text.find_first_not_of(delims), end = 0;
    
    while((end = text.find_first_of(delims, start)) != std::string::npos)
    {
        tokens.push_back(text.substr(start, end - start));
        start = text.find_first_not_of(delims, end);
    }
    if(start != std::string::npos)
        tokens.push_back(text.substr(start));
    
    return tokens;

}

void MarkovWordChain::LoadTextIntoVectorByChar(string fname)
{
    string aLine;
    regex spc("\\s+");
    
    ifstream file(fname);
    if (!file) {
        cout << "unable to open file";
        exit(1);
    }
    string aChar;
    
    aChar = file.get();
    while(*aChar.c_str() != EOF) {
       // string str = string(&aChar);
        Words.push_back(aChar);
        aChar = file.get();
        printf("FU: %s\n",aChar.c_str());
    }
    file.close();
    
    printf("Number of Chars: %lu\n",Words.size());
}


void MarkovWordChain::LoadTextIntoVector(string fname)
{
    string aLine;
    vector<string> wordsInLine;
    regex spc("\\s+");
    regex tab("\\t+");
    regex rline("\\r+");
    regex nline("\\n+");
    regex quot("\"");
    
    regex ex("\\!");
    regex at("\\@");
    regex hs("\\#");
    regex ds("\\$");
    regex ps("\\%");
    regex cs("\\^");
    regex am("\\&");
    regex ms("\\*");
    regex lp("\\(");
    regex rp("\\)");
    regex pd("\\.");
    regex cm("\\,");
    regex qm("\\?");
    regex smc("\\;");
    regex cl("\\:");
    regex bs("\\");
    regex fs("\\/");
    regex pls("\\+");
    regex eqs("\\=");
    regex rbr("\\[");
    regex lbr("\\]");
    regex rbbr("\\{");
    regex lbbr ("\\}");
    regex ltx("\\<");
    regex gts("\\>");
    
   
    ifstream file(fname);
    if (!file) {
        cout << "unable to open file";
        exit(1);
    }
    while(getline(file,aLine)) {
        if(getRemoveQuot()) {
            aLine = regex_replace(aLine, quot, "");
        }
        if(getRemovePunc()) {
            aLine = regex_replace(aLine, quot, "");
            aLine = regex_replace(aLine, ex, "");
            aLine = regex_replace(aLine, at, "");
            aLine = regex_replace(aLine, hs, "");
            aLine = regex_replace(aLine, ds, "");
            aLine = regex_replace(aLine, ps, "");
            aLine = regex_replace(aLine, cs, "");
            aLine = regex_replace(aLine, am, "");
            aLine = regex_replace(aLine, ms, "");
           // aLine = regex_replace(aLine, lp, "");
           // aLine = regex_replace(aLine, rp, "");
            aLine = regex_replace(aLine, pd, "");
            aLine = regex_replace(aLine, cm, "");
            aLine = regex_replace(aLine, qm, "");
            aLine = regex_replace(aLine, smc, "");
            aLine = regex_replace(aLine, cl, "");
            //aLine = regex_replace(aLine, bs, "");
            //aLine = regex_replace(aLine, fs, "");
            aLine = regex_replace(aLine, pls, "");
            aLine = regex_replace(aLine, eqs, "");
           // aLine = regex_replace(aLine, rbr, "");
           // aLine = regex_replace(aLine, lbr, "");
           // aLine = regex_replace(aLine, rbbr, "");
           // aLine = regex_replace(aLine, lbbr, "");
            aLine = regex_replace(aLine, ltx, "");
            aLine = regex_replace(aLine, gts, "");
        }
        aLine = regex_replace(aLine, tab, " ");
        aLine = regex_replace(aLine, nline, " ");
        aLine = regex_replace(aLine, rline, " ");
        aLine = regex_replace(aLine, spc, " ");
        for(string::iterator itS = aLine.begin(); itS != aLine.end(); itS++) {
            string str;
            str = *itS;
            Letters.push_back(str);
        }
        wordsInLine = splitString(aLine, " ");
        for(vector<string>::iterator itS =  wordsInLine.begin(); itS != wordsInLine.end(); itS++) {
            Words.push_back(*itS);
        }
        wordsInLine.clear();
    }
    
}

void MarkovWordChain::LoadTextFromWordsToLetters()
{
    for(vector<string>::iterator itS = Words.begin(); itS != Words.end(); itS++) {
        for(string::iterator itSS = itS->begin(); itSS != itS->end(); itSS++) {
            string str; // = &*itSS;
            str = *itSS;
            Letters.push_back(str);
        }
    }
}

void MarkovWordChain::CreateMarkovWordChain()
{
    int clevel = 0;
    for(int i=0; i<nlevels; i++) {
        clevel = nlevels - i;
        for(int j=0; j<Words.size()-clevel; j++) {
            string activeString;
            for(int k=0; k<clevel; k++) {
                if(k > 0)
                    activeString.append(" ");
                activeString.append(Words[j+k]);
            }
            itMrk = myMarkovChain.find(activeString);
            itMrkCp = myMarkovChainCopy.find(activeString);
            
            if(itMrk == myMarkovChain.end()) {
                wordMap *aWordMap = new wordMap;
                aWordMap->insert(std::pair<string,float>(Words[j+clevel],1.0));
                myMarkovChain.insert(std::pair<string,wordMap>(activeString,*aWordMap));
                myMarkovChainCopy.insert(std::pair<string,wordMap>(activeString,*aWordMap));
                delete aWordMap;
            } else {
                wordMap::iterator itMM = itMrk->second.find(Words[j+clevel]);
                wordMap::iterator itMMCp = itMrkCp->second.find(Words[j+clevel]);
                if(itMM == itMrk->second.end()) {
                    wordMap *aWordMap = &itMrk->second;
                    aWordMap->insert(std::pair<string,float>(Words[j+clevel],1.0));
                    wordMap *aWordMapCp = &itMrkCp->second;
                    aWordMapCp->insert(std::pair<string,float>(Words[j+clevel],1.0));
                } else {
                    itMM->second = itMM->second + 1.0;
                    itMMCp->second = itMMCp->second + 1.0;
                }
            }
        }
    }
    printf("Finished building markov chain: %lu\n",myMarkovChain.size());
}

void MarkovWordChain::CreateMarkovCharChain()
{
    printf("Letters size: %lu %d\n",Letters.size(),nlevels);
    for(int i=0; i<Letters.size()-nlevels; i++) {
        string keyString;
        string nextString;
        //build up the keyString and nextString
        for(int j=0; j<nlevels; j++) {
            keyString.append(Letters[i+j]);
        }
        //get next nlevel characters
        for(int j=0; j<nlevels; j++) {
            nextString.append(Letters[i+nlevels+j]);
        }
        itMrk = myMarkovChain.find(keyString);
  
        if(itMrk == myMarkovChain.end()) {
            wordMap *aWordMap = new wordMap;
            aWordMap->insert(std::pair<string,float>(nextString,1.0));
            myMarkovChain.insert(std::pair<string,wordMap>(keyString,*aWordMap));
            delete aWordMap;
        } else {
            wordMap::iterator itMM = itMrk->second.find(nextString);
            if(itMM == itMrk->second.end()) {
                wordMap *aWordMap = &itMrk->second;
                aWordMap->insert(std::pair<string,float>(nextString,1.0));
            } else {
                itMM->second = itMM->second + 1.0;
            }
        }
    }
    
    printf("Finished building markov CHAR chain: %lu fw: %s\n",myMarkovChain.size(),GetFirstWord().c_str());
}


void MarkovWordChain::ComputeProbabilities()
{
    for(markovChain::iterator itM = myMarkovChain.begin(); itM != myMarkovChain.end(); itM++) {
        float sum = 0.0;
        for(wordMap::iterator itMM = itM->second.begin(); itMM != itM->second.end(); itMM++) {
            sum += itMM->second;
        }
        for(wordMap::iterator itMM = itM->second.begin(); itMM != itM->second.end(); itMM++) {
            itMM->second = itMM->second/sum;
        }
    }
    
    for(markovChain::iterator itM = myMarkovChain.begin(); itM != myMarkovChain.end(); itM++) {
        //go over word map pushing back values in tmpVector
        vector<float> tmpVector;
        for(wordMap::iterator itMM = itM->second.begin(); itMM != itM->second.end(); itMM++) {
            tmpVector.push_back(itMM->second);
        }
        //sort the vector
        std::sort (tmpVector.begin(), tmpVector.end(), mySort2);
        map<float,int> uniqProbs;
        for(vector<float>::iterator itV = tmpVector.begin(); itV != tmpVector.end(); itV++) {
            map<float,int>::iterator itF = uniqProbs.find(*itV);
            if(itF == uniqProbs.end()) {
                uniqProbs[*itV] = 0;
            } else {
                uniqProbs[*itV] = uniqProbs[*itV] + 1;
            }
        }
        float lastProb = 0.0;
        rWordMap *aRWordMap = new rWordMap;
        for(map<float,int>::iterator itV = uniqProbs.begin(); itV != uniqProbs.end(); itV++) {
            for(wordMap::iterator itMM = itM->second.begin(); itMM != itM->second.end(); itMM++) {
                if(itMM->second == itV->first) {
                    aRWordMap->insert(std::pair<float,string>((lastProb+itV->first),itMM->first));
                    lastProb = (itV->first+lastProb);
                }
            }
        }
        myProbChain.insert(std::pair<string,rWordMap>(itM->first,*aRWordMap));
        delete aRWordMap;
        tmpVector.clear();
        uniqProbs.clear();
    }
}

void MarkovWordChain::LoadChaosMap(float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,mapType myMtype)
{
    myMap = new ChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    myMap->Run();
}

void MarkovWordChain::LoadChaosMap()
{
    myMap = new ChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    myMap->Run();
}

void MarkovWordChain::DeleteChaosMap()
{
    setUseChaosMap(false);
    delete myMap;
}

void MarkovWordChain::MakeDistribution(int num, float f1, float f2)
{
    distribCount = 0;
    if(Distribution.size() != 0)
        Distribution.clear();
    float rnum = 0.0;
    if(getMarkovDist() == RANDOM) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::uniform_real_distribution<float> rdist(f1,f2);
        rdist.reset();
        for(int i=0; i<num; i++) {
            rnum = rdist(generator);
            Distribution.push_back(rnum);
        }

    }
    if(getMarkovDist() == LOGISTIC_MAP || getMarkovDist() == EXP_MAP || getMarkovDist() == MOUSE_MAP) {
        LoadChaosMap();
        for(logMap::iterator lmIt = myMap->coordMap.begin(); lmIt != myMap->coordMap.end(); lmIt++) {
            for(vector<double>::iterator itV = lmIt->second.begin(); itV != lmIt->second.end(); itV++) {
                Distribution.push_back(*itV);
            }
        }
    }
    if(getMarkovDist() == NORMAL) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::normal_distribution<float> ndist(f1,f2);
        for(int i=0; i<num; i++) {
            rnum = ndist(generator);
            Distribution.push_back(rnum);
        }
        NormalizeDistribution();
    }
    if(getMarkovDist() == BINOMIAL) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::binomial_distribution<int> bdist(f1,f2);
        for(int i=0; i<num; i++) {
            rnum = (float)bdist(generator);
            Distribution.push_back(rnum);
        }
        NormalizeDistribution();
    }
    if(getMarkovDist() == EXPONENTIAL) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::exponential_distribution<float> edist(f1);
        for(int i=0; i<num; i++) {
            rnum = edist(generator);
            Distribution.push_back(rnum);
        }
        NormalizeDistribution();
    }
    if(getMarkovDist() == GEOMETRIC) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::geometric_distribution<int> gdist(f1);
        for(int i=0; i<num; i++) {
            rnum = (float)gdist(generator);
            Distribution.push_back(rnum);
        }
        NormalizeDistribution();
    }
    if(getMarkovDist() == LOGNORMAL) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::lognormal_distribution<float> ldist(f1,f2);
        for(int i=0; i<num; i++) {
            rnum = ldist(generator);
            Distribution.push_back(rnum);
        }
        NormalizeDistribution();
    }

    if(getMarkovDist() == POISSON) {
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator ((unsigned)seed);
        std::poisson_distribution<int> pdist(f1);
        for(int i=0; i<num; i++) {
            rnum = (float)pdist(generator);
            Distribution.push_back(rnum);
        }
        NormalizeDistribution();
    }
}

void MarkovWordChain::NormalizeDistribution()
{
    float min = 9999.0;
    float max = -9999.0;
    //get min/max
    for(int i=0; i<(int)Distribution.size(); i++) {
        if(Distribution[i] < min)
            min = Distribution[i];
        if(Distribution[i] > max)
            max = Distribution[i];
    }
    //normalize
    for(int i=0; i<(int)Distribution.size(); i++) {
        Distribution[i] = (Distribution[i] - min)/(max-min);
    }
    
}

float MarkovWordChain::GetProbability()
{
    float rnum = 0.0;
    
    rnum = Distribution[distribCount];
    distribCount++;
    
    if(distribCount == (int)Distribution.size())
        distribCount = 0;

    return rnum;
}

bool MarkovWordChain::GetNextProbChain()
{
    //this function sets the iterator in the markov chain
    //which will point to a map of words as keys with probabilities as values
    //if this is multi-level and the end of the chain, the last
    //word in firstWord, is parsed out and used to start a new first
    //level chain
    bool foundLinkInChain;
    itRMrk = myProbChain.find(firstWord);
    if(itRMrk == myProbChain.end() && firstWordInChain) {
        foundLinkInChain = false;
    } else {
        if(itRMrk == myProbChain.end()) {
            vector<string> splitWords = splitString(firstWord, " ");
            int idx = (int)splitWords.size()-1;
            itRMrk = myProbChain.find(splitWords[idx]);
            firstWord.clear();
            firstWord = splitWords[idx];
        }
        firstWordInChain = false;
        foundLinkInChain = true;
    }
    return foundLinkInChain;
 }

bool MarkovWordChain::GetNextProbChainByChar()
{
    bool foundLinkInChain;
    itRMrk = myProbChain.find(firstWord);
    if(itRMrk == myProbChain.end()) {
        printf("DIDn't find fw: %s\n",firstWord.c_str());
    }
    if(itRMrk == myProbChain.end() && firstWordInChain) {
        foundLinkInChain = false;
    } else {
        if(itRMrk == myProbChain.end()) {
            //use last word
            itRMrk = myProbChain.find(lastFoundWord);
            printf("Using lastWOrd as bail out... %s\n",lastFoundWord.c_str());
        } else {
            firstWordInChain = false;
            foundLinkInChain = true;
        }
    }
    return foundLinkInChain;
}

void MarkovWordChain::GetNextWordInProbChain()
{
    float rnum = 0.0;//((double) rand())/RAND_MAX;
    float prevProb = 0.0;
    
    rnum = GetProbability();
    
    for(rWordMap::iterator itMM = itRMrk->second.begin(); itMM != itRMrk->second.end(); itMM++) {
        if(rnum > prevProb && rnum < itMM->first) {
            foundWord.clear();
            foundWord.append(itMM->second);
            firstWord.append(" ");
            firstWord.append(itMM->second);
        }
        prevProb = itMM->first;
    }
}

void MarkovWordChain::GetNextStrInProbChainByChar()
{
    float rnum = 0.0;//((double) rand())/RAND_MAX;
    float prevProb = 0.0;
    
    rnum = GetProbability();
    
    for(rWordMap::iterator itMM = itRMrk->second.begin(); itMM != itRMrk->second.end(); itMM++) {
        if(rnum > prevProb && rnum < itMM->first) {
            lastFoundWord.clear();
            lastFoundWord.append(firstWord);
            foundWord.clear();
            foundWord.append(itMM->second);
            firstWord.clear();
            firstWord.append(itMM->second);
        }
        prevProb = itMM->first;
    }
}


void MarkovWordChain::SetWordMapPointer(string word)
{
    markovChain::iterator itCh = myMarkovChain.find(word);
    wMapPointer = &itCh->second;
}

string MarkovWordChain::GetFoundWord()
{
    return foundWord;
}

string MarkovWordChain::GetFirstWord()
{
    return firstWord;
}

float MarkovWordChain::GetNextWordCount(string bwrd, string wrd)
{
    float count = 0.0;
    itMrkCp = myMarkovChainCopy.find(bwrd);
    if(itMrkCp != myMarkovChainCopy.end()) {
        wordMap::iterator itM = itMrkCp->second.find(wrd);
        if(itM != itMrkCp->second.end()) {
            count = itM->second;
        }
    }
    return count;
}

float MarkovWordChain::GetNextWordPerc(string bwrd, string wrd)
{
    float perc = 0.0;
    itMrk = myMarkovChain.find(bwrd);
    if(itMrk != myMarkovChain.end()) {
        wordMap::iterator itM = itMrk->second.find(wrd);
        if(itM != itMrk->second.end()) {
            perc = itM->second;
        }
    }
    return perc;
}

float MarkovWordChain::GetBaseWordSum(string wrd)
{
    float sum = 0.0;
    itMrkCp = myMarkovChainCopy.find(wrd);
    if(itMrkCp != myMarkovChainCopy.end()) {
        for(wordMap::iterator itM = itMrkCp->second.begin(); itM != itMrkCp->second.end(); itM++) {
            sum = sum + itM->second;
        }
    }
    return sum;
}