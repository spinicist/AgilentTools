//
//  ZipFile.cpp
//  NiftiImage
//
//  Created by Tobias Wood on 18/09/2013.
//  Copyright (c) 2013 Tobias Wood. All rights reserved.
//

#include "Nifti/ZipFile.h"

using namespace std;

namespace Nifti {

//******************************
#pragma mark Methods for ZipFile
//******************************
ZipFile::ZipFile() : m_plainFile(nullptr), m_gzipFile(nullptr) {}

ZipFile::ZipFile(ZipFile &&z) {
	m_plainFile = z.m_plainFile;
	m_gzipFile = z.m_gzipFile;
	z.m_plainFile = nullptr;
	z.m_gzipFile = nullptr;
}

ZipFile &ZipFile::operator=(ZipFile &&z) {
	m_plainFile = z.m_plainFile;
	m_gzipFile = z.m_gzipFile;
	z.m_plainFile = nullptr;
	z.m_gzipFile = nullptr;
	return *this;
}

bool ZipFile::open(const string &path, const string &mode, const bool zip) {
	if (m_gzipFile || m_plainFile) {
		close();
	}
	
	if (zip) {
		m_gzipFile = gzopen(path.c_str(), mode.c_str());
	} else {
		m_plainFile = fopen(path.c_str(), mode.c_str());
	}
	
	if (m_gzipFile || m_plainFile) {
		return true;
	} else {
		return false;
	}
}

void ZipFile::close() {
	if (m_gzipFile)
		gzclose(m_gzipFile);
	else if (m_plainFile)
		fclose(m_plainFile);
	m_gzipFile = nullptr;
	m_plainFile = nullptr;
}

/*! Attempts to read the specified number of bytes into the buffer
 *
 * Currently the best we can do for sizes is unsigned int, as this is what
 * gzread uses
 *
 */
size_t ZipFile::read(void *buff, size_t size) {
	if (m_gzipFile) {
		size_t remaining = size, totalRead = 0;
		char *cbuff = (char *)buff;
		while (remaining > 0) {
			// Maximum read size for libz is <int>::max(), now do some
			// careful casting to get the read size right
			unsigned chunkSize = (remaining < numeric_limits<int>::max()) ? static_cast<unsigned>(remaining) : numeric_limits<int>::max();
			int nread = gzread(m_gzipFile, cbuff, chunkSize);
			if (nread <= 0) {
				return 0;
			}
			remaining -= nread;
			if (nread < chunkSize) {
				return totalRead;
			}
			cbuff += nread;
			totalRead += nread;
		}
		return totalRead;
	} else if (m_plainFile) {
		size_t nread = fread(buff, size, 1, m_plainFile) * size;
		if (ferror(m_plainFile)) {
			return 0;
		}
		return nread;
	} else { // Can't read if we don't have a valid file handle open
		return 0;
	}
}

size_t ZipFile::write(const void *buff, int size)
{
	if (buff == nullptr) {
		return 0;
	}
	if (m_gzipFile) {
		unsigned remaining = size, totalWritten = 0;
		char *chunk = (char *)buff;
		while(remaining > 0 ) {
			unsigned chunkSize = (remaining < numeric_limits<int>::max()) ? remaining : numeric_limits<int>::max();
			int nwritten = gzwrite(m_gzipFile, chunk, chunkSize);
			if (nwritten == 0) {
				return 0;
			}
			remaining -= nwritten;
			if (nwritten < chunkSize) {
				return totalWritten;
			}
			chunk += nwritten;
			totalWritten += nwritten;
		}
		return totalWritten;
	} else if (m_plainFile) {
		size_t nwritten = fwrite(buff, size, 1, m_plainFile) * size;
		if (ferror(m_plainFile)) {
			return 0;
		}
		return nwritten;
	} else { // Can't write anything to a closed file
		return 0;
	}
}

bool ZipFile::seek(long offset, int whence) {
	if (m_gzipFile) {
		long int pos = gzseek(m_gzipFile, offset, whence);
		return (pos != -1);
	} else if (m_plainFile) {
		return (fseek(m_plainFile, offset, whence) == 0);
	} else {
		return false;
	}
}

long ZipFile::tell() const {
	if (m_gzipFile) {
		return gztell(m_gzipFile);
	} else if (m_plainFile) {
		return ftell(m_plainFile);
	} else {
		return 0;
	}
}

void ZipFile::flush() {
	if (m_gzipFile)
		gzflush(m_gzipFile, Z_FINISH);
	else if (m_plainFile)
		fflush(m_plainFile);
}

std::ostream &operator<<(std::ostream &os, const ZipFile &zf) {
	os << "ZipFile instance " << &zf << ": ";
	if (zf.m_gzipFile) {
		os << "GZ file, " << zf.m_gzipFile;
	} else if (zf.m_plainFile) {
		os << "Plain file, " << zf.m_plainFile;
	} else {
		os << "No file open.";
	}
	return os;
}

} // End namespace Nifti
