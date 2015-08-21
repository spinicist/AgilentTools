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
            VectorXcf fft_in = a.slice<1>({0,y,z},{nx,0,0}).asArray();
            VectorXcf fft_out(nx);
            fft.inv(fft_out, fft_in);
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
            VectorXcf fft_in = a.slice<1>({x,0,z},{0,ny,0}).asArray();
            VectorXcf fft_out(ny);
            fft.inv(fft_out, fft_in);
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
            VectorXcf fft_in = a.slice<1>({x,y,0},{0,0,nz}).asArray();
            VectorXcf fft_out(nz);
            fft.inv(fft_out, fft_in);
            a.slice<1>({x,y,0},{0,0,nz/2}).asArray() = fft_out.tail(nz/2);
            a.slice<1>({x,y,nz/2},{0,0,nz/2}).asArray() = fft_out.head(nz/2);
        }
    }
}

void reconMGE(Agilent::FID &fid, const string &outpath, const bool kspace);
void reconMGE(Agilent::FID &fid, const string &outpath, const bool kspace) {
    int nx = fid.procpar().realValue("np") / 2;
    int ny = fid.procpar().realValue("nv");
    int nz = fid.procpar().realValue("nv2");
    int narray = fid.procpar().realValue("arraydim");
    int ne = fid.procpar().realValue("ne");

    float lx = fid.procpar().realValue("lro") / nx;
    float ly = fid.procpar().realValue("lpe") / ny;
    float lz = fid.procpar().realValue("lpe2") / nz;

    Nifti::Header outHdr(nx, ny, nz, narray * ne, lx, ly, lz, 1, Nifti::DataType::COMPLEX128);
    Nifti::File output(outHdr, outpath);
    int vol = 0;
    for (int a = 0; a < narray; a++) {
        cout << "Reading block " << a << endl;
        shared_ptr<vector<complex<float>>> block = make_shared<vector<complex<float>>>();
        *block = fid.readBlock(a);
        int e_offset = 0;
        for (int e = 0; e < ne; e++) {
            cout << "Processing echo " << e << endl;
            MultiArray<complex<float>, 3> k({nx, ny, nz}, block, {1,ne*nx,ne*nx*ny}, e_offset);

            if (!kspace) {
                fft_and_shift_X(k);
                fft_and_shift_Y(k);
                fft_and_shift_Z(k);
            }
            cout << "Writing volume" << endl;
            output.writeVolumes(k.begin(), k.end(), vol, 1);
            vol++;
            e_offset += nx;
        }
    }
    output.close();
}

void reconMP2RAGE(Agilent::FID &fid, const string &outpath, const bool kspace);
void reconMP2RAGE(Agilent::FID &fid, const string &outpath, const bool kspace) {
    int nx = fid.procpar().realValue("np") / 2;
    int ny = fid.procpar().realValue("nv");
    int nz = fid.procpar().realValue("nv2");

    float lx = fid.procpar().realValue("lro") / nx;
    float ly = fid.procpar().realValue("lpe") / ny;
    float lz = fid.procpar().realValue("lpe2") / nz;

    ArrayXi pelist = fid.procpar().realValues("pelist").cast<int>();

    Nifti::Header outHdr(nx, ny, nz, 2, lx, ly, lz, 1, Nifti::DataType::COMPLEX128);
    Nifti::File output(outHdr, outpath);
    MultiArray<complex<float>, 4> k({nx, ny, nz, 2});

    for (int z = 0; z < nz; z++) {
        cout << "Reading block " << z << endl;
        vector<complex<float>> block = fid.readBlock(z);

        int i = 0;
        for (int y = 0; y < ny; y++) {
            int yind = ny / 2 + pelist[y];
            for (int x = 0; x < nx; x++) {
                k[{x, y, z, 0}] = block.at(i++);
            }
            for (int x = 0; x < nx; x++) {
                k[{x, y, z, 1}] = block.at(i++);
            }
        }
    }

    if (!kspace) {
        for (int v = 0; v < 2; v++) {
            MultiArray<complex<float>, 3> vol = k.slice<3>({0,0,0,v},{-1,-1,-1,0});
            fft_and_shift_X(vol);
            fft_and_shift_Y(vol);
            fft_and_shift_Z(vol);
        }
    }
    cout << "Writing volume" << endl;
    output.writeVolumes(k.begin(), k.end(), 0, 2);
    output.close();
}

int main(int argc, char **argv) {
    int indexptr = 0, c;
    string outPrefix = "";
    bool zip = false, kspace = false;

    static struct option long_options[] = {
        {"out", required_argument, 0, 'o'},
        {"zip", no_argument, 0, 'z'},
        {"kspace", required_argument, 0, 'k'},
        {0, 0, 0, 0}
    };
    static const char *short_options = "o:zk";

    while ((c = getopt_long(argc, argv, short_options, long_options, &indexptr)) != -1) {
        switch (c) {
        case 0: break; // It was an option that just sets a flag.
        case 'o': outPrefix = string(optarg); break;
        case 'z': zip = true; break;
        case 'k': kspace = true; break;
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

    if (seqfil.substr(0, 5) == "mge3d") {
        cout << "MGE Recon" << endl;
        reconMGE(fid, outPath, kspace);
    } else if (seqfil.substr(0, 7) == "mp2rage") {
        cout << "mp2rage recon" << endl;
        reconMP2RAGE(fid, outPath, kspace);
    }

    return 0;
}

