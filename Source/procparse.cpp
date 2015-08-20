//
//  main.c
//  procparse
//
//  Created by Tobias Wood on 10/07/2012.
//  Copyright (c) 2012 Tobias Wood. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <getopt.h>

#include "Nifti/Nifti.h"
#include "Nifti/ExtensionCodes.h"
#include "procpar.h"

using namespace std;
using namespace Agilent;
using namespace Nifti;

static int partial = false, verbose = false, verbatim = false;
static struct option long_options[] =
{
	{"partial", no_argument, &partial, true},
	{"verbose", no_argument, &verbose, true},
	{"verbatim", no_argument, &verbatim, true},
	{"in", required_argument, 0, 'i'},
	{0, 0, 0, 0}
};
const string usage {
"procparse - A utility to find interesting information in Agilent procpar files.\n\
\n\
Usage: procparse [opts] file1 par1 par2 ... parN\n\
par1 to parN are parameter names to search for in procpar. If none are specified \
then the whole file will be listed.\n\
Options:\n\
 --verbose, -v  : Print filenames, parameter names, and other information.\n\
 --verbatim, -b : Print the full parameter definition\n\
 --partial, -p  : Allow partial matches for parameter names\n\
 --in file, -i  : Read additional procpar files (can specify more than once).\n"
};

int main(int argc, char **argv) {
	int indexptr = 0, c;
	list<string> paths;
	while ((c = getopt_long(argc, argv, "phvbi:", long_options, &indexptr)) != -1) {
		switch (c) {
			case 0: break; // It was an option that just sets a flag.
			case 'v': verbose = true; break;
			case 'b': verbatim = true; break;
			case 'p': partial = true; break;
			case 'i': paths.emplace_back(optarg); break;
			case 'h': cout << usage << endl; break;
			default: cout << "Unknown option " << optarg << endl;
		}
	}
	
	if ((argc - optind) <= 0) {
		cout << "No procpar file specified." << endl << usage << endl;
		return EXIT_FAILURE;
	}
	paths.emplace_front((argv[optind++]));
		
	vector<ProcPar> pps;
	for (auto &p : paths) {
		ProcPar pp;
		try {
			if ((p.find(".nii") != string::npos) || (p.find(".hdr") != string::npos)) {
				File nii(p);
				const list<Extension> &exts = nii.extensions();
				bool found = false;
				for (auto &e : exts) {
					if (e.code() == NIFTI_ECODE_COMMENT) {
						string s(e.data().begin(), e.data().end());
						stringstream ss(s);
						ss >> pp;
						pps.push_back(pp);
						found = true;
					}
				}
				if (!found) {
					throw(runtime_error("Could not find procpar in header of file: " + p));
				}
			} else {
				ifstream pp_file(p);
				pp_file >> pp;
				if (!pp_file.eof()) {
					throw(runtime_error("Failed to read contents of file: " + p));
				}
				pps.push_back(pp);
			}
		} catch (exception &e) {
			cerr << e.what() << endl;
			return EXIT_FAILURE;
		}
	}
	
	while (optind < argc) {
		string searchName(argv[optind]);
		auto pp_it = pps.begin();
		auto path_it = paths.begin();
		for (; pp_it != pps.end() && path_it != paths.end(); pp_it++, path_it++) {
			if (verbose) {
				cout << *path_it << " ";
			}
			if (partial) {
				size_t matches(0);
				vector<string> names = pp_it->names();
				if (verbose)
					cout << "Partial matches for: " << searchName << endl;
				for (auto &n : names) {
					if (n.find(searchName) != string::npos) {
						if (verbatim) {
							cout << pp_it->parameter(n) << endl;
						} else {
							if (verbose) {
								cout << n << ": ";
							}
							cout << pp_it->parameter(n).print_values() << endl;
						}
						matches++;
					}
				}
				if (verbose)
					cout << matches << " matches." << endl << endl;
			} else if (pp_it->contains(searchName)) {
				if (verbatim) {
					cout << pp_it->parameter(searchName) << endl;
				} else {
					if (verbose) {
						cout << searchName << ": ";
					}
					cout << pp_it->parameter(searchName).print_values() << endl;
				}
			} else {
				if (verbose)
					cout << "Parameter not found: " << searchName << endl;
			}
		}
		optind++;
	}
    return EXIT_SUCCESS;
}

