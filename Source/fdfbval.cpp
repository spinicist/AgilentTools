//
//  fdfbval.cpp
//
//  Created by Tobias Wood on 19/05/2014.
//
//  Copyright Tobias Wood 2014

#include <string>
#include <iostream>
#include <getopt.h>

#include "QUIT/Util.h"
#include "Nifti/Nifti.h"
#include "Nifti/ExtensionCodes.h"

using namespace std;
using namespace QUIT;

const string usage = "\
fdfbval - A utility for extracting diffusion vectors and values from procpar.\n\
\n\
Usage: fdfbval [options] input_file\n\
The input file can be a procpar file or nifti with procpar stored in the\n\
header.\n\
\n\
Options:\n\
	--fsl, -f  : Output in a format for FSL (two files, default)\n\
	--dtk, -d  : Output in a format for dti_recon from TrackVis (one file)\n\
	--help, -h : Print this message and quit.\n";

static struct option long_options[] =
{
	{"fsl", optional_argument, 0, 'f'},
	{"dtk", optional_argument, 0, 'd'},
	{"help", optional_argument, 0, 'h'},
	{0, 0, 0, 0}
};

enum class Mode { FSL, DTK };
static Mode outMode = Mode::FSL;

int main(int argc, char **argv) {
	int indexptr = 0, c;
	while ((c = getopt_long(argc, argv, "dfh", long_options, &indexptr)) != -1) {
		switch (c) {
			case 'd': outMode = Mode::DTK; break;
			case 'f': outMode = Mode::FSL; break;
			case 'h':
				cout << usage << endl;
				return EXIT_SUCCESS;
				break;
			case '?': // getopt will print an error message
				return EXIT_FAILURE;
		}
	}
	if ((argc - optind) != 1) {
		cout << "Exactly one input file (procpar or nifti) required." << endl << usage << endl;
		return EXIT_FAILURE;
	}

	string iname(argv[optind]), outprefix;
	cout << "Reading diffusion directions from: " << iname << endl;
	Agilent::ProcPar pp;
	try {
		Nifti::File f(iname);
		ReadPP(f, pp);
		outprefix = f.basePath();
	} catch (runtime_error &e) {
		ifstream f(iname);
		f >> pp;
		outprefix = iname;
	}

	auto image = pp.realValues("image");
	auto bval  = pp.realValues("bvalue");
	auto dro   = pp.realValues("dro");
	auto dpe   = pp.realValues("dpe");
	auto dsl   = pp.realValues("dsl");
	size_t nvol = (image == 1).cast<int>().sum();
	Eigen::ArrayXf outval(nvol), outro(nvol), outpe(nvol), outsl(nvol);
	size_t j = 0;
	for (size_t i = 0; i < image.rows(); i++) {
		if (image[i] == 1) {
			outval[j] = bval[i];
			outro[j]  = dro[i];
			outpe[j]  = dpe[i];
			outsl[j]  = dsl[i];
			j++;
		}
	}

	switch (outMode) {
	case Mode::FSL: {
		cout << "Writing diffusion values to:     " << outprefix+".bval" << endl;
		cout << "Writing diffusion directions to: " << outprefix+".bvec" << endl;
		ofstream ovals(outprefix + ".bval");
		ofstream ovecs(outprefix + ".bvec");

		ovals << outval.transpose() << endl;
		ovecs << outro.transpose() << endl;
		ovecs << outpe.transpose() << endl;
		ovecs << outsl.transpose() << endl;

		ovals.close();
		ovecs.close();
	} break;
	case Mode::DTK: {
		cout << "Writing diffusion data to: " << outprefix+".gm" << endl;
		ofstream ofile(outprefix+".gm");

		for (size_t i = 0; i < outval.size(); i++) {
			ofile << outro[i] << ", " << outpe[i] << ", " << outsl[i] << ", " << outval[i] << endl;
		}

		ofile.close();
	} break;
	}
	cout << "Finished." << endl;
}
