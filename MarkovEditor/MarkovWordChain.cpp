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
    myMarkovChain.clear();
    myProbChain.clear();
    chainIsReady = false;
}

void MarkovWordChain::SetupMarkovChain()
{
    CreateMarkovWordChain();
    ComputeProbabilities();
    if(getUseChaosMap())
        LoadChaosMap(parBegin,parEnd,x,steps,itr,kit,mpar,myMtype);
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
        wordsInLine = splitString(aLine, " ");
        for(vector<string>::iterator itS =  wordsInLine.begin(); itS != wordsInLine.end(); itS++) {
            Words.push_back(*itS);
        }
        wordsInLine.clear();
    }
    
}

void MarkovWordChain::CreateNLevelMarkovChain()
{
    for(int i=0; i<Words.size(); i++) {
        string activeString;
        activeString.append(Words[i]);
        for(int j=1; j<=nlevels; j++) {
            markovChainNLevel::iterator itChain = myMarkovChainNLevel.find(activeString);
            //check to see if a levelmap has been created for activeString, if not create
            if(itChain == myMarkovChainNLevel.end()) {
                levelMap *aLevelMap = new levelMap;
                wordMap *aWordMap = new wordMap;
                aWordMap->insert(std::pair<string,float>(Words[j+i],1.0));
                aLevelMap->insert(std::pair<int,wordMap>(j,*aWordMap));
                myMarkovChainNLevel.insert(std::pair<string,levelMap>(activeString,*aLevelMap));
                delete aWordMap;
                delete aLevelMap;
            } else {
                //check for wordMap at this level, if not create, else addto
                levelMap::iterator itLM = itChain->second.find(j);
                if(itLM == itChain->second.end()) {
                    wordMap *aWordMap = new wordMap;
                    aWordMap->insert(std::pair<string,float>(Words[j+i],1.0));
                    itChain->second.insert(std::pair<int,wordMap>(j,*aWordMap));
                    delete aWordMap;
                } else {
                    //check for wordMap, else addto wordMap
                    wordMap::iterator itWM = itLM->second.find(Words[j+i]);
                    if(itWM == itLM->second.end()) {
                        wordMap *aWordMap = new wordMap;
                        aWordMap->insert(std::pair<string,float>(Words[j+i],1.0));
                        itChain->second.insert(std::pair<int,wordMap>(j,*aWordMap));
                        delete aWordMap;
                    } else {
                        itWM->second = itWM->second + 1;
                    }
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
                    itMM->second = itMM->second + 1;
                }
            }
        }
    }
    printf("Finished building markov chain: %lu\n",myMarkovChain.size());
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

void MarkovWordChain::GetNextWordInProbChain()
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
    float prevProb = 0.0;
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

string MarkovWordChain::GetFoundWord()
{
    return foundWord;
}

string MarkovWordChain::GetFirstWord()
{
    return firstWord;
}

