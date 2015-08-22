//
//  main.cpp
//  Nrecon
//
//  Created by Tobias Wood on 21/11/2012.
//
//

#include <string>
#include <iostream>
#include <getopt.h>

#include "Eigen/Dense"
#include "unsupported/Eigen/fft"

#include "fid.h"
#include "niiNifti.h"
#include "MultiArray.h"

using namespace std;
using namespace Eigen;

void fft_and_shift_X(MultiArray<complex<float>, 3> &a) {
    FFT<float> fft;
    int nx = a.dims()[0];
    for (int z = 0; z < a.dims()[2]; z++) {
        for (int y = 0; y < a.dims()[1]; y++) {
            VectorXcf fft_in(nx);
            fft_in.tail(nx/2) = a.slice<1>({0,y,z},{nx/2,0,0}).asArray();
            fft_in.head(nx/2) = a.slice<1>({nx/2,y,z},{nx/2,0,0}).asArray();
            VectorXcf fft_out(nx);
            fft.fwd(fft_out, fft_in);
            a.slice<1>({0,y,z},{nx/2,0,0}).asArray() = fft_out.tail(nx/2);
            a.slice<1>({nx/2,y,z},{nx/2,0,0}).asArray() = fft_out.head(nx/2);
        }
    }
}

void fft_and_shift_Y(MultiArray<complex<float>, 3> &a) {
    FFT<float> fft;
    int ny = a.dims()[1];
    for (int z = 0; z < a.dims()[2]; z++) {
        for (int x = 0; x < a.dims()[0]; x++) {
            VectorXcf fft_in(ny);
            fft_in.tail(ny/2) = a.slice<1>({x,0,z},{0,ny/2,0}).asArray();
            fft_in.head(ny/2) = a.slice<1>({x,ny/2,z},{0,ny/2,0}).asArray();
            VectorXcf fft_out(ny);
            fft.fwd(fft_out, fft_in);
            a.slice<1>({x,0,z},{0,ny/2,0}).asArray() = fft_out.tail(ny/2);
            a.slice<1>({x,ny/2,z},{0,ny/2,0}).asArray() = fft_out.head(ny/2);
        }
    }
}

void fft_and_shift_Z(MultiArray<complex<float>, 3> &a) {
    FFT<float> fft;
    int nz = a.dims()[2];
    for (int x = 0; x < a.dims()[0]; x++) {
        for (int y = 0; y < a.dims()[1]; y++) {
            VectorXcf fft_in(nz);
            fft_in.tail(nz/2) = a.slice<1>({x,y,0},{0,0,nz/2}).asArray();
            fft_in.head(nz/2) = a.slice<1>({x,y,nz/2},{0,0,nz/2}).asArray();
            VectorXcf fft_out(nz);
            fft.fwd(fft_out, fft_in);
            a.slice<1>({x,y,0},{0,0,nz/2}).asArray() = fft_out.tail(nz/2);
            a.slice<1>({x,y,nz/2},{0,0,nz/2}).asArray() = fft_out.head(nz/2);
        }
    }
}

MultiArray<complex<float>, 4> reconMGE(Agilent::FID &fid);
MultiArray<complex<float>, 4> reconMGE(Agilent::FID &fid) {
    int nx = fid.procpar().realValue("np") / 2;
    int ny = fid.procpar().realValue("nv");
    int nz = fid.procpar().realValue("nv2");
    int narray = fid.procpar().realValue("arraydim");
    int ne = fid.procpar().realValue("ne");

    MultiArray<complex<float>, 4> vols({nx, ny, nz, narray*ne});
    int vol = 0;
    for (int a = 0; a < narray; a++) {
        cout << "Reading block " << a << endl;
        shared_ptr<vector<complex<float>>> block = make_shared<vector<complex<float>>>();
        *block = fid.readBlock(a);
        int e_offset = 0;
        for (int e = 0; e < ne; e++) {
            cout << "Reading echo " << e << endl;
            MultiArray<complex<float>, 3> this_vol({nx, ny, nz}, block, {1,ne*nx,ne*nx*ny}, e_offset);
            MultiArray<complex<float>, 3> slice = vols.slice<3>({0,0,0,vol},{-1,-1,-1,0});

            auto it1 = this_vol.begin();
            auto it2 = slice.begin();
            while (it1 != this_vol.end()) {
                *it2++ = *it1++;
            }
            vol++;
            e_offset += nx;
        }
    }
    return vols;
}

MultiArray<complex<float>, 4> reconMP2RAGE(Agilent::FID &fid);
MultiArray<complex<float>, 4> reconMP2RAGE(Agilent::FID &fid) {
    int nx = fid.procpar().realValue("np") / 2;
    int ny = fid.procpar().realValue("nv");
    int nz = fid.procpar().realValue("nv2");

    ArrayXi pelist = fid.procpar().realValues("pelist").cast<int>();
    MultiArray<complex<float>, 4> k({nx, ny, nz, 2});

    for (int z = 0; z < nz; z++) {
        cout << "Reading block " << z << endl;
        vector<complex<float>> block = fid.readBlock(z);

        int i = 0;
        for (int v = 0; v < 2; v++) {
            for (int y = 0; y < ny; y++) {
                int yind = ny / 2 - 1 - pelist[y];
                for (int x = 0; x < nx; x++) {
                    k[{x, yind, z, v}] = block.at(i++);
                }
            }
        }
    }

    return k;
}

int main(int argc, char **argv) {
    int indexptr = 0, c;
    string outPrefix = "";
    bool zip = false, kspace = false;
    Nifti::DataType dtype = Nifti::DataType::COMPLEX64;
    static struct option long_options[] = {
        {"out", required_argument, 0, 'o'},
        {"zip", no_argument, 0, 'z'},
        {"kspace", required_argument, 0, 'k'},
        {0, 0, 0, 0}
    };
    static const char *short_options = "o:zkm";

    while ((c = getopt_long(argc, argv, short_options, long_options, &indexptr)) != -1) {
        switch (c) {
        case 0: break; // It was an option that just sets a flag.
        case 'o': outPrefix = string(optarg); break;
        case 'z': zip = true; break;
        case 'k': kspace = true; break;
        case 'm': dtype = Nifti::DataType::FLOAT32; break;
        default: cout << "Unknown option " << optarg << endl;
        }
    }

    if ((argc - optind) <= 0) {
        cout << "No .fids specified" << endl;
        return EXIT_FAILURE;
    }

    string inPath(argv[optind]);
    size_t fileSep = inPath.find_last_of("/") + 1;
    size_t fileExt = inPath.find_last_of(".");
    if ((fileExt == string::npos) || (inPath.substr(fileExt) != ".fid")) {
        cerr << inPath << " is not a valid .fid directory" << endl;
    }
    string outPath = outPrefix + inPath.substr(fileSep, fileExt - fileSep) + ".nii";
    if (zip)
        outPath = outPath + ".gz";

    Agilent::FID fid(inPath);
    cout << fid.print_info() << endl;

    string apptype = fid.procpar().stringValue("apptype");
    string seqfil  = fid.procpar().stringValue("seqfil");

    cout << "apptype = " << apptype << endl;
    cout << "seqfil  = " << seqfil << endl;

    if (apptype != "im3D") {
        throw(runtime_error("apptype " + apptype + " not supported."));
    }

    MultiArray<complex<float>, 4> vols;
    if (seqfil.substr(0, 5) == "mge3d") {
        cout << "MGE Recon" << endl;
        vols = reconMGE(fid);
    } else if (seqfil.substr(0, 7) == "mp2rage") {
        cout << "mp2rage recon" << endl;
        vols = reconMP2RAGE(fid);
    }

    cout << "Dims " << vols.dims().transpose() << endl;
    if (!kspace) {
        for (int v = 0; v < vols.dims()[3]; v++) {
            cout << "FFTing vol " << v << endl;
            MultiArray<complex<float>, 3> vol = vols.slice<3>({0,0,0,v},{-1,-1,-1,0});
            fft_and_shift_X(vol);
            fft_and_shift_Y(vol);
            fft_and_shift_Z(vol);
        }
    }
    cout << "Writing volume" << endl;

    float lx = fid.procpar().realValue("lro") / vols.dims()[0];
    float ly = fid.procpar().realValue("lpe") / vols.dims()[1];
    float lz = fid.procpar().realValue("lpe2") / vols.dims()[2];
    ArrayXf voxdims(4); voxdims << lx, ly, lz, 1;
    Nifti::Header outHdr(vols.dims(), voxdims, dtype);
    Nifti::File output(outHdr, outPath);
    output.writeVolumes(vols.begin(), vols.end(), 0, vols.dims()[3]);
    output.close();
    return 0;
}

