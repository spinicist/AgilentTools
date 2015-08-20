#include "Nifti/Nifti.h"
#include "Nifti/Internal.h"

using namespace std;
using namespace Eigen;

namespace Nifti {

File::File() :
	m_mode(Mode::Closed), m_gz(false), m_swap(false),
	m_basepath(""), m_header(), m_nifti_version(Version::Nifti1)
{

}

File::~File()
{
	if (m_mode != Mode::Closed)
		close();
}

File::File(const File &other) :
	m_basepath(other.m_basepath),
	m_header(other.m_header), m_extensions(other.m_extensions),
	m_mode(other.m_mode), m_nii(other.m_nii),
	m_gz(other.m_gz), m_nifti_version(other.m_nifti_version),
	m_swap(other.m_swap)
{
	if (m_mode == Mode::Read) {
		m_file.open(imagePath(), "rb", m_gz);
		m_file.seek(other.m_file.tell(), SEEK_SET);
	} else if (m_mode == Mode::Write) {
		m_file.open(imagePath(), "wb", m_gz);
		m_file.seek(other.m_file.tell(), SEEK_SET);
	}
}

File::File(File &&other) noexcept :
	m_file(std::move(other.m_file)),
	m_basepath(other.m_basepath),
	m_header(other.m_header), m_extensions(other.m_extensions),
	m_mode(other.m_mode), m_nii(other.m_nii),
	m_gz(other.m_gz), m_nifti_version(other.m_nifti_version),
	m_swap(other.m_swap)
{
	other.m_mode = Mode::Closed;
}

File::File(const Header &hdr, const string &filename,
           const list<Extension> &exts, const Version v) : File() {
	m_header = hdr;
	m_nifti_version = v;
	m_extensions = exts;
	open(filename, Mode::Write);
}

File::File(const string &filename, const Mode m) : File() {
	open(filename, m);
}

/*
 * Open/Header Routines
 */

void File::open(const string &path, const Mode &mode) {
	if (m_mode != Mode::Closed) {
		throw(std::runtime_error("Cannot open " + path + ", file: " + basePath() + " is already open."));
	}
	switch (mode) {
		case (Mode::Closed):
			throw(std::runtime_error("Asked to open file " + path + " in Closed mode, does not make sense."));
			break;
		case (Mode::Read): case (Mode::ReadHeader):
			setPaths(path);
			if(!m_file.open(headerPath(), "rb", m_gz)) {
				throw(std::runtime_error("Failed to open file: " + headerPath()));
			}
			readHeader();
			readExtensions();
			break;
		case (Mode::Write):
			setPaths(path);
			if(!m_file.open(headerPath(), "wb", m_gz)) {
				throw(std::runtime_error("Failed to open file: " + headerPath()));
			}
			writeHeader();
			writeExtensions();
		break;
	}
	if (mode == Mode::ReadHeader) {
		m_file.close();
	} else if (m_nii) {
		// We are opening a .nii, keep the file open
		m_mode = mode;
	} else {
		// We are opening a .hdr/.img pair, close the header and open the image
		bool result;
		m_file.close();
		if (mode == Mode::Read)
			result = m_file.open(imagePath(), "rb", m_gz);
		else
			result = m_file.open(imagePath(), "wb", m_gz);
		if (!result) {
			throw(std::runtime_error("Could not open image file: " + imagePath()));
		}
		// Only set the mode here when we have successfully opened the file and
		// not thrown any errors. Throwing an error triggers the destructor and
		// we don't want to be in the wrong state there.
		m_mode = mode;
	}
}

File::operator bool() const {
	if (m_mode == Mode::Closed)
		return false;
	else
		return true;
}

void File::close() {
	if (m_mode == Mode::Closed) {
		throw(std::logic_error("Cannot close already closed file: " + imagePath()));
	} else if ((m_mode == Mode::Read) || (m_mode == Mode::ReadHeader)) {
		m_file.close();
		m_mode = Mode::Closed;
	} else if (m_mode == Mode::Write) {
		// If we've been writing subvolumes then we may not have written a complete file
		// Write a single zero-byte at the end to persuade the OS to write a file of the
		// correct size.
		m_file.seek(0, SEEK_END);
		long correctEnd = (m_header.dims().prod() * TypeInfo(m_header.datatype()).size + m_header.voxoffset());
		char zero{0};
		long pos = m_file.tell();
		if (pos < correctEnd) {
			m_file.seek(correctEnd - 1, SEEK_SET);
			m_file.write(&zero, 1);
		}
		m_file.flush();
		m_file.close();
		m_mode = Mode::Closed;
	}
}

void File::setPaths(const string &path) {
	size_t lastDot = path.find_last_of(".");
	string ext;
	if (path.substr(lastDot + 1) == "gz") {
		m_gz = true;
		size_t extDot = path.find_last_of(".", lastDot - 1);
		ext = path.substr(extDot + 1, lastDot - extDot - 1);
		m_basepath = path.substr(0, extDot);
	} else {
		m_gz = false;
		ext = path.substr(lastDot + 1);
		m_basepath = path.substr(0, lastDot);
	}
	if (ext == "hdr" || ext == "img") {
		m_nii = false;
	} else if (ext == "nii") {
		m_nii = true;
	} else {
		throw(std::invalid_argument("Invalid NIfTI extension for file: " + path));
	}
}

void File::readHeader() {
	int32_t tag;
	if (m_file.read(&tag, sizeof(tag)) < sizeof(tag)) {
		throw(std::runtime_error("Could not read first 4 bytes from " + headerPath()));
	}
	
	// Work out the NIfTI version and byte-order
	// As per Mark Jenkinson's pseudocode
	m_swap = false;
	if (tag == 348) {
		m_nifti_version = Version::Nifti1;
	} else if (tag == 540) {
		m_nifti_version = Version::Nifti2;
	} else {
		SwapBytes(1, 4, &tag);
		m_swap = true;
		if (tag == 348) {
			m_nifti_version = Version::Nifti1;
		} else if (tag == 540) {
			m_nifti_version = Version::Nifti2;
		} else {
			throw(std::runtime_error(headerPath() + " does not contain a valid NIfTI header."));
		}
	}

	m_file.seek(0, SEEK_SET);
	if (m_nifti_version == Version::Nifti1) {
		struct nifti_1_header n1hdr;
		if (m_file.read(&n1hdr, sizeof(n1hdr)) < sizeof(n1hdr)) {
			throw(std::runtime_error("Could not read header from " + headerPath()));
		}
		// Check the magic string is set to one of the possible NIfTI values,
		// otherwise process as an ANALYZE file
		int valid = ((n1hdr.magic[0]=='n' && n1hdr.magic[3]=='\0') &&
		             (n1hdr.magic[1]=='i' || n1hdr.magic[1]=='+') &&
		             (n1hdr.magic[2]=='1')) ? true : false;
		if (!valid) {
			throw(std::runtime_error(headerPath() + " does not contain a valid NIfTI-1 header."));
		}
		if (m_swap)
			SwapNiftiHeader(&n1hdr);
		m_header = Header(n1hdr);
	} else if (m_nifti_version == Version::Nifti2) {
		struct nifti_2_header n2hdr;
		if (m_file.read(&n2hdr, sizeof(n2hdr)) < sizeof(n2hdr)) {
			throw(std::runtime_error("Could not read header from " + headerPath()));
		}
		int valid = ((n2hdr.magic[0]=='n' && n2hdr.magic[3]=='\0') &&
					 (n2hdr.magic[1]=='i' || n2hdr.magic[1]=='+') &&
					 (n2hdr.magic[2]=='2')) ? true : false;
		if (!valid) {
			throw(std::runtime_error(headerPath() + " does not contain a valid NIfTI-2 header."));
		}
		m_header = Header(n2hdr);
	}
}

void File::writeHeader() {
	m_header.setMagic(m_nifti_version, m_nii);
	m_header.setVoxoffset(m_nifti_version, m_nii, totalExtensionSize());
	switch (m_nifti_version) {
		case Version::Nifti1: {
			struct nifti_1_header nhdr = (nifti_1_header)m_header;
			if(m_file.write(&nhdr, sizeof(nhdr)) < sizeof(nhdr)) {
				throw(std::runtime_error("Could not write header to file: " + headerPath()));
			}
		} break;
		case Version::Nifti2: {
			struct nifti_2_header nhdr = (nifti_2_header)m_header;
			if(m_file.write(&nhdr, sizeof(nhdr)) < sizeof(nhdr)) {
				throw(std::runtime_error("Could not write header to file: " + headerPath()));
			}
		} break;
	}
}

/*
 *  Extensions
 */

void File::addExtension(const int code, const vector<char> &data) {
	m_extensions.emplace_back(code, data);
}

void File::addExtension(const Extension &e) {
	m_extensions.push_back(e);
}

const list<Extension> &File::extensions() const {
	return m_extensions;
}

void File::setExtensions(const list<Extension> &newExts) {
	m_extensions = newExts;
}

void File::clearExtensions() {
	m_extensions.clear();
}

int File::totalExtensionSize() {
	int total = 0;
	for (auto ext: m_extensions) {
		total += ext.size();
	}
	return total;
}

void File::readExtensions()
{
	long target = m_header.voxoffset();
	if (!m_nii) {
		m_file.seek(0, SEEK_END);
		target = m_file.tell();
	}
	m_file.seek(sizeof(nifti_1_header), SEEK_SET);
	char extender[4];
	if (m_file.read(extender, 4) != 4) {
		throw(std::runtime_error("While checking for extensions hit end of file: " + headerPath()));
	}
	if (extender[0] != 1) // There are no extensions
		return;
	
	while (m_file.tell() < target) {		
		if(m_file.tell() > target - 16 ){
			throw(std::runtime_error("Insufficient space for remaining extensions in file: " + headerPath()));
		}
		
		int size, code;
		long bytesRead = m_file.read(&size, 4);
		bytesRead += m_file.read(&code, 4);
		if (bytesRead != 8) {
			throw(std::runtime_error("Error while reading extension size and code in file: " + headerPath()));
		}
		
		if (m_swap) {
			SwapBytes(1, 4, &size);
			SwapBytes(1, 4, &code);
		}
		
		vector<char> dataBytes(size - 8);
		if (m_file.read(dataBytes.data(), size - 8) < (size - 8)) {
			throw(std::runtime_error("Could not read extension in file: " + headerPath()));
		}
		m_extensions.emplace_back(code, dataBytes);

		if (m_nii && (m_file.tell() > m_header.voxoffset())) {
			throw(std::runtime_error("Went past start of voxel data while reading extensions in file: " + headerPath()));
		}
	}
}

void File::writeExtensions() {
	m_file.seek(sizeof(nifti_1_header), SEEK_SET);
	char extender[4] = {0, 0, 0, 0};
	if (m_extensions.size() > 0)
		extender[0] = 1;
	if (m_file.write(extender, 4) < 4) {
		throw(std::runtime_error("Could not write extender block to file: " + headerPath()));
	}
	
	for (auto ext : m_extensions) {
		if (ext.rawSize() > numeric_limits<int>::max()) {
			throw(std::runtime_error("Extension is larger than File standard permits in file: " + headerPath()));
		}
		int size = ext.size();
		int padding = ext.padding();
		long bytesWritten = m_file.write(&size, sizeof(int));
		int code = ext.code();
		bytesWritten += m_file.write(&code, sizeof(int));
		if (bytesWritten != (2*sizeof(int))) {
			throw(std::runtime_error("Could not write extension size and code to file: " + headerPath()));
		}
		if (m_file.write(ext.data().data(), static_cast<int>(ext.rawSize())) != static_cast<int>(ext.rawSize())) {
			throw(std::runtime_error("Could not write extension data to file: " + headerPath()));
		}
		if (padding) {
			vector<char> pad(ext.padding(), 0);
			if (m_file.write(pad.data(), ext.padding()) != ext.padding()) {
				throw(std::runtime_error("Could not write extension padding to file: " + headerPath()));
			}
		}
	}
	if ((m_file.tell() - totalExtensionSize() - 4) != sizeof(nifti_1_header)) {
		throw(std::runtime_error("Wrote wrong number of bytes for extensions to file: " + headerPath()));
	}
}

/**
 *  Returns the path without any extension
 */
const string &File::basePath() const { return m_basepath; }

/**
 *  Returns the path to the image file (will differ from the header for .hdr/.img pairs)
 */
string File::imagePath() const {
	string path(m_basepath);
	if (m_nii) {
		path += ".nii";
	} else {
		path += ".img";
	}
	if (m_gz)
		path += ".gz";
	
	return path;
}

/**
 *  Returns the path to the header (will differ from the image for .hdr/.img pairs)
 */
string File::headerPath() const {
	string path(m_basepath);
	if (m_nii) {
		path += ".nii";
	} else {
		path += ".hdr";
	}
	if (m_gz)
		path += ".gz";
	
	return path;
}

const Header &File::header() const { return m_header; }
void File::setHeader(const Header &h) {
	if (m_mode != Mode::Closed) {
		throw(std::runtime_error("Cannot call setHeader on an open file."));
	}
	m_header = h;
}
Index File::rank() const { return m_header.rank(); }
Index File::dim(const size_t d) const { return m_header.dim(d); }
IndexArray File::dims() const { return m_header.fulldims(); }
MatrixSize File::matrix() const { return m_header.matrix(); }

/**
  * Seeks to a particular voxel on the disk.
  *
  * @param target Desired voxel to seek to on disk.
  *
  * @throws std::out_of_range if the target is outside the image dimensions.
  * @throws std::runtime_error if the seek fails.
  */
void File::seekToVoxel(const IndexArray &inTarget) {
	if (inTarget.rows() > dims().rows()) {
		throw(std::out_of_range("Target voxel has too many dimensions."));
	}
	if ((inTarget >= dims().head(inTarget.rows())).any()) {
		throw(std::out_of_range("Target voxel is outside image dimensions."));
	}
	IndexArray target = inTarget.head(rank());
	size_t index = (target * m_header.strides()).sum() * m_header.typeInfo().size + m_header.voxoffset();
	size_t current = m_file.tell();
	if ((index - current != 0) && !m_file.seek(index - current, SEEK_CUR)) {
		stringstream ss; ss << target.transpose();
		throw(std::runtime_error("Failed to seek to target voxel: " + ss.str() +
		                         ", index: " + to_string(index) +
		                         ", current: " + to_string(current) +
		                         " in file: " + imagePath()));
	}
}

/**
  *   Fills the allocated byte array with bytes read from the open NIfTI image.
  *
  *   Internal function to actually read bytes from an image file.
  *   @param buffer Array to store read bytes in.
  */
void File::readBytes(std::vector<char> &buffer) {
	if (!(m_mode == Mode::Read)) {
		throw(std::logic_error("File not opened for reading: " + imagePath()));
	}
	if (buffer.size() > 0) {
		size_t nread = m_file.read(buffer.data(), buffer.size());
		if (nread != buffer.size()) {
			throw(std::runtime_error("Error reading data from file: " + imagePath()));
		}
		const auto swapsize = m_header.typeInfo().swapsize;
		if (swapsize > 1 && m_swap)
			SwapBytes(buffer.size() / swapsize, swapsize, buffer.data());
	}
}

/**
  *   Writes bytes to the open NIfTI image from the supplied array
  *
  *   Internal function to actually write bytes to an image file.
  *   @param buffer Array of bytes to write
  */
void File::writeBytes(const std::vector<char> &buffer) {
	if (!(m_mode == Mode::Write)) {
		throw(std::logic_error("File not opened for writing: " + imagePath()));
	}
	if (buffer.size() > 0) {
		if (m_file.write(buffer.data(), static_cast<unsigned int>(buffer.size())) != buffer.size()) {
			throw(std::runtime_error("Wrote wrong number of bytes from file: " + imagePath()));
		}
	}
}

/**
 * @brief File::dataSize
 * @return The size in bytes of the data portion (voxels) of the file
 */
size_t File::dataSize() const {
	return dims().prod() * m_header.typeInfo().size;
}

} // End namespace Nifti
