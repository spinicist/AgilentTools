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
	_numBlocks(0), _numTraces(0), _numPoints(0),
	_bytesPerPoint(0), _bytesPerTrace(0), _bytesPerBlock(0),
	_status(0), _version_id(0), _numBlockHeaders(0)
{

}

FIDFile::FIDFile(const string& path) {
	open(path);
}

void FIDFile::open(const string& path) {
	FileHeader hdr;
	_file.open(path, ios::in | ios::binary);
	if (_file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr))) {
		// FID files are BIG endian, so swap if the host is little endian
		_swap = HostEndianness() == LittleEndian;
		if (_swap)
			SwapFileHeader(&hdr);
		
		_numBlocks = hdr.nblocks;
		_numTraces = hdr.ntraces;
		_numPoints = hdr.np;
		_bytesPerPoint = hdr.ebytes;
		_bytesPerTrace = hdr.tbytes;
		_bytesPerBlock = hdr.bbytes;
		_status = bitset<16>(hdr.status);
		_version_id = bitset<16>(hdr.vers_id);
		_numBlockHeaders = hdr.nbheaders;
	}
}

FIDFile::~FIDFile() {
	_file.close();
}

const int FIDFile::nBlocks() const { return _numBlocks; }
const int FIDFile::nTraces() const { return _numTraces; }
const int FIDFile::nPointsPerTrace() const { return _numPoints; }
const int FIDFile::nPointsPerBlock() const { return _numPoints * _numTraces; }
const int FIDFile::nComplexPerTrace() const { return _numPoints / 2; }
const int FIDFile::nComplexPerBlock() const { return _numPoints * _numTraces / 2; }
FIDFile::FIDType FIDFile::dataType() const {
	if (_status[3])
		return Float32Type;
	else if (_status[2])
		return Int32Type;
	else
		return Int16Type;
}

const complex<double> *FIDFile::readBlock(int index) {
	_file.seekg(sizeof(FileHeader) + index * _bytesPerBlock);
	BlockHeader hdr;
	double scale;
	if (_file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr))) {
		if (_swap)
			SwapBlockHeader(&hdr);
		scale = hdr.scale;
		// No scaling is signified by a zero :-(
		if (scale == 0)
			scale = 1;
	}
	complex<double> *block = new complex<double>[nComplexPerBlock()];
	// _bytesPerBlock includes the 28 byte block header
	int numBytes = _bytesPerTrace * _numTraces;
	char *bytes = new char[numBytes];
	if (_file.read(bytes, numBytes)) {
		switch (dataType()) {
			case Float32Type: {
				float *ptr = reinterpret_cast<float *>(bytes);
				if (_swap) SwapEndianness(ptr, nPointsPerBlock());
				for (int i = 0; i < nComplexPerBlock(); i++) {
					block[i].real(ptr[i*2] / scale);
					block[i].imag(ptr[i*2+1] / scale);
				}
			} break;
			case Int32Type: {
				int32_t *ptr = reinterpret_cast<int32_t *>(bytes);
				if (_swap) SwapEndianness(ptr, nPointsPerBlock());
				for (int i = 0; i < nComplexPerBlock(); i++) {
					block[i].real(ptr[i*2] / scale);
					block[i].imag(ptr[i*2+1] / scale);
				}
			} break;
			case Int16Type: {
				int16_t *ptr = reinterpret_cast<int16_t *>(bytes);
				if (_swap) SwapEndianness(ptr, nPointsPerBlock());
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
	
	ss << "Number of blocks: " << _numBlocks << endl
	   << "Number of traces per block: " << _numTraces << endl
	   << "Number of points per trace: " << _numPoints << endl
	   << "Number of bytes per point: " << _bytesPerPoint << endl
	   << "Number of bytes per trace: " << _bytesPerTrace << endl
	   << "Number of bytes per block: " << _bytesPerBlock << endl
	   << "Status bits: " << _status << " Version/ID bits: " << _version_id << endl
	   << "Number of block headers per block: " << _numBlockHeaders << endl;
	return ss.str();
}

} // End namespace Agilent
