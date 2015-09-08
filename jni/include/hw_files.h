#ifndef hw_files_include_h
#define hw_files_include_h

#include <vector>
#include <string>
#include "hw_server.h"
#include "hw_net_base.h"

class hw_files
{
public:
	enum{
		FILE_ALL = 0,
		FILE_NORMAL = 1,
		FILE_MOTION = 2
	};
	hw_files(int slot,SYSTEMTIME beg,SYSTEMTIME end,int type);
	virtual ~hw_files();

	virtual bool refresh() = 0;
	int count();
	bool get_file(int index,rec_file_t& file);

protected:
	int m_slot;
	SYSTEMTIME m_beg;
	SYSTEMTIME m_end;
	int m_type;
	std::vector<rec_file_t> m_files;
};

class hw_net_files
	: public hw_files
	  ,public hw_net_base
{
public:
	hw_net_files(server_ref server, int slot,SYSTEMTIME beg,SYSTEMTIME end,int type = hw_files::FILE_ALL);
	virtual ~hw_net_files();
	
	bool refresh();

protected:
	bool on_protocol_come(hw_msg_ptr& msg);
	bool wait_file_finsished(int timeout = 50000);

protected:
	bool m_file_finished;
	server_ref m_server;
};

class hw_net_smart_files
	: public hw_net_files
{
public:
	hw_net_smart_files(server_ref server,int slot,SYSTEMTIME beg,SYSTEMTIME end,RECT search_rt);
	~hw_net_smart_files();

	bool refresh();

private:
	RECT m_search_rt;
};

class hw_net_files_page
: public hw_net_files
{
public:
	hw_net_files_page(server_ref server, int slot,int sream,SYSTEMTIME beg,SYSTEMTIME end,int order_by_time,int time_type, Pagination* page_info,int type = hw_files::FILE_ALL);
	virtual ~hw_net_files_page();

	bool refresh();

    Pagination get_page_info();

protected:
	bool on_protocol_come(hw_msg_ptr& msg);
    int m_stream;
    int m_order_by_time;
    int m_time_type;
    Pagination m_page_info;
    Pagination m_page_ret;
    bool m_is_get_page_ret;
};
typedef boost::shared_ptr<hw_files> file_list_ref;

#endif

