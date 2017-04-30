#ifndef TABLEGROUP_HPP
#define TABLEGROUP_HPP


class TableGroup : public std::vector<TableDevice::Ptr> {
public:
	typedef std::shared_ptr<TableGroup> Ptr;
	
private:
	SerialPort::Ptr sp;
	MessageReceiver mr;

	std::vector<TableDevice::Ptr> devices;
	std::vector<Message> messages;
	
public:

	SerialPort::Ptr& getSerialPort(void) { return this->sp; }
	const SerialPort::Ptr& getSerialPort(void) const { return this->sp; }

	MessageReceiver& getReceiver(void) { return this->mr; }
	const MessageReceiver& getReceiver(void) const { return this->mr; }

	TableGroup()
		: sp(nullptr)
	{
		messages.reserve(32);
		
	}
	
	TableGroup(SerialPort::Ptr& sp)
		: sp(sp)
	{
		messages.reserve(32);
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
	
	void addMessageToQueue(Message& msg)
	{
		this->messages.push_back(msg);
	}
	
	size_t getDeviceCount(void) const { return this->devices.size(); }
	
	std::vector<TableDevice::Ptr>::iterator begin(void) { return this->devices.begin(); }
	std::vector<TableDevice::Ptr>::iterator end(void) { return this->devices.end(); }
	std::vector<TableDevice::Ptr>::const_iterator begin(void) const { return this->devices.begin(); }
	std::vector<TableDevice::Ptr>::const_iterator end(void) const { return this->devices.end(); }
	
};

#endif // TABLEGROUP_HPP
