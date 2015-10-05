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
#include <exception>
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

MultiArray<float, 3> Hanning3D(MultiArray<float, 3>::Index dims, const float a);
MultiArray<float, 3> Hanning3D(MultiArray<float, 3>::Index dims, const float a) {
    const int nx = dims[0];
    const int ny = dims[1];
    const int nz = dims[2];
    const int x_2 = nx / 2;
    const int y_2 = ny / 2;
    const int z_2 = nz / 2;
    const float r_m = sqrt(static_cast<float>(x_2*x_2 + y_2*y_2 + z_2*z_2));
    MultiArray<float, 3> filter(dims);
    for (int z = 0; z < nz; z++) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                float rad = sqrt(static_cast<float>((x - x_2)*(x - x_2) + (y - y_2)*(y - y_2) + (z - z_2)*(z - z_2))) / r_m;
                filter[{x,y,z}] = (1 + a*cos(M_PI*rad)) / (1 + a);
            }
        }
    }
    return filter;
}

MultiArray<float, 3> Tukey3D(MultiArray<float, 3>::Index dims, const float a, const float q);
MultiArray<float, 3> Tukey3D(MultiArray<float, 3>::Index dims, const float a, const float q) {
    const int nx = dims[0];
    const int ny = dims[1];
    const int nz = dims[2];
    const int x_2 = nx / 2;
    const int y_2 = ny / 2;
    const int z_2 = nz / 2;
    const float r_m = sqrt(static_cast<float>(x_2*x_2 + y_2*y_2 + z_2*z_2));
    MultiArray<float, 3> filter(dims);
    for (int z = 0; z < nz; z++) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                float rad = sqrt(static_cast<float>((x - x_2)*(x - x_2) + (y - y_2)*(y - y_2) + (z - z_2)*(z - z_2))) / r_m;
                filter[{x,y,z}] = (rad <= (1 - a)) ? 1 : 0.5*((1+q)+(1-q)*cos((M_PI/a)*(rad - 1 + a)));
            }
        }
    }
    return filter;
}

void ApplyFilter3D(MultiArray<complex<float>, 3> ks, MultiArray<float, 3> filter);
void ApplyFilter3D(MultiArray<complex<float>, 3> ks, MultiArray<float, 3> filter) {
    if ((ks.dims() != filter.dims()).any()) {
        throw(runtime_error("K-space and filter dimensions do not match."));
    }

    auto k_it = ks.begin();
    auto f_it = filter.begin();
    while (k_it != ks.end()) {
        *k_it = (*k_it) * (*f_it);
        k_it++;
        f_it++;
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
    const int nx = fid.procpar().realValue("np") / 2;
    const int ny = fid.procpar().realValue("nv");
    const int nz = fid.procpar().realValue("nv2");
    const int nseg = fid.procpar().realValue("nseg");
    const int ny_per_seg = ny / nseg;
    const int nti = (fid.procpar().stringValue("mp3rage_flag") == "y") ? 3 : 2;
    ArrayXi pelist = fid.procpar().realValues("pelist").cast<int>();
    MultiArray<complex<float>, 4> k({nx, ny, nz, nti});

    if (verbose) {
        cout << "Reading mp3rage fid" << endl;
        cout << "Expecting " << nti << " inversion times" << endl;
    }
    for (int z = 0; z < nz; z++) {
        if (verbose) cout << "Reading block " << z << endl;
        vector<complex<float>> block = fid.readBlock(z);

        int i = 0;
        int yseg = 0;
        for (int s = 0; s < nseg; s++) {
            for (int v = 0; v < nti; v++) {
                for (int y = 0; y < (ny / nseg); y++) {
                    int yind = ny / 2 + pelist[yseg + y];
                    for (int x = 0; x < nx; x++) {
                        k[{x, yind, z, v}] = block.at(i++);
                    }
                }
            }
            yseg += ny_per_seg;
        }
    }

    return k;
}

enum class Filters { None, Hanning, Tukey };
static struct option long_options[] = {
    {"out", required_argument, 0, 'o'},
    {"zip", no_argument, 0, 'z'},
    {"kspace", required_argument, 0, 'k'},
    {"procpar", no_argument, 0, 'p'},
    {"filter", required_argument, 0, 'f'},
    {"mag", no_argument, 0, 'm'},
    {"fa", required_argument, 0, 'a'},
    {"fq", required_argument, 0, 'q'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};
static const char *short_options = "o:zkmpf:v";
const string usage {
"fid2nii - A utility to reconstruct Agilent fid bundles in nifti format.\n\
\n\
Usage: fid2nii [opts] image1 image2 ... imageN\n\
image1 to imageN are paths to the Agilent .fid folders\n\
Options:\n\
    --verbose, -v  : Print out extra info (e.g. after each volume is written).\n\
    --out, -o      : Specify an output prefix.\n\
    --zip, -z      : Create .nii.gz files\n\
    --mag, -m      : Save magnitude images, not complex.\n\
    --scale, -s N  : Multiply image dimensions by N (set to 10 for use with SPM).\n\
    --procpar, -p  : Embed procpar in the nifti header.\n\
    --kspace, -k   : Don't FFT, write out k-space instead.\n\
    --filter, -f h : Use a Hanning filter.\n\
                 t : Use a Tukey filter.\n\
    --fa=X         : Specify the filter alpha parameter.\n\
    --fq=X         : Specify the q parameter (Tukey only).\n"
};

int main(int argc, char **argv) {
    int indexptr = 0, c;
    string outPrefix = "";
    bool zip = false, kspace = false, procpar = false;
    Filters filterType = Filters::None;
    float f_a = 0, f_q = 0;
    Nifti::DataType dtype = Nifti::DataType::COMPLEX64;
    Affine3f scale; scale = Scaling(1.f);

    while ((c = getopt_long(argc, argv, short_options, long_options, &indexptr)) != -1) {
        switch (c) {
        case 0: break; // It was an option that just sets a flag.
        case 'o': outPrefix = string(optarg); break;
        case 'z': zip = true; break;
        case 'k': kspace = true; break;
        case 'm': dtype = Nifti::DataType::FLOAT32; break;
        case 's': scale = Scaling(static_cast<float>(atof(optarg))); break;
        case 'f':
            switch (*optarg) {
            case 'h':
                filterType = Filters::Hanning;
                f_a = 0.1;
                break;
            case 't':
                filterType = Filters::Tukey;
                f_a = 0.75;
                f_q = 0.25;
                break;
            default:
                cerr << "Unknown filter type: " << string(optarg, 1) << endl;
                return EXIT_FAILURE;
            }
            break;
        case 'a':
            if (filterType == Filters::None) {
                cerr << "No filter type specified, so f_a is invalid" << endl;
                return EXIT_FAILURE;
            }
            f_a = atof(optarg);
            break;
        case 'q':
            if (filterType != Filters::Tukey) {
                cerr << "Filter type is not Tukey, f_q is invalid" << endl;
                return EXIT_FAILURE;
            }
            f_q = atof(optarg);
            break;
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

        /*
         * Assemble k-Space
         */
        MultiArray<complex<float>, 4> vols;
        if (seqfil.substr(0, 5) == "mge3d") {
            vols = reconMGE(fid);
        } else if (seqfil.substr(0, 7) == "mp3rage") {
            vols = reconMP2RAGE(fid);
        } else {
            cerr << "Recon for " << seqfil << " not implemented, skipping." << endl;
            continue;
        }

        /*
         * Build and apply filter
         */
        MultiArray<float, 3> filter;
        switch (filterType) {
        case Filters::None: break;
        case Filters::Hanning:
            if (verbose) cout << "Building Hanning filter" << endl;
            filter = Hanning3D(vols.dims().head(3), f_a);
            break;
        case Filters::Tukey:
            if (verbose) cout << "Building Tukey filter" << endl;
            filter = Tukey3D(vols.dims().head(3), f_a, f_q);
            break;
        }
        if (filterType != Filters::None) {
            if (verbose) cout << "Applying filter" << endl;
            for (int v = 0; v < vols.dims()[3]; v++) {
                MultiArray<complex<float>, 3> vol = vols.slice<3>({0,0,0,v},{-1,-1,-1,0});
                ApplyFilter3D(vol,filter);
            }
        }
        /*
         * FFT
         */
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
        Affine3f xform  = scale * fid.procpar().calcTransform();
        ArrayXf voxdims = (Affine3f(xform.rotation()).inverse() * xform).matrix().diagonal();
        Nifti::Header outHdr(vols.dims(), voxdims, dtype);
        outHdr.setTransform(xform);
        Nifti::File output(outHdr, outPath, exts);
        output.writeVolumes(vols.begin(), vols.end(), 0, vols.dims()[3]);
        output.close();
    }
    return 0;
}

