#ifndef hw_head_include_h
#define hw_head_include_h
#include "his_net_socket.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class hw_msg
{
public:

	hw_msg();

	~hw_msg();

	int head_len();
	char* head();

	int body_len();
	char* body();
    bool set_body_len(int body_len);
    bool set_body_capability(int capability);
    int capability_len();

	bool set_body(const char* body,int len);

private:
    struct protocolHead m_head;
    int m_body_capability;
    int m_body_len;
    char* m_body;
};

typedef boost::shared_ptr<hw_msg> hw_msg_ptr;

#endif

