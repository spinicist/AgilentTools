//
//  FID.h
//  Nrecon
//
//  Created by Tobias Wood on 11/12/2012.
//
//

#ifndef AGILENT_FID
#define AGILENT_FID

#include <string>
#include <iostream>
#include <fstream>
#include <complex>
#include <map>
#include <exception>

#include "util.h"
#include "FIDFile.h"
#include "procpar.h"

using namespace std;

namespace Agilent {
class FID {
	private:
		string m_bundlePath;
		FIDFile m_fid;
		ProcPar m_procpar;
	
	public:
		FID(const string &path);
		//~FID();
		
		const string print_info() const;
        complex<double> *readBlock(const int i);
        complex<double> *readAllBlocks();
        const ProcPar &procpar() const;
};

} // End namespace Nrecon

#endif /* defined(__Nrecon__FID__) */
