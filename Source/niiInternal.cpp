//
//  Internal.cpp
//  NiftiImage
//
//  Created by Tobias Wood on 18/09/2013.
//  Copyright (c) 2013 Tobias Wood. All rights reserved.
//

#include "Internal.h"

using namespace std;

namespace Nifti {

/*! Swap size bytes at a time from the given array of n sets of bytes
 *
 *  Declared void * so that the fields from the headers can be passed through
 *  without casting.
 */
void SwapBytes(size_t n, size_t size, void *bytes)
{
	size_t i;
	char *cp0 = (char *)bytes, *cp1, *cp2;
	char swap;
	
	for(i=0; i < n; i++) {
		cp1 = cp0;
		cp2 = cp0 + (size-1);
		while (cp2 > cp1) {
			swap = *cp1; *cp1 = *cp2; *cp2 = swap;
			cp1++; cp2--;
		}
		cp0 += size;
	}
}

/*
 *  Byte swap the individual fields of a NIFTI-1 header struct.
 */
void SwapNiftiHeader(struct nifti_1_header *h)
{
	SwapBytes(1, 4, &h->sizeof_hdr);
	SwapBytes(1, 4, &h->extents);
	SwapBytes(1, 2, &h->session_error);
	
	SwapBytes(8, 2, h->dim);
	SwapBytes(1, 4, &h->intent_p1);
	SwapBytes(1, 4, &h->intent_p2);
	SwapBytes(1, 4, &h->intent_p3);
	
	SwapBytes(1, 2, &h->intent_code);
	SwapBytes(1, 2, &h->datatype);
	SwapBytes(1, 2, &h->bitpix);
	SwapBytes(1, 2, &h->slice_start);
	
	SwapBytes(8, 4, h->pixdim);
	
	SwapBytes(1, 4, &h->vox_offset);
	SwapBytes(1, 4, &h->scl_slope);
	SwapBytes(1, 4, &h->scl_inter);
	SwapBytes(1, 2, &h->slice_end);
	
	SwapBytes(1, 4, &h->cal_max);
	SwapBytes(1, 4, &h->cal_min);
	SwapBytes(1, 4, &h->slice_duration);
	SwapBytes(1, 4, &h->toffset);
	SwapBytes(1, 4, &h->glmax);
	SwapBytes(1, 4, &h->glmin);
	
	SwapBytes(1, 2, &h->qform_code);
	SwapBytes(1, 2, &h->sform_code);
	
	SwapBytes(1, 4, &h->quatern_b);
	SwapBytes(1, 4, &h->quatern_c);
	SwapBytes(1, 4, &h->quatern_d);
	SwapBytes(1, 4, &h->qoffset_x);
	SwapBytes(1, 4, &h->qoffset_y);
	SwapBytes(1, 4, &h->qoffset_z);
	
	SwapBytes(4, 4, h->srow_x);
	SwapBytes(4, 4, h->srow_y);
	SwapBytes(4, 4, h->srow_z);
	
	return ;
}

/*
 *! Byte swap as an ANALYZE 7.5 header
 */
void SwapAnalyzeHeader(nifti_analyze75 * h)
{
	SwapBytes(1, 4, &h->sizeof_hdr);
	SwapBytes(1, 4, &h->extents);
	SwapBytes(1, 2, &h->session_error);
	
	SwapBytes(8, 2, h->dim);
	SwapBytes(1, 2, &h->unused8);
	SwapBytes(1, 2, &h->unused9);
	SwapBytes(1, 2, &h->unused10);
	SwapBytes(1, 2, &h->unused11);
	SwapBytes(1, 2, &h->unused12);
	SwapBytes(1, 2, &h->unused13);
	SwapBytes(1, 2, &h->unused14);
	
	SwapBytes(1, 2, &h->datatype);
	SwapBytes(1, 2, &h->bitpix);
	SwapBytes(1, 2, &h->dim_un0);
	
	SwapBytes(8, 4, h->pixdim);
	
	SwapBytes(1, 4, &h->vox_offset);
	SwapBytes(1, 4, &h->funused1);
	SwapBytes(1, 4, &h->funused2);
	SwapBytes(1, 4, &h->funused3);
	
	SwapBytes(1, 4, &h->cal_max);
	SwapBytes(1, 4, &h->cal_min);
	SwapBytes(1, 4, &h->compressed);
	SwapBytes(1, 4, &h->verified);
	
	SwapBytes(1, 4, &h->glmax);
	SwapBytes(1, 4, &h->glmin);
	
	SwapBytes(1, 4, &h->views);
	SwapBytes(1, 4, &h->vols_added);
	SwapBytes(1, 4, &h->start_field);
	SwapBytes(1, 4, &h->field_skip);
	
	SwapBytes(1, 4, &h->omax);
	SwapBytes(1, 4, &h->omin);
	SwapBytes(1, 4, &h->smax);
	SwapBytes(1, 4, &h->smin);
}

/*
 * Internal function convert the Intent enum to the correct NIFTI_INTENT_CODE
 *
 *\sa NIFTI1_INTENT_CODES group in nifti1.h
 */
int CodeForIntent(const Intent i) {
	switch (i) {
		case (Intent::None):                 return NIFTI_INTENT_NONE;
		case (Intent::Correlation):          return NIFTI_INTENT_CORREL;
		case (Intent::TTest):                return NIFTI_INTENT_TTEST;
		case (Intent::FTest):                return NIFTI_INTENT_FTEST;
		case (Intent::ZScore):               return NIFTI_INTENT_ZSCORE;
		case (Intent::ChiSquared):           return NIFTI_INTENT_CHISQ;
		case (Intent::Beta):                 return NIFTI_INTENT_BETA;
		case (Intent::Binomial):             return NIFTI_INTENT_BINOM;
		case (Intent::Gamma):                return NIFTI_INTENT_GAMMA;
		case (Intent::Poisson):              return NIFTI_INTENT_POISSON;
		case (Intent::Normal):               return NIFTI_INTENT_NORMAL;
		case (Intent::FTestNonCentral):      return NIFTI_INTENT_FTEST_NONC;
		case (Intent::ChiSquaredNonCentral): return NIFTI_INTENT_CHISQ_NONC;
		case (Intent::Logistic):             return NIFTI_INTENT_LOGISTIC;
		case (Intent::Laplace):              return NIFTI_INTENT_LAPLACE;
		case (Intent::Uniform):              return NIFTI_INTENT_UNIFORM;
		case (Intent::TTestNonCentral):      return NIFTI_INTENT_TTEST_NONC;
		case (Intent::Weibull):              return NIFTI_INTENT_WEIBULL;
		case (Intent::Chi):                  return NIFTI_INTENT_CHI;
		case (Intent::InverseGuassian):      return NIFTI_INTENT_INVGAUSS;
		case (Intent::ExtremeValue):         return NIFTI_INTENT_EXTVAL;
		case (Intent::PValue):               return NIFTI_INTENT_PVAL;
		case (Intent::LogPValue):            return NIFTI_INTENT_LOGPVAL;
		case (Intent::Log10PValue):          return NIFTI_INTENT_LOG10PVAL;
		// Non-statistic Intents Below
		case (Intent::Estimate):             return NIFTI_INTENT_ESTIMATE;
		case (Intent::Label):                return NIFTI_INTENT_LABEL;
		case (Intent::Neuroname):            return NIFTI_INTENT_NEURONAME;
		case (Intent::MatrixGeneral):        return NIFTI_INTENT_GENMATRIX;
		case (Intent::MatrixSymmetric):      return NIFTI_INTENT_SYMMATRIX;
		case (Intent::VectorDisplacement):   return NIFTI_INTENT_DISPVECT;
		case (Intent::Vector):               return NIFTI_INTENT_VECTOR;
		case (Intent::Pointset):             return NIFTI_INTENT_POINTSET;
		case (Intent::Triangle):             return NIFTI_INTENT_TRIANGLE;
		case (Intent::Quaternion):           return NIFTI_INTENT_QUATERNION;
		case (Intent::Dimensionless):        return NIFTI_INTENT_DIMLESS;
		// GIFTI Intents
		case (Intent::Timeseries):           return NIFTI_INTENT_TIME_SERIES;
		case (Intent::NodeIndex):            return NIFTI_INTENT_NODE_INDEX;
		case (Intent::RGBVector):            return NIFTI_INTENT_RGB_VECTOR;
		case (Intent::RGBAVector):           return NIFTI_INTENT_RGBA_VECTOR;
		case (Intent::Shape):                return NIFTI_INTENT_SHAPE;
		default:
			throw(std::out_of_range("Invalid Intent code (probably unitialised)."));
			break;
	}
}

/*
 * Internal function convert the Intent enum to the correct NIFTI_INTENT_CODE
 *
 *\sa NIFTI1_INTENT_CODES group in nifti1.h
 */
Intent IntentForCode(const int c) {
	switch (c) {
		case (NIFTI_INTENT_NONE):        return Intent::None;
		case (NIFTI_INTENT_CORREL):      return Intent::Correlation;
		case (NIFTI_INTENT_TTEST):       return Intent::TTest;
		case (NIFTI_INTENT_FTEST):       return Intent::FTest;
		case (NIFTI_INTENT_ZSCORE):      return Intent::ZScore;
		case (NIFTI_INTENT_CHISQ):       return Intent::ChiSquared;
		case (NIFTI_INTENT_BETA):        return Intent::Beta;
		case (NIFTI_INTENT_BINOM):       return Intent::Binomial;
		case (NIFTI_INTENT_GAMMA):       return Intent::Gamma;
		case (NIFTI_INTENT_POISSON):     return Intent::Poisson;
		case (NIFTI_INTENT_NORMAL):      return Intent::Normal;
		case (NIFTI_INTENT_FTEST_NONC):  return Intent::FTestNonCentral;
		case (NIFTI_INTENT_CHISQ_NONC):  return Intent::ChiSquaredNonCentral;
		case (NIFTI_INTENT_LOGISTIC):    return Intent::Logistic;
		case (NIFTI_INTENT_LAPLACE):     return Intent::Laplace;
		case (NIFTI_INTENT_UNIFORM):     return Intent::Uniform;
		case (NIFTI_INTENT_TTEST_NONC):  return Intent::TTestNonCentral;
		case (NIFTI_INTENT_WEIBULL):     return Intent::Weibull;
		case (NIFTI_INTENT_CHI):         return Intent::Chi;
		case (NIFTI_INTENT_INVGAUSS):    return Intent::InverseGuassian;
		case (NIFTI_INTENT_EXTVAL):      return Intent::ExtremeValue;
		case (NIFTI_INTENT_PVAL):        return Intent::PValue;
		case (NIFTI_INTENT_LOGPVAL):     return Intent::LogPValue;
		case (NIFTI_INTENT_LOG10PVAL):   return Intent::Log10PValue;
		// Non-statistic Intents Below
		case (NIFTI_INTENT_ESTIMATE):    return Intent::Estimate;
		case (NIFTI_INTENT_LABEL):       return Intent::Label;
		case (NIFTI_INTENT_NEURONAME):   return Intent::Neuroname;
		case (NIFTI_INTENT_GENMATRIX):   return Intent::MatrixGeneral;
		case (NIFTI_INTENT_SYMMATRIX):   return Intent::MatrixSymmetric;
		case (NIFTI_INTENT_DISPVECT):    return Intent::VectorDisplacement;
		case (NIFTI_INTENT_VECTOR):      return Intent::Vector;
		case (NIFTI_INTENT_POINTSET):    return Intent::Pointset;
		case (NIFTI_INTENT_TRIANGLE):    return Intent::Triangle;
		case (NIFTI_INTENT_QUATERNION):  return Intent::Quaternion;
		case (NIFTI_INTENT_DIMLESS):     return Intent::Dimensionless;
		// GIFTI Intents
		case (NIFTI_INTENT_TIME_SERIES): return Intent::Timeseries;
		case (NIFTI_INTENT_NODE_INDEX):  return Intent::NodeIndex;
		case (NIFTI_INTENT_RGB_VECTOR):  return Intent::RGBVector;
		case (NIFTI_INTENT_RGBA_VECTOR): return Intent::RGBAVector;
		case (NIFTI_INTENT_SHAPE):       return Intent::Shape;
		default:
			throw(std::out_of_range("Invalid Intent code: " + to_string(c)));
			break;
	}
}

/*
 * Internal function to look up a string representation of the Intent name
 */
std::string IntentName(Intent i) {
	switch (i) {
		case(Intent::None):                 return "None";
		case(Intent::Correlation):          return "Correlation statistic";
		case(Intent::TTest):                return "T-statistic";
		case(Intent::FTest):                return "F-statistic";
		case(Intent::ZScore):               return "Z-score";
		case(Intent::ChiSquared):           return "Chi-squared distribution";
		case(Intent::Beta):                 return "Beta distribution";
		case(Intent::Binomial):             return "Binomial distribution";
		case(Intent::Gamma):                return "Gamma distribution";
		case(Intent::Poisson):              return "Poisson distribution";
		case(Intent::Normal):               return "Normal distribution";
		case(Intent::FTestNonCentral):      return "F-statistic noncentral";
		case(Intent::ChiSquaredNonCentral): return "Chi-squared noncentral";
		case(Intent::Logistic):             return "Logistic distribution";
		case(Intent::Laplace):              return "Laplace distribution";
		case(Intent::Uniform):              return "Uniform distribition";
		case(Intent::TTestNonCentral):      return "T-statistic noncentral";
		case(Intent::Weibull):              return "Weibull distribution";
		case(Intent::Chi):                  return "Chi distribution";
		case(Intent::InverseGuassian):      return "Inverse Gaussian distribution";
		case(Intent::ExtremeValue):         return "Extreme Value distribution";
		case(Intent::PValue):               return "P-value";
		case(Intent::LogPValue):            return "Log P-value";
		case(Intent::Log10PValue):          return "Log10 P-value";
		case(Intent::Estimate):             return "Estimate";
		case(Intent::Label):                return "Label index";
		case(Intent::Neuroname):            return "NeuroNames index";
		case(Intent::MatrixGeneral):        return "General matrix";
		case(Intent::MatrixSymmetric):      return "Symmetric matrix";
		case(Intent::VectorDisplacement):   return "Displacement vector";
		case(Intent::Vector):               return "Vector";
		case(Intent::Pointset):             return "Pointset";
		case(Intent::Triangle):             return "Triangle";
		case(Intent::Quaternion):           return "Quaternion";
		case(Intent::Dimensionless):        return "Dimensionless number";
		case(Intent::Timeseries):           return "GIFTI Timeseries";
		case(Intent::NodeIndex):            return "GIFTI Node Index";
		case(Intent::RGBVector):            return "GIFTI RGB Vector";
		case(Intent::RGBAVector):           return "GIFTI RGBA Vector";
		case(Intent::Shape):                return "GIFTI Shape";
		default:
			throw(std::out_of_range("Invalid Intent (probably unitialised)"));
	};
}

} // End namespace Nifti
