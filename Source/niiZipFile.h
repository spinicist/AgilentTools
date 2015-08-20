/** \file ZipFile.h
 \brief Declaration for a wrapper around gzipped or unzipped files
 - Written by Tobias Wood, IoP KCL
 - Based on znzlib (Thanks to Robert Cox et al)
 - This code is released to the public domain. Do with it what you will.
 */
#ifndef LIBNIFTI_ZIPFILE
#define LIBNIFTI_ZIPFILE

#include <cstdio>
#include <string>
#include <iostream>
#include <limits>
#include <exception>

#include <zlib.h>

namespace Nifti {

/*! Utility class that wraps unzipped and zipped files into one object */
// zlib 1.2.5 and above support a "Transparent" mode that would remove the need for this,
// but Mac OS is stuck on 1.2.1
class ZipFile {
	private:
		FILE *m_plainFile;
		gzFile m_gzipFile;
		
	public:
		ZipFile();
		ZipFile(const ZipFile &z) = delete;
		ZipFile(ZipFile &&z);
		ZipFile &operator=(const ZipFile &z) = delete;
		ZipFile &operator=(ZipFile &&z);
		
		bool open(const std::string &path, const std::string &mode, const bool zip);
		void close();
		size_t read(void *buff, size_t size);   //!< Attempts to reads size bytes from the image file to buff. Returns actual number read.
		size_t write(const void *buff, int size); //!< Attempts to write size bytes from buff to the image file. Returns actual number written.
		bool seek(long offset, int whence);       //!< Seeks to the specified position in the file. Returns true if successful.
		long tell() const;                        //!< Returns the current position in the file
		void flush();                             //!< Flushes unwritten buffer contents
	
	friend std::ostream &operator<<(std::ostream &os, const ZipFile &zf);
};

} // End namespace Nifti

#endif // NIFTI_ZIPFILE
