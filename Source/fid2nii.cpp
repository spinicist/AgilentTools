//
//  main.cpp
//  Nrecon
//
//  Created by Tobias Wood on 21/11/2012.
//
//

#include <string>
#include <iostream>
#include <algorithm>
#include <getopt.h>

#include "Eigen/Dense"
#include "unsupported/Eigen/FFT"

#include "fid.h"
#include "niiNifti.h"
#include "MultiArray.h"

using namespace std;
using namespace Eigen;

bool verbose = false;

void phase_correct_3(MultiArray<complex<float>, 3> & a, Agilent::FID &fid) {
    float ppe = fid.procpar().realValue("ppe");
    float ppe2 = fid.procpar().realValue("ppe2");

    float lpe = fid.procpar().realValue("lpe");
    float lpe2 = fid.procpar().realValue("lpe2");

    float ph = -2*M_PI*ppe/lpe;
    float ph2 = -2*M_PI*ppe2/lpe2;

    for (int z = 0; z < a.dims()[2]; z++) {
        const complex<float> fz = polar(1.f, ph2*z);
        for (int y = 0; y < a.dims()[1]; y++) {
            const complex<float> fy = polar(1.f, ph*y);
            for (int x = 0; x < a.dims()[0]; x++) {
                complex<float> val = a[{x,y,z}];
                val = val * fy * fz;
                a[{x,y,z}] = val;
            }
        }
    }
}

void fft_shift_3(MultiArray<complex<float>, 3> & a) {
    int x2 = a.dims()[0] / 2;
    int y2 = a.dims()[1] / 2;
    int z2 = a.dims()[2] / 2;

    MultiArray<complex<float>, 3>::Index size_half{x2,y2,z2};
    MultiArray<complex<float>, 3>::Index start_1{0,0,0};
    MultiArray<complex<float>, 3>::Index start_2{x2,y2,z2};

    for (int i = 0; i < 4; i++) {
        auto local_1 = start_1;
        auto local_2 = start_2;
        if (i < 3) {
            local_1[i] = a.dims()[i] / 2;
            local_2[i] = 0;
        }

        auto octant_1 = a.slice<3>({local_1}, {size_half});
        auto octant_2 = a.slice<3>({local_2}, {size_half});

        std::swap_ranges(octant_1.begin(), octant_1.end(), octant_2.begin());
    }
}

void fft_X(MultiArray<complex<float>, 3> &a) {
    FFT<float> fft;
    int nx = a.dims()[0];
    for (int z = 0; z < a.dims()[2]; z++) {
        for (int y = 0; y < a.dims()[1]; y++) {
            VectorXcf fft_in = a.slice<1>({0,y,z},{nx,0,0}).asArray();
            VectorXcf fft_out(nx);
            fft.fwd(fft_out, fft_in);
            a.slice<1>({0,y,z},{nx,0,0}).asArray() = fft_out;
        }
    }
}

void fft_Y(MultiArray<complex<float>, 3> &a) {
    FFT<float> fft;
    int ny = a.dims()[1];
    for (int z = 0; z < a.dims()[2]; z++) {
        for (int x = 0; x < a.dims()[0]; x++) {
            VectorXcf fft_in = a.slice<1>({x,0,z},{0,ny,0}).asArray();
            VectorXcf fft_out(ny);
            fft.fwd(fft_out, fft_in);
            a.slice<1>({x,0,z},{0,ny,0}).asArray() = fft_out;
        }
    }
}

void fft_Z(MultiArray<complex<float>, 3> &a) {
    FFT<float> fft;
    int nz = a.dims()[2];
    for (int x = 0; x < a.dims()[0]; x++) {
        for (int y = 0; y < a.dims()[1]; y++) {
            VectorXcf fft_in = a.slice<1>({x,y,0},{0,0,nz}).asArray();
            VectorXcf fft_out(nz);
            fft.fwd(fft_out, fft_in);
            a.slice<1>({x,y,0},{0,0,nz}).asArray() = fft_out;
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
    if (verbose) cout << "Reading MGE fid" << endl;
    for (int a = 0; a < narray; a++) {
        if (verbose) cout << "Reading block " << a << endl;
        shared_ptr<vector<complex<float>>> block = make_shared<vector<complex<float>>>();
        *block = fid.readBlock(a);
        int e_offset = 0;
        for (int e = 0; e < ne; e++) {
            if (verbose)  cout << "Reading echo " << e << endl;
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
    int nti = 3;
    ArrayXi pelist = fid.procpar().realValues("pelist").cast<int>();
    MultiArray<complex<float>, 4> k({nx, ny, nz, nti});

    if (verbose) {
        cout << "Reading mpXrage fid" << endl;
        cout << "Expecting " << nti << " inversion times" << endl;
    }
    for (int z = 0; z < nz; z++) {
        if (verbose) cout << "Reading block " << z << endl;
        vector<complex<float>> block = fid.readBlock(z);

        int i = 0;
        for (int v = 0; v < nti; v++) {
            for (int y = 0; y < ny; y++) {
                int yind = ny / 2 + pelist[y];
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
    bool zip = false, kspace = false, procpar = false;
    Nifti::DataType dtype = Nifti::DataType::COMPLEX64;
    static struct option long_options[] = {
        {"out", required_argument, 0, 'o'},
        {"zip", no_argument, 0, 'z'},
        {"kspace", required_argument, 0, 'k'},
        {"procpar", no_argument, 0, 'p'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    static const char *short_options = "o:zkmpv";

    while ((c = getopt_long(argc, argv, short_options, long_options, &indexptr)) != -1) {
        switch (c) {
        case 0: break; // It was an option that just sets a flag.
        case 'o': outPrefix = string(optarg); break;
        case 'z': zip = true; break;
        case 'k': kspace = true; break;
        case 'm': dtype = Nifti::DataType::FLOAT32; break;
        case 'p': procpar = true; break;
        case 'v': verbose = true; break;
        case '?': // getopt will print an error message
            return EXIT_FAILURE;
        default:
            cout << "Unhandled option " << string(1, c) << endl;
            return EXIT_FAILURE;
        }
    }

    if ((argc - optind) <= 0) {
        cout << "No .fids specified" << endl;
        return EXIT_FAILURE;
    }

    while (optind < argc) {
        string inPath(argv[optind++]);
        size_t fileSep = inPath.find_last_of("/") + 1;
        size_t fileExt = inPath.find_last_of(".");
        if ((fileExt == string::npos) || (inPath.substr(fileExt) != ".fid")) {
            cerr << inPath << " is not a valid .fid directory" << endl;
        }
        string outPath = outPrefix + inPath.substr(fileSep, fileExt - fileSep) + ".nii";
        if (zip)
            outPath = outPath + ".gz";

        Agilent::FID fid(inPath);


        string apptype = fid.procpar().stringValue("apptype");
        string seqfil  = fid.procpar().stringValue("seqfil");

        if (apptype != "im3D") {
            cerr << "apptype " << apptype << " not supported, skipping." << endl;
            continue;
        }

        if (verbose) {
            cout << fid.print_info() << endl;
            cout << "apptype = " << apptype << endl;
            cout << "seqfil  = " << seqfil << endl;
        }

        MultiArray<complex<float>, 4> vols;
        if (seqfil.substr(0, 5) == "mge3d") {
            vols = reconMGE(fid);
        } else if (seqfil.substr(0, 7) == "mp3rage") {
            vols = reconMP2RAGE(fid);
        } else {
            cerr << "Recon for " << seqfil << " not implemented, skipping." << endl;
            continue;
        }

        if (!kspace) {
            for (int v = 0; v < vols.dims()[3]; v++) {
                if (verbose) cout << "FFTing vol " << v << endl;
                MultiArray<complex<float>, 3> vol = vols.slice<3>({0,0,0,v},{-1,-1,-1,0});
                phase_correct_3(vol, fid);
                fft_shift_3(vol);
                fft_X(vol);
                fft_Y(vol);
                fft_Z(vol);
                fft_shift_3(vol);
            }
        }

        if (verbose) cout << "Writing file: " << outPath << endl;
        float lx = fid.procpar().realValue("lro") / vols.dims()[0];
        float ly = fid.procpar().realValue("lpe") / vols.dims()[1];
        float lz = fid.procpar().realValue("lpe2") / vols.dims()[2];
        ArrayXf voxdims(4); voxdims << lx, ly, lz, 1;
        list<Nifti::Extension> exts;
        if (procpar) {
            if (verbose) cout << "Embedding procpar" << endl;
            ifstream pp_file(inPath + "/procpar", ios::binary);
            pp_file.seekg(ios::end);
            size_t fileSize = pp_file.tellg();
            pp_file.seekg(ios::beg);
            vector<char> data; data.reserve(fileSize);
            data.assign(istreambuf_iterator<char>(pp_file), istreambuf_iterator<char>());
            exts.emplace_back(NIFTI_ECODE_COMMENT, data);
        }
        Nifti::Header outHdr(vols.dims(), voxdims, dtype);
        outHdr.setTransform(fid.procpar().calcTransform());
        Nifti::File output(outHdr, outPath, exts);
        output.writeVolumes(vols.begin(), vols.end(), 0, vols.dims()[3]);
        output.close();
    }
    return 0;
}

