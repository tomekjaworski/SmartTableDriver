#ifndef TABLEGROUP_HPP
#define TABLEGROUP_HPP


class TableGroup : public std::vector<TableDevice::Ptr> {
public:
	typedef std::shared_ptr<TableGroup> Ptr;
	
private:
	SerialPort::Ptr sp;
	std::vector<TableDevice::Ptr> devices;
	
public:

	SerialPort::Ptr& getSerialPort(void) { return this->sp; }
	const SerialPort::Ptr& getSerialPort(void) const { return this->sp; }

	TableGroup()
		: sp(nullptr)
	{
		//
	}
	
	TableGroup(SerialPort::Ptr& sp)
		: sp(sp)
	{
		//
	}
	
	TableDevice::Ptr findByAddress(device_address_t addr)
	{
		auto it = std::find_if(devices.begin(), devices.end(), [addr](const TableDevice::Ptr& pdev) { return pdev->getAddress() == addr; });
		if (it == devices.end())
			return nullptr;
			
		return *it;
	}
	
	void addTableDevice(TableDevice::Ptr& pdev)
	{
		this->devices.push_back(pdev);
	}
	
	std::vector<TableDevice::Ptr>::iterator begin(void) { return this->devices.begin(); }
	std::vector<TableDevice::Ptr>::iterator end(void) { return this->devices.end(); }
	std::vector<TableDevice::Ptr>::const_iterator begin(void) const { return this->devices.begin(); }
	std::vector<TableDevice::Ptr>::const_iterator end(void) const { return this->devices.end(); }
	
};

#endif // TABLEGROUP_HPP
