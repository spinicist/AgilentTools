/** \file Header.h
 \brief Declaration for Header class
 - Written by Tobias Wood, IoP KCL
 - This code is released to the public domain. Do with it what you will.
 */
#ifndef LIBNIFTI_HEADER_H
#define LIBNIFTI_HEADER_H

#include <string>
#include <iostream>
#include <map>
#include <limits>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "Nifti/Enum.h"

namespace Nifti {

struct nifti_1_header;
struct nifti_2_header;

DataType DataTypeForCode(const int code);

struct DataTypeInfo {
	DataType type;
	size_t code, size, swapsize;
	std::string name;
}; //!< Contains all the information needed to read/write a Nifti datatype
const DataTypeInfo &TypeInfo(const DataType dt);

enum class XForm { Unknown, ScannerAnatomy, AlignedAnatomy, Talairach, MNI_152 };
const std::string XFormName(const XForm t);
int XFormCode(const XForm c);
XForm XFormForCode(const int c);

class Header {
	private:
		Indices m_dim;     //!< Number of voxels in each dimension. Note that here we do NOT store the rank in dim[0], so only 7 elements required.
		Indices m_strides; //!< Strides into the data on disk.
		Eigen::Array<float, 7, 1> m_voxdim;  //!< Size of each voxel. As above, only 7 elements because the rank is not stored.
		Index m_voxoffset;                   //!< Offset to start of voxel data.
		DataTypeInfo m_typeinfo;             //!< Information for datatype on disk.
		std::string m_magic;                 //!< Magic String
		Eigen::Affine3f m_qform, m_sform;    //!< Tranformation matrices from voxel indices to physical co-ords.
		XForm m_qcode, m_scode;              //!< Codes to define what the transformations represent.

		void calcStrides();

	public:
		Header();                              //!< Default constructor
		Header(const struct nifti_1_header &hdr);    //!< Construct a header from a nifti_1_header struct
		Header(const struct nifti_2_header &hdr);    //!< Construct a header from a nifti_2_header struct
		Header(const int nx, const int ny, const int nz, const int nt,
			  const float dx, const float dy, const float dz, const float dt,
			  const DataType dtype = DataType::FLOAT32); //!< Constructs a header with the specified dimension and voxel sizes.
		Header(const IndexArray &dim, const Eigen::ArrayXf &voxdim,
			  const DataType dtype = DataType::FLOAT32); //!< Constructs a header with the specified dimension and voxel sizes.

		explicit operator nifti_1_header() const;
		explicit operator nifti_2_header() const;
		friend std::ostream &operator<<(std::ostream &os, const Header &h);

		const std::string &magic() const;
		void setMagic(const Version v, const bool isNii);
		const DataTypeInfo &typeInfo() const;
		DataType datatype() const;
		void setDatatype(const DataType dt);

		Index rank() const;                                 //!< Get the rank (number of dimensions) of the image.
		Index dim(const Index d) const;                     //!< Get the size (voxel count) of a dimension. Valid dimensions are 1-7.
		void setDim(const Index d, const Index size);       //!< Set the size (voxel count) of a dimension. Valid dimensions are 1-7.
		IndexArray dims() const;                            //!< Get the active dimension sizes (i.e. up to rank())
		Indices fulldims() const;                           //!< Get all 7 dimension sizes.
		MatrixSize matrix() const;                          //!< Return the first 3 dimensions (i.e. just the spatial ones)
		IndexArray strides() const;                         //!< Get the strides for this image.
		Index voxoffset() const;                            //!< Return the offset into the file where voxels actually start.
		void setVoxoffset(const Version v,
		                  const bool isNii,
		                  const size_t extSize);            //!< Calculates and sets the correct voxel offset
		float voxDim(const size_t d) const;                 //!< Get the voxel size along dimension d. Valid dimensions are 1-7.
		Eigen::ArrayXf voxDims() const;                     //!< Get all voxel sizes.
		void setVoxDim(const size_t d, const float f);      //!< Set the voxel size along dimension d. Valid dimensions are 1-7.
		void setVoxDims(const Eigen::ArrayXf &newVoxDims);  //!< Set all voxel sizes.

		void setTransform(const Eigen::Affine3f &t, const XForm tc = XForm::ScannerAnatomy); //!< Set the qform and sform from a 4x4 general matrix. The qform will be set to closest matching linear XForm, the sform will be an exact copy.
		const Eigen::Affine3f &transform() const;           //!< Return the XForm with the highest priority.
		const Eigen::Affine3f &qform() const;               //!< Return just the qform.
		const Eigen::Affine3f &sform() const;               //!< Return just the sform.
		const XForm &qcode() const;                         //!< Find out what transformation the qform represents.
		const XForm &scode() const;                         //!< Find out what transformation the sform represents.
		bool matchesSpace(const Header &other) const;       //!< Check if voxel dimensions, data size and XForm match
		bool matchesVoxels(const Header &other) const;      //!< Looser check if voxel dimensions and data size match

		float scaling_slope;          //!< Slope of scaling between data on disk and in memory.
		float scaling_inter;          //!< Intercept of scaling between data on disk and in memory.
		float calibration_min;        //!< Suggested minimum for display.
		float calibration_max;        //!< Suggested maximum for display.

		int freq_dim ;                //!< Index of the frequency encode direction (1-3).
		int phase_dim;                //!< Index of the phase encode direction (1-3).
		int slice_dim;                //!< Index of the slice direction (1-3).

		int   slice_code;             //!< code for slice timing pattern
		int   slice_start;            //!< index for start of slices
		int   slice_end;              //!< index for end of slices
		float slice_duration;         //!< time between individual slices
		float toffset;                //!< time coordinate offset

		int xyz_units;                //!< dx,dy,dz units: NIFTI_UNITS_* code
		int time_units;               //!< dt       units: NIFTI_UNITS_* code
		Intent intent;                //!< What this data actually means (e.g. p-values)
		float intent_p1;              //!< A value associated with the intent, e.g. distribution parameter
		float intent_p2;              //!< A value associated with the intent, e.g. distribution parameter
		float intent_p3;              //!< A value associated with the intent, e.g. distribution parameter
		std::string intent_name;      //!< optional description of intent data
		std::string description;      //!< optional text to describe dataset
		std::string aux_file;         //!< auxiliary filename

		const std::string &spaceUnits() const;
		const std::string &timeUnits() const;
		std::string intentName() const;
		const std::string &sliceName() const;
};

} // End namespace Nifti

#endif // NIFTI_HEADER_H
