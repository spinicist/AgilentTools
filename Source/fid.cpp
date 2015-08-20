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
	m_bundlePath = path;
	// Remove any trailing directory delimiters
	if (m_bundlePath.back() == '/')
		m_bundlePath.resize(path.size() - 1);
	std::string ext = m_bundlePath.substr(m_bundlePath.find_last_of(".") + 1);
	if (ext != "fid")
		throw(runtime_error("Invalid extension for FID Bundle: " + m_bundlePath));
	m_fid.open(m_bundlePath + "/fid");
	ifstream fpp(m_bundlePath + "/procpar");
	fpp >> m_procpar;
	if (!fpp)
		throw(runtime_error("Failed to read procpar within FID Bundle: " + m_bundlePath));
	if (!m_procpar.contains("seqcon") || !m_procpar.contains("apptype"))
		throw(runtime_error("No apptype or seqcon found in FID Bundle: " + m_bundlePath));
}

const string FID::print_info() const {
	stringstream ss;
	
	ss << "FID Bundle: " << m_bundlePath << endl
	   << m_fid.print_header() << endl
	   << "Procpar contains " << m_procpar.count() << " parameters." << endl;
	return ss.str();
}

complex<double> *FID::readKSpace() {
	complex<double> *kSpace = new complex<double>[m_fid.nComplexPerBlock() * m_fid.nBlocks()];
	
	int blockOffset = 0;
	for (int b = 0; b < m_fid.nBlocks(); b++) {
		const complex<double> *thisBlock = m_fid.readBlock(b);
		for (int k = 0; k < m_fid.nComplexPerBlock(); k++)
			kSpace[blockOffset + k] = thisBlock[k];
		blockOffset += m_fid.nComplexPerBlock();
		delete[] thisBlock;
	}
	return kSpace;
}

const int FID::nVolumes() const { return 1; }
const int FID::nDim0() const { return static_cast<int>(m_procpar.realValue("np") / 2); }
const int FID::nDim1() const { return static_cast<int>(m_procpar.realValue("nv")); }
const int FID::nDim2() const {
	string appType = m_procpar.stringValue("apptype");
	if (appType == "im2D")
		return static_cast<int>(m_procpar.realValue("ns"));
	else if (appType == "im3D")
		return static_cast<int>(m_procpar.realValue("nv2"));
	else
		throw(runtime_error("Unknown application type: " + appType));
}

} // End namespace Nrecon
