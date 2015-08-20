/** \file Header.h
 \brief Enums to replace NIFTI_... definitions
 - Written by Tobias Wood, IoP KCL
 - This code is released to the public domain. Do with it what you will.
 */
#ifndef LIBNIFTI_ENUM_H
#define LIBNIFTI_ENUM_H

#include <Eigen/Core>

namespace Nifti {

typedef size_t Index;
static const Index MaxIndex = std::numeric_limits<Index>::max();
typedef Eigen::Array<Index, Eigen::Dynamic, 1> IndexArray;
typedef Eigen::Array<Index, 7, 1> Indices;
typedef Eigen::Array<Index, 3, 1> MatrixSize;

enum class Mode : char { Closed = 0, Read = 'r', ReadHeader = 'h', Write = 'w' };
enum class Version { Nifti1, Nifti2 };
enum class DataType {
	UINT8, UINT16, UINT32, UINT64, INT8, INT16, INT32, INT64,
	FLOAT32, FLOAT64, FLOAT128, COMPLEX64, COMPLEX128, COMPLEX256,
	RGB24, RGBA32
};
enum class Intent {
	None, Correlation, TTest, FTest, ZScore, ChiSquared, Beta, Binomial,
	Gamma, Poisson, Normal, FTestNonCentral, ChiSquaredNonCentral,
	Logistic, Laplace, Uniform, TTestNonCentral, Weibull, Chi,
	InverseGuassian, ExtremeValue, PValue, LogPValue, Log10PValue,
	Estimate, Label, Neuroname, MatrixGeneral, MatrixSymmetric,
	VectorDisplacement, Vector, Pointset, Triangle, Quaternion,
	Dimensionless,
	Timeseries, NodeIndex, RGBVector, RGBAVector, Shape
};

}

#endif // ENUM_H
