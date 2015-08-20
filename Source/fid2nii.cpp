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
#include "niiNifti.h"
#include "MultiArray.h"

using namespace std;
using namespace Eigen;

int main(int argc, const char * argv[])
{
    if (argc != 2) {
        cout << "No filename specified" << endl;
        return EXIT_FAILURE;
    }

	string path(argv[1]);
	
    Agilent::FID thisFid(path);
    cout << thisFid.print_info() << endl;

    string apptype = thisFid.procpar().stringValue("apptype");
    string seqfil  = thisFid.procpar().stringValue("seqfil");

    cout << "apptype = " << apptype << endl;
    cout << "seqfil  = " << seqfil << endl;

    if (apptype != "im3D") {
        throw(runtime_error("apptype " + apptype + " not supported."));
    }

    int nx = thisFid.procpar().realValue("np") / 2;
    int ny = thisFid.procpar().realValue("nv");
    int nz = thisFid.procpar().realValue("nv2");
    int narray = thisFid.procpar().realValue("arraydim");
    int ne = thisFid.procpar().realValue("ne");

    float lx = thisFid.procpar().realValue("lro") / nx;
    float ly = thisFid.procpar().realValue("lpe") / ny;
    float lz = thisFid.procpar().realValue("lpe2") / nz;

    Nifti::Header outHdr(nx, ny, nz, narray * ne, lx, ly, lz, 1, Nifti::DataType::COMPLEX128);
    Nifti::File output(outHdr, "output.nii");

    int vol = 0;
    for (int a = 0; a < narray; a++) {
        cout << "Reading block " << a << endl;
        shared_ptr<vector<complex<float>>> block = make_shared<vector<complex<float>>>();
        *block = thisFid.readBlock(a);
        int e_offset = 0;
        for (int e = 0; e < ne; e++) {
            cout << "Processing echo " << e << endl;
            MultiArray<complex<float>, 3> kspace({nx, ny, nz}, block, {ne,ne*nx,ne*nx*ny}, e_offset);

            // FFTs (with fftshift equivalent
            FFT<float> fft;
            for (int z = 0; z < nz; z++) {
                for (int y = 0; y < ny; y++) {
                    VectorXcf fft_in = kspace.slice<1>({0,y,z},{nx,0,0}).asArray();
                    VectorXcf fft_out(nx);
                    fft.inv(fft_out, fft_in);
                    kspace.slice<1>({0,y,z},{nx/2,0,0}).asArray() = fft_out.tail(nx/2);
                    kspace.slice<1>({nx/2,y,z},{nx/2,0,0}).asArray() = fft_out.head(nx/2);
                }
            }
            for (int z = 0; z < nz; z++) {
                for (int x = 0; x < nx; x++) {
                    VectorXcf fft_in = kspace.slice<1>({x,0,z},{0,-1,0}).asArray();
                    VectorXcf fft_out(ny);
                    fft.inv(fft_out, fft_in);
                    kspace.slice<1>({x,0,z},{0,ny/2,0}).asArray() = fft_out.tail(ny/2);
                    kspace.slice<1>({x,ny/2,z},{0,ny/2,0}).asArray() = fft_out.head(ny/2);
                }
            }

            // FFT Third Dimension
            for (int x = 0; x < nx; x++) {
                for (int y = 0; y < ny; y++) {
                    VectorXcf fft_in = kspace.slice<1>({x,y,0},{0,0,-1}).asArray();
                    VectorXcf fft_out(nz);
                    fft.inv(fft_out, fft_in);
                    kspace.slice<1>({x,y,0},{0,0,nz/2}).asArray() = fft_out.tail(nz/2);
                    kspace.slice<1>({x,y,nz/2},{0,0,nz/2}).asArray() = fft_out.head(nz/2);
                }
            }
            cout << "Writing volume" << endl;
            output.writeVolumes(kspace.begin(), kspace.end(), vol, 1);
            vol++;
            e_offset++;
        }
    }
    output.close();


    /*
    outHdr.setTransform(outTransform.cast<float>());
    outHdr.setDim(4, nOutImages);
	output.setDim(1, thisFid.nDim0());
	output.setDim(2, thisFid.nDim1());
	output.setDim(3, thisFid.nDim2());
	output.setDim(4, thisFid.nVolumes());
	output.setDatatype(NIFTI_TYPE_FLOAT32);
    Nifti::File output(outHdr, "output.nii.gz");
	output.writeAllVolumes(kSpace);
	output.close();
    */
    return 0;
}

