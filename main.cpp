/**    PSblank
* By: Javier Oscar Cordero Pérez
* License: MIT-X
* Description: Add extra blank pages at the end of a DSC-conforming PostScript.
* Copyright 2014
**/

#include <cstdlib> // atoi()
#include <iostream> // console IO
#include <fstream> // file IO
#include <sstream> // read and store multiple data types from PS
#include "javistd.h" // header and debug
using namespace std;

javistd program( "PSblank", "1.0",
    "Adds extra blank pages at the end of a DSC-conforming PostScript.\n" );

static int showUsage()
{
    program.showHeader();
    cout << "Usage:\t<num_of_additional_pages> <input_file> <output_file>\n";
    return 0;
}

int die( string msg )
{
    cout << msg << '\n';
    return 0;
}

int main( int argc, char* argv[] )
{
    // PROGRAM VARIABLES
    bool useStdIn = false,
         useStdOut = false;
    ifstream instream;
    ofstream outstream;
    streambuf *currentOut,
              *backup=cout.rdbuf();
    int toAdd=0;

    // VARIABLES FROM POSTSCRIPT
    string line="";
    string lastPageName="";
    int lastPageId=0,
        lastPBoundingBox[4]={0};

    // ARGUMENT PROCESSING
    toAdd = atoi(argv[1]);
    if (argc<2 || toAdd==0)
        return showUsage();
    else if (string(argv[1])=="-h"||string(argv[1])=="--help"
           ||string(argv[1])=="/h"||string(argv[1])=="/help")
        return showUsage();

    // POSITIONAL ARGUMENTS, determine input and destination streams.
    switch ( argc-2 ) // argc-2 may be replaced with arrayOfPosArgs.lenght()
    {
        case 0:
            // Use stdin and stdout.
            useStdOut = true;
        case 1:
            instream.open( argv[2] );
            if ( !instream.is_open() )
                useStdIn = true;
            else
            {
                // If file at argv[2] isn't DSC-PS, assume STDIN.
                getline(instream, line);
                if ( static_cast<bool>(line.compare(0,1,"%")) )
                    useStdIn = true;
                else
                    // If it reads fine, use stdout.
                    useStdOut = true;
            }
            break;
        case 2:
            instream.open( argv[2] );
            if ( !instream.is_open() )
                die("Failed to open input file.\n");
            else
            {
                getline(instream, line);
                // If file at argv[2] isn't DSC-PS, quit.
                if ( static_cast<bool>(line.compare(0,1,"%")) )
        die("Input file's header isn't that of a DSC-conforming PostScript.");
            }
            break;
            // Use parsed files.
        default:
            return showUsage();
            // Accept no less or more than 1-3 arguments.
    }
    // continue case 1
    if (useStdIn)
    {
        getline(cin, line);
        if ( static_cast<bool>(line.compare(0,10,"%!PS-Adobe")) )
            return showUsage();
    }
    // continue case 0
    else if ( argc-2==0 )
        return showUsage();

    // SET INPUT SOURCE
    if (!useStdIn)
        cin.rdbuf( instream.rdbuf() );

    // SET OUTPUT DESTINATION
    if (!useStdOut)
    {
        // Open said file and make sure it opened.
        /*
          GS has trouble rendering files following the CR+LF EOL convention.
          This program outputs in binary mode and uses <'\n'> instead of <endl>,
          to enforce the LF EOL convention used in *nix systems.
          Note that using stdout on Windows will produce CR+LF files anyway.
        */
        outstream.open( argc<4? argv[2] : argv[3] , ios::trunc | ios::binary );
        if ( !outstream.is_open() )
            die( "Could not open output file.\n" );
        // If so, redirect cout to file.
        currentOut = outstream.rdbuf();   // get file's streambuf
        cout.rdbuf( currentOut );         // assign streambuf to cout
    } // end <if> not using stdout. No <else> because stdout is used by default.

    // READY!
    if ( line=="" )
        getline( cin, line );
    if ( !cin.fail() )
        do
        {
            // GET CURRENT PAGE
            if ( !static_cast<bool>(line.compare(0,7,"%%Page:")) )
            {
                string temp;
                stringstream localStream;
                localStream << line;
                localStream >> temp >> lastPageName >> lastPageId;
                cout << line << '\n';
                //cout << temp <<' '<< lastPageName <<' '<< lastPageId << '\n';
            }
            // GET CURRENT PageBoundingBox
            else if(!static_cast<bool>(line.compare(0,18,"%%PageBoundingBox:")))
            {
                string temp;
                stringstream localStream;
                localStream << line;
                localStream >> temp >> lastPBoundingBox[0]
                    >> lastPBoundingBox[1] >> lastPBoundingBox[2]
                    >> lastPBoundingBox[3];
                cout << line << '\n';
                //cout << temp << ' ' << lastPBoundingBox[0] << ' '
                //    << lastPBoundingBox[1] << ' ' << lastPBoundingBox[2]
                //    << ' ' << lastPBoundingBox[3] << '\n';
            }
            // ADD BLANK PAGE AT END
            else if ( !static_cast<bool>(line.compare(0,9,"%%Trailer")) )
            {
                for (int i=0; i<toAdd; i++)
                {
                    // Follow the DSC convention for each page.
                    cout << "%%PageTrailer" << '\n' << '\n';
                    cout << "%%Page: " << lastPageId+1+i << ' '
                                       << lastPageId+1+i << '\n';
                    cout << "%%PageBoundingBox: " << lastPBoundingBox[0] <<' '
                        << lastPBoundingBox[1] <<' '<< lastPBoundingBox[2] <<' '
                        << lastPBoundingBox[3] << '\n';
                    cout << "%%EndPageComments" << '\n';
                    cout << "%%BeginPageSetup" << '\n';
                    cout << "%%EndPageSetup" << '\n';
                    // Show the page in PostScript.
                    cout << "showpage" << '\n';
                }
                //cout << temp << ' ' << lastPageId+1 << "]%%) =" << '\n';
                cout << line << '\n';
            }
            // UPDATE TOTAL PAGES
            else if ( !static_cast<bool>(line.compare(0,8,"%%Pages:")) ||
                      !static_cast<bool>(line.compare(0,7,"/Pages ")) )
            {
                if ( !static_cast<bool>(line.compare(9,7,"(atend)")) )
                    cout << line << '\n';
                else
                {
                    int totalPages=0;
                    stringstream localStream;
                    string tmp1,tmp2="";
                    localStream << line;
                    localStream >> tmp1 >> totalPages >> tmp2;
                    cout << tmp1 << ' ' << totalPages+toAdd;
                    if ( static_cast<bool>(tmp2.length()) )
                        cout <<' '<< tmp2;
                    cout << '\n';
                }
            }
            else
                cout << line << '\n';
        // GET NEXT LINE
        } while ( getline( cin, line ) );

    // Close file.
    cout.flush();
    cout.rdbuf(backup);
    outstream.close();

    return 0;
}
