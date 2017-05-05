#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <sstream>

class Location {
	int row;
	int col;
	
public:	
	int getRow(void) const { return this->row; }
	int getColumn(void) const { return this->col; }
	
public:
	Location(const Location& loc)
		: row(loc.row), col(loc.col)
	{
		
	}
	
	Location(int col, int row)
		: row(row), col(col)
	{
		
	}
	
	std::string toString(void) const
	{
		return ((std::ostringstream&)(std::ostringstream() << "[X=" << this->col << "; Y=" << this->row << "]")).str();
	}
};

#endif // LOCATION_HPP
