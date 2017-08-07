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
    mmode = BY_CHAR;
    curlevel = 1;
    srand((unsigned int)time(NULL));
}

MarkovWordChain::MarkovWordChain(string fname, string seedWord, int numLevels,float parBegin, float parEnd, float x,int steps,int itr,int kit,float mpar,int mt)
{
    useChaosMap = true;
    firstRun = true;
    nlevels = numLevels;
    setMyMtype(mt);
    LoadTextIntoVector(fname);
    CreateMarkovWordChain();
    ComputeProbabilities();
    LoadChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    firstWord.append(seedWord);
    printf("MarkovChain: %s %s\n",fname.c_str(),firstWord.c_str());
}

void MarkovWordChain::ClearMarkovChain()
{
    for(markovChain::iterator chIt = myMarkovChain.begin(); chIt != myMarkovChain.end(); chIt++) {
        chIt->second.clear();
    }
    myMarkovChain.clear();
    for(markovRChain::iterator chrIt = myProbChain.begin(); chrIt != myProbChain.end(); chrIt++) {
        chrIt->second.clear();
    }
    myProbChain.clear();
    chainIsReady = false;
    curlevel = 1;
    if(getUseChaosMap()) {
        delete myMap;
        setUseChaosMap(false);
    }
}

void MarkovWordChain::SetupMarkovChain()
{
    CreateMarkovWordChain();
    ComputeProbabilities();
    if(getUseChaosMap())
        LoadChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    chainIsReady = true;
    firstRun = true;
}

void MarkovWordChain::SetupCharMarkovChain()
{
    CreateMarkovCharChain();
    ComputeProbabilities();
    if(getUseChaosMap())
        LoadChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    chainIsReady = true;
    firstRun = true;
}

void MarkovWordChain::ClearNLevelMarkovChain()
{
    for(markovChainNLevel::iterator chIt = myMarkovChainNLevel.begin(); chIt != myMarkovChainNLevel.end(); chIt++) {
        for(levelMap::iterator lmIt = chIt->second.begin(); lmIt != chIt->second.end(); lmIt++) {
            lmIt->second.clear();
        }
        chIt->second.clear();
    }
    myMarkovChainNLevel.clear();
    for(markovProbChainNLevel::iterator chpIt = myProbChainNLevel.begin(); chpIt != myProbChainNLevel.end(); chpIt++) {
        for(rLevelMap::iterator lmrIt = chpIt->second.begin(); lmrIt != chpIt->second.end(); lmrIt++) {
            lmrIt->second.clear();
        }
        chpIt->second.clear();
    }
    myProbChainNLevel.clear();
    curlevel = 1;
    chainIsReady = false;
    if(getUseChaosMap()) {
        delete myMap;
        setUseChaosMap(false);
    }
}

void MarkovWordChain::SetupNLevelMarkovChain()
{
    CreateNLevelMarkovChain();
    ComputeNLevelProbabilities();
    if(getUseChaosMap())
        LoadChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
    chainIsReady = true;
    firstRun = true;
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

    ifstream file(fname);
    if (!file) {
        cout << "unable to open file";
        exit(1);
    }
    while(getline(file,aLine)) {
        aLine = regex_replace(aLine, quot, "");
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

void MarkovWordChain::CreateNLevelMarkovChain()
{
    for(int i=0; i<Words.size()-(nlevels+1); i++) {
        string activeString;
        activeString.append(Words[i]);
        markovChainNLevel::iterator itChain = myMarkovChainNLevel.find(activeString);
        //if iterator is at end, this is first record for this word
        //create N level deep chain
        if(itChain == myMarkovChainNLevel.end()) {
            //create a levelMap to go with word i
            levelMap *aLevelMap = new levelMap;
            //insert the string and levelmap
            myMarkovChainNLevel.insert(std::pair<string,levelMap>(activeString,*aLevelMap));
            //copy was made delete levelmap
            delete aLevelMap;
            //get an iterator to N Level markovchain, second member will be levelmap
            markovChainNLevel::iterator itNLevelChainIt = myMarkovChainNLevel.find(activeString);
            for(int j=1; j<=nlevels; j++) {
                //create a wordMap for each level j
                wordMap *aWordMap = new wordMap;
                aWordMap->insert(std::pair<string,float>(Words[j+i],1.0));
                itNLevelChainIt->second.insert(std::pair<int,wordMap>(j,*aWordMap));
                delete aWordMap;
            }
        } else {
            //add to exisiting chain, itChain->second is levelmap
            for(int j=1; j<=nlevels; j++) {
                //get wordMap iterator for each level j, if entry existsing increment count,
                //else insert
                wordMap::iterator wrdMapIt = itChain->second[j].find(Words[i+j]);
                if(wrdMapIt == itChain->second[j].end()) {
                   itChain->second[j].insert(std::pair<string,float>(Words[i+j],1.0));
                } else {
                    wrdMapIt->second = wrdMapIt->second + 1.0;
                }
            }
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
            if(itMrk == myMarkovChain.end()) {
                wordMap *aWordMap = new wordMap;
                aWordMap->insert(std::pair<string,float>(Words[j+clevel],1.0));
                myMarkovChain.insert(std::pair<string,wordMap>(activeString,*aWordMap));
                delete aWordMap;
            } else {
                wordMap::iterator itMM = itMrk->second.find(Words[j+clevel]);
                if(itMM == itMrk->second.end()) {
                    wordMap *aWordMap = &itMrk->second;
                    aWordMap->insert(std::pair<string,float>(Words[j+clevel],1.0));
                } else {
                    itMM->second = itMM->second + 1.0;
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
    //    printf("keyString: %s string: %s\n",keyString.c_str(),nextString.c_str());
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

void MarkovWordChain::ComputeNLevelProbabilities()
{
    for(markovChainNLevel::iterator itNL = myMarkovChainNLevel.begin(); itNL != myMarkovChainNLevel.end(); itNL++) {
        for(levelMap::iterator itLM = itNL->second.begin(); itLM != itNL->second.end(); itLM++) {
            float sum = 0.0;
            for(wordMap::iterator itMM = itLM->second.begin(); itMM != itLM->second.end(); itMM++) {
                sum += itMM->second;
            }
            for(wordMap::iterator itMM = itLM->second.begin(); itMM != itLM->second.end(); itMM++) {
                itMM->second = itMM->second/sum;
            }
        }
    }
    for(markovChainNLevel::iterator itNL = myMarkovChainNLevel.begin(); itNL != myMarkovChainNLevel.end(); itNL++) {
        rLevelMap *aRLevelMap = new rLevelMap;
        for(levelMap::iterator itM = itNL->second.begin(); itM != itNL->second.end(); itM++) {
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
            aRLevelMap->insert(std::pair<int,rWordMap>(itM->first,*aRWordMap));
            delete aRWordMap;
            tmpVector.clear();
            uniqProbs.clear();
        }
        myProbChainNLevel.insert(std::pair<string,rLevelMap>(itNL->first,*aRLevelMap));
        delete aRLevelMap;
    } //end iterate over NLevel markov chain
    printf("Finished bulding markov chain with %lu unique words!\n",myProbChainNLevel.size());
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

float MarkovWordChain::GetProbability()
{
    float rnum = 0.0;//((double) rand())/RAND_MAX;
    if(useChaosMap) {
        if(firstRun) {
            firstRun = false;
            parItr = myMap->coordMap.begin();
            probItr = parItr->second.begin();
        }  else {
            probItr++;
            if(probItr == parItr->second.end()) {
                parItr++;
                if(parItr == myMap->coordMap.end()) {
                    parItr = myMap->coordMap.begin();
                    probItr = parItr->second.begin();
                } else {
                    probItr = parItr->second.begin();
                }
            }
        }
        rnum = *probItr;
    } else {
        rnum = (float)rand()/RAND_MAX;
    }

    return rnum;
}

void MarkovWordChain::GetNextProbChain()
{
    //this function sets the iterator in the markov chain
    //which will point to a map of words as keys with probabilities as values
    //if this is multi-level and the end of the chain, the last
    //word in firstWord, is parsed out and used to start a new first
    //level chain
    itRMrk = myProbChain.find(firstWord);
    if(itRMrk == myProbChain.end()) {
        vector<string> splitWords = splitString(firstWord, " ");
        int idx = (int)splitWords.size()-1;
        itRMrk = myProbChain.find(splitWords[idx]);
        firstWord.clear();
        firstWord = splitWords[idx];
    }
 }

void MarkovWordChain::GetNextProbChainByChar()
{
    itRMrk = myProbChain.find(firstWord);
    if(itRMrk == myProbChain.end()) {
        printf("First Word: %s not FOUND First word size: %lu!\n",firstWord.c_str(),firstWord.size());
        for(markovRChain::iterator itM = myProbChain.begin(); itM != myProbChain.end(); itM++) {
            printf("WORD: %s mapSIze: %lu\n",itM->first.c_str(),itM->second.size());
        }
    }
}

void MarkovWordChain::GetNextNLevelProbChain()
{
    
    if(curlevel == 1) {
        itNRMrk = myProbChainNLevel.find(firstWord);
       
    }
    
    itRLvl = itNRMrk->second.find(curlevel);
    
    curlevel = curlevel+1;
    
    if(curlevel == nlevels+1) {
        curlevel = 1;
        
    }
    
    printf("Currentl Level is %d wmap size is %lu\n",curlevel,itRLvl->second.size());
    
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
            foundWord.clear();
            foundWord.append(itMM->second);
            firstWord.clear();
            firstWord.append(itMM->second);
        }
        prevProb = itMM->first;
    }

}
void MarkovWordChain::GetNextWordInNLevelProbChain()
{
    float rnum = 0.0;//((double) rand())/RAND_MAX;
    float prevProb = 0.0;
    
    rnum = GetProbability();
    for(rWordMap::iterator itMM = itRLvl->second.begin(); itMM != itRLvl->second.end(); itMM++) {
        printf("%s %f %f %f\n",itMM->second.c_str(),itMM->first,prevProb,rnum);
        if(rnum > prevProb && rnum < itMM->first) {
            foundWord.clear();
            foundWord.append(itMM->second);
            firstWord.clear();
            firstWord.append(itMM->second);
        }
        prevProb = itMM->first;
    }
}

string MarkovWordChain::GetFoundWord()
{
    return foundWord;
}

string MarkovWordChain::GetFirstWord()
{
    return firstWord;
}

