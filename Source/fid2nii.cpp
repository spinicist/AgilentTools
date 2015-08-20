//
//  main.cpp
//  Nrecon
//
//  Created by Tobias Wood on 21/11/2012.
//
//

#include <string>
#include <iostream>

#include "Eigen/Dense"
#include "unsupported/Eigen/fft"

#include "fid.h"
#include "Nifti.h"

using namespace std;
using namespace Eigen;

int main(int argc, const char * argv[])
{
	string path(argv[1]);
	
	Recon::FID thisFid(path);
	cout << thisFid.print_info() << endl;
	complex<double> *kSpace = thisFid.readKSpace();
	
	cout << thisFid.nDim0() << " " << thisFid.nDim1() << " " << thisFid.nDim2() << endl;
	
	// FFT First & Second Dimension
	for (int z = 0; z < thisFid.nDim2(); z++) {
		int stride = thisFid.nDim0() * thisFid.nDim1();
		
		Map<Matrix<complex<double>, Dynamic, Dynamic>> slice(kSpace + z*stride, thisFid.nDim0(), thisFid.nDim1());
		FFT<double> fft;
		for (int i = 0; i < slice.rows(); i++) {
			Matrix<complex<double>, 1, Dynamic> fft_row(slice.cols());
			fft.inv(fft_row, slice.row(i));
			slice.row(i) = fft_row;
		}
		for (int j = 0; j < slice.cols(); j++) {
			Matrix<complex<double>, Dynamic, 1> fft_col(slice.rows());
			fft.inv(fft_col, slice.col(j));
			slice.col(j) = fft_col;
		}
	}
	
	// FFT Third Dimension
	for (int x = 0; x < thisFid.nDim0(); x++) {
		Map<Matrix<complex<double>, Dynamic, Dynamic>, Unaligned, Stride<Dynamic, Dynamic>>
			slice(kSpace + x, thisFid.nDim1(), thisFid.nDim2(),
			      Stride<Dynamic, Dynamic>(thisFid.nDim0() * thisFid.nDim1(), thisFid.nDim0()));
		FFT<double> fft;
		for (int k = 0; k < slice.rows(); k++) {
			Matrix<complex<double>, 1, Dynamic> fft_row(slice.cols());
			fft.inv(fft_row, slice.row(k));
			slice.row(k) = fft_row;
		}
	}
	
	Nifti output;
	output.setDim(1, thisFid.nDim0());
	output.setDim(2, thisFid.nDim1());
	output.setDim(3, thisFid.nDim2());
	output.setDim(4, thisFid.nVolumes());
	output.setDatatype(NIFTI_TYPE_FLOAT32);
	output.open("output.nii.gz", Nifti::Mode::Write);
	output.writeAllVolumes(kSpace);
	output.close();
	
    return 0;
}

