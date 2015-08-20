//
//  FIDFile.cpp
//  Nrecon
//
//  Created by Tobias Wood on 21/11/2012.
//  Copyright (c) 2012 Tobias Wood. All rights reserved.
//

#include "FIDFile.h"

namespace Agilent {

void FIDFile::SwapFileHeader(FileHeader *hdr) {
	SwapEndianness(&hdr->nblocks);
	SwapEndianness(&hdr->ntraces);
	SwapEndianness(&hdr->np);
	SwapEndianness(&hdr->ebytes);
	SwapEndianness(&hdr->tbytes);
	SwapEndianness(&hdr->bbytes);
	SwapEndianness(&hdr->vers_id);
	SwapEndianness(&hdr->status);
	SwapEndianness(&hdr->nbheaders);
}

void FIDFile::SwapBlockHeader(BlockHeader *hdr) {
	SwapEndianness(&hdr->scale);
	SwapEndianness(&hdr->status);
	SwapEndianness(&hdr->index);
	SwapEndianness(&hdr->mode);
	SwapEndianness(&hdr->ctcount);
	SwapEndianness(&hdr->lpval);
	SwapEndianness(&hdr->rpval);
	SwapEndianness(&hdr->lvl);
	SwapEndianness(&hdr->tlt);
}

FIDFile::FIDFile() :
    m_numBlocks(0), m_numTraces(0), m_numPoints(0),
    m_bytesPerPoint(0), m_bytesPerTrace(0), m_bytesPerBlock(0),
    m_status(0), m_version_id(0), m_numBlockHeaders(0)
{

}

FIDFile::FIDFile(const string& path) {
	open(path);
}

void FIDFile::open(const string& path) {
	FileHeader hdr;
    m_file.open(path, ios::in | ios::binary);
    if (m_file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr))) {
		// FID files are BIG endian, so swap if the host is little endian
        m_swap = HostEndianness() == LittleEndian;
        if (m_swap)
			SwapFileHeader(&hdr);
		
        m_numBlocks = hdr.nblocks;
        m_numTraces = hdr.ntraces;
        m_numPoints = hdr.np;
        m_bytesPerPoint = hdr.ebytes;
        m_bytesPerTrace = hdr.tbytes;
        m_bytesPerBlock = hdr.bbytes;
        m_status = bitset<16>(hdr.status);
        m_version_id = bitset<16>(hdr.vers_id);
        m_numBlockHeaders = hdr.nbheaders;
	}
}

FIDFile::~FIDFile() {
    m_file.close();
}

const int FIDFile::nBlocks() const { return m_numBlocks; }
const int FIDFile::nTraces() const { return m_numTraces; }
const int FIDFile::nPointsPerTrace() const { return m_numPoints; }
const int FIDFile::nPointsPerBlock() const { return m_numPoints * m_numTraces; }
const int FIDFile::nComplexPerTrace() const { return m_numPoints / 2; }
const int FIDFile::nComplexPerBlock() const { return m_numPoints * m_numTraces / 2; }
FIDFile::FIDType FIDFile::dataType() const {
    if (m_status[3])
		return Float32Type;
    else if (m_status[2])
		return Int32Type;
	else
		return Int16Type;
}

const complex<double> *FIDFile::readBlock(int index) {
    m_file.seekg(sizeof(FileHeader) + index * m_bytesPerBlock);
	BlockHeader hdr;
	double scale;
    if (m_file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr))) {
        if (m_swap)
			SwapBlockHeader(&hdr);
		scale = hdr.scale;
		// No scaling is signified by a zero :-(
		if (scale == 0)
			scale = 1;
	}
	complex<double> *block = new complex<double>[nComplexPerBlock()];
	// _bytesPerBlock includes the 28 byte block header
    int numBytes = m_bytesPerTrace * m_numTraces;
	char *bytes = new char[numBytes];
    if (m_file.read(bytes, numBytes)) {
		switch (dataType()) {
			case Float32Type: {
				float *ptr = reinterpret_cast<float *>(bytes);
                if (m_swap) SwapEndianness(ptr, nPointsPerBlock());
				for (int i = 0; i < nComplexPerBlock(); i++) {
					block[i].real(ptr[i*2] / scale);
					block[i].imag(ptr[i*2+1] / scale);
				}
			} break;
			case Int32Type: {
				int32_t *ptr = reinterpret_cast<int32_t *>(bytes);
                if (m_swap) SwapEndianness(ptr, nPointsPerBlock());
				for (int i = 0; i < nComplexPerBlock(); i++) {
					block[i].real(ptr[i*2] / scale);
					block[i].imag(ptr[i*2+1] / scale);
				}
			} break;
			case Int16Type: {
				int16_t *ptr = reinterpret_cast<int16_t *>(bytes);
                if (m_swap) SwapEndianness(ptr, nPointsPerBlock());
				for (int i = 0; i < nComplexPerBlock(); i++) {
					block[i].real(ptr[i*2] / scale);
					block[i].imag(ptr[i*2+1] / scale);
				}
			} break;
		}
	}
	return block;
}

const string FIDFile::print_header() const {
	stringstream ss;
	
    ss << "Number of blocks: " << m_numBlocks << endl
       << "Number of traces per block: " << m_numTraces << endl
       << "Number of points per trace: " << m_numPoints << endl
       << "Number of bytes per point: " << m_bytesPerPoint << endl
       << "Number of bytes per trace: " << m_bytesPerTrace << endl
       << "Number of bytes per block: " << m_bytesPerBlock << endl
       << "Status bits: " << m_status << " Version/ID bits: " << m_version_id << endl
       << "Number of block headers per block: " << m_numBlockHeaders << endl;
	return ss.str();
}

} // End namespace Agilent
