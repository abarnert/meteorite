/***********************************(GPL)********************************
*   Meteorite is MKV/Matroska Video Repair Engine.                      *
*   Copyright (C) 2009  Erdem U. Altinyurt                              *
*                                                                       *
*   This program is free software; you can redistribute it and/or       *
*   modify it under the terms of the GNU General Public License         *
*   as published by the Free Software Foundation; either version 2      *
*   of the License.                                                     *
*                                                                       *
*   This program is distributed in the hope that it will be useful,     *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*   GNU General Public License for more details.                        *
*                                                                       *
*   You should have received a copy of the GNU General Public License   *
*   along with this program;                                            *
*   if not, write to the Free Software Foundation, Inc.,                *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*               home  : meteorite.sourceforge.net                       *
*               email : spamjunkeater at gmail.com                      *
*************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <math.h>
#include <vector>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#include <wx/thread.h>
#include <wx/gauge.h>
#include <wx/msgdlg.h>
//#include <omp.h>

#include <algorithm>

#include "endian.h"

#ifndef _meteorite_h_
#define _meteorite_h_

#define METEORITE_VERSION "0.12 Beta Development"

using namespace std;

enum types { subElements, uInteger, sInteger, floating, date, str, UTF8, binary, error };

int EBMLsize( uint64_t val );
int ByteCount( uint64_t val );
int ByteCount( int64_t val );
int64_t& to_bigendianWR( int64_t x );
uint64_t& to_bigendianWR( uint64_t x );
uint64_t EBMLtouInteger(char *bfr, unsigned *ptr=NULL);
char* EBMLtoWritebuff( uint64_t x );
uint64_t uIntegertoEbml( uint64_t val );

class ID_Element{	//Virtual Class that used by other elements
	public:
	ID_Element();
	ID_Element( string _name, uint64_t _location=0 );//forward decleration for need of IDDB
	ID_Element( string _name, uint32_t _ID, types _type, uint64_t _location=0 );
	string name;
	uint32_t ID;
	short IDSize();
	enum types type;
	uint64_t location;
virtual void print( bool endline=true );
virtual void write( ofstream& );
virtual unsigned Size( void );
virtual ~ID_Element();
	};
class stringElement : public ID_Element{	//Class that handles String and UTF elements.
	public:
		stringElement( ID_Element id);
		string data;
	void print( bool endline );
	unsigned Size( void );
	void write( ofstream &outfile );
	string read( char *bfr, unsigned *ptr=NULL );
	};
class uintElement : public ID_Element{		//Class that handles Unsigned integer elements.
	public:
	uintElement( ID_Element id );
	uint64_t data;
	void print(bool endline=true);
	unsigned Size( void );
	void write( ofstream &outfile );
	uint64_t read( char *bfr, unsigned *ptr=NULL );
	};
class sintElement : public ID_Element{		//Class that handles Signed integer elements.
	public:
	sintElement( ID_Element id);
	int64_t data;
	void print(bool endline=true);
	unsigned Size( void );
	void write( ofstream &outfile );
	int64_t read( char *bfr, unsigned *ptr=NULL );
	};
class floatElement : public ID_Element{	//Class that handles floating-point elements.
	public:
	floatElement( ID_Element id);
	double data;
	void print(bool endline=true);
	unsigned Size( void );
	void write( ofstream &outfile );
	double read( char *bfr, unsigned *ptr=NULL );
	};
class subElement : public ID_Element{		//Class that handles sub elements.
	public:
	subElement( string name, uint64_t location=0 );
	subElement( ID_Element id);
	vector <ID_Element *> data;
	void print(bool endline=true);
	unsigned Size( void );
	unsigned subsize( void );
	void write( ofstream &outfile );		//non recursive function
	~subElement();
	};
class binElement : public ID_Element{		//Class that handles binary elements.

	public:
	binElement( ID_Element id);
	char* data;
	int datasize;
	void print(bool endline=true);
	unsigned Size( void );
	void write( ofstream &outfile );
	void read( char *bfr, unsigned *ptr=NULL );
	~binElement();
	};
class dateElement : public uintElement{	//Class that handles date elements.

	public:
	dateElement( ID_Element id);
	void print(bool endline=true);
	unsigned Size( void );
	void write( ofstream &outfile );
	};
class HeadIDs{		//This class holds head elements for searching.
	public:
	void AddElement( ID_Element *eID );
	ID_Element* Search( uint32_t ID );
	vector< ID_Element* > idvector;
	};
class IDDB{		//This class for include all nodes for searching. ID Database in short.
	public:
	HeadIDs EBML;
	HeadIDs Segment;
	HeadIDs MetaSeek;
	HeadIDs Info;
	HeadIDs Attachments;
	HeadIDs Cluster;
	HeadIDs Cues;
	HeadIDs Tracks;
	HeadIDs Tags;
	HeadIDs Chapters;
	vector< HeadIDs > hivector;

	ID_Element* Search( uint32_t ID );
	HeadIDs* SearchHeadID( uint32_t ID );
	uint32_t IDof( string name );
	types TypeOf( string name );
	ID_Element* SearchName( string name );
	IDDB();
	};
struct Block{		//Block structure for read/store block informations. Structure don't store binary data of block itself
	Block();
	void Print();
	bool Valid;
	unsigned TrackNum;
	short TimeCode;
	uint8_t Flags;
	bool KeyFlag;
	int Duration;
	unsigned Size;
	unsigned Frames;
	enum lace { Lace_NO=0 ,Lace_Xiph=1, Lace_Fixed=2, Lace_EBML=3 };
	lace Lacing;
	};

#define bfr_size 64*1024*1024	//64MB buffer
class Meteorite{	//The main class of the poject_Meteorite
	public:
		Meteorite( wxGauge *WxGauge=NULL );
	protected:
		wxGauge    *WxGauge;

		IDDB DB;
		char four_cc[5];
		bool update_gauge( int percent );
		uint64_t filesize;

	public:
		void Init(void);
		void SetGauge( wxGauge* );
		uint32_t IDof( string name );
		bool is_IDof( char* bfr, string name);
		ID_Element* FindElementID( HeadIDs hid, char *bfr );
		ID_Element* FindElementID( char *bfr );
		void skip( char *bfr, unsigned& i, types type );
		subElement* TreeParser( char* a );

		// Generate and return parsed subElement chunk pointer.
		subElement* TreeParserChunkMaster( char *bfr, unsigned length, uint64_t _location=0 );

		// Generate SubElements inners and attach them to subElment el_ptr
		void TreeParserChunk( char *bfr, unsigned length, subElement *el_ptr);

		void TreeParserPrint( subElement* el_ptr, unsigned lvl=0 );
		void TreeParserPrintHR( subElement* el_ptr, unsigned lvl=0 );
		void TreeParserCompare( subElement* el_ptr1, subElement* el_ptr2 );

		// Writes el_ptr's ingredients to file, not el_ptr self!
		void TreeParserWrite( subElement* el_ptr, ofstream &outfile, unsigned lvl=0 );
		// Writes el_ptr self!
		void TreeParserWriteMaster( subElement* el_ptr, ofstream &outfile, unsigned lvl=0 );

		bool CheckCluster( char *bfr );
		subElement* ClusterRecover( char *bfr, unsigned size, unsigned MaxTrackNum );
inline bool IsKeyFrame( char *data, unsigned size, char *four_cc );
		Block BlockGroupAnalysis( char *bfr, unsigned TrackCount );
		Block SimpleBlockAnalysis( char *bfr );
		Block BlockDataAnalysis( char *bfr, unsigned size, bool SimpleBlock=false );
		uint64_t ClusterDuration( subElement* el_ptr, unsigned default_duration, unsigned TrackNo );
		uint64_t ClusterDuration( vector< Block >& vb, unsigned DefaultFrameDuration, unsigned TrackNo );
		uint64_t ClusterMaximumBlockTimePlusDuration( vector< Block >& vb, unsigned DefaultFrameDuration, unsigned TrackNo );
		short ClusterMinimumBlockTime( vector< Block >& vb, unsigned TrackNo );
		char* Clusterart();
		void Generate_CueTimeCodes( vector< Block >& vb, subElement* Cues_ptr, uint64_t ClusterPosition, uint64_t ClusterTimeCode, unsigned DefaultFrameDuration, unsigned TrackNo=1 );
		subElement* MakeCuePoint( uint64_t ClusterPosition, unsigned TrackNo, uint64_t CueTime );
		vector<Block> ClusterBlocksAnalysis( subElement* el_ptr, subElement* root );
		void ClusterAnalysis( char *bfr, unsigned defaultduration, uint64_t cluster_nanotimecode,
								uint64_t ThisClusterPosition, uint64_t& ClusterLocalDuration,
								short& ClusterMinusTimeElement, subElement *NewCues, unsigned TrackNum );
ID_Element* Get( subElement *el_ptr, uint32_t IDofKey );
ID_Element* Get( subElement *el_ptr, vector< uint32_t > hugo, unsigned vctr = 0);
		void TreeSearchID( subElement *el_ptr, uint32_t IDofKey, vector< ID_Element* >& a );
		void fourcc_finder( subElement* root );
		uint32_t GetDefaultFrameDuration( subElement* root, unsigned TrackNum );
		int GetTrackCount( subElement* root );
		bool ClusterSectionRepair( subElement* root, uint64_t& read_from,
							uint64_t SegmentStart, uint64_t SegmentSize,
							ifstream& myfile, string output );
		bool Repair( string source, string target=string() );
		void ClusterBlocksTimeRepair( subElement* root, subElement* cluster_ptr );
		bool ClustersCopier( subElement* root, uint64_t SegmentStart, uint64_t SegmentSize, uint64_t& read_from,
								  subElement *CueTCodes, vector<uint64_t>& seeks, uint64_t& Duration,
								ifstream& infile, ofstream& outfile );
subElement* Regenerate_SeekHead( vector <uint64_t> seeks );
		void Repair_Segment_Size( subElement* root, ifstream& infile, ofstream& outfile );
		void Repair_Info_Duration( subElement* Segment_ptr, uint64_t Duration );
subElement* Regenerate_MetaSeek( subElement* Segment_ptr, uint64_t SegmentStart, uint64_t NewSeekHeadPos, uint64_t NewCueTimePos );
		void Adjust_Void_Sections( subElement *Segment_ptr, uint64_t SegmentStart, uint64_t ClusterStart, ofstream& outfile );
	};
#endif
