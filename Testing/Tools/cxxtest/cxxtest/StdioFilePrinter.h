// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

//
// The StdioFilePrinter is a simple TestListener that
// just prints "OK" if everything goes well, otherwise
// reports the error in the format of compiler messages.
// This class uses <stdio.h>, i.e. FILE * and fprintf().
//

#include <cxxtest/ErrorFormatter.h>
#include <stdio.h>

namespace CxxTest 
{
    class StdioFilePrinter : public ErrorFormatter
    {
    public:
        StdioFilePrinter( FILE *o, const char *preLine = ":", const char *postLine = "" ) :
            ErrorFormatter( new Adapter(o), preLine, postLine ) {}
        virtual ~StdioFilePrinter() { delete outputStream(); }

    private:
        class Adapter : public OutputStream
        {
            Adapter( const Adapter & );
            Adapter &operator=( const Adapter & );
            
            FILE *_o;
            
        public:
            Adapter( FILE *o ) : _o(o) {}
            void flush() { fflush( _o ); }
            OutputStream &operator<<( unsigned i ) { fprintf( _o, "%u", i ); return *this; }
            OutputStream &operator<<( const char *s ) { fputs( s, _o ); return *this; }
            OutputStream &operator<<( Manipulator m ) { return OutputStream::operator<<( m ); }
        };
    };
}
