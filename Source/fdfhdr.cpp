//
//  fdfhdr_main.cpp
//
//  Created by Tobias Wood on 12/09/2013.
//  Copyright (c) 2013 Tobias Wood. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <getopt.h>

#include "fdf.h"

using namespace std;
using namespace Agilent;

static int partial = false, verbose = false;
static struct option long_options[] = {
	{"partial", no_argument, &partial, true},
	{"verbose", no_argument, &verbose, true},
	{"in", required_argument, 0, 'i'},
	{0, 0, 0, 0}
};
const string usage {
"fdfhdr - A utility to extract info from the header of Agilent fdf files.\n\
\n\
Usage: fdfhdr [opts] file1 par1 par2 ... parN\n\
par1 to parN are parameter names to find in the header. If none are specified \
then the whole file will be listed.\n\
Options:\n\
 -p, --partial:    Print parameters that are partial matches.\n\
 -i, --in file:    Check additional fdf files (can specify more than once).\n\
 -v, --verbose:    Print more information.\n"
};

int main(int argc, char **argv) {
	int indexptr = 0, c;
	list<string> paths;
	while ((c = getopt_long(argc, argv, "fphvi:", long_options, &indexptr)) != -1) {
		switch (c) {
			case 0: break; // It was an option that just sets a flag.
			case 'p': partial = true; break;
			case 'h': cout << usage << endl; break;
			case 'v': verbose = true; break;
			case 'i': paths.emplace_back(optarg); break;
			default: cout << "Unknown option " << optarg << endl;
		}
	}
	
	if ((argc - optind) <= 0) {
		cout << "No fdf file specified." << endl << usage << endl;
		return EXIT_FAILURE;
	}
	paths.emplace_front((argv[optind++]));
		
	vector<fdfFile> fdfs;
	for (auto &p : paths) {
		try {
			if (p.substr(p.size() - 3) == "fdf") {
				fdfs.push_back(p);
			}
		} catch (exception &e) {
			cout << "Could not read file " << p << ", not a valid procpar file." << endl;
		}
	}
	
	if (optind == argc) { // No particular parameters specified, just print whole header
		for (auto &fdf : fdfs) {
			auto &hdr = fdf.header();
			for (auto &f : hdr) {
				cout << f.second << endl;
			}
		}
	}	
	while (optind < argc) {
		string searchName(argv[optind]);
		if (verbose) {
			cout << "Searching for parameter: " << searchName << endl;
		}
		for (auto &fdf : fdfs) {
			if (verbose) {
				cout << "In file: " << fdf.path() << endl;
			}
			auto &hdr = fdf.header();
			if (partial) {
				size_t matches(0);
				for (auto &f : hdr) {
					if (f.first.find(searchName) != string::npos) {
						cout << f.second << endl;
						matches++;
					}
				}
				cout << matches << " matches." << endl;
			} else if (hdr.find(searchName) != hdr.end()) {
					cout << hdr.find(searchName)->second << endl;
			} else {
				if (verbose)
					cout << "Not found." << endl;
			}
		}
		optind++;
	}
    return EXIT_SUCCESS;
}

