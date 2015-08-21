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

void reconMGE(Agilent::FID &fid);
void reconMGE(Agilent::FID &fid) {
    int nx = fid.procpar().realValue("np") / 2;
    int ny = fid.procpar().realValue("nv");
    int nz = fid.procpar().realValue("nv2");
    int narray = fid.procpar().realValue("arraydim");
    int ne = fid.procpar().realValue("ne");

    float lx = fid.procpar().realValue("lro") / nx;
    float ly = fid.procpar().realValue("lpe") / ny;
    float lz = fid.procpar().realValue("lpe2") / nz;

    Nifti::Header outHdr(nx, ny, nz, narray * ne, lx, ly, lz, 1, Nifti::DataType::COMPLEX128);
    Nifti::File output(outHdr, "output.nii");
    int vol = 0;
    for (int a = 0; a < narray; a++) {
        cout << "Reading block " << a << endl;
        shared_ptr<vector<complex<float>>> block = make_shared<vector<complex<float>>>();
        *block = fid.readBlock(a);
        int e_offset = 0;
        for (int e = 0; e < ne; e++) {
            cout << "Processing echo " << e << endl;
            MultiArray<complex<float>, 3> kspace({nx, ny, nz}, block, {1,ne*nx,ne*nx*ny}, e_offset);

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
            e_offset += nx;
        }
    }
    output.close();
}

void reconMP2RAGE(Agilent::FID &fid);
void reconMP2RAGE(Agilent::FID &fid) {
    int nx = fid.procpar().realValue("np") / 2;
    int ny = fid.procpar().realValue("nv");
    int nz = fid.procpar().realValue("nv2");

    float lx = fid.procpar().realValue("lro") / nx;
    float ly = fid.procpar().realValue("lpe") / ny;
    float lz = fid.procpar().realValue("lpe2") / nz;

    ArrayXi pelist = fid.procpar().realValues("pelist").cast<int>();

    Nifti::Header outHdr(nx, ny, nz, 2, lx, ly, lz, 1, Nifti::DataType::COMPLEX128);
    Nifti::File output(outHdr, "output.nii");
    MultiArray<complex<float>, 4> k({nx, ny, nz, 2});

    for (int z = 0; z < nz; z++) {
        cout << "Reading block " << z << endl;
        vector<complex<float>> block = fid.readBlock(z);

        int i = 0;
        for (int y = 0; y < ny; y++) {
            int yind = ny / 2 + pelist[y];
            for (int x = 0; x < nx; x++) {
                k[{x, yind, z, 0}] = block.at(i++);
            }
            for (int x = 0; x < nx; x++) {
                k[{x, yind, z, 1}] = block.at(i++);
            }
        }
    }


    // FFTs (with fftshift equivalent
    FFT<float> fft;
    for (int v = 0; v < 2; v++) {
        for (int z = 0; z < nz; z++) {
            for (int y = 0; y < ny; y++) {
                VectorXcf fft_in = k.slice<1>({0,y,z,v},{nx,0,0,0}).asArray();
                VectorXcf fft_out(nx);
                fft.inv(fft_out, fft_in);
                k.slice<1>({0,y,z,v},{nx/2,0,0,0}).asArray() = fft_out.tail(nx/2);
                k.slice<1>({nx/2,y,z,v},{nx/2,0,0,0}).asArray() = fft_out.head(nx/2);
            }
        }
        for (int z = 0; z < nz; z++) {
            for (int x = 0; x < nx; x++) {
                VectorXcf fft_in = k.slice<1>({x,0,z,v},{0,ny,0,0}).asArray();
                VectorXcf fft_out(ny);
                fft.inv(fft_out, fft_in);
                k.slice<1>({x,0,z,v},{0,ny/2,0,0}).asArray() = fft_out.tail(ny/2);
                k.slice<1>({x,ny/2,z,v},{0,ny/2,0,0}).asArray() = fft_out.head(ny/2);
            }
        }

        // FFT Third Dimension
        for (int x = 0; x < nx; x++) {
            for (int y = 0; y < ny; y++) {
                VectorXcf fft_in = k.slice<1>({x,y,0,v},{0,0,nz,0}).asArray();
                VectorXcf fft_out(nz);
                fft.inv(fft_out, fft_in);
                k.slice<1>({x,y,0,v},{0,0,nz/2,0}).asArray() = fft_out.tail(nz/2);
                k.slice<1>({x,y,nz/2,v},{0,0,nz/2,0}).asArray() = fft_out.head(nz/2);
            }
        }
    }
    cout << "Writing volume" << endl;
    output.writeVolumes(k.begin(), k.end(), 0, 2);
    output.close();
}

int main(int argc, const char * argv[])
{
    if (argc != 2) {
        cout << "No filename specified" << endl;
        return EXIT_FAILURE;
    }

	string path(argv[1]);
	
    Agilent::FID fid(path);
    cout << fid.print_info() << endl;

    string apptype = fid.procpar().stringValue("apptype");
    string seqfil  = fid.procpar().stringValue("seqfil");

    cout << "apptype = " << apptype << endl;
    cout << "seqfil  = " << seqfil << endl;

    if (apptype != "im3D") {
        throw(runtime_error("apptype " + apptype + " not supported."));
    }

    if (seqfil.substr(0, 5) == "mge3d") {
        cout << "MGE Recon" << endl;
        reconMGE(fid);
    } else if (seqfil.substr(0, 7) == "mp2rage") {
        cout << "mp2rage recon" << endl;
        reconMP2RAGE(fid);
    }


    /*
    outHdr.setTransform(outTransform.cast<float>());
    outHdr.setDim(4, nOutImages);
    output.setDim(1, fid.nDim0());
    output.setDim(2, fid.nDim1());
    output.setDim(3, fid.nDim2());
    output.setDim(4, fid.nVolumes());
	output.setDatatype(NIFTI_TYPE_FLOAT32);
    Nifti::File output(outHdr, "output.nii.gz");
	output.writeAllVolumes(kSpace);
	output.close();
    */
    return 0;
}

