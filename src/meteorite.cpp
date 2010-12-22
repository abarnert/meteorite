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

#include "meteorite.h"
using namespace std;

///Notice!: I try to make this program BigEndian compatible but
///I don't test this program on architectures.
///If you have ppc laptop, please test & debug.
///Or donate me a ppc MacOSX laptop for development, I prefer orange one please :D

class IDDB* GlobalDB;	//Used in binElement::print()

int EBMLsize( uint64_t val ){	//measures EBML number size. EBML is BigEndian on memory;
// TODO (death#1#): //BigEndian?
	int byte_size=0;
	for( int i = 7 ; i >= 0 ; i-- ){	//run for detech how many byte required to indicate value
		if( (val >> i*8) & 0xFF )
			byte_size = 8-i;
		}
	if(byte_size == 0 )
		byte_size++;
	return byte_size;
	}
int ByteCount( uint64_t val ){		//ByteCounter for unsigned integers.
// TODO (death#1#): //BigEndian?
	int byte_size=1;
	for( int i = 0 ; i < 8 ; i++ )	//run for detech how many byte required to indicate value
		if( (( val >> i*8) & 0xFF) != 0x00 )
			byte_size = i+1;
	return byte_size;
	}
int ByteCount( int64_t val ){		//ByteCounter for signed integers.
// TODO (death#1#): //BigEndian?
	int byte_size=0;
	for( int i = 0 ; i < 8 ; i++ ){	//run for detech how many byte required to indicate value
		if( val > 0 ){
			if( (( val >> i*8) & 0xFF) != 0x00 ){
				if(((val>> i*8) & 0x80) == 0x80 )
					byte_size = i+2;
				else
					byte_size = i+1;
				}
			}
		else{
			if( (( val >> i*8 ) & 0xFF) != 0xFF ){
				if(((val>> i*8) & 0x80) != 0x80 )	//negative bit check
					byte_size = i+2;
				else
					byte_size = i+1;
				}
			}
		}
	if( byte_size == 0)
		byte_size++;
	return byte_size;
	}
int64_t& to_bigendianWR( int64_t x ){	//Converts signed integer to bigEndian for write operation
	static int64_t val;
	val = x;
	if( ByteCount( x ) == 1 || x == 0 )
		return val;
	if( !is_bigendian() ){
		val = endian_swap(val);		//0xA700....
		int i = ByteCount( x );		//i=2
		val >>= (8-i)*8;			//0xA700
		return val;
		}
	return val;
	}
uint64_t& to_bigendianWR( uint64_t x ){ //Converts unsigned integer to bigEndian for write operation
	static uint64_t val;
	val = x;
	if( ByteCount( x ) == 1 || x == 0 )
		return val;
	if( !is_bigendian() ){
		val = endian_swap(val);
		for( int i = 0 ; i < 8 ; i ++ ){
			if((( val >> i*8 ) & 0xFF) != 0x00 ){
				val >>= i*8;
				return val;
				}
			}
		}
	return val;
	}
uint64_t EBMLtouInteger(char *bfr, unsigned *ptr){	//Converts EBML to unsigned integer value
	uint8_t tmp = *reinterpret_cast< uint8_t* >( bfr );
// TODO (death#1#): //BigEndian?
	int i;
	for( i=7 ; i >= 0 ; i-- )
		if( (tmp >> i) & 0x01 )
			break;
	i = 8 - i;
	uint64_t x = *reinterpret_cast< uint64_t* >( bfr );
	x = make_bigendian( x );
	uint64_t f = 1;				//0x00000..01
	f = f << (64-i);				//0x01000....
	f = f xor 0xFFFFFFFFFFFFFFFFLL; //0x10111....
	x and_eq f;					//0xX0XXX....
	x = x >> (64-i*8);
	if( ptr != NULL )
		*ptr += i;
	return x;
	}
int64_t EBMLtosInteger(char *bfr, unsigned *ptr){	//Converts EBML to signed integer value
	/**Lacing sizes: Uses Range Shifting
	Only the 2 first ones will be coded, 800 gives 0x320 0x4000 = 0x4320
	After,  500 is coded as -300 : - 0x12C + 0x1FFF + 0x4000 = 0x5ED3.
	(0x4000 makes range of 0x4000 numbers, 1/2 is possitive, so 0x2000 = 1, 1FFF means 0)
	The size of the last frame is deduced from the total size of the Block.
	*/
	unsigned bytesize = 0;
	int64_t x = EBMLtouInteger( bfr, &bytesize );

	int64_t f = 0x80;
	for(unsigned i = 1 ; i < bytesize ; i++ )
		f *=f;
	f-=1;	//FFFF like now
	f/=2;
	x-=f;	//normalized!
	if( ptr != NULL )
		*ptr += bytesize;
	return x;
	}
char* EBMLtoWritebuff( uint64_t x ){	//Converts EBML to Writeable buffer of chars
	static uint64_t val;
	val = x >> (8-EBMLsize( x ))*8;
	return reinterpret_cast<char*>(&val);
	}
uint64_t uIntegertoEBML( uint64_t val ){	//Converts unsigned integer to EBML
	if( not is_bigendian() ){//little endian
		uint64_t f = 1;
		for( int i = 0 ; f <= val + 1 ;i ++){	//val == 0x7F, than EBML needed 407F, not FF. FF means undefined length for live streams...
			f *= 0x80;
			if( i > 8 ){
				cerr << "uintegertoebml() error: " << val;
				return 0;
				}
			}
		val or_eq f;
///OldCode
//			  if( val+1 < 0x80 ) val |= 0x80;	//+1 for avoid 0xFF in X = 7F.
//		else if( val+1 < 0x4000 ) val |= 0x4000;// 0xFF means stream size unknown. Special Case
//		else if( val+1 < 0x200000 ) val |= 0x200000;
//		else if( val+1 < 0x10000000 ) val |= 0x10000000;
//		else if( val+1 < 0x0800000000 ) val |= 0x0800000000;
//		else if( val+1 < 0x040000000000 ) val |= 0x040000000000;
//		else if( val+1 < 0x02000000000000 ) val |= 0x02000000000000;
//		else if( val+1 < 0x0100000000000000 ) val |= 0x0100000000000000;
//		else{
//			cerr << "uintegertoebml() error: " << val;
//			val = 0;
//			}
		val = make_bigendian( val );			// 0x123456 writen as 56 34 12 on Little Endian
		}
	else{//big endian
		uint64_t f = 1;
		for( int i = 0 ; i < 8 ;i ++){
			f *= 0x80;
			if( val + 1 < f ){
				val |= 0x80/(i+1);
				}
				break;
			}
///OldCode
//			  if( val+1 < 0x80 ) val |= 0x80;
//		else if( val+1 < 0x4000 ) val |= 0x40;
//		else if( val+1 < 0x200000 ) val |= 0x20;
//		else if( val+1 < 0x10000000 ) val |= 0x10;
//		else if( val+1 < 0x0800000000 ) val |= 0x08;
//		else if( val+1 < 0x040000000000 ) val |= 0x04;
//		else if( val+1 < 0x02000000000000 ) val |= 0x02;
//		else if( val+1 < 0x0100000000000000 ) val |= 0x01;
		}
	return val;
	}

//Virtual Class Functions that make possible other elements type possible
ID_Element::ID_Element( string _name, uint32_t _ID, types _type, uint64_t _location ){
		name = _name;
		ID = _ID;
		type = _type;
		location = _location;
		}
ID_Element::ID_Element( string _name, uint64_t _location ){
		ID_Element* tmp = GlobalDB->SearchName( _name );
		if( tmp != NULL ){
			name = tmp->name;
			ID = tmp->ID;
			type = tmp->type;
			location = _location;
			}
		}
short ID_Element::IDSize(){
		if( (ID >> 24) & 0x80 )	return 1;
		if( (ID >> 24) & 0x40 )	return 2;
		if( (ID >> 24) & 0x20 )	return 3;
		if( (ID >> 24) & 0x10 )	return 4;
		else return 0;
		}
void ID_Element::print( bool endline ){ cout << name << (endline ? "\n" : "" ); }
void ID_Element::write( ofstream& ){}	//Not ımplemented because of virtual class
unsigned ID_Element::Size( void ){return -1;}
ID_Element::~ID_Element(){}

//Class that handles String and UTF elements.
stringElement::stringElement( ID_Element id) : ID_Element( id ){ type = str; }
void stringElement::print( bool endline ){ cout << name << ": " << data << ": size " << Size() << (endline ? "\n" : "" ); }
unsigned stringElement::Size( void ){
	return IDSize() + EBMLsize(uIntegertoEBML( data.size() )) + data.size();
	}
void stringElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	outfile.write( EBMLtoWritebuff( uIntegertoEBML( data.size() ) ) , EBMLsize(uIntegertoEBML( data.size() ) ));
	outfile.write( data.c_str(), data.size() );
	}
string stringElement::read( char *bfr, unsigned *ptr ){
	unsigned i = 0;
	unsigned size = EBMLtouInteger( bfr,&i );
	data.assign( bfr+i, size );
	if( ptr != NULL )
		*ptr += i+size;
	return data;
	}

//Class that handles Unsigned integer elements.
uintElement::uintElement( ID_Element id ) : ID_Element( id ){ type = uInteger; data = 0; }
void uintElement::print(bool endline){ cout << name << ": " << data << (endline ? "\n" : "" ); }
unsigned uintElement::Size( void ){
	return IDSize() + EBMLsize(uIntegertoEBML(ByteCount( data ))) + ByteCount( data );
	}
void uintElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	outfile.write( EBMLtoWritebuff( uIntegertoEBML( ByteCount( data ) )), EBMLsize(uIntegertoEBML( ByteCount( data ) )) );
	outfile.write( reinterpret_cast<char*>(&to_bigendianWR( data )), ByteCount( data ));
	}
uint64_t uintElement::read( char *bfr, unsigned *ptr ){
	short size = EBMLtouInteger( bfr, ptr );
	if(size == 1)
		data = *reinterpret_cast< uint8_t* >( bfr+1 );
	else if(size == 2)
		data = make_bigendian(*reinterpret_cast< uint16_t* >( bfr+1 ));
	else if(size == 3){
		data = make_bigendian(*reinterpret_cast< uint32_t* >( bfr+1 ));
		data >>= 8;
		}
	else if(size == 4)
		data = make_bigendian(*reinterpret_cast< uint32_t* >( bfr+1 ));
	else if(size > 4 and size <= 8 ){
		data = make_bigendian(*reinterpret_cast< uint64_t* >( bfr+1 ));
		data >>= 8*(8-size);
		}
	else
		cout << "Error on readuInteger() size: " << size << endl;

	if( ptr != NULL )
		*ptr+=size;
	return data;
	}

//Class that handles Signed integer elements.
sintElement::sintElement( ID_Element id) : ID_Element( id ){ type = sInteger;  data = 0; }
void sintElement::print(bool endline){ cout << name << ": " << data << (endline ? "\n" : "" ); }
unsigned sintElement::Size( void ){
	return IDSize() + EBMLsize(uIntegertoEBML(ByteCount( data ))) + ByteCount( data );
// TODO (death#1#): Check if 3 byte sints?	}
void sintElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	outfile.write( EBMLtoWritebuff( uIntegertoEBML( ByteCount( data ) )), EBMLsize(uIntegertoEBML( ByteCount( data ) )) );
	outfile.write( reinterpret_cast<char*>(&to_bigendianWR( data )), ByteCount( data ));
	}
int64_t sintElement::read( char *bfr, unsigned *ptr ){
	short size = EBMLtouInteger( bfr, ptr );
	if(size == 1)
		data = *reinterpret_cast< int8_t* >( bfr+1 );
	else if(size == 2)
		data = make_bigendian(*reinterpret_cast< int16_t* >( bfr+1 ));
	else if(size == 3){
		data = make_bigendian(*reinterpret_cast< int32_t* >( bfr+1 ));
		data >>= 8;
		if(data & 0x00800000){	//move sign to end	take ret -2
			data ^= 0xFFFFFFFF;	//take inverse
			data++;				//need +1
			data &= 0x007FFFFF;	//filter it				+2
			data = 0 - data;	// than substract from zero.	-2
// TODO (death#1#): need test
			}
		}
	else if(size == 4)
		data = make_bigendian(*reinterpret_cast< int32_t* >( bfr+1 ));
	else if(size == 8)
		data = make_bigendian(*reinterpret_cast< int64_t* >( bfr+1 ));
	else
		cout << "Error on readsInteger() size: " << size << endl;
	if( ptr != NULL )
		*ptr+=size;
	return data;
	}

//Class that handles floating-point elements.
floatElement::floatElement( ID_Element id) : ID_Element( id ){ type = floating;  data = 0.0; }
void floatElement::print(bool endline){ cout << name << ": " << data << ": size " << Size() << (endline ? "\n" : "" ); }
unsigned floatElement::Size( void ){
	return IDSize() + EBMLsize( uIntegertoEBML(sizeof( data )) ) + sizeof( data );
	// TODO (death#1#): Why always write as double??
	}
void floatElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	outfile.write( EBMLtoWritebuff( uIntegertoEBML( sizeof( data ) )), EBMLsize(uIntegertoEBML( sizeof( data ) )) );
	outfile.write( reinterpret_cast<char*>(&make_bigendian( data )), sizeof( data ));
	}
double floatElement::read( char *bfr, unsigned *ptr ){
	short size = EBMLtouInteger( bfr, ptr );
	if(size == 4)
		data = make_bigendian(*reinterpret_cast< float* >( bfr+1 ));
	else if(size == 8)
		data = make_bigendian(*reinterpret_cast< double* >( bfr+1 ));
	else
		cout << "Error on readFloat()" << endl;
	if( ptr != NULL )
		*ptr+=size;
	return data;
}

//Class that handles sub elements.
subElement::subElement( string name, uint64_t location ) : ID_Element( name, location ){ type = subElements; }
subElement::subElement( ID_Element id) : ID_Element( id ){ type = subElements; }
void subElement::print(bool endline){ cout << name << ": size " << Size() << (endline ? "\n" : "" ); }
unsigned subElement::Size( void ){
	int sz =  0;
	for( vector<ID_Element*>::iterator it = data.begin() ; it != data.end(); it++ )
		sz += (*it)->Size();
	return IDSize()+EBMLsize(uIntegertoEBML(sz))+sz;
	}
unsigned subElement::subsize( void ){
	int sz =  0;
	if( data.size() != 0 )
		for( vector<ID_Element*>::iterator it = data.begin() ; it != data.end(); it++ )
			sz += (*it)->Size();
	return sz;
	}
//This function is NOT recursive!
void subElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	int sz =  0;
	int z = 0;
	ID_Element *processor;	//Needed to use for debugging.
	for( vector<ID_Element*>::iterator it = data.begin() ; it != data.end(); it++ ){
		//sz += (*it)->size();	//non debugged old constant
		processor = *it;
		z = processor->Size();
		sz += z;
		}
	outfile.write( EBMLtoWritebuff( uIntegertoEBML( sz )), EBMLsize(uIntegertoEBML( sz )) );
	///Removed line bottom
	//TreeParserWrite( ) //Treeparser automaticly writes sub-branches. This function onyl write ID & SubChunks size
	}
subElement::~subElement(){
	for( vector< ID_Element* >::iterator itr=data.begin() ; itr != data.end() ; itr++ )
		delete *itr;
		data.clear();
	}

//Class that handles binary elements.
binElement::binElement( ID_Element id) : ID_Element( id ){ type = binary; }
void binElement::print( bool endline ){
		cout << name << ": ";
		for( int i=0 ; i < min(10,datasize) ; i++ )
			printf( "%02X", *reinterpret_cast< uint8_t* >(data+i) );
		if( datasize == 4 ){
			ID_Element *eptr = GlobalDB->Search( make_bigendian(*reinterpret_cast<uint32_t*>(data)) );
			if( eptr not_eq NULL )
				cout << " (" << eptr->name << ')';
			}
		cout << ": size " << Size();
		if( endline )
			cout << endl;
		}
unsigned binElement::Size( void ){
	return IDSize() + EBMLsize( uIntegertoEBML(datasize) ) + datasize;
	}
void binElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	outfile.write( EBMLtoWritebuff(uIntegertoEBML( datasize )), EBMLsize(uIntegertoEBML( datasize )) );
	outfile.write( data, datasize );
	}
void binElement::read( char *bfr, unsigned *ptr ){
	//					new_el->datasize = EBMLtouInteger( bfr + i, &i );
	//					new_el->data = new char [new_el->datasize];
	//					memcpy( new_el->data, bfr+i, new_el->datasize);
	//					i+=new_el->datasize;
	unsigned z = 0;
	datasize = EBMLtouInteger( bfr, &z );
	data = new char [datasize];
	memcpy( data, bfr+z, datasize );
	if( ptr != NULL )
		*ptr+=datasize+z;
	}
binElement::~binElement(){
	delete [] data;
	}

//Class that handles date elements.
dateElement::dateElement( ID_Element id) : uintElement( id ){ type = date;  data = 0; }
void dateElement::print(bool endline){ cout << name << ": " << data << ": size " << Size() << (endline ? "\n" : "" ); }
unsigned dateElement::Size( void ){
		return IDSize() + EBMLsize( uIntegertoEBML(sizeof( data )) ) + sizeof( data );
		// TODO (death#1#): Check datasize of date as byte? beause some date's uses 4 byte version???
		}
void dateElement::write( ofstream &outfile ){
	outfile.write( reinterpret_cast<char*>(&make_bigendian(ID) ), IDSize() );
	outfile.write( EBMLtoWritebuff(uIntegertoEBML( sizeof( data ) )), EBMLsize(uIntegertoEBML( sizeof( data ) )) );
	outfile.write( reinterpret_cast<char*>(&to_bigendianWR( data )), sizeof( data ));
	}

void HeadIDs::AddElement( ID_Element *eID ){
		idvector.push_back( eID );
		}
ID_Element* HeadIDs::Search( uint32_t ID ){
		for( uint16_t i = 0 ; i < idvector.size() ; i++ ){
			uint32_t mask = 0xFFFFFFFF << (8*(4-(idvector.at(i)->IDSize())));	//mask buffer with ID size
			if( idvector.at(i)->ID == (ID & mask) )
				return (idvector.at(i));
			}
		return NULL;
		}

ID_Element* IDDB::Search( uint32_t ID ){
	for( uint16_t j = 0 ; j < hivector.size() ; j++ )
		for( uint16_t i = 0 ; i < hivector.at(j).idvector.size() ; i++ ){
			uint32_t mask = 0xFFFFFFFF << (8*(4-hivector.at(j).idvector.at(i)->IDSize()));
			if( hivector.at(j).idvector.at(i)->ID == (ID & mask) )
				return (hivector.at(j).idvector.at(i));
			}
	return NULL;
	}
HeadIDs* IDDB::SearchHeadID( uint32_t ID ){
	for( uint16_t i = 0 ; i < hivector.size() ; i++ ){
			uint32_t mask = 0xFFFFFFFF << (8*(4-(hivector.at(i).idvector.at(0)->IDSize())));
			if( hivector.at(i).idvector.at(0)->ID == (ID & mask) )
				return &(hivector.at(i));
			}
	return NULL;
	}
uint32_t IDDB::IDof( string name ){
	ID_Element *processor = SearchName( name );
	if( processor != NULL )
		return processor->ID;
	else{
		cout << "No ID for token: " << name << endl;
		return 0x00;
		}
	}
types IDDB::TypeOf( string name ){
	ID_Element *processor = SearchName( name );
	if( processor != NULL )
		return processor->type;
	else
		return error;
	}
ID_Element* IDDB::SearchName( string name ){
	for( uint16_t j = 0 ; j < hivector.size() ; j++ )
		for( uint16_t i = 0 ; i < hivector.at(j).idvector.size() ; i++ )
			if(hivector.at(j).idvector.at(i)->name == name )
				return hivector.at(j).idvector.at(i);
	return NULL;
	}
IDDB::IDDB(){	//the_IDDB, IDDatabase Implementation
	GlobalDB = this;
	EBML.AddElement( new ID_Element( "EBMLFile",			0x1A45DFA3, subElements ) );	//First one has to be Main ID
	EBML.AddElement( new ID_Element( "EBMLVersion",				0x4286, uInteger ) );
	EBML.AddElement( new ID_Element( "EBMLReadVersion",			0x42F7, uInteger ) );
	EBML.AddElement( new ID_Element( "EBMLMaxIDLength",			0x42F2, uInteger ) );
	EBML.AddElement( new ID_Element( "EBMLMaxSizeLength"	,	0x42F3, uInteger ) );
	EBML.AddElement( new ID_Element( "DocType",					0x4282, str ) );
	EBML.AddElement( new ID_Element( "DocTypeVersion",			0x4287, uInteger ) );
	EBML.AddElement( new ID_Element( "DocTypeReadVersion",		0x4285, uInteger ) );
	EBML.AddElement( new ID_Element( "CRC-32",					  0xBF, binary ) );
	EBML.AddElement( new ID_Element( "Void",					  0xEC, binary ) );

	Segment.AddElement(  new ID_Element( "Segment",			0x18538067, subElements ) );

	MetaSeek.AddElement( new ID_Element( "SeekHead",		0x114D9B74, subElements ) );
	MetaSeek.AddElement( new ID_Element( "Seek",				0x4DBB, subElements ) );
	MetaSeek.AddElement( new ID_Element( "SeekID",				0x53AB, binary ) );
	MetaSeek.AddElement( new ID_Element( "SeekPosition",		0x53AC, uInteger ) );
	MetaSeek.AddElement( new ID_Element( "CRC-32",				  0xBF, binary ) );
	MetaSeek.AddElement( new ID_Element( "Void",				  0xEC, binary ) );

	Info.AddElement( new ID_Element( "Info",				0x1549A966, subElements ) );
	Info.AddElement( new ID_Element( "SegmentUID",				0x73A4, binary ) );
	Info.AddElement( new ID_Element( "SegmentFilename",			0x7384, UTF8 ) );
	Info.AddElement( new ID_Element( "PrevUID",				  0x3CB923, binary ) );
	Info.AddElement( new ID_Element( "PrevFilename",		  0x3C83AB, UTF8 ) );
	Info.AddElement( new ID_Element( "NextUID",				  0x3EB923, binary ) );
	Info.AddElement( new ID_Element( "NextFilename",		  0x3E83BB, UTF8 ) );
	Info.AddElement( new ID_Element( "SegmentFamily",			0x7384, binary ) );
	Info.AddElement( new ID_Element( "ChapterTranslate",		0x6924, subElements ) );
	Info.AddElement( new ID_Element( "ChapterTranslateEditionUID",0x69FC, uInteger ) );
	Info.AddElement( new ID_Element( "ChapterTranslateCodec",	0x69BF, uInteger ) );
	Info.AddElement( new ID_Element( "ChapterTranslateID",		0x69A5, binary ) );
	Info.AddElement( new ID_Element( "TimecodeScale",		  0x2AD7B1, uInteger ) );
	Info.AddElement( new ID_Element( "Duration",				0x4489, floating ) );
	Info.AddElement( new ID_Element( "DateUTC",					0x4461, date ) );
	Info.AddElement( new ID_Element( "Title",					0x7BA9, UTF8 ) );
	Info.AddElement( new ID_Element( "MuxingApp",				0x4D80,  UTF8 ) );
	Info.AddElement( new ID_Element( "WritingApp",				0x5741, UTF8 ) );
	Info.AddElement( new ID_Element( "CRC-32",					  0xBF, binary  ) );
	Info.AddElement( new ID_Element( "Void",					  0xEC,  binary  ) );

	Cluster.AddElement( new ID_Element( "Cluster",			0x1F43B675, subElements ) );
	Cluster.AddElement( new ID_Element( "Timecode",				  0xE7, uInteger ) );
	Cluster.AddElement( new ID_Element( "SilentTracks",			0x5854, subElements ) );
	Cluster.AddElement( new ID_Element( "SilentTrackNumber",	0x58D7, uInteger ) );
	Cluster.AddElement( new ID_Element( "Position",				  0xA7, uInteger ) );
	Cluster.AddElement( new ID_Element( "PrevSize",				  0xAB, uInteger ) );
	Cluster.AddElement( new ID_Element( "BlockGroup",			  0xA0, subElements ) );
	Cluster.AddElement( new ID_Element( "Block",				  0xA1, binary ) );
	Cluster.AddElement( new ID_Element( "BlockVirtual",			  0xA2, binary ) );
	Cluster.AddElement( new ID_Element( "BlockAdditions",		0x75A1, subElements ) );
	Cluster.AddElement( new ID_Element( "BlockMore",			  0xA6, subElements ) );
	Cluster.AddElement( new ID_Element( "BlockAddID",			  0xEE, uInteger ) );
	Cluster.AddElement( new ID_Element( "BlockAdditional",		  0xA5, binary ) );
	Cluster.AddElement( new ID_Element( "BlockDuration",		  0x9B, uInteger ) );
	Cluster.AddElement( new ID_Element( "ReferencePriority",	  0xFA, uInteger ) );
	Cluster.AddElement( new ID_Element( "ReferenceBlock",		  0xFB, sInteger ) );
	Cluster.AddElement( new ID_Element( "ReferenceVirtual",		  0xFD, sInteger ) );
	Cluster.AddElement( new ID_Element( "CodecState",			  0xA4, binary ) );
	Cluster.AddElement( new ID_Element( "Slices",				  0x8E, subElements ) );
	Cluster.AddElement( new ID_Element( "TimeSlice",			  0xE8, subElements ) );
	Cluster.AddElement( new ID_Element( "LaceNumber",			  0xCC, uInteger ) );
	Cluster.AddElement( new ID_Element( "FrameNumber",			  0xCD, uInteger ) );
	Cluster.AddElement( new ID_Element( "BlockAdditionID",		  0xCB, uInteger ) );
	Cluster.AddElement( new ID_Element( "Delay",				  0xCE, uInteger ) );
	Cluster.AddElement( new ID_Element( "Duration",				  0xCF, uInteger ) );
	Cluster.AddElement( new ID_Element( "SimpleBlock",			  0xA3, binary ) );
	Cluster.AddElement( new ID_Element( "EncryptedBlock",		  0xAF, binary ) );
	Cluster.AddElement( new ID_Element( "CRC-32",					  0xBF, binary ) );
	Cluster.AddElement( new ID_Element( "Void",					  0xEC, binary ) );

	Tracks.AddElement( new ID_Element( "Tracks",			0x1654AE6B, subElements ) );
	Tracks.AddElement( new ID_Element( "TrackEntry",			  0xAE, subElements ) );
	Tracks.AddElement( new ID_Element( "TrackNumber",			  0xD7, uInteger ) );
	Tracks.AddElement( new ID_Element( "TrackUID",				0x73C5, uInteger ) );
	Tracks.AddElement( new ID_Element( "TrackType",				  0x83, uInteger ) );
	Tracks.AddElement( new ID_Element( "FlagEnabled",			  0xB9, uInteger ) );
	Tracks.AddElement( new ID_Element( "FlagDefault",			  0x88, uInteger ) );
	Tracks.AddElement( new ID_Element( "FlagForced",			0x55AA, uInteger ) );
	Tracks.AddElement( new ID_Element( "FlagLacing",			  0x9C, uInteger ) );
	Tracks.AddElement( new ID_Element( "MinCache",				0x6DE7, uInteger ) );
	Tracks.AddElement( new ID_Element( "MaxCache",				0x6DF8, uInteger ) );
	Tracks.AddElement( new ID_Element( "DefaultDuration",	  0x23E383, uInteger ) );
	Tracks.AddElement( new ID_Element( "TrackTimecodeScale", 0x23314F, floating ) );
	Tracks.AddElement( new ID_Element( "TrackOffset",			0x537F, sInteger ) );
	Tracks.AddElement( new ID_Element( "MaxBlockAdditionID",	0x55EE, uInteger ) );
	Tracks.AddElement( new ID_Element( "Name",					0x536E, UTF8 ) );
	Tracks.AddElement( new ID_Element( "Language",			  0x22B59C, str ) );
	Tracks.AddElement( new ID_Element( "CodecID",				  0x86, str ) );
	Tracks.AddElement( new ID_Element( "CodecPrivate",			0x63A2, binary ) );
	Tracks.AddElement( new ID_Element( "CodecName",			  0x258688, UTF8 ) );
	Tracks.AddElement( new ID_Element( "AttachmentLink",		0x7446, uInteger ) );
	Tracks.AddElement( new ID_Element( "CodecSettings",		  0x3A9697, UTF8 ) );

	Tracks.AddElement( new ID_Element( "CodecInfoURL",		  0x3B4040, str ) );
	Tracks.AddElement( new ID_Element( "CodecDownloadURL",	  0x26B240, str ) );
	Tracks.AddElement( new ID_Element( "CodecDecodeAll",		  0xAA, uInteger ) );

	Tracks.AddElement( new ID_Element( "TrackOverlay",			0x6FAB, uInteger ) );
	Tracks.AddElement( new ID_Element( "TrackTranslate",		0x6624, subElements ) );
	Tracks.AddElement( new ID_Element( "TrackTranslateEditionUID",0x66FC, uInteger ) );
	Tracks.AddElement( new ID_Element( "TrackTranslateCodec",	0x66BF, uInteger ) );
	Tracks.AddElement( new ID_Element( "TrackTranslateTrackID",	0x66A5, binary ) );

	Tracks.AddElement( new ID_Element( "Video",				  0xE0, subElements ) );
	Tracks.AddElement( new ID_Element( "FlagInterlaced",	  0x9A, uInteger ) );
	Tracks.AddElement( new ID_Element( "StereoMode",		0x53B8, uInteger ) );
	Tracks.AddElement( new ID_Element( "PixelWidth",		  0xB0, uInteger ) );
	Tracks.AddElement( new ID_Element( "PixelHeight",		  0xBA, uInteger ) );
	Tracks.AddElement( new ID_Element( "PixelCropBottom",	0x54AA, uInteger ) );
	Tracks.AddElement( new ID_Element( "PixelCropTop",		0x54BB, uInteger ) );
	Tracks.AddElement( new ID_Element( "PixelCropLeft",		0x54CC, uInteger ) );
	Tracks.AddElement( new ID_Element( "PixelCropRight",	0x54DD, uInteger ) );
	Tracks.AddElement( new ID_Element( "DisplayWidth",		0x54B0, uInteger ) );
	Tracks.AddElement( new ID_Element( "DisplayHeight",		0x54BA, uInteger ) );
	Tracks.AddElement( new ID_Element( "DisplayUnit",		0x54B2, uInteger ) );
	Tracks.AddElement( new ID_Element( "AspectRatioType",	0x54B3, uInteger ) );
	Tracks.AddElement( new ID_Element( "ColourSpace",	  0x2EB524, binary ) );
	Tracks.AddElement( new ID_Element( "GammaValue",	  0x2FB523, floating ) );
	Tracks.AddElement( new ID_Element( "FrameRate",		  0x2383E3, floating ) );

	Tracks.AddElement( new ID_Element( "Audio",					  0xE1, subElements ) );
	Tracks.AddElement( new ID_Element( "SamplingFrequency",		  0xB5, floating ) );
	Tracks.AddElement( new ID_Element( "OutputSamplingFrequency",0x78B5, floating ) );
	Tracks.AddElement( new ID_Element( "Channels",				  0x9F, uInteger ) );
	Tracks.AddElement( new ID_Element( "ChannelPositions",		0x7B7D, binary ) );
	Tracks.AddElement( new ID_Element( "BitDepth",				0x6264, uInteger ) );

	Tracks.AddElement( new ID_Element( "ContentEncodings",		0x6D80, subElements ) );
	Tracks.AddElement( new ID_Element( "ContentEncoding",		0x6240, subElements ) );
	Tracks.AddElement( new ID_Element( "ContentEncodingOrder",	0x5031, uInteger ) );
	Tracks.AddElement( new ID_Element( "ContentEncodingScope",	0x5032, uInteger ) );
	Tracks.AddElement( new ID_Element( "ContentEncodingType",	0x5033, uInteger ) );
	Tracks.AddElement( new ID_Element( "ContentCompression",	0x5034, subElements ) );
	Tracks.AddElement( new ID_Element( "ContentCompAlgo",		0x4254, uInteger ) );
	Tracks.AddElement( new ID_Element( "ContentCompSettings",	0x4255, binary ) );
	Tracks.AddElement( new ID_Element( "ContentEncryption",		0x5035, subElements ) );
	Tracks.AddElement( new ID_Element( "ContentEncAlgo",		0x47E1, uInteger ) );
	Tracks.AddElement( new ID_Element( "ContentEncKeyID",		0x47E2, binary ) );
	Tracks.AddElement( new ID_Element( "ContentSignature",		0x47E3, binary ) );
	Tracks.AddElement( new ID_Element( "ContentSigKeyID",		0x47E4, binary ) );
	Tracks.AddElement( new ID_Element( "ContentSigAlgo",		0x47E5, uInteger ) );
	Tracks.AddElement( new ID_Element( "ContentSigHashAlgo",	0x47E6, uInteger ) );
	Tracks.AddElement( new ID_Element( "CRC-32",				  0xBF, binary  ) );
	Tracks.AddElement( new ID_Element( "Void",					  0xEC, binary  ) );

	Cues.AddElement( new ID_Element( "Cues",			0x1C53BB6B, subElements ) );
	Cues.AddElement( new ID_Element( "CuePoint",			  0xBB, subElements ) );
	Cues.AddElement( new ID_Element( "CueTime",				  0xB3, uInteger ) );
	Cues.AddElement( new ID_Element( "CueTrackPositions",	  0xB7, subElements ) );
	Cues.AddElement( new ID_Element( "CueTrack",			  0xF7, uInteger ) );
	Cues.AddElement( new ID_Element( "CueClusterPosition",	  0xF1, uInteger ) );
	Cues.AddElement( new ID_Element( "CueBlockNumber",		0x5378, uInteger ) );
	Cues.AddElement( new ID_Element( "CueCodecState",		  0xEA, uInteger ) );
	Cues.AddElement( new ID_Element( "CueReference",		  0xDB, subElements  ) );
	Cues.AddElement( new ID_Element( "CueRefTime",			  0x96, uInteger ) );
	Cues.AddElement( new ID_Element( "CueRefCluster",		  0x97, uInteger  ) );
	Cues.AddElement( new ID_Element( "CueRefNumber",		0x535F, uInteger  ) );
	Cues.AddElement( new ID_Element( "CueRefCodecState",	  0xEB, uInteger  ) );
	Cues.AddElement( new ID_Element( "CRC-32",				  0xBF, binary  ) );
	Cues.AddElement( new ID_Element( "Void",				  0xEC, binary  ) );

	Attachments.AddElement( new ID_Element( "Attachments",	0x1941A469, subElements ) );
	Attachments.AddElement( new ID_Element( "AttachedFile",		0x61A7, subElements ) );
	Attachments.AddElement( new ID_Element( "FileDescription",	0x467E, UTF8 ) );
	Attachments.AddElement( new ID_Element( "FileName",			0x466E, UTF8 ) );
	Attachments.AddElement( new ID_Element( "FileMimeType",		0x4660, str ) );
	Attachments.AddElement( new ID_Element( "FileData",			0x465C, binary ) );
	Attachments.AddElement( new ID_Element( "FileUID",			0x46AE, uInteger ) );
	Attachments.AddElement( new ID_Element( "FileReferral",		0x4675, binary ) );
	Attachments.AddElement( new ID_Element( "CRC-32",			  0xBF, binary  ) );
	Attachments.AddElement( new ID_Element( "Void",				  0xEC, binary  ) );

	Chapters.AddElement( new ID_Element( "Chapters",		0x1043A770, subElements ) );
	Chapters.AddElement( new ID_Element( "EditionEntry",		0x45B9, subElements ) );
	Chapters.AddElement( new ID_Element( "EditionUID",			0x45BC, uInteger ) );
	Chapters.AddElement( new ID_Element( "EditionFlagHidden",	0x45BD, uInteger ) );
	Chapters.AddElement( new ID_Element( "EditionFlagDefault",	0x45DB, uInteger ) );
	Chapters.AddElement( new ID_Element( "EditionFlagOrdered",	0x45DD, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterAtom",			  0xB6, subElements ) );
	Chapters.AddElement( new ID_Element( "ChapterUID",			0x73C4, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterTimeStart",	  0x91, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterTimeEnd",		  0x92, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterFlagHidden",	  0x98, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterFlagEnabled",	0x4598, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterSegmentUID",	0x6E67, binary ) );
	Chapters.AddElement( new ID_Element( "ChapterSegmentEditionUID",0x6EBD, binary ) );
	Chapters.AddElement( new ID_Element( "ChapterPhysicalEquiv",0x63C3, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterTrack",		  0x8F, subElements ) );
	Chapters.AddElement( new ID_Element( "ChapterTrackNumber",	  0x89, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapterDisplay",		  0x80, subElements ) );
	Chapters.AddElement( new ID_Element( "ChapString",			  0x85, UTF8 ) );
	Chapters.AddElement( new ID_Element( "ChapLanguage",		0x437C, str ) );
	Chapters.AddElement( new ID_Element( "ChapCountry",			0x437E, str ) );
	Chapters.AddElement( new ID_Element( "ChapProcess",			0x6944, subElements ) );
	Chapters.AddElement( new ID_Element( "ChapProcessCodecID",	0x6955, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapProcessPrivate",	0x450D, binary ) );
	Chapters.AddElement( new ID_Element( "ChapProcessCommand",	0x6911, subElements ) );
	Chapters.AddElement( new ID_Element( "ChapProcessTime",		0x6922, uInteger ) );
	Chapters.AddElement( new ID_Element( "ChapProcessData",		0x6933, binary ) );
	Chapters.AddElement( new ID_Element( "CRC-32",				  0xBF, binary  ) );
	Chapters.AddElement( new ID_Element( "Void",				  0xEC, binary  ) );

	Tags.AddElement( new ID_Element( "Tags",		0x1254C367, subElements ) );
	Tags.AddElement( new ID_Element( "Tag",				0x7373, subElements ) );
	Tags.AddElement( new ID_Element( "Targets",			0x63C0, subElements ) );
	Tags.AddElement( new ID_Element( "TargetTypeValue",	0x68CA, uInteger ) );
	Tags.AddElement( new ID_Element( "TargetType",		0x63CA, str) );
	Tags.AddElement( new ID_Element( "TrackUID",		0x63C5, uInteger ) );
	Tags.AddElement( new ID_Element( "EditionUID",		0x63C9, uInteger ) );
	Tags.AddElement( new ID_Element( "ChapterUID",		0x63C4, uInteger ) );
	Tags.AddElement( new ID_Element( "AttachmentUID",	0x63C6, uInteger ) );
	Tags.AddElement( new ID_Element( "SimpleTag",		0x67C8, subElements ) );
	Tags.AddElement( new ID_Element( "TagName",			0x45A3, UTF8) );
	Tags.AddElement( new ID_Element( "TagLanguage",		0x447A, str) );
	Tags.AddElement( new ID_Element( "TagDefault",		0x4486, uInteger ) );
	Tags.AddElement( new ID_Element( "TagString",		0x4487, UTF8) );
	Tags.AddElement( new ID_Element( "TagBinary",		0x4485, binary ) );
	Tags.AddElement( new ID_Element( "CRC-32",			  0xBF, binary  ) );
	Tags.AddElement( new ID_Element( "Void",			  0xEC, binary  ) );

	//Adding all vectors to hivector.
	hivector.push_back( EBML );
	hivector.push_back( Segment );
	hivector.push_back( MetaSeek );
	hivector.push_back( Info );
	hivector.push_back( Cluster );
	hivector.push_back( Tracks );
	hivector.push_back( Cues );
	hivector.push_back( Attachments );
	hivector.push_back( Chapters );
	hivector.push_back( Tags );

	if( not is_bigendian() )
		for( uint16_t j = 0 ; j < hivector.size() ; j++ )
			for( uint16_t i = 0 ; i < hivector.at(j).idvector.size() ; i++ )	//if hw is little endian, store ID's as bigEndian
				hivector.at(j).idvector.at(i)->ID <<= (8*(4-EBMLsize( make_bigendian( hivector.at(j).idvector.at(i)->ID ))));
	}

Block::Block(){
		KeyFlag = true;
		Valid = false;
		TrackNum = TimeCode = Flags = Size = Duration = Lacing, Frames = 0;
		}
void Block::Print( ){
		cout << "\tBlock"
//			 << "\tKeyFlag : " << KeyFlag
//			 << "\tValid = " << Valid
			 << " Size:" << setw (6) << Size
//			 << "\tDuration : " << Duration
			 << " Track: " << TrackNum
			 << " BlockTimeCode:" << setw (5) << TimeCode;
		printf( " Flags: 0x%02X ", Flags );
		cout << " Lace: " << (  Lacing == Lace_NO ? "  NO " :
								Lacing == Lace_Xiph ? " Xiph" :
								Lacing == Lace_EBML ? " EBML" :
								Lacing == Lace_Fixed ? "Fixed" : "Error" );
		if( Lacing not_eq Lace_NO )
			cout << "\tFrames: " << Frames;
		}

Meteorite::Meteorite( wxGauge *WxGauge_ ){
	Init();
	WxGauge = WxGauge_;
	}
void Meteorite::SetGauge( wxGauge *WxGauge_){
	WxGauge = WxGauge_;
}

void Meteorite::Init(void){
	filesize=0;
	}
bool Meteorite::update_gauge( int percent ){	//return indicate program live, false on kill req
	if(WxGauge){
		if( WxGauge->GetValue() != percent ){		//Checks value is changed or not
			wxMutexGuiEnter();
			WxGauge->SetValue( percent );
			wxYield();
			wxMutexGuiLeave();
			}
		}
	else{
//		wxString value;
//		wxGetEnv( wxT("COLUMNS"), &value);
//		std::cout << "\r" << value.ToAscii() << " "  << target_file.ToAscii() << "\t\%" << percent;
		}

	if( !wxThread::This()->IsMain() )							//Checks if functions is running on thread
		if( wxThread::This()->TestDestroy() ){		//Checks if thread has termination request
//			close_files(true);				//Releases files
//			infile.close();
//			outfile.close();
//			MemoLogWriter(_("Operation stoped by user.\n"));
			return false;
			}
	return true;
	}

uint32_t Meteorite::IDof( string name ){
	return DB.IDof( name );
	}
bool Meteorite::is_IDof( char* bfr, string name){
	ID_Element *elptr = FindElementID( bfr );
	if( elptr != NULL )
		return DB.IDof( name ) == elptr->ID;
	else
		return false;
	}
ID_Element* Meteorite::FindElementID( HeadIDs hid, char *bfr ) {
	uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t* >( bfr ) );
	return hid.Search( p32 );
	}
ID_Element* Meteorite::FindElementID( char *bfr ) {
	uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t* >( bfr ) );
	return DB.Search( p32 );
	}

void Meteorite::skip( char *bfr, unsigned& i, types type ){	//Skips that element without processing.
	if( type == subElements )
		EBMLtouInteger( bfr+i,&i);
	else{
		unsigned a = 0;
		unsigned size = EBMLtouInteger( bfr+i, &a );
		i += a+size;
		}
	return;
	}

subElement* Meteorite::TreeParser( char* a ){	//Parsa a buffer by TreeParser(tm) code :D
		ifstream myfile( a, ios::binary );
		if( ! myfile.is_open() )
			return NULL;
		Init();

		char *buffer = new char [bfr_size];
		myfile.read( buffer, 16 );

		subElement *ret = new subElement( ID_Element( "root", 0x00, subElements ) );
		subElement *tmp = ret;

		uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t*>( buffer ) );
		unsigned i=4;
		unsigned read_from = 0;
		unsigned ebml_size = EBMLtouInteger( buffer+i ); //(buffer[4] & 0x7F);
		if( p32 != IDof( "EBMLFile" ) ){	//EBML file
			cerr << "input file is not in EBML/matroska format!";
			return NULL;
			}
		uint64_t SegmentSize = 10;

		while( read_from < ebml_size+SegmentSize+4+4 ){
			myfile.seekg( read_from );
			myfile.read( buffer, 16 );
			p32 = make_bigendian( *reinterpret_cast< uint32_t* >( buffer ));
			i = 4;
			if( is_IDof(buffer, "Void" )){
				i = 1;
				unsigned size = EBMLtouInteger( buffer+1, &i );
				//cout << "Void Size: " << size << endl;
				read_from += i+size;
				}
			else if(	p32 == IDof( "EBMLFile" ) or
						p32 == IDof( "SeekHead" ) or
						p32 == IDof( "Info" ) or
						p32 == IDof( "Tracks" ) or
						p32 == IDof( "Attachments" ) or
						p32 == IDof( "Cues" ) or
						p32 == IDof( "Chapters" ) or
						p32 == IDof( "Tags" ) ){
				unsigned size = EBMLtouInteger( buffer+i , &i);	//Get token size
				myfile.seekg( read_from );
				myfile.read( buffer, size+i );

				HeadIDs *hid = DB.SearchHeadID( p32 );				//search ID at database
				if( hid not_eq NULL ){
					for( unsigned x = 0 ; x< size+i ;){
						ID_Element *processor = FindElementID( *hid, buffer+x );	//find element at database
						if( processor != NULL ){
							x+=processor->IDSize();									//Parsing ID size
							unsigned sz = EBMLtouInteger(buffer+x, &x);			//EBML to integer conversion to find size
							TreeParserChunk( buffer, sz+i, tmp );					//Parse SubChunks recursively.
							x+=sz;
							}
						else{
							uint32_t b = make_bigendian( *reinterpret_cast<uint32_t*>( buffer+i ));
							cout << "Warning token not found on " << hid->idvector.at(0)->name <<": " << hex << b << endl;
							}
						}
					}
//					PrintParserUniform( buffer, size+i );	//Old Parser dropped near v0.06?
				read_from += size+i;
				}
			else if( p32 == IDof( "Segment" ) ){
				tmp = new subElement( *FindElementID( buffer ) );
				ret->data.push_back( tmp );

				SegmentSize = EBMLtouInteger( buffer+i, &i );
				read_from += i;
				}
			else if( p32 == IDof( "Cluster" ) ){	//Skipping clusters without Segments!
				int size = EBMLtouInteger( buffer+i, &i );
				read_from += size+i;
				}
			}
		delete buffer;
		return ret;
		}

// Generate and return parsed subElement chunk pointer.
subElement* Meteorite::TreeParserChunkMaster( char *bfr, unsigned length, uint64_t _location ){
	subElement *el_ptr = new subElement( *FindElementID( bfr ) );
	unsigned i = 0;
	if( el_ptr not_eq NULL and el_ptr->type==subElements ){
		i+=el_ptr->IDSize();
		EBMLtouInteger( bfr+i, &i );
		el_ptr->location = _location;
		//i+=size;

		while( i < length ){
			ID_Element *processor = FindElementID( bfr+i );
			if( processor != NULL ){
				i+=processor->IDSize();
				if( processor->type == subElements ){
					unsigned x = i-processor->IDSize();
					unsigned size = EBMLtouInteger( bfr+i, &i );
					el_ptr->data.push_back( TreeParserChunkMaster( bfr+x, size+(i-x) ) );
					i+=size;
					}
				else if( processor->type == uInteger ){
					uintElement *new_el = new uintElement( *processor );
					new_el->read( bfr + i, &i );
					el_ptr->data.push_back( new_el );
					}
				else if( processor->type == sInteger ){
					sintElement *new_el = new sintElement( *processor );
					new_el->read( bfr + i, &i );
					el_ptr->data.push_back( new_el );
					}
				else if( processor->type == str or processor->type == UTF8 ){
					stringElement *new_el = new stringElement( *processor );
					new_el->read( bfr + i, &i );
					el_ptr->data.push_back( new_el );
					}
				else if( processor->type == binary ){
					binElement *new_el = new binElement( *processor );
					new_el->read( bfr + i, &i );
					el_ptr->data.push_back( new_el );
					}
				else if( processor->type == floating ){
					floatElement *new_el = new floatElement( *processor );
					new_el->read( bfr + i, &i );
					el_ptr->data.push_back( new_el );
					}
				else if( processor->type == date ){
					dateElement *new_el = new dateElement( *processor );
					new_el->read( bfr + i, &i );
					el_ptr->data.push_back( new_el );
					}
				else{
					cerr << "Error : No token found on TreeParserChunk();" << endl;
					}
				}
			else{
				cerr << "Error on TreeParserChunkMaster();" << endl;
				break;
				}
			}
		return el_ptr;
		}
	else{
		cerr << "Error on TreeParserChunkMaster(), Parsing with non-subElement" << endl;
		return NULL;
		}
	}

// Generate SubElements inners and attach them to subElment el_ptr
void Meteorite::TreeParserChunk( char *bfr, unsigned length, subElement *el_ptr){
	for( unsigned i = 0 ; i< length ;){
		ID_Element *processor = FindElementID( bfr+i );
		if( processor != NULL ){
			i+=processor->IDSize();
			if( processor->type == subElements ){
				subElement *new_el = new subElement( *processor );
				unsigned size = EBMLtouInteger( bfr+i, &i );
				TreeParserChunk( bfr+i, size, new_el );
				i+=size;
				el_ptr->data.push_back( new_el );
				}
			else if( processor->type == uInteger ){
				uintElement *new_el = new uintElement( *processor );
				new_el->read( bfr + i, &i );
				el_ptr->data.push_back( new_el );
				}
			else if( processor->type == sInteger ){
				sintElement *new_el = new sintElement( *processor );
				new_el->read( bfr + i, &i );
				el_ptr->data.push_back( new_el );
				}
			else if( processor->type == str or processor->type == UTF8 ){
				stringElement *new_el = new stringElement( *processor );
				new_el->read( bfr + i, &i );
				el_ptr->data.push_back( new_el );
				}
			else if( processor->type == binary ){
				binElement *new_el = new binElement( *processor );
				new_el->read( bfr + i, &i );
				el_ptr->data.push_back( new_el );
				}
			else if( processor->type == floating ){
				floatElement *new_el = new floatElement( *processor );
				new_el->read( bfr + i, &i );
				el_ptr->data.push_back( new_el );
				}
			else if( processor->type == date ){
				dateElement *new_el = new dateElement( *processor );
				new_el->read( bfr + i, &i );
				el_ptr->data.push_back( new_el );
				}
			else{
				cerr << "Error : No token found on TreeParserChunk();" << endl;
				}
			}
		else{
			cerr << "Error on TreeParserChunk();" << endl;
			}
		}
	}
void Meteorite::TreeParserPrint( subElement* el_ptr, unsigned lvl ){	//Prints Tree
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		for( unsigned a = 0 ; a < lvl ;a++ )
			cout << ' ';
		if( el_ptr->data.at(i)->type == subElements )
			cout << " +";
		else
			cout << "|-";
		el_ptr->data.at(i)->print();
		if( el_ptr->data.at(i)->type == subElements )
			TreeParserPrint( dynamic_cast<subElement*>(el_ptr->data.at(i)), lvl+1 );
		}
	}
void Meteorite::TreeParserCompare( subElement* el_ptr1,subElement* el_ptr2 ){	//Compares two Tree (For Development)
	ofstream a1("/temp/a", ios_base::trunc);
	ofstream b1("/temp/b", ios_base::trunc);
	TreeParserWriteMaster( el_ptr1, a1 );
	TreeParserWriteMaster( el_ptr2, b1 );
	a1.close();
	b1.close();
	ifstream a2("/temp/a", ios::binary);
	ifstream b2("/temp/b", ios::binary);
	a2.seekg( 0, ios_base::end);
	b2.seekg( 0, ios_base::end);
	int asize = a2.tellg();
	int bsize = b2.tellg();
	if(asize not_eq bsize ){
		cout << "Sizes are not same!" << endl;
		cout << endl << "el_ptr1 here:" << endl;
		TreeParserPrintHR( el_ptr1 );
		cout << endl << "el_ptr2 here:" << endl;
		TreeParserPrintHR( el_ptr2 );
		getchar();
		}

	a2.close();
	b2.close();
	}
void Meteorite::TreeParserPrintHR( subElement* el_ptr, unsigned lvl ){ //Prints Tree but in Human Readable form used for Block Data show
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		if( el_ptr->data.at(i)->ID == IDof("CuePoint") or
			el_ptr->data.at(i)->ID == IDof("Seek") or
			el_ptr->data.at(i)->ID == IDof("BlockGroup") )
			cout << endl;

		else if (el_ptr->data.at(i)->ID == IDof("Block") or
			el_ptr->data.at(i)->ID == IDof("SimpleBlock") )
			{
			if( el_ptr->data.at(i)->ID == IDof("SimpleBlock") ){
				cout << endl << el_ptr->data.at(i)->name;
				cout << " Size: " <<  setw (5) << dynamic_cast<binElement*>(el_ptr->data.at(i))->datasize << "\t";
				}
			binElement* Block_ptr = dynamic_cast<binElement*>(el_ptr->data.at(i));
			Block bl = BlockDataAnalysis( Block_ptr->data, Block_ptr->datasize, el_ptr->data.at(i)->ID == IDof("SimpleBlock"));
			bl.Print();
			}
		else
			el_ptr->data.at(i)->print( false );

		if( el_ptr->data.at(i)->type == subElements ){
			cout << el_ptr->data.at(i)->name;
			cout << " Size: " << dynamic_cast<subElement*>(el_ptr->data.at(i))->subsize() << "\t";
			TreeParserPrintHR( dynamic_cast<subElement*>(el_ptr->data.at(i)), lvl+1 );
			}
		else
			cout <<"\t";
		}
	}
// Writes el_ptr's ingredients to file, not el_ptr self!
void Meteorite::TreeParserWrite( subElement* el_ptr, ofstream &outfile, unsigned lvl ){
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		el_ptr->data.at(i)->write( outfile );
		if( el_ptr->data.at(i)->type == subElements )
			TreeParserWrite( dynamic_cast<subElement*>(el_ptr->data.at(i)), outfile, lvl+1 );
		outfile.flush();
		}
	}
// Writes el_ptr self!
void Meteorite::TreeParserWriteMaster( subElement* el_ptr, ofstream &outfile, unsigned lvl ){
	el_ptr->write( outfile );
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		if( el_ptr->data.at(i)->type == subElements )
			TreeParserWriteMaster( dynamic_cast<subElement*>(el_ptr->data.at(i)), outfile, lvl+1 );
		else
			el_ptr->data.at(i)->write( outfile );
		outfile.flush();
		}
	}

bool Meteorite::CheckCluster( char *bfr ){	//Checks cluster if its broken or not
	if( bfr not_eq NULL ){
		uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t* >( bfr ));
		unsigned i = 4;
		if( p32 == IDof( "Cluster" ) ){
			unsigned size = EBMLtouInteger( bfr+i, &i );
			size+=i;
			ID_Element *processor=NULL;
			while( i< size ){
				processor = FindElementID( DB.Cluster, bfr+i );
				if( processor not_eq NULL ){
					i+=processor->IDSize();
					skip( bfr, i, processor->type );
					}
				else{
					uint32_t a = make_bigendian( *reinterpret_cast<uint32_t*>( bfr+i ));
					cout << "ID token \"0x" << hex << a << dec << "\" not found on "  << DB.Cluster.idvector.at(0)->name
					 <<" list. Probably broken cluster. " << endl;
					return false;
					}
				}
			return true;
			}
		else
			cout << "Buffer not starting with _Cluster_ token" << endl;
		}
	return false;
}
subElement* Meteorite::ClusterRecover( char *bfr, unsigned rsize, unsigned MaxTrackNum ){	//Recovers cluster and return new cluster pointer
	if( bfr not_eq NULL ){
		uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t* >( bfr ));
		unsigned i = 4;
		if( p32 == IDof( "Cluster" ) ){
			unsigned size = EBMLtouInteger( bfr+i, &i );
			size+=i;
			subElement* newcluster= new subElement( "Cluster" );
			ID_Element *processor=NULL;
			// If Buffer under run, size constant become corrupt, using rsize for that
			while( i < rsize){ // and ( i < size )
				processor = FindElementID( DB.Cluster, bfr+i );
				if( processor not_eq NULL ){
					int idsz = processor->IDSize();
					unsigned ebmlsz = 0;
					unsigned sz = EBMLtouInteger( bfr+i+idsz, &ebmlsz );

					if( is_IDof( bfr+i, "BlockGroup" )){
						Block bl = BlockGroupAnalysis( bfr+i, MaxTrackNum );
						if( not bl.Valid )
							return newcluster;
						}
					else if( is_IDof( bfr+i, "SimpleBlock" )){
						Block bl = SimpleBlockAnalysis( bfr+i );
						if( not bl.Valid )
							return newcluster;
						}

					TreeParserChunk( bfr+i, idsz+sz, newcluster );

					i+=idsz+sz+ebmlsz;
					//skip( bfr, i, processor->type );
					}
				else{
					uint32_t a = make_bigendian( *reinterpret_cast<uint32_t*>( bfr+i ));
					cout << "ID token \"0x" << hex << a << dec << "\" not found on "  << DB.Cluster.idvector.at(0)->name
					 <<" list. Probably broken cluster. " << endl;
					return newcluster;
					}
				}
			return newcluster;
			}
		else
			cout << "Buffer not starting with _Cluster_ token" << endl;
			return NULL;
		}
	return false;
	}
inline bool Meteorite::IsKeyFrame( char *data, unsigned size, char *four_cc ){ //Finds KeyFrames for indexing
	uint32_t flag;
	memcpy(reinterpret_cast<char*>(&flag), data, 4);
	flag = make_littleendian( flag );
	if( !strncmp( four_cc, "DIV3", 4 ) ||
		!strncmp( four_cc, "MP43", 4 )){
		if( (flag & 0x00007001)==0x00007001 && ( (0xFFFF7F3F | flag) == 0xFFFF7F3F) )
			return true;
		else
			return false;
		}
	else if( !strncmp( four_cc, "DIVX", 4 ) ||
			  !strncmp( four_cc, "DX50", 4 ) ||
			  !strncmp( four_cc, "XVID", 4 ))
		//return ( (flag & 0xB6000000)==0xB0000000 || (flag & 0xB6000000)==0 );
		return (flag & 0x06000000)==0;

	else if( !strncmp( four_cc, "SVQ1", 4 )) //Sorenson Video 1
		return (flag & 0x01000000)==0 ;	  //For last 2 bit : 0 = I frame | 1 = P frame | 2 = B frame - Accept for I & B frames

	else if( !strncmp( four_cc, "VP30", 4 ) || // Theora &
			  !strncmp( four_cc, "VP31", 4 ) || // TrueMotion VP codecs
			  !strncmp( four_cc, "VP32", 4 ) ||
			  !strncmp( four_cc, "VP40", 4 ) ||
			  !strncmp( four_cc, "VP50", 4 ) ||
			  !strncmp( four_cc, "VP60", 4 ) || // FLV4?
			  !strncmp( four_cc, "VP61", 4 ) ||
			  !strncmp( four_cc, "VP62", 4 ))
			  //!strncmp( four_cc, "VP6F", 4 )) // ffmpeg (open source) - FFmpeg VP6 / Flash') ?
		return (flag & 0x00000080)==0;

	//	else if( !strncmp( four_cc, "VP70", 4 ) || // TrueMotion VP7 codecs - PROBLEM!
	//			  !strncmp( four_cc, "VP71", 4 ) ||
	//			  !strncmp( four_cc, "VP72", 4 ) )
	//		return ( flag & 0x00000001)==0;

	else if( !strncmp( four_cc, "H264", 4 ) || !strncmp( four_cc, "AVC1", 4 )){
			uint32_t nalu_len = 0;
			char* data_nextnalu = data;
			int pos = 0;
			int avcflag = 0;
			while( pos+4 < size ){
				data_nextnalu = data+pos;
				avcflag |= 1 << (data_nextnalu[4] & 0x0F)-1;
// TODO (death#1#): bigendian				nalu_len = make_bigendian( *reinterpret_cast< uint32_t*>( data_nextnalu ));
				pos += nalu_len+4;
				}
			return ((avcflag>>(5-1)) & 0x01);

//			int pos = 0;
//			bool z = false;
//			while( pos < size ){
//				if( data[++pos]!=0x00 ){
//					z = (data[pos+1] & 0x1F)==0x07;
//					break;
//					}
//				}
//			return z;
#define NALU_START_CODE 0x00000001
#define NALU_TYPE_NON_IDR_SLICE  0x01
#define NALU_TYPE_DP_A_SLICE     0x02
#define NALU_TYPE_DP_B_SLICE     0x03
#define NALU_TYPE_DP_C_SLICE     0x04
#define NALU_TYPE_IDR_SLICE      0x05
#define NALU_TYPE_SEI            0x06
#define NALU_TYPE_SEQ_PARAM      0x07
#define NALU_TYPE_PIC_PARAM      0x08
#define NALU_TYPE_ACCESS_UNIT    0x09
#define NALU_TYPE_END_OF_SEQ     0x0a
#define NALU_TYPE_END_OF_STREAM  0x0b
#define NALU_TYPE_FILLER_DATA    0x0c

//			if( flag & 0x01000000 )					// Some NAL has start_code_prefix_one_4bytes others start_code_prefix_one_3bytes
//				z = (data[4] & 0x1F)==0x07;		// if nal_unit_type 7 - Sequence parameter set.
//			else if( flag & 0x00010000)				// 0x1F filters last 5 bit which equals nal_unit_type
//				z = (data[3] & 0x1F)==0x07;		// IDR frame needed to be Type 5 (NAL_IDR_SLICE).
//			else									// but 7 (NAL_SPS) looks working. I might add 5 later.
//				z = false;						// 6 (NAL_SEI) could lucky number too..
//			return z;
		}

	else if( !strncmp( four_cc, "WMV1", 4 ))
		return (flag & 0x00000040)==0;
	else if( !strncmp( four_cc, "WMV2", 4 ))
		return (flag & 0x00000080)==0;
	else if( !strncmp( four_cc, "WMV3", 4 ))
		return (flag & 0x00000020)==0;
	else
		return (flag & 0x06000000)==0;	// Defaulting XVID codec flag.
	}
Block Meteorite::BlockGroupAnalysis( char *bfr, unsigned TrackCount=2 ){				//Analyzing BlockGroup Data and return Block Informations
	Block bl;
	if( bfr not_eq NULL ){
		unsigned i = 1;
		if( is_IDof( bfr, "BlockGroup" )){
			unsigned size = bl.Size = EBMLtouInteger( bfr+i, &i );
			size+=i;
			ID_Element *processor=NULL;
			while( i < size ){
				processor = FindElementID( DB.Cluster, bfr+i );
				if( processor not_eq NULL ){
					i+=processor->IDSize();
					if( processor->ID == IDof("Cluster") or
						processor->ID == IDof("Timecode") or
						processor->ID == IDof("SilentTracks") or
						processor->ID == IDof("Position") or
						processor->ID == IDof("PrevSize") or
						processor->ID == IDof("BlockGroup") or
						processor->ID == IDof("SimpleBlock")
						){
						//Those are Level 1 and 2 IDs that in Level 3 area means invalid blockgroup.
						bl.Valid = false;
						return bl;
						}
					else if( processor->ID == IDof( "Block" ) ){
						unsigned sz = EBMLtouInteger( bfr+i, &i );
						bl = BlockDataAnalysis( bfr+i, sz );
						i+=sz;
						if( bl.TrackNum > TrackCount){
							bl.Valid = false;
							return bl;
							}
						bl.Valid = true;
						}
					else if( processor->ID == IDof( "BlockDuration") ){
						uintElement z( ID_Element( "BlockDuration") );
						bl.Duration = z.read( bfr+i, &i );
						}
					else
						skip( bfr, i, processor->type );
					}
				else{
//					uint32_t a = make_bigendian( *reinterpret_cast<uint32_t*>( bfr+i ));
//					cout << "ID token \"0x" << hex << a << dec << "\" not found on "  << DB.Cluster.idvector.at(0)->name <<" list. Probably broken Block. " << endl;
					bl.Valid = false;
					return bl;
					}
				}
			}
		}
	return bl;
	}
Block Meteorite::SimpleBlockAnalysis( char *bfr ){										//Analyzing SimpleBlock data layer for BlockTara Analysis
	Block bl;
	if( bfr not_eq NULL ){
		if( is_IDof( bfr, "SimpleBlock" ) ){
			unsigned i = 1;
			unsigned sz = EBMLtouInteger( bfr+i, &i );
			bl = BlockDataAnalysis( bfr+i, sz, true );
			}
		}
		bl.Valid = true;
	return bl;
	}
Block Meteorite::BlockDataAnalysis( char *bfr, unsigned size, bool SimpleBlock ){		//Lowest Level Block Analyzer that analyses Block Data
	Block bl;
	unsigned i=0;
	bl.Size = size;
	bl.Valid = false;
	bl.KeyFlag = false;
	bl.Duration = 0;//??
	bl.TrackNum = EBMLtouInteger( bfr+i, &i );
	bl.TimeCode = make_bigendian(*reinterpret_cast< int16_t* >( bfr+i ));
	i+=2;
	bl.Flags = *reinterpret_cast< uint8_t* >( bfr + i++ );
//	if(RefreshKeysFromStream)//Recalculate KeyFlags

	switch((bl.Flags >> 1) & 0x03){
		case 0x00:
			bl.Lacing = Block::Lace_NO;
			bl.Frames = 1;
			break;
		case 0x01:
			bl.Lacing = Block::Lace_Xiph;
			bl.Frames = *reinterpret_cast< uint8_t* >( bfr+i++ )+1;	//+1 for lacing store FrameCount-1
//			TODO
			break;
		case 0x03:
			{
			bl.Lacing = Block::Lace_EBML;
			bl.Frames = *reinterpret_cast< uint8_t* >( bfr+i++ )+1;	//+1 for lacing store FrameCount-1
			/**Range Shift:
			Lacing sizes: only the 2 first ones will be coded, 800 gives 0x320 0x4000 = 0x4320,
			500 is coded as -300 : - 0x12C + 0x1FFF + 0x4000 = 0x5ED3.
			(0x4000 leaves 0x4000 probabilities /2 = 0x2000 equal zero and 0x1FFF means 0)
			The size of the last frame is deduced from the total size of the Block.
			*/
			unsigned frm = 0, tfrm = 0;
			for( int j = 1 ; j < bl.Frames ; j++ ){
				if( j == 1 )
					frm += EBMLtouInteger(bfr+i, &i);
				else
					frm += EBMLtosInteger(bfr+i, &i);

				tfrm += frm;
				if( size < tfrm or frm <= 0 )	//invalid frame!
					break;
				//cout << "Frame " << j << " size :" << frm << endl;
				}

			break;
			}

		case 0x02:
			bl.Lacing = Block::Lace_Fixed;
			bl.Frames = *reinterpret_cast< uint8_t* >( bfr+i++ )+1;	//+1 for lacing store FrameCount-1
			break;
		}
	if( bl.Lacing == Block::Lace_NO )
		bl.KeyFlag = IsKeyFrame( bfr+i++, size, four_cc );
	return bl;

//	if( SimpleBlock )
//		bl.KeyFlag = bl.Flags >> 7;
	}
void Meteorite::ClusterAnalysis( char *bfr, unsigned defaultduration, uint64_t cluster_nanotimecode,
									uint64_t ThisClusterPosition, uint64_t& ClusterLocalDuration,
									short& ClusterMinusTimeElement, subElement *NewCues, unsigned TrackNum ){//Analyzes Clusters and finds cluster duration and Clusters minus elements
	if( bfr not_eq NULL ){
		uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t* >( bfr ) );
		unsigned i = 4;
		unsigned BlockCount = 0 ;
		if( p32 == IDof( "Cluster" ) ){
			unsigned size = EBMLtouInteger( bfr+i, &i );
			size+=i;
			ID_Element *processor=NULL;
			while( i< size ){
				processor = FindElementID( DB.Cluster, bfr+i );
				if( processor != NULL ){
					if(processor->ID == IDof( "BlockGroup" ) ){
						Block bl = BlockGroupAnalysis( bfr+i );
						if( bl.Valid ){
							if( bl.TrackNum == TrackNum ){
								BlockCount++;
								if( bl.KeyFlag or (cluster_nanotimecode == 0 and BlockCount == 1)){
	//									CueTimeCodes.push_back( bl.TimeCode*1000000 + cluster_nanotimecode );

									subElement *CP = new subElement( "CuePoint" );
									uintElement *CT = new uintElement( ID_Element( "CueTime" ) );
									if(cluster_nanotimecode == 0 and BlockCount == 1)
										CT->data = bl.TimeCode;
									else
										CT->data = rint((ClusterLocalDuration + cluster_nanotimecode) / 1000000.0);
									//CueTimeCodes.push_back( rint((ClusterLocalDuration + cluster_nanotimecode) / 1000000.0) );
									// TODO (death#1#): ERRORR!! 1000.0000 need read from tracks...
									subElement *CTP = new subElement( "CueTrackPositions" );
									uintElement *CTr = new uintElement( ID_Element( "CueTrack" ) );
									CTr->data = TrackNum;
									uintElement *CCP = new uintElement( ID_Element( "CueClusterPosition" ) );
									CCP->data = ThisClusterPosition;

									CTP->data.push_back( CTr );
									CTP->data.push_back( CCP );

									CP->data.push_back( CT );
									CP->data.push_back( CTP );

									NewCues->data.push_back( CP );
									}

								ClusterMinusTimeElement = min( bl.TimeCode , ClusterMinusTimeElement );
								ClusterLocalDuration  += (bl.Duration==0 ? defaultduration : /*bl.Duration*/ 2*defaultduration );
// TODO (death#1#): ERRORR!! - ???
								}
							}
						}
					i+=processor->IDSize();
					skip( bfr, i, processor->type );
					}
				else{
					uint32_t a = make_bigendian( *reinterpret_cast<uint32_t*>( bfr+i ) );
					cout << "ID token \"0x" << hex << a << dec << "\" not found on "  << DB.Cluster.idvector.at(0)->name
					 <<" list. Probably broken cluster. " << endl;
					//cout << "Cluster Valid thru byte : " << i << ", Recovering" << endl;
					return;
					}
				}
			}
		//return;
		}
	}
vector< Block > Meteorite::ClusterBlocksAnalysis( subElement* el_ptr, subElement* root ){//Analyzing All blocks at clusters and returns blocks infos as a vector
	vector< Block > BlocksVector;
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		if( el_ptr->data.at(i)->ID == IDof("BlockGroup")){
			subElement* BlockGroup_ptr = dynamic_cast< subElement* >( el_ptr->data.at(i) );
			Block bl;
			for( unsigned j = 0 ; j < BlockGroup_ptr->data.size() ; j++ ){
				if( BlockGroup_ptr->data.at(j)->ID == IDof( "Block" ) ){
					binElement* Block_ptr = dynamic_cast<binElement*>(BlockGroup_ptr->data.at(j));
					bl = BlockDataAnalysis( Block_ptr->data, Block_ptr->datasize );
					if( bl.Frames > 1 )
						bl.Duration = bl.Frames*GetDefaultFrameDuration( root, bl.TrackNum );
					}
				else if( BlockGroup_ptr->data.at(j)->ID == IDof( "BlockDuration" ) )
					bl.Duration = dynamic_cast< uintElement* >(BlockGroup_ptr->data.at(j))->data;
				}
			BlocksVector.push_back( bl );
			}
		else if( el_ptr->data.at(i)->ID == IDof("SimpleBlock") ){
			binElement* SimpleBlock_ptr = dynamic_cast< binElement* >( el_ptr->data.at(i) );
			Block bl;
			bl = BlockDataAnalysis( SimpleBlock_ptr->data, SimpleBlock_ptr->datasize, true );
			if( bl.Frames > 1 )
				bl.Duration = bl.Frames*GetDefaultFrameDuration( root, bl.TrackNum );
			BlocksVector.push_back( bl );
			}
		else if( el_ptr->data.at(i)->type == subElements ){
			cout << "ClusterDuration(): el_ptr->data.at(i)->type == subElements " << endl;
			}
		}
	return BlocksVector;
	}
uint64_t Meteorite::ClusterDuration( vector< Block >& vb, unsigned DefaultFrameDuration, unsigned TrackNo ){
	uint64_t total_duration = 0;
	for( vector< Block >::iterator it= vb.begin() ; it != vb.end() ; it++ )
		if( it->TrackNum == TrackNo ){
			Block z = *it;
			//total_duration += (it->Duration==0 ? DefaultFrameDuration : (rint(it->Duration / (DefaultFrameDuration / 1000000.0))*DefaultFrameDuration ));
			total_duration += (it->Duration==0 ? (DefaultFrameDuration*it->Frames) : it->Duration );
			}
	return total_duration;
	}
uint64_t Meteorite::ClusterMaximumBlockTimePlusDuration( vector< Block >& vb, unsigned DefaultFrameDuration, unsigned TrackNo ){
	uint64_t MaxTCodeAndDur = 0;
	for( vector< Block >::iterator it= vb.begin() ; it != vb.end() ; it++ )
		if( it->TrackNum == TrackNo ){
			Block z = *it;
			MaxTCodeAndDur = max( MaxTCodeAndDur, (static_cast<uint64_t>(it->TimeCode)*1000000 + (it->Duration==0 ? (DefaultFrameDuration*it->Frames) : it->Duration )));
			}
	return MaxTCodeAndDur;
	}
uint64_t Meteorite::ClusterDuration( subElement* el_ptr, unsigned defaultduration, unsigned TrackNo ){
	uint64_t ClusterDuration = 0;
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		if( el_ptr->data.at(i)->ID == IDof("BlockGroup")
			//or el_ptr->data.at(i)->ID == IDof("SimpleBlock")
			){
			subElement* BlockGroup_ptr = dynamic_cast< subElement* >( el_ptr->data.at(i) );
			Block bl;
			for( unsigned j = 0 ; j < BlockGroup_ptr->data.size() ; j++ ){
				if( BlockGroup_ptr->data.at(j)->ID == IDof( "Block" ) ){
					binElement* Block_ptr = dynamic_cast<binElement*>(BlockGroup_ptr->data.at(j));
					bl = BlockDataAnalysis( Block_ptr->data, Block_ptr->datasize );
					}

				else if( BlockGroup_ptr->data.at(j)->ID == IDof( "BlockDuration" ) )
					bl.Duration = dynamic_cast< uintElement* >(BlockGroup_ptr->data.at(j))->data;
				}
			if( bl.TrackNum == TrackNo )
				ClusterDuration += (bl.Duration==0 ? defaultduration : (rint(bl.Duration / (defaultduration / 1000000.0))*defaultduration ));
			}
		else if( el_ptr->data.at(i)->type == subElements ){
			cout << "ClusterDuration(): el_ptr->data.at(i)->type == subElements " << endl;
			}
		}
	return ClusterDuration;
	}
void Meteorite::ClusterBlocksTimeRepair( subElement* root, subElement* el_ptr ){
	const unsigned MaxTracksCount = 10;
	if( GetTrackCount(root) >= MaxTracksCount+1 ){
		cout << "Error, This stream includes " << GetTrackCount(root) << "stream but max " << MaxTracksCount << "Allowed" << endl;
		}

	uint64_t ClusterTrackDuration[ MaxTracksCount+1 ];
	for( unsigned i = 0 ; i < MaxTracksCount+1 ; i ++ ){	//take ClusterTrackDuration[ 1 ] indicates first track. 0 not used!
		ClusterTrackDuration[ i ] = 0;
		}

	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		if( el_ptr->data.at(i)->ID == IDof("BlockGroup")
			//or el_ptr->data.at(i)->ID == IDof("SimpleBlock")
			){
			subElement* BlockGroup_ptr = dynamic_cast< subElement* >( el_ptr->data.at(i) );
			Block bl;
			for( unsigned j = 0 ; j < BlockGroup_ptr->data.size() ; j++ ){
				if( BlockGroup_ptr->data.at(j)->ID == IDof( "Block" ) ){
					char* bfr = dynamic_cast<binElement*>(BlockGroup_ptr->data.at(j))->data;
					unsigned p=0;
					bl.TrackNum = EBMLtouInteger( bfr+p, &p );
					bl.TimeCode = make_bigendian(*reinterpret_cast< int16_t* >( bfr+p ));
					bl.Flags = *reinterpret_cast< uint8_t* >( bfr+p+2 );
					switch((bl.Flags >> 1) & 0x03){
						case 0x00:
							bl.Lacing = Block::Lace_NO;
							break;
						case 0x01:
							bl.Lacing = Block::Lace_Xiph;
							break;
						case 0x03:
							bl.Lacing = Block::Lace_EBML;
							break;
						case 0x02:
							bl.Lacing = Block::Lace_Fixed;
							break;
						}
					if( bl.Lacing ){
						bl.Frames = *reinterpret_cast< uint8_t* >( bfr+i+3 );
						}

					bl.TimeCode = rint(ClusterTrackDuration[bl.TrackNum] / 1000000.0);
					*reinterpret_cast< int16_t* >( bfr+p ) = make_bigendian( bl.TimeCode );//update Blocksbuffer Tcode
					ClusterTrackDuration[bl.TrackNum] += GetDefaultFrameDuration( root, bl.TrackNum);
					}
				else if( BlockGroup_ptr->data.at(j)->ID == IDof( "BlockDuration" ) )
					bl.Duration = dynamic_cast< uintElement* >(BlockGroup_ptr->data.at(j))->data;
				}
//			if( bl.TrackNum == TrackNo )
//				ClusterDuration += (bl.Duration==0 ? defaultduration : (rint(bl.Duration / (defaultduration / 1000000.0))*defaultduration ));
			}
		else if( el_ptr->data.at(i)->type == subElements ){
			cout << "ClusterDuration(): el_ptr->data.at(i)->type == subElements " << endl;
			}
		}
//	return ClusterDuration;
	}
short Meteorite::ClusterMinimumBlockTime( vector< Block >& vb, unsigned TrackNo ){
	short minimumtime = 0x7FFF;
	for( vector< Block >::iterator it= vb.begin() ; it != vb.end() ; it++ )
		if( it->TrackNum == TrackNo )
			minimumtime = min(it->TimeCode, minimumtime );
	return minimumtime;
	}
char* Meteorite::Clusterart(){
	return "*********************************************************************\n"\
			"*                            `,. ,;:`                               *\n"\
			"*                         `;''''''++++;'':                          *\n"\
			"*                      `:;''+++#+++#+#''++'.                        *\n"\
			"*                     :''++'++++'####+++####'                       *\n"\
			"*                   `';;''+'#+++######+######'                      *\n"\
			"*                   ;;''++#+#+'##@############:                     *\n"\
			"*                  .;;''+++++++###@@@@#########.                    *\n"\
			"*                  :;';'''+##+####@@#@@@####@##+                    *\n"\
			"*                 ,:;++++++#+####@#@#@@@###@#@##.                   *\n"\
			"*                 ;;;;'''++##########@@@###@@###;                   *\n"\
			"*                 ;';;'''+++##+++###########@####                   *\n"\
			"*                `'';;;::'''+++'+++++####+###@###`                  *\n"\
			"*                :;;;,``.,:;'''''''''++'+++######,                  *\n"\
			"*                :;;;`` `,:;;;'''''''''''''+#####;                  *\n"\
			"*                ,;;.`  `,:;;;''''''''''''''+####;                  *\n"\
			"*                .;;`   `,:;;;''''''''''''''+####+                  *\n"\
			"*                .;.````.,:;;;'''''''''''''++####+                  *\n"\
			"*                ,;.`````,:;;''''''''''''''++#####                  *\n"\
			"*                ,:.`````.:;'''''''''''''''++#####                  *\n"\
			"*                .;,`````,:;'';''''''''++''++#####                  *\n"\
			"*                `;:````.,:;'''''''''+++'''''####+                  *\n"\
			"*                 ';````.,;'''''''+++####''''+####:`                *\n"\
			"*                 ;:`.,:;+++'''++###@#+'#@+''+#@#+''                *\n"\
			"*                 `:,'++####+''+#@#@@@++##'+''##+''+;               *\n"\
			"*                 .,:++#####+'++@@@@@#+#@##+''##+''+'               *\n"\
			"*                ,`;,'#######''#@@@@@#@###+'''##'''''               *\n"\
			"*                ;;;;+#+#####''+@@@@@@####+'''+#+''':               *\n"\
			"*                ;',;''#####;,''#@@@@@@@@#''''+#'+''                *\n"\
			"*                .;,;;'++###:`'''#@@@@@#+''''''#'+''                *\n"\
			"*                 ;.`:'+#+',.`;'''+++++++''''''#++'.                *\n"\
			"*                 ,.```.,:;,``;''+'+++'''''''''+'''                 *\n"\
			"*                  .```.,::: `;''''++''''''''''''';                 *\n"\
			"*                  .```.:;';.,'''+++++'''''''''''':                 *\n"\
			"*                  ````.:'';:,+'+++++++'''''''''''                  *\n"\
			"*                   ``.,;';..,'++++++++'''''''''                    *\n"\
			"*                   `..,;:,.,:++++++++++''''''''`                   *\n"\
			"*                   `..::,:..,;''+''++++''''''''                    *\n"\
			"*                    ..,:;'.:''++#####+++++'''''                    *\n"\
			"*                    ..,:++'.:;+++++++++++++++''                    *\n"\
			"*                    `.,:+'`.,;+++''++++++++++''`                   *\n"\
			"*                     ,,:;,,.:'+++++++++++++++''.                   *\n"\
			"*                     `,,:,,,;'++++++++++++++''',                   *\n"\
			"*                      ,,,,,,::;++++++++++++++'':                   *\n"\
			"*                       :,.,:,:'++++++++++++++'''                   *\n"\
			"*                        :,,::'+++++++++++++++''';                  *\n"\
			"*                         ,,;;'++++++++++++++'''''`                 *\n"\
			"*                          :;'++++######+++++''''''                 *\n"\
			"*                          `.'++#######++++++'''''''`               *\n"\
			"*                           ..,'++++++++++++'+''''''##              *\n"\
			"*                           ..,,;+++++++++++''''''+@##+             *\n"\
			"*                           `.,,;'+++++++++''''''+@#####+.          *\n"\
			"*                          ,+..,:'+'''+''+'''''+#@@@@#######:       *\n"\
			"*                         ,#+`.,:;''''+''''''+#@@@@@@@@@@#####'     *\n"\
			"*                       ,##+#`..,:;;'''''''+#@@@@@@@@@@@@@@@####:   *\n"\
			"*                      ####+#``.,,:;'''''+#@@@@@@@@@@@@@@@@@@@@###'`*\n"\
			"*                '#########+++``.,:;''''#@@@@@@@@@@@@@@@@@@@@@@#####*\n"\
			"*               ###########++#;`.,,:;;+@#@@@@@@@@@@@@@@@@@@@@@@@@###*\n"\
			"*            '+###########+'###``,,:;##@#@@#@@@@@@@@@@@@@@@@@@@@@@@#*\n"\
			"*           ##+############+#+##.,,+######@@@@@@@@@@@@@@@@@@@@@@@@@@*\n"\
			"*         ++###@##########++#+###'#######@@@@@@@@@@@@@@@@@@@@@@@@@@@*\n"\
			"*        #'##+#@#####+####++#+++########@@@@@@@#@#@##@@@@@@@@@@@@@@@*\n"\
			"*       `#+###@###########+++##########@@@@@@##@###########@@@@@@@@@*\n"\
			"*      :+#+##+@#####+#+@##++##+++#####@@@@@@@#@###############@@@@@@*\n"\
			"*     :'##+###@@####+##@##++#########@@@@@###@############@@@@###@@#*\n"\
			"*    .++##+##+@@@######@########+####@@@@#############@@###@@@@@@###*\n"\
			"*   `+####+###@########@#++++########@@@@############@#@@@@@##@@@###*\n"\
			"*                          I am watching you!                       *\n"\
			"*********************************************************************\n";
	}
subElement* Meteorite::MakeCuePoint( uint64_t ClusterPosition, unsigned TrackNo, uint64_t CueTime ){
	subElement *CP = new subElement( "CuePoint" );
	uintElement *CT = new uintElement( ID_Element( "CueTime" ) );
//			if(ClusterNanoTimeCode == 0 and BlockCount == 1)
//				CT->data = it->TimeCode;
//			else
		CT->data = CueTime;
	//CueTimeCodes.push_back( rint((ClusterLocalDuration + cluster_nanotimecode) / 1000000.0) );
	// TODO (death#1#): ERRORR!! 1000.0000 need read from tracks...
	subElement *CTP = new subElement( "CueTrackPositions" );
	uintElement *CTr = new uintElement( ID_Element( "CueTrack" ) );
	CTr->data = TrackNo;
	uintElement *CCP = new uintElement( ID_Element( "CueClusterPosition" ) );
	CCP->data = ClusterPosition;

	CTP->data.push_back( CTr );
	CTP->data.push_back( CCP );

	CP->data.push_back( CT );
	CP->data.push_back( CTP );
	return CP;
	}
void Meteorite::Generate_CueTimeCodes( vector< Block >& vb, subElement* Cues_ptr, uint64_t ClusterPosition, uint64_t ClusterNanoTimeCode, unsigned DefaultFrameDuration, unsigned TrackNo ){
	uint64_t ClusterLocalDuration = 0;
	for( vector< Block >::iterator it= vb.begin() ; it != vb.end() ; it++ )
		if( it->TrackNum == TrackNo ){
//			it->Print();
			uint64_t CueTime = rint((ClusterLocalDuration + ClusterNanoTimeCode) / 1000000.0);
			if( it->KeyFlag
				or (ClusterNanoTimeCode==0 and ClusterLocalDuration==0) ){	//Forces first Video Block to keyflag
				subElement* CuePoint_ptr = MakeCuePoint( ClusterPosition, TrackNo, CueTime );
				Cues_ptr->data.push_back( CuePoint_ptr );
				}
			ClusterLocalDuration += (it->Duration==0 ? DefaultFrameDuration : (rint(it->Duration / (DefaultFrameDuration / 1000000.0))*DefaultFrameDuration ));
			}
//	return minimumtime;

//	if( bl.KeyFlag or (cluster_nanotimecode == 0 and BlockCount == 1)){
	//	CueTimeCodes.push_back( bl.TimeCode*1000000 + cluster_nanotimecode );
	}
// Search for hugo vector at el_ptr tree. RECURSIVELY.
ID_Element* Meteorite::Get( subElement *el_ptr, vector< uint32_t > hugo, unsigned vctr ){
	ID_Element* ret;
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		ret = el_ptr->data.at(i);
//			cout << "Get(): vctr= " << vctr << " name= " << ret->name << endl;
		if( ret->ID == hugo.at(vctr) ){
			if( vctr == hugo.size()-1 )
				return el_ptr->data.at(i);
			vctr++;
			}
//			el_ptr->data.at(i)->print();
		if( el_ptr->data.at(i)->type == subElements ){
			ret = Get( dynamic_cast<subElement*>(el_ptr->data.at(i)), hugo, vctr );
			if( ret not_eq NULL )
				return ret;
			}
		}
	return NULL;
	}
ID_Element* Meteorite::Get( subElement *el_ptr, uint32_t IDofKey ){
	vector<uint32_t> a;
	a.push_back( IDofKey );
	return Get( el_ptr, a );
	}
void Meteorite::TreeSearchID( subElement *el_ptr, uint32_t IDofKey, vector< ID_Element* >& a ){
	ID_Element* ptr;
	for( unsigned i=0 ; i < el_ptr->data.size() ; i++ ){
		ptr = el_ptr->data.at(i);
//			cout << "Get(): vctr= " << vctr << " name= " << ret->name << endl;
		if( ptr->ID == IDofKey ){
			a.push_back(ptr);
			}
//			el_ptr->data.at(i)->print();
		if( el_ptr->data.at(i)->type == subElements ){
			TreeSearchID( dynamic_cast<subElement*>(el_ptr->data.at(i)), IDofKey, a );
			}
		}
	}

void Meteorite::fourcc_finder( subElement* root ){
	vector<uint32_t> a;
	a.clear();
	a.push_back( IDof("Tracks") );
	a.push_back( IDof("CodecID"));
	stringElement *Coid = dynamic_cast<stringElement*>(Get( root, a ));
	if( Coid->data == "V_MS/VFW/FOURCC"){
		a.clear();
		a.push_back( IDof("Tracks") );
		a.push_back( IDof("CodecPrivate") );
		binElement *b = dynamic_cast<binElement*>(Get( root, a ));
		if( b != NULL )
			strncpy( four_cc, b->data+16, 4 );
		four_cc[4]='\0';
		}
	else if( Coid->data == "V_MPEG4/ISO/AVC" ){
		strncpy( four_cc, "AVC1\0", 5 );
		}
	else{
		cout << "Unsupported CodecID : " << Coid->data << endl;
		}
	}
uint32_t Meteorite::GetDefaultFrameDuration( subElement* root, unsigned TrackNum ){
//	vector<uint32_t> a;
//	a.push_back( IDof("Tracks") );
//	a.push_back( IDof("DefaultDuration") );
//	uintElement *defdur_p = dynamic_cast<uintElement*>(Get( root, a ));
//	a.clear();
//	if( defdur_p not_eq NULL)
//		return defdur_p->data;
//	else{
//		cerr << "Error : No Default Duration detected!" << endl;
//		exit(1);
//		}
	vector<ID_Element*> a;
	TreeSearchID( root, IDof("TrackEntry"), a );
	for( vector< ID_Element* >::iterator it = a.begin() ; it != a.end() ; it++ ){
		if( *it not_eq NULL ){
			subElement* TrackEntry_ptr = dynamic_cast<subElement*>(*it);
			if( TrackEntry_ptr not_eq NULL ){
				uintElement* TrackNumber_ptr = dynamic_cast< uintElement* >( Get( TrackEntry_ptr, IDof( "TrackNumber" ) ) );
				if( TrackNumber_ptr not_eq NULL ){
					if( TrackNumber_ptr->data == TrackNum ){
						uintElement* DefaultDuration_ptr = dynamic_cast< uintElement* >( Get( TrackEntry_ptr, IDof( "DefaultDuration" ) ) );
						if( DefaultDuration_ptr not_eq NULL )
							return DefaultDuration_ptr->data;
						}
					}
				}
			}
		}

//	a.clear();
//	if( defdur_p not_eq NULL)
//		return defdur_p->data;
//	else{
//		cerr << "Error : No Default Duration detected!" << endl;
//		exit(1);
//		}

	}
int Meteorite::GetTrackCount( subElement* root){
	vector<ID_Element*> a;
	TreeSearchID( root, IDof("TrackNumber"), a );
	uint64_t cnt = 0;
	for( vector< ID_Element* >::iterator it = a.begin() ; it != a.end() ; it++ ){
		uintElement* TrackNumber_ptr = dynamic_cast< uintElement* >( *it );
		if( TrackNumber_ptr not_eq NULL )
			cnt = max( cnt, TrackNumber_ptr->data );
		}
	if( a.size() != cnt ){
		cout << "Max TrackNum is " << cnt << " while there is actualy " << a.size() << "records" << endl;
		getchar();
		}
	return cnt;
	}


bool Meteorite::Repair( string source, string target ){
	///Please don't try to merge with Meteorite and DivFix++ for release.
	///I will merge them when time comes.
	///There is MKV check and strip and overwrite like options needed to implemented.
	///After DivFix++ will use Meteorite engine as internal code or via dll...
	///Being different program will provide better support speed for MKV.
	///If you are reading this lines, you might familiar with coding.
	///So think about join to development of Meteorite and/or DivFix++
	///Thanks, Erdem
	ifstream myfile( source.c_str(), ios::binary );
		if( !myfile.is_open() )
			return false;
	cout << endl << endl ;
	cout << "Meteorite Code version " << METEORITE_VERSION << endl;
	cout << "This Software is under GPL license v2." << endl;
	cout << "All rights reserved (R) 2009 (C) Erdem U. Altunyurt" << endl;

	myfile.seekg( 0, ios::end );
	filesize = myfile.tellg();
	myfile.seekg( 0, ios::beg );
	myfile.clear();

	char *buffer = new char [bfr_size];
	myfile.read( buffer, 16 );
	uint64_t read_from = 0;
	uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t*>( buffer ) );
	unsigned i=4;
	if( p32 != IDof( "EBMLFile" ) ){	//EBML file
		cout << "input file is not in EBML/matroska format!";
		return false;
		}
	unsigned ebml_size = EBMLtouInteger( buffer+i ); //(buffer[4] & 0x7F);
	uint64_t SegmentSize = 10;
	uint64_t SegmentStart = 0;
	vector <uint64_t> seeks;
	subElement *tmp_ptr=NULL, *root=NULL;
	bool seekhead = true;
	root = new subElement( ID_Element( "Root", 0x00, subElements ) );
	tmp_ptr = root;

	myfile.seekg(0, ios::end);
	uint64_t filelenght = myfile.tellg();

	while(( read_from < ebml_size+SegmentSize+4+4 ) and !myfile.eof() and read_from < filelenght ){
		if( not update_gauge( rint( (100*read_from) / filesize) ))
			return 0;
		myfile.seekg( read_from );
		myfile.read( buffer, 16 );
		p32 = make_bigendian( *reinterpret_cast< uint32_t* >( buffer ) );
		i = 4;

		if( is_IDof(buffer, "Void" )){
			i = 1;
			unsigned size = EBMLtouInteger( buffer+1, &i );
			cout << "Void Size: " << size << endl;
			//TreeParserChunk( buffer, size+i, tmp_ptr );
			read_from += i+size;
			}

		else if(	p32 == IDof( "EBMLFile" ) or
					p32 == IDof( "Info")  or
					p32 == IDof( "Attachments" ) or
					p32 == IDof( "Chapters" ) or
					p32 == IDof( "Tags" )){
			int size = EBMLtouInteger( buffer+i, &i );
			myfile.seekg( read_from );
			myfile.read( buffer, size+i );
			ID_Element *processor = FindElementID( buffer );
			if( processor not_eq NULL ){
				tmp_ptr->data.push_back( TreeParserChunkMaster( buffer, size+i, read_from ) );
				read_from += size+i;
				}
			else{
				uint32_t a = make_bigendian( *reinterpret_cast<uint32_t*>( buffer+i ));
				cout << "Warning token not found : " << hex << a << endl;
				}
			}

		else if( p32 == IDof( "SeekHead" ) ){
			unsigned size = EBMLtouInteger( buffer+i, &i );
			myfile.seekg( read_from );
			myfile.read( buffer, size+i );
			if( seekhead ){
				tmp_ptr->data.push_back( TreeParserChunkMaster( buffer, size+i, read_from ) );
				seekhead= false; //renders second seekhead useless.
				}
			read_from += size+i;
			}
		else if( p32 == IDof( "Cues" ) ){
			int size = EBMLtouInteger( buffer+i, &i );
			read_from += size+i;
			}
		else if( p32 == IDof( "Tracks" ) ){
			int size = EBMLtouInteger( buffer+i, &i );
			myfile.seekg( read_from );
			myfile.read( buffer, size+i );
			read_from += size+i;
			cout << "Track Segment Size:" << size << endl;
			tmp_ptr->data.push_back( TreeParserChunkMaster( buffer, size, read_from ) );

			}
		else if( p32 == IDof( "Segment" ) ){
			uint64_t size = EBMLtouInteger( buffer+i, &i );
			SegmentSize = size;
			cout << "Segment Size:" << SegmentSize << endl;
			read_from += i;
			SegmentStart = read_from;
			subElement *a = new subElement( "Segment", read_from );
			tmp_ptr->data.push_back( a );
			tmp_ptr = a;
			}
		else if( p32 == IDof( "Cluster" ) ){
			cout << endl << "Detected MKV Structure" << endl;
			TreeParserPrint( root );
			string a = "ABCDEFGH";
			cout << "Source: " << source << endl;
			int found = source.find_last_of('/');
			string output;
			if( target.empty() )
				output = source.substr(0,found+1) + string("Meteorite.") + source.substr(found+1);
			else
				output = target;
			cout << "Target: " << output << endl;

			if( not ClusterSectionRepair( root, read_from, SegmentStart, SegmentSize, myfile, output ) )
				remove( output.c_str() );
				//wxRemoveFile( output );
			//ClusterSectionRepair( root, read_from, SegmentStart, SegmentSize, myfile, "/mnt/GaMeR/meteorite.mkv" );
			}

		}//while
	delete buffer;
	delete root;
	return true;
	}
bool Meteorite::ClusterSectionRepair( subElement* root, uint64_t& read_from, uint64_t SegmentStart, uint64_t SegmentSize, ifstream& myfile, string output ){

	cout << "EBML size = " << root->data.at(0)->Size() << endl
		<< "Segment size = " << root->data.at(1)->Size() << endl;
	int ClusterStart =  read_from;
	cout << "First Cluster located at: " << ClusterStart << endl;

	ofstream outfile( output.c_str(), ios::binary );
	if( !outfile.is_open() ){
		cerr << "Cannot create file meteorite.mkv" << endl;
		return false;
		}

	char *buffer = new char [read_from+1];
	myfile.seekg(0);
	myfile.read( buffer, read_from );
	outfile.write( buffer, read_from );
	delete buffer;

	vector<uint64_t> seeks;

	subElement *CueTCodes = new subElement( ID_Element( "Cues" ) );

	uint64_t Duration=0;
	if( not ClustersCopier( root, SegmentStart, SegmentSize, read_from,
													CueTCodes, seeks,
													Duration,//returns
													myfile, outfile ) )
		return false;



///				Regenerate SeekHead & Cue Time Codes
	subElement *NewSeekHead = Regenerate_SeekHead( seeks );
	TreeParserPrintHR( NewSeekHead );
	cout << endl;
	TreeParserPrintHR( CueTCodes );
	cout << endl;

	uint64_t NewSeekHeadPos = outfile.tellp();
	TreeParserWriteMaster( NewSeekHead, outfile );
	uint64_t NewCueTimePos = outfile.tellp();
	TreeParserWriteMaster( CueTCodes, outfile );

	delete NewSeekHead;
	delete CueTCodes;

	subElement* Segment_ptr = dynamic_cast< subElement* >( Get( root , IDof("Segment")) );

	Repair_Info_Duration( Segment_ptr, Duration );//need to be before Metaseek because of change of Info lenght
	Regenerate_MetaSeek( Segment_ptr, SegmentStart, NewSeekHeadPos, NewCueTimePos);

	Repair_Segment_Size( root, myfile, outfile );
	Adjust_Void_Sections( Segment_ptr, SegmentStart, ClusterStart, outfile );
	return true;
	}
//This is main function that regenerates Cluster sections on new file
bool Meteorite::ClustersCopier( subElement* root, uint64_t SegmentStart, uint64_t SegmentSize, uint64_t& read_from,
						  subElement *CueTCodes, vector<uint64_t>& seeks, uint64_t& Duration,
						  ifstream& infile, ofstream& outfile ){
	fourcc_finder( root );
	int cnt=1;

	uint64_t LastClusterVideoTimeCode = 0;
	uint64_t LastClusterAudioTimeCode = 0;
	uint64_t LastClusterTimeCode = 0;
	uint64_t LastClusterTimeCodeMsec = 0;
	int tv=0, ta=0;

	uint64_t LastClusterEnd = 0;
	subElement* MetaSeek_ptr= dynamic_cast< subElement* >(Get( root, IDof("SeekHead") ));
	for( unsigned i = 0 ; i < MetaSeek_ptr->data.size() ; i++){
		subElement* processor = dynamic_cast< subElement*>( MetaSeek_ptr->data.at(i) );
		if( processor->data.at(1)->ID == IDof("SeekPosition") ){
			if( dynamic_cast< uintElement* >(processor->data.at(1))->data > read_from )
				LastClusterEnd = dynamic_cast< uintElement* >(processor->data.at(1))->data;
			}
		}
	if( LastClusterEnd == 0 )
			LastClusterEnd = SegmentStart + SegmentSize;

	cout << "Checking for Cluster queue up to : " << LastClusterEnd << endl;

	int VideoTrackNo = -1; //first video track
	int AudioTrackNo = -1; //first audio track
	int TrackCount = -1;
	subElement* Tracks_ptr = dynamic_cast< subElement* >( Get( root , IDof("Tracks")) );
	if( Tracks_ptr not_eq NULL ){
		TrackCount = Tracks_ptr->data.size();
		for( unsigned i = 0 ; i < TrackCount ; i++ ){
			subElement* tmp_ptr = dynamic_cast< subElement* >( Tracks_ptr->data.at(i) );
			subElement* Video_ptr = dynamic_cast< subElement* >( Get( tmp_ptr, IDof("Video")) );
			subElement* Audio_ptr = dynamic_cast< subElement* >( Get( tmp_ptr, IDof("Audio")) );
			if( Video_ptr not_eq NULL )
				VideoTrackNo = dynamic_cast< uintElement* >( Get( tmp_ptr , IDof("TrackNumber")) )->data;
			if( Audio_ptr not_eq NULL )
				AudioTrackNo = dynamic_cast< uintElement* >( Get( tmp_ptr , IDof("TrackNumber")) )->data;
			}
		}
	if( VideoTrackNo == -1 ){
		cout << "Fatal: Cannot find main video track" << endl;
		exit(2);
		}
	cout << "Video TrackNumber: " << VideoTrackNo << endl;
	cout << "Audio TrackNumber: " << AudioTrackNo << endl;
	uint32_t DefaultVideoFrameDuration = GetDefaultFrameDuration( root, VideoTrackNo );
	uint32_t DefaultAudioFrameDuration = GetDefaultFrameDuration( root, AudioTrackNo );

	char *buffer = new char [bfr_size];

	while( read_from < LastClusterEnd ){
		if( infile.eof() ){
			cout << "Eof()!" << endl;
			break;
			}
		infile.seekg( read_from );
		infile.read( buffer, 16 );
		subElement* newCluster_ptr = NULL; //will be used for BlockGroup or SimpleBlock Merge Block.

		uint32_t p32 = make_bigendian( *reinterpret_cast< uint32_t* >( buffer ) );
		unsigned i = 4;
		if( p32 == IDof( "Cluster" ) ){
			unsigned size = EBMLtouInteger( buffer+i, &i );

			infile.seekg( read_from );
			infile.read( buffer, size+i );
			if( not CheckCluster( buffer )){
				cout << "Corrupted cluster at " << read_from << " ,recovering..." << endl;
				}
			unsigned rsize = infile.gcount();
			if( rsize != size+i )
				cout << "Error : Corrupted cluster! Buffer under run! Readed: " << rsize << " - Size+i: " << size+i << endl;

			uint64_t ClusterPosition = - SegmentStart + outfile.tellp();
			seeks.push_back( ClusterPosition );	//store cluster locations for generate cluster seekhead

			//subElement *cluster = TreeParserChunkMaster( buffer );
			subElement *cluster = ClusterRecover( buffer, rsize, GetTrackCount(root) );		//Generate Cluster Tree

			//TreeParserPrintHR(cluster);

			vector<Block> vb = ClusterBlocksAnalysis( cluster, root );//Generate Block hashes

			///Here 3 different method I tried to success on guessing exact time code for Clusters but I failed at all.
			///So, Don't expect some reference code exactly correct. It has dıfference by 1 microsecconds
			///I think Matroska implementation is faulty but mine :D

			if(0){///Method : Calculation via frame numbers
				int a=0, v=0;
				for( vector< Block >::iterator it= vb.begin() ; it != vb.end() ; it++ )
					if( it->TrackNum == AudioTrackNo ){
						Block z = *it;
						//total_duration += (it->Duration==0 ? DefaultFrameDuration : (rint(it->Duration / (DefaultFrameDuration / 1000000.0))*DefaultFrameDuration ));
						a += it->Frames;
						}
					else if( it->TrackNum == VideoTrackNo ){
						Block z = *it;
						v += (it->Duration==0 ? 1 : rint(it->Duration / (DefaultVideoFrameDuration / 1000000.0)) );
						}

				ta +=a;
				tv +=v;
				double tvfps = 1000000000.0 / DefaultVideoFrameDuration;
				double tca=ta * 1000000000.0 / DefaultAudioFrameDuration;
				double tcv=tv * 1000000000.0 / tvfps;
			}///Calculation via frame numbers

//				uint64_t ClusterLocalDuration=0;
//				short ClusterMinimumTimeElement=0x7FFF;
//				ClusterAnalysis( buffer,DefaultVideoFrameDuration,LastClusterTimeCode, ClusterPosition, ClusterLocalDuration, ClusterMinimumTimeElement, CueTCodes, TrackNo );
/////////////////////////////

			if(0){///Calculate via Cluster Video and Audio Durations Method
				uint64_t ClusterLocalVideoDuration = ClusterDuration( vb, DefaultVideoFrameDuration, VideoTrackNo );
				uint64_t ClusterLocalAudioDuration = ClusterDuration( vb, DefaultAudioFrameDuration, AudioTrackNo );

				int64_t ClusterVideoMinimumTimeElement = ClusterMinimumBlockTime( vb, VideoTrackNo );
				int64_t ClusterAudioMinimumTimeElement = ClusterMinimumBlockTime( vb, AudioTrackNo );

				int64_t ClusterVideoMaximumTimeElementAndDuration = ClusterMaximumBlockTimePlusDuration( vb, DefaultVideoFrameDuration, VideoTrackNo );
				int64_t ClusterAudioMaximumTimeElementAndDuration = ClusterMaximumBlockTimePlusDuration( vb, DefaultAudioFrameDuration, AudioTrackNo );

	///			LastClusterTimeCodeMsec += ClusterVideoMaximumTimeElementAndDuration - ClusterVideoMinimumTimeElement;
	///			LastClusterTimeCode += ClusterLocalDuration;	//LCTC takes RAW cluster time code (not effected by CMTE )
				LastClusterVideoTimeCode += ClusterLocalVideoDuration;// + ClusterVideoMinimumTimeElement*1000000;/
				LastClusterAudioTimeCode += ClusterLocalAudioDuration;// + ClusterAudioMinimumTimeElement*1000000;
				LastClusterTimeCode = min( LastClusterVideoTimeCode, LastClusterAudioTimeCode );
				LastClusterTimeCodeMsec = rint(LastClusterTimeCode/1000000.0);
			}

			uint64_t ClusterLocalDuration = ClusterDuration( vb, DefaultVideoFrameDuration, VideoTrackNo );
			int64_t ClusterMinimumTimeElement = ClusterMinimumBlockTime( vb, VideoTrackNo );

			if(LastClusterTimeCode == 0 and ClusterMinimumTimeElement > 0 )	//Sometime, video start later. This fixes duration of first cluster
				ClusterLocalDuration += rint((DefaultVideoFrameDuration*ClusterMinimumTimeElement*1000000.0)/DefaultVideoFrameDuration);

			/// New Cluster Timecode = LastCluster TCode + LClusterDuration() + ( Tcode of Current Cluster )
			/// ClusterDuration()  = LCluster Block MaxTcode + duration or Default Duration
			uint64_t ClusterTimeCode = 0;
			if( LastClusterTimeCode != 0 )
				ClusterTimeCode = (LastClusterTimeCode - ( (ClusterMinimumTimeElement == 0) ? 0 : ClusterMinimumTimeElement*1000000 ));


			uintElement *tc = dynamic_cast<uintElement*>( Get( cluster, IDof("Timecode") ));
			tc->data = rint(ClusterTimeCode / 1000000.0);

			Generate_CueTimeCodes( vb, CueTCodes,	//Add KeyFrames to Cues Tree
								ClusterPosition, ClusterTimeCode, DefaultVideoFrameDuration, VideoTrackNo );

			cout << "\nCluster " << cnt++ << " Processed on byte "<< read_from
							<< " Timecode(c): " << (uint64_t) rint(ClusterTimeCode / 1000000.0) << " Total " << vb.size() << " Blocks";

			LastClusterTimeCode += ClusterLocalDuration;	//LCTC takes RAW cluster time code (not effected by CMTE )
			TreeParserWriteMaster( cluster, outfile );
			delete cluster;


			read_from += rsize;
			}
		else{	// Search cluster or even blocks
			infile.seekg( read_from );
			infile.read( buffer, bfr_size );
			int readed = infile.gcount();

			uint32_t src = make_bigendian( IDof( "Cluster" ) );
//			#pragma omp parallel for private(i) schedule(static,5) num_threads(4)
			for(unsigned j=0 ; j < readed-4 ; j++ ){
				if( 0 == memcmp( (buffer+j), &src , 4 ) ){	//Cluster!
					read_from += j;
					if( j + 4 + 2 < bfr_size ){
						uint32_t zzz = make_bigendian(IDof("SeekID"));
						if( 0 == memcmp( (buffer+j+4), &zzz , 2 ) )
							cout << "Metaseek ClusterID found!" << endl;
						else{
							infile.clear(); 	//Clears EOF!
							///This pile of code needed to revisited
//							if( newCluster_ptr not_eq NULL ){
//								uint64_t ClusterPosition = - SegmentStart + outfile.tellp();
//								seeks.push_back( ClusterPosition );
//
//								ClusterBlocksTimeRepair( root, newCluster_ptr );
//
//								vector<Block> vb = ClusterBlocksAnalysis( newCluster_ptr, root );
//								uint64_t ClusterLocalVideoDuration = ClusterDuration( vb, DefaultVideoFrameDuration, VideoTrackNo );
//								uint64_t ClusterLocalAudioDuration = ClusterDuration( vb, DefaultAudioFrameDuration, AudioTrackNo );
//								uint64_t ClusterLocalDuration = min(ClusterLocalVideoDuration,ClusterLocalAudioDuration);
//
//								short ClusterMinimumTimeElement = ClusterMinimumBlockTime( vb, VideoTrackNo );
//								Generate_CueTimeCodes( vb, CueTCodes,	//Add KeyFrames to Cues Tree
//														ClusterPosition, LastClusterTimeCode, DefaultVideoFrameDuration, VideoTrackNo );
//
//								//uintElement *tc = dynamic_cast<uintElement*>( Get( newCluster_ptr, IDof("Timecode") ));
//								//tc->data = rint(LastClusterTimeCode / 1000000.0);
//
//								cout << "Recovered " << vb.size() << " blocks as new cluster." << endl;
//								cout << "Cluster " << cnt++ << " Found on byte "<< read_from << " Duration " << rint(ClusterLocalDuration / 1000000.0)
//									<< " Timecode(*): " << rint(LastClusterTimeCode / 1000000.0) << " Total " << vb.size() << " Blocks"<< endl;
//
//								LastClusterTimeCode += ClusterLocalDuration;
//								TreeParserWriteMaster( newCluster_ptr, outfile );
//								delete newCluster_ptr;
//
//								}
							cout << "\rSearch found _Cluster_ ID at " << read_from <<  endl;
							}
						}
					break;
					}

				else if( buffer[j] == (char)0xA0 and 0){//BlockGroup!
					unsigned x = 1;
					uint64_t sz = EBMLtouInteger( buffer+j+x , &x);
					//int sz = EBMLtouInteger( buffer+j , &j);
					if( j+sz+x < readed ){	//potential blockGroup compleetely in the buffer...
						ID_Element* processor = FindElementID( GlobalDB->Cluster, buffer+j+x);	// Verifies there was a token after this.
						if( processor not_eq NULL ){
							Block bl = BlockGroupAnalysis( buffer+j , GetTrackCount(root) );
							if( bl.Valid ){
								cout << "BlockGroup ID found on " << read_from+j << " Size:" << sz << endl;
								if( newCluster_ptr == NULL ){
									newCluster_ptr = new subElement( "Cluster" );
									uintElement* TimeCode_ptr = new uintElement( ID_Element( "Timecode" ) );
									TimeCode_ptr->data = rint(LastClusterTimeCode / 1000000.0);
									newCluster_ptr->data.push_back( TimeCode_ptr );
									}
								TreeParserChunk( buffer+j, sz+x, newCluster_ptr );
								j += sz+x-1;	//-1 for j++ at for loop
								}
							}
						}
					else if( readed - j < 1024*1024 ){//1MB
						read_from += j-1;
						break;
						}
					}
				else if( buffer[j] == (char)0xA3  and 0){//SimpleBlock!
					j++;
					uint64_t sz = EBMLtouInteger( buffer+j , &j);
					if( j+sz < readed ){
						ID_Element* processor = FindElementID( GlobalDB->Cluster, buffer+j+sz);
						if( processor not_eq NULL and
							( processor->ID == IDof("SimpleBlock") or
							  processor->ID == IDof("Cluster") or
							  processor->ID == IDof("BlockGroup")
							) )
							cout << "SimpleBlock ID found!" << endl;
						}
					else
						cout << "SimpleBlock Test Case overflows block size!" << endl;
					}

				else if( !(j % (1024*1024) ))
					cout << "Searching at: " << ((read_from + j)/(1024*1024)) << " MB\r";
				else if( j == bfr_size - 5 ){
					read_from += j;
					break;
					}
				}
			}

		//service routines
		if( not update_gauge( read_from*100/filesize ) ){
			return false;
			}
		}
	delete buffer;
	Duration = LastClusterTimeCode;
	return true;
}

subElement* Meteorite::Regenerate_SeekHead( vector <uint64_t> seeks ){
	subElement *NewSeekHead = new subElement( "SeekHead" );
	for( vector <uint64_t>::iterator it = seeks.begin() ; it != seeks.end() ; it ++ ){
		subElement *Seek = new subElement( "Seek" );
		binElement *SeekID = new binElement( ID_Element( "SeekID" ) );
		SeekID->data = new char[4];
		SeekID->data[0] = 0x1F;
		SeekID->data[1] = 0x43;
		SeekID->data[2] = static_cast<char>(0xB6);
		SeekID->data[3] = 0x75;
		SeekID->datasize =4;
		uintElement *SeekPosition = new uintElement( ID_Element( "SeekPosition" ) );
		SeekPosition->data = *it;

		Seek->data.push_back(SeekID);
		Seek->data.push_back(SeekPosition);
		NewSeekHead->data.push_back(Seek);
		//int a = ByteCount( *it );
		//cout << "Seek Size: " << 10+a << "\tSeekID: 1F 43 B6 75 = Cluster\tSeekPosition: " << *it << endl;
		}
return NewSeekHead;
}
void Meteorite::Repair_Segment_Size( subElement* root, ifstream& infile, ofstream& outfile ){
	uint64_t SegmentSize = outfile.tellp();
	cout << "Segment size : " << SegmentSize << endl;
	if( root->data.at(0)->name not_eq "EBMLFile" ){
		cout << "No EBML at indice 0" << endl;
		return;
		}
	int ebmlend = root->data.at(0)->Size();
//	TreeParserPrint( dynamic_cast<subElement*>(root->data.at(0)) );
	TreeParserPrint( root );

	infile.clear();
	infile.seekg( ebmlend+4 );
	outfile.seekp( ebmlend+4 );
	char* bfr = new char [17];
	infile.read( bfr, 16 );

	unsigned i = 0;
	EBMLtouInteger( bfr, &i );
	SegmentSize -= ebmlend + 4 + i;
	cout << "Segment EBMLsize = " << i << endl;
//	switch(i){
//		case 1: zf = SegmentSize | 0x80;break;
//		case 2: zf = SegmentSize | 0x4000;break;
//		case 3: zf = SegmentSize | 0x200000;break;
//		case 4: zf = SegmentSize | 0x10000000;break;
//		case 5: zf = SegmentSize | 0x0800000000;break;
//		case 6: zf = SegmentSize | 0x040000000000;break;
//		case 7: zf = SegmentSize | 0x02000000000000;break;
//		case 8: zf = SegmentSize | 0x0100000000000000;break;
//		}
	uint64_t zf = 1;
	for( int j=0 ; j < i ; j++)
		zf *=  0x80;
	zf or_eq SegmentSize;

	outfile.write( reinterpret_cast<char*>(&to_bigendianWR(zf)), i);	//writes segment size
}
void Meteorite::Repair_Info_Duration( subElement* Segment_ptr, uint64_t Duration ){
	vector<uint32_t> a;
	a.push_back(IDof("Info"));
	a.push_back(IDof("Duration"));
	floatElement *Duration_ptr= dynamic_cast< floatElement* >(Get( Segment_ptr, a ));
	a.clear();
	Duration_ptr->data = Duration/1000000.0;
// TODO (death#1#): 1M is error!
	a.push_back(IDof("Info"));
	a.push_back(IDof("WritingApp"));
	stringElement *WritingApp_ptr= dynamic_cast< stringElement* >(Get( Segment_ptr, a ));
	a.clear();
	WritingApp_ptr->data = string("Meteorite ")+string(METEORITE_VERSION);


	a.push_back(IDof("Info"));
	a.push_back(IDof("MuxingApp"));
	stringElement *MuxingApp_ptr= dynamic_cast< stringElement* >(Get( Segment_ptr, a ));
	a.clear();
	MuxingApp_ptr->data = string("Meteorite ")+string(METEORITE_VERSION);


	}
subElement* Meteorite::Regenerate_MetaSeek( subElement* Segment_ptr, uint64_t SegmentStart, uint64_t NewSeekHeadPos, uint64_t NewCueTimePos ){
	subElement *MetaSeek_ptr= dynamic_cast< subElement* >(Get( Segment_ptr, IDof("SeekHead") ));

	MetaSeek_ptr->data.clear();
	subElement *Seek_ptr;
	for( unsigned i = 1 ; i < Segment_ptr->data.size() ; i++ ){
		Seek_ptr = new subElement( "Seek" );
		binElement *SeekID_ptr =  new binElement( ID_Element( "SeekID" ) );
		uintElement *SeekPosition_ptr = new uintElement( ID_Element( "SeekPosition" ) );
		SeekID_ptr->data = new char [4];
		memcpy( SeekID_ptr->data, &make_bigendian((Segment_ptr->data.at(i)->ID)), 4 );
		SeekID_ptr->datasize =4;
		int sz = 200; //assuming metaseek table is max of 200 bytes.
		for( unsigned j = 1 ; j < i ; j++ ){
			cout << "Segment_ptr->data.at(" << j << ")->name : " << Segment_ptr->data.at(j)->name << endl;
			cout << "Segment_ptr->data.at(" << j << ")->size() : " << Segment_ptr->data.at(j)->Size() << endl;
			sz += Segment_ptr->data.at(j)->Size();
			}
		SeekPosition_ptr->data = sz;
		Seek_ptr->data.push_back( SeekID_ptr );
		Seek_ptr->data.push_back( SeekPosition_ptr );
		MetaSeek_ptr->data.push_back( Seek_ptr );
		}

	Seek_ptr = new subElement( "Seek" );
	binElement *SeekID_ptr =  new binElement( ID_Element( "SeekID" ) );
	uintElement *SeekPosition_ptr = new uintElement( ID_Element( "SeekPosition" ) );
	SeekID_ptr->data = new char [4];
	memcpy( SeekID_ptr->data, &make_bigendian( IDof("SeekHead") ), 4 );
	SeekID_ptr->datasize =4;
	SeekPosition_ptr->data = NewSeekHeadPos - SegmentStart;
	Seek_ptr->data.push_back( SeekID_ptr );
	Seek_ptr->data.push_back( SeekPosition_ptr );
	MetaSeek_ptr->data.push_back( Seek_ptr );

	Seek_ptr = new subElement( "Seek" );
	SeekID_ptr =  new binElement( ID_Element( "SeekID" ) );
	SeekPosition_ptr = new uintElement( ID_Element( "SeekPosition" ) );
	SeekID_ptr->data = new char [4];
	memcpy( SeekID_ptr->data, &make_bigendian( IDof("Cues") ), 4 );
	SeekID_ptr->datasize =4;
	SeekPosition_ptr->data = NewCueTimePos - SegmentStart;
	Seek_ptr->data.push_back( SeekID_ptr );
	Seek_ptr->data.push_back( SeekPosition_ptr );
	MetaSeek_ptr->data.push_back( Seek_ptr );
	return MetaSeek_ptr;
	}
void Meteorite::Adjust_Void_Sections( subElement *Segment_ptr, uint64_t SegmentStart, uint64_t ClusterStart, ofstream& outfile ){
	if ( Segment_ptr->name not_eq "Segment" ){
		cout << "Segment pointer is not points actual segment!";
		return;
		}
	subElement* MetaSeek_ptr = dynamic_cast< subElement*>( Get( Segment_ptr , IDof("SeekHead") ) );
	///Add void to metaseek
	int voidsize = 200 - MetaSeek_ptr->Size()-1;	//-1 void id
	if( voidsize > 127 )
		voidsize-=2;	//-2 for size
	else
		voidsize--;

	binElement* void_ptr = new binElement( ID_Element( "Void" ) );
	void_ptr->data = new char[ voidsize ];
	for( int i = 0 ; i < voidsize ; i++ )
		void_ptr->data[i]=0;

	void_ptr->datasize = voidsize;

	vector< ID_Element*>::iterator iter;
	iter = Segment_ptr->data.begin();
	iter++;
	Segment_ptr->data.insert( iter, void_ptr );

	///Add last void before clusters
	int areasize = ClusterStart - SegmentStart;	//Segment start to -> first cluster size

	voidsize = areasize - Segment_ptr->subsize() - 1;

	int q = EBMLsize(uIntegertoEBML( voidsize ));
	voidsize -=  q;
	int w = EBMLsize(uIntegertoEBML( voidsize ));
	if(q not_eq w){
		cerr << "FATAL error on calculating void size" << endl;
		return;
		}

	binElement* void2_ptr = new binElement( ID_Element("Void") );
	void2_ptr->data = new char[voidsize];
	for( int i = 0 ; i < voidsize ; i++ )
		void2_ptr->data[i]=0;

	void2_ptr->datasize = voidsize;

//				dynamic_cast<subElement*>(root->data.at(1))->data.push_back(); // segment++ void
	Segment_ptr->data.push_back( void2_ptr );
	cout << endl << endl << endl;
	TreeParserPrint( Segment_ptr );

	outfile.seekp( SegmentStart );
	TreeParserWrite( Segment_ptr, outfile );
	}
