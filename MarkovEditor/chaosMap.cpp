#include "chaosMap.h"
bool mySort (double i,double j) { return (i<j); }
ChaosMap::ChaosMap(double bpar, double epar, double initX, int nsteps, int iter, int keep_itr, double mpar, mapType mType)
{

	setBeginPar(bpar);
	setEndPar(epar);
	setX(initX);
	setNSteps(nsteps);
	setIter(iter);
	setKeepIter(keep_itr);
	setMousePar(mpar);
	myMapType = mType;
	mTol = 0.0001;
    aMax = -9999.0;
    aMin = 9999.0;
    yMax = -9999.0;
    yMin = 9999.0;
}

void ChaosMap::Run()
{

   ComputeIterates();
   ComputePeriods();
   //ComputeConstant();
   NormalizeData();
}

double ChaosMap::GetY(double xIn)
{
   double yOut = 0.0;
   switch(myMapType) {
	case LOG:
	  yOut = m_a*xIn*(1.0-xIn);
	break;
	case EXP:
	  yOut = pow((m_a + 1.0/xIn),(xIn/m_a));
	break;
	case MOUSE:
	  yOut = exp(-myMPar*pow(xIn,2)) + m_a;
	break;
        default:
	break;
   }
return yOut;
}
	
void ChaosMap::ComputeIterates()
{
 
   double x = myX; 
   double y;
   m_istep = (myEPar-myBPar)/myNSteps;
   for(int i=0; i<myNSteps; i++) {
	m_a = myBPar + (m_istep*i);
	for(int k=0; k<myIter; k++) {
	  y = GetY(x); 
	  x = y;
	  if(k >= myKeepIter)
		myLogMap[m_a].push_back(x);
	}
   }
}

void ChaosMap::ComputePeriods()
{

   for(logMap::iterator itM = myLogMap.begin(); itM != myLogMap.end(); itM++) {
	int count = 0;
	int pcount = 1;
	double lastX = 0.0;
	std::vector<double> xIteratesCopy(itM->second);
	std::sort (xIteratesCopy.begin(), xIteratesCopy.end(), mySort);
	for(vector<double>::iterator itV = xIteratesCopy.begin(); itV != xIteratesCopy.end(); itV++) {
	   if(count > 0) {
	     if(fabs(*itV-lastX) > mTol)
	       pcount = pcount + 1;
	   }
	   lastX = *itV;
	   count++;
	}
	periodMap[itM->first] = pcount;
   }
}

void ChaosMap::ComputeConstant()
{

   int lastP = 0;
   int count = 0;
   map<double,int> rPars;
   for(map<double,int>::iterator itM = periodMap.begin(); itM != periodMap.end(); itM++) {
	if(count > 0) {
	   if(itM->second > lastP) {
		rPars[itM->first] = itM->second;
	   }
	}
	count++;
	lastP = itM->second;
   }
}
   
	

void ChaosMap::NormalizeData() 
{

     //get min max
     for(logMap::iterator itM = myLogMap.begin(); itM != myLogMap.end(); itM++) {
        if(itM->first > aMax)
          aMax = itM->first;
        if(itM->first < aMin)
          aMin = itM->first;
        for(vector<double>::iterator itV = itM->second.begin(); itV != itM->second.end(); itV++) {
           if(*itV > yMax)
             yMax = *itV;
           if(*itV < yMin)
             yMin = *itV;
        }
     }
     //normalize between 0.0 and 1.0
     double nPar,yVal;
     for(logMap::iterator itM = myLogMap.begin(); itM != myLogMap.end(); itM++) {
        nPar = ((double)itM->first-aMin)/(aMax-aMin);
        int pcount = 0;
        for(vector<double>::iterator itV = itM->second.begin(); itV != itM->second.end(); itV++) {
           if(pcount < periodMap[itM->first]) {
                yVal = (*itV-yMin)/(yMax-yMin);
                coordMap[nPar].push_back(yVal);
           }
           pcount++;
        }
     }
}


