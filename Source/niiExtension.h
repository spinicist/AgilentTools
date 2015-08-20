/** \file Header.h
 \brief Code for dealing with Nifti Extensions
 - Written by Tobias Wood, IoP KCL
 - This code is released to the public domain. Do with it what you will.
 */
#ifndef LIBNIFTI_EXTENSION_H
#define LIBNIFTI_EXTENSION_H

namespace Nifti {

/*
 *  Nifti Extension Class.
 *
 *  Provides a minimal way to read and write Nifti extensions as
 *  vectors of bytes.
 */
class Extension {
	private:
		int m_code;          //!< Extension code, one of the NIFTI_ECODE_ values
		std::vector<char> m_data; //!< Raw data, with no byte swapping (length is esize-8)

	public:
		static const std::string &CodeName(const int code);

		Extension(int code, std::vector<char> data);
		Extension(int size, int code, char *data);
		size_t rawSize() const;
		int size() const;
		int padding() const;
		int code() const;
		const std::string &codeName() const;
		void setCode(int code);

		const std::vector<char> &data() const;
		void setData(const std::vector<char> &data);
};

} // End namespace Nifti

#endif // EXTENSION_H
