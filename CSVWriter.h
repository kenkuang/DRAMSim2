/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/
#ifndef _CSV_WRITER_H_
#define _CSV_WRITER_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>

using std::vector; 
using std::ostream;
/*
 * CSVWriter: Writes CSV data with headers to an underlying ofstream 
 * 	This wrapper is meant to look like an ofstream, but it captures 
 * 	the names of each field and prints it out to a header before printing
 * 	the CSV data below. 
 *
 * 	Note: the first finalize() will not print the values out, only the headers.
 * 	One way to fix this problem would be to use a sstringstream (or something) 
 * 	to buffer out the values and flush them all in one go instead of passing them 
 * 	directly to the underlying stream as is the case now. 
 *
 * 	Example usage: 
 *
 * 	CSVWriter sw(cout);               // send output to cout
 * 	sw <<"Bandwidth" << 0.5; // value ignored
 * 	sw <<"Latency" << 5;     // value ignored
 * 	sw.finalize();                      // flush the header 
 * 	sw <<"Bandwidth" << 1.5; // field name ignored
 * 	sw <<"Latency" << 15;     // field name ignored
 * 	sw.finalize(); 							// values printed to csv line
 * 	sw <<"Bandwidth" << 2.5; // field name ignored
 * 	sw <<"Latency" << 25;     // field name ignored
 * 	sw.finalize(); 							// values printed to csv line
 *
 * 	The output of this example will be: 
 *
 * 	Bandwidth,Latency
 * 	1.5,15
 * 	2.5,25
 *
 */
#define MAX_TMP_STR 64
// length of a two digit subscript: [xx]
#define SINGLE_INDEX_LEN 4 


namespace DRAMSim {
	// Single index
	static char *indexStr(const char *baseName, unsigned channel)
	{
		if (strlen(baseName)+SINGLE_INDEX_LEN > MAX_TMP_STR)
		{
			ERROR("Your string is too long for the stats, increase MAX_TMP_STR"); 
			abort(); 
		}
		char tmp_str[MAX_TMP_STR]; 
		snprintf(tmp_str, MAX_TMP_STR,"%s[%u]", baseName, channel); 
		return strndup(tmp_str, MAX_TMP_STR); 
	}

	// two indices 
	static char *indexStr(const char *baseName, unsigned channel, unsigned rank)
	{
		if (strlen(baseName)+(2*SINGLE_INDEX_LEN) > MAX_TMP_STR)
		{
			ERROR("Your string is too long for the stats, increase MAX_TMP_STR"); 
			abort(); 
		}
		char tmp_str[MAX_TMP_STR]; 
		snprintf(tmp_str, MAX_TMP_STR,"%s[%u][%u]", baseName, channel, rank); 
		return strndup(tmp_str, MAX_TMP_STR); 
	}

	// three indices	
	static char *indexStr(const char *baseName, unsigned channel, unsigned rank, unsigned bank)
	{
		if (strlen(baseName)+(3*SINGLE_INDEX_LEN) > MAX_TMP_STR)
		{
			ERROR("Your string is too long for the stats, increase MAX_TMP_STR"); 
			exit(-1); 
		}
		char tmp_str[MAX_TMP_STR]; 
		snprintf(tmp_str, MAX_TMP_STR,"%s[%u][%u][%u]", baseName, channel, rank, bank); 
		return strndup(tmp_str, MAX_TMP_STR); 
	}

	class CSVWriter {
		// where the output will eventually go 
		ostream &output; 
		vector<string> fieldNames; 
		bool finalized; 
		unsigned idx; 
		public: 

		// Functions
		void finalize()
		{
			//TODO: tag unlikely
			if (!finalized)
			{
				for (unsigned i=0; i<fieldNames.size(); i++)
				{
					output << fieldNames[i] << ",";
				}
				output << std::endl << std::flush;
				finalized=true; 
			}
			else
			{
				if (idx < fieldNames.size()) 
				{
					printf(" Number of fields doesn't match values (fields=%u, values=%u), check each value has a field name before it\n", idx, (unsigned)fieldNames.size());
				}
				idx=0; 
				output << std::endl; 
			}
		}

		// Constructor 
		CSVWriter(ostream &_output) : output(_output), finalized(false), idx(0)
		{}

		// Insertion operators for field names
		CSVWriter &operator<<(const char *name)
		{
			if (!finalized)
			{
				fieldNames.push_back(string(name));
			}
			return *this; 
		}

		CSVWriter &operator<<(const string &name)
		{
			if (!finalized)
			{
				fieldNames.push_back(string(name));
			}
			return *this; 
		}

		// Insertion operators for types 
		// All of the other types just need to pass through to the underlying
		// ofstream, so just write this small wrapper function to make the
		// whole thing less verbose
#define ADD_TYPE(T) \
		CSVWriter &operator<<(T value) \
		{                                \
			if (finalized)                \
			{                             \
				output << value <<",";     \
				idx++;                     \
			}                             \
			return *this;                 \
		}                      

	ADD_TYPE(int);
	ADD_TYPE(unsigned); 
	ADD_TYPE(long);
	ADD_TYPE(uint64_t);
	ADD_TYPE(float);
	ADD_TYPE(double);

	}; // class CSVWriter


} // namespace BOBSim

#endif // _CSV_WRITER_H_
