//
//  FID.cpp
//  Nrecon
//
//  Created by Tobias Wood on 11/12/2012.
//
//

#include "FID.h"

namespace Agilent {

FID::FID(const string &path) {
    if (path.length() == 0) {
        throw(runtime_error(string(__PRETTY_FUNCTION__) + "\nEmpty path specified"));
    }
	m_bundlePath = path;
	// Remove any trailing directory delimiters
	if (m_bundlePath.back() == '/')
        m_bundlePath.resize(m_bundlePath.size() - 1);
	std::string ext = m_bundlePath.substr(m_bundlePath.find_last_of(".") + 1);
	if (ext != "fid")
		throw(runtime_error("Invalid extension for FID Bundle: " + m_bundlePath));
    const string fidPath = m_bundlePath + "/fid";
    const string ppPath = m_bundlePath + "/procpar";
    ifstream fpp(ppPath);
    if (!fpp)
        throw(runtime_error("Could not open " + ppPath));
	fpp >> m_procpar;
    if (!fpp.eof())
        throw(runtime_error("Failed to read entirety of " + ppPath));
	if (!m_procpar.contains("seqcon") || !m_procpar.contains("apptype"))
        throw(runtime_error("No apptype or seqcon found in " + ppPath));
    m_fid.open(fidPath); // This will throw on error
}

const string FID::print_info() const {
	stringstream ss;
	
	ss << "FID Bundle: " << m_bundlePath << endl
	   << m_fid.print_header() << endl
	   << "Procpar contains " << m_procpar.count() << " parameters." << endl;
	return ss.str();
}

std::vector<complex<float>> FID::readBlock(const int i) {
    if ((i > -1) && (i < m_fid.nBlocks())) {
        return m_fid.readBlock(i);
    } else {
        throw(runtime_error(string(__PRETTY_FUNCTION__) + "\nInvalid block number " + to_string(i)));
    }
}

std::vector<complex<float>> FID::readAllBlocks() {
    std::vector<complex<float>> all(m_fid.nComplexPerBlock() * m_fid.nBlocks());

    int blockOffset = 0;
    for (int b = 0; b < m_fid.nBlocks(); b++) {
        std::vector<complex<float>> thisBlock = readBlock(b);
        for (int k = 0; k < m_fid.nComplexPerBlock(); k++)
            all[blockOffset + k] = thisBlock[k];
        blockOffset += m_fid.nComplexPerBlock();
    }
    return all;
}

const ProcPar &FID::procpar() const { return m_procpar; }

} // End namespace Agilents
