/*
 *  fid.h
 *  Nrecon
 *
 *  Copyright Tobias Wood 2015
 *
 */

#ifndef AGILENT_FID
#define AGILENT_FID

#include <string>
#include <iostream>
#include <fstream>
#include <complex>
#include <map>
#include <exception>

#include "util.h"
#include "fidFile.h"
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
        std::vector<complex<float>> readBlock(const int i);
        std::vector<complex<float>> readAllBlocks();
        const ProcPar &procpar() const;
};

} // End namespace Nrecon

#endif /* defined(__Nrecon__FID__) */
