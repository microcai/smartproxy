/**
 * splice.hpp , implements the splice syntactics.
 */

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
namespace asio = boost::asio;

namespace avsocks{

#define ASIO_READ_PLACEHOLDERS asio::placeholders::error,asio::placeholders::bytes_transferred

#define ASIO_WRITE_PLACEHOLDERS asio::placeholders::error,asio::placeholders::bytes_transferred

#define SPLICE_SIZE 1024*1024*64

template < class T , class S1, class S2>
class splice : public boost::enable_shared_from_this<splice<T,S1,S2> >{
public:
	using boost::enable_shared_from_this<splice<T,S1,S2> >::shared_from_this;
	typedef boost::shared_ptr<splice>	pointer;

	splice(boost::shared_ptr<T> _owner, S1 _s1, S2&& _s2)
		:s1(_s1),s2(std::move(_s2)),owner(_owner){}

	void start(){
		s1.async_read_some(s1s2buf.prepare(SPLICE_SIZE),
			boost::bind(&splice<T,S1,S2>::s1s2_handle_read,shared_from_this(),ASIO_READ_PLACEHOLDERS)
		);
		s2.async_read_some(s2s1buf.prepare(SPLICE_SIZE),
			boost::bind(&splice<T,S1,S2>::s2s1_handle_read,shared_from_this(),ASIO_READ_PLACEHOLDERS)
		);
	}

	~splice(){

	}
private:
	void s1s2_handle_read(const boost::system::error_code & ec, std::size_t bytes_transferred){
		if(!ec){
			s1s2buf.commit(bytes_transferred);
			boost::asio::async_write(s2, s1s2buf, boost::asio::transfer_all(),
				boost::bind(&splice<T,S1,S2>::s1s2_handle_write,shared_from_this(),ASIO_WRITE_PLACEHOLDERS)
			);
		}
		else if (ec == boost::asio::error::eof){
			boost::system::error_code ec;
			s2.lowest_layer().shutdown(asio::socket_base::shutdown_both,ec);//->close();
		}
		else {
			boost::system::error_code ec;
			s2.close(ec);
		}
	}
	void s1s2_handle_write(const boost::system::error_code & ec, std::size_t bytes_transferred){
		if(!ec){
			s1s2buf.consume(bytes_transferred);

			std::cerr << "upload " << bytes_transferred << " bytes to upstream\n";
			s1.async_read_some(s1s2buf.prepare(SPLICE_SIZE),
				boost::bind(&splice<T,S1,S2>::s1s2_handle_read,shared_from_this(),ASIO_READ_PLACEHOLDERS)
			);
		}
	}
	void s2s1_handle_read(const boost::system::error_code & ec, std::size_t bytes_transferred){
		if(!ec){
			s2s1buf.commit(bytes_transferred);
			s1.async_write_some(s2s1buf.data(),
				boost::bind(&splice<T,S1,S2>::s2s1_handle_write,shared_from_this(),ASIO_WRITE_PLACEHOLDERS)
			);
		}else if (ec == boost::asio::error::eof){
			boost::system::error_code ec;
			s1.lowest_layer().shutdown(asio::socket_base::shutdown_both,ec);
		}else{
			boost::system::error_code ec;
			s1.close(ec);
		}
	}
	void s2s1_handle_write(const boost::system::error_code & ec, std::size_t bytes_transferred){
		if(!ec){
			s2s1buf.consume(bytes_transferred);
			s2.async_read_some(s2s1buf.prepare(SPLICE_SIZE),
				boost::bind(&splice<T,S1,S2>::s2s1_handle_read,shared_from_this(),ASIO_READ_PLACEHOLDERS)
			);
		}else{
			boost::system::error_code ec;
			s1.lowest_layer().shutdown(asio::socket_base::shutdown_both,ec);
		}
	}
private:
	asio::streambuf	s1s2buf,s2s1buf;
	S1						s1; //两个 socket
	S2						s2; //两个 socket
	boost::shared_ptr<T>	owner;
};

#undef  ASIO_READ_PLACEHOLDERS
#undef  ASIO_WRITE_PLACEHOLDERS

} // namespace avsocks.