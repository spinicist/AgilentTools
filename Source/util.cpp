//
//  recon_util.cpp
//  Nrecon
//
//  Created by Tobias Wood on 11/12/2012.
//
//

#include "util.h"

namespace Agilent {

Endianness HostEndianness() {
	union {
		unsigned char ch[sizeof(short)];
		short         ss;
	} byteOrder;
	// Little endian will put this in byteOrder.ch[0],
	// big endian will put this in byteOrder.ch[1]
	byteOrder.ss = 1;
	if (byteOrder.ch[1] == 1)
		return BigEndian;
	else
		return LittleEndian;
}

} // End namespace Agilent