#include "utils/source_buffer.h"
#include "exceptions.h"
#include <fstream>
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;
using namespace mtpython::utils;

SourceBuffer::SourceBuffer(const string& source, mtpython::parse::SourceType src_type)
{
	load_source(source);
	this->src_type = src_type;
}

SourceBuffer::SourceBuffer(vector<char>::iterator first, vector<char>::iterator last) : buf(first, last), fname("<no file>")
{
	pos = 0;
}

int SourceBuffer::load_source(const string& source)
{	
	int idx = 0;

	buf.clear();
	
	char tmp = 0;
	line_offset.push_back(0);
	/* line 1 starts from offset 0 */
	line_offset.push_back(0);
	while (true) {
		tmp = 0;
		if (idx == source.length()) break;
		tmp = source[idx++];
		/* deal with CRLF */
		if (tmp == '\n' || tmp == '\r') {
			buf.push_back('\n');
			line_offset.push_back(buf.size());
		}
		while (tmp == '\n' || tmp == '\r') {
			if (idx == source.length()) break;
			tmp = source[idx++];
		}
		buf.push_back(tmp);
	}

	line_offset.push_back(buf.size());
	if (buf.back() != '\n') buf.push_back('\n');

	pos = 0;

	return 0;
}

int SourceBuffer::load_file(const string& filename)
{
	ifstream str;

	fname.assign(filename);

	str.open(filename.c_str());
	if (!str) {
		throw mtpython::FileNotFoundException(("unable to open file " + filename).c_str());
		return 1;
	}
	
	buf.clear();
	
	char tmp = 0;
	line_offset.push_back(0);
	/* line 1 starts from offset 0 */
	line_offset.push_back(0);
	while (true) {
		tmp = 0;
		if (str.eof()) break;
		str.read(&tmp, 1);
		/* deal with CRLF */
		if (tmp == '\n' || tmp == '\r') {
			buf.push_back('\n');
			line_offset.push_back(buf.size());
		}
		while (tmp == '\n' || tmp == '\r') {
			if (str.eof()) break;
			str.read(&tmp, 1);
		}
		if (tmp == 0) break;
		buf.push_back(tmp);
	}

	line_offset.push_back(buf.size());

	pos = 0;
	str.close();

	src_type = mtpython::parse::SourceType::ST_FILE_INPUT;
	
	return 0;
}
	
mtpython::parse::SourceType SourceBuffer::get_src_type()
{
	return src_type;
}

char SourceBuffer::read()
{
	if (eof()) return -1;
	char c = buf[pos++];
	if (c == '\0') return -1;
	return c;
}

int SourceBuffer::tell_pos()
{
	return pos;
}

void SourceBuffer::seek(int pos)
{
	this->pos = pos;
}

bool SourceBuffer::eof()
{
	return (pos == buf.size());
}

int SourceBuffer::size()
{
	return buf.size();
}

vector<char>::iterator SourceBuffer::begin()
{
	return buf.begin();
}

vector<char>::iterator SourceBuffer::end()
{
	return buf.end();
}

std::string SourceBuffer::get_line(int line_nr)
{
	vector<char>::iterator it;
	string ret = "";
	for (it = begin() + line_offset[line_nr]; it < begin() + line_offset[line_nr + 1]; it++)
		/* no trailing CRLF */
		if (*it != '\n' && *it !='\r') ret += *it;

	return ret;
}

void SourceBuffer::dump()
{
	copy(buf.begin(), buf.end(), ostream_iterator<char>(cout, ""));
}
