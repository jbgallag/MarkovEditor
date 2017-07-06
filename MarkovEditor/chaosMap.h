#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>

using namespace std;

typedef map<double,vector<double> > logMap;

enum mapType {LOG,EXP,MOUSE};

class ChaosMap {

public:

	ChaosMap(double beginPar, double endPar, double initX, int nSteps, int iterations, int keep_it, double mPar, mapType mtype);
        inline void setBeginPar(double beginPar) { myBPar = beginPar;};
        inline double getBeginPar() { return myBPar;};
	inline void setEndPar(double endPar) { myEPar = endPar;};
        inline double getEndPar() { return myEPar;};	
	inline void setX(double initX) { myX = initX;};
        inline double getX() { return myX;};
        inline void setNSteps(int nSteps) { myNSteps = nSteps;};
        inline int getNSteps() { return myNSteps;};
	inline void setIter(int iterations) { myIter = iterations;};
        inline int getIter() {  return myIter;};
	inline void setKeepIter(int keep_it) { myKeepIter = keep_it;};
        inline int getKeepIter(){ return myKeepIter;};

        inline void setMousePar(double mPar) { myMPar = mPar;};
        inline double getMousePar() { return myMPar;};

	void ComputeIterates();
        void ComputePeriods();
        void ComputeConstant();
        void NormalizeData();
	void Run();
	double GetY(double xIn);

	//public data structures
	logMap coordMap;
	logMap myLogMap;
    map<double,int> periodMap; 



	private:
	  double myBPar,myEPar,myX,myMPar,mTol;
	  double m_a,m_istep,aMin,aMax,yMin,yMax;
	  int myNSteps,myIter,myKeepIter;

	  mapType myMapType;
	  
};
