#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <sstream>

class Location {
	int row;
	int col;
	
public:	
	int GetRow(void) const { return this->row; }
	int GetColumn(void) const { return this->col; }
	
public:
    Location(void) : row(-1), col(-1) {
	    //
	}

	Location(const Location& loc)
		: row(loc.row), col(loc.col) {
		//
	}
	
	Location(int row, int col)
		: row(row), col(col) {
		//
	}
	
	std::string ToString(void) const {
		return ((std::ostringstream&)(std::ostringstream() << "[X=" << this->col << "; Y=" << this->row << "]")).str();
	}
};

#endif // LOCATION_HPP
