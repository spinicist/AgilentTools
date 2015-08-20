/** \file Internal.h
 \brief Inline internal library functions that the user should not need.
 - Written by Tobias Wood, IoP KCL
 - This code is released to the public domain. Do with it what you will.
 */
#ifndef LIBNIFTI_INTERNAL_INL
#define LIBNIFTI_INTERNAL_INL

inline float FixFloat(const float f) {
	if (std::isfinite(f))
		return f;
	else
		return 0.;
}

#endif
