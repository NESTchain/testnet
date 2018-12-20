/*
* Copyright (c) 2015 Cryptonomex, Inc., and contributors.
* Copyright (c) 2018- ¦ÌNEST Foundation, and contributors.
*
* The MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <graphene/app/sync_ntp.hpp>
#include <fc/network/udp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/thread/thread.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <sstream>
#include <iomanip>

#ifdef linux
#include <sys/time.h>
#endif

namespace graphene { namespace app {

#pragma pack(1)
	struct ntpPacket {
		uint8_t  flags;
		uint8_t  stratum;
		uint8_t  poll;
		uint8_t  precision;
		uint32_t root_delay;
		uint32_t root_dispersion;
		uint8_t  referenceID[4];
		uint32_t ref_ts_sec;
		uint32_t ref_ts_frac;
		uint32_t origin_ts_sec;
		uint32_t origin_ts_frac;
		uint32_t recv_ts_sec;
		uint32_t recv_ts_frac;
		uint32_t trans_ts_sec;
		uint32_t trans_ts_frac;
	};
#pragma pack()

	std::vector<std::string> sync_ntp_manager::ntp_server_vec
	{
		/* aliyun
		ntp1.aliyun.com  time1.aliyun.com
		ntp2.aliyun.com  time2.aliyun.com
		ntp3.aliyun.com  time3.aliyun.com
		ntp4.aliyun.com  time4.aliyun.com
		ntp5.aliyun.com  time5.aliyun.com
		ntp6.aliyun.com  time6.aliyun.com
		ntp7.aliyun.com  time7.aliyun.com
		ntp.aliyun.com
		*/
		"203.107.6.88",
		"120.25.115.19",
		"120.25.115.20",
		"120.25.108.11",
		"120.24.166.46", //ntp-sz.chl.la

		/* tencent
		time1.cloud.tencent.com
		time2.cloud.tencent.com
		time3.cloud.tencent.com
		time4.cloud.tencent.com
		time5.cloud.tencent.com
		*/
		"139.199.215.251",
		"111.230.189.174",
		"139.199.214.202",
		"134.175.254.134",
		"134.175.253.104",

		/* north-east university */
		"202.118.1.47",  //ntp.neu.edu.cn  ntp2.neu.edu.cn
		"202.118.1.48",  //ntp3.neu.edu.cn
		//"202.118.1.130", //ntp.neu6.edu.cn
		//"202.118.1.81",  //clock.neu.edu.cn
		//"202.112.31.197", //ntp.synet.edu.cn
		//"202.112.29.82",  //s2f.time.edu.cn

		/* apple
		time0.apple.com
		time1.apple.com
		time2.apple.com
		time3.apple.com
		*/
		"17.254.0.26",
		"17.254.0.27",
		"17.254.0.28",
		"17.254.0.31",
		"17.253.68.251" //time.apple.com
	};

	void sync_ntp_manager::startup()
	{
		sync_ntp_loop();
	}

	void sync_ntp_manager::sync_ntp_loop()
	{
		fc::time_point now = fc::time_point::now();
		fc::time_point next_wakeup(now + fc::minutes(10));
		fc::schedule([this]{ sync_ntp_run(); }, next_wakeup, "Get NTP Server Time");
	}

	void sync_ntp_manager::sync_ntp_run()
	{
		fc::async([this](){ get_ntp_server_time(); });
		sync_ntp_loop();
	}

	void sync_ntp_manager::get_ntp_server_time()
	{
		fc::udp_socket sock;
		sock.open();

		size_t index = get_random_index();
		std::string ntp_server_ip = ntp_server_vec[index];

		//ntp_server_ip = "0.0.0.123";       // test for sending error
		//ntp_server_ip = "123.123.123.123"; // test for receiving error

		std::string ntp_server_ip_port = ntp_server_ip + ":123";
		fc::ip::endpoint ntp_server = fc::ip::endpoint::from_string(ntp_server_ip_port);

		ntpPacket req_data{0}, recv_data{0};
		req_data.flags = 0xe3;

		int64_t client_send_time = fc::time_point::now().time_since_epoch().count();

		try
		{
			sock.send_to((char*)&req_data, sizeof(req_data), ntp_server);
		}
		catch (...)
		{
			elog("Error occurs when sending NTP packet to index ${index} server ${server}",
				("index", index)("server", ntp_server_ip));
			return;
		}

		try
		{
			sock.receive_from((char*)&recv_data, sizeof(recv_data), ntp_server);
		}
		catch (...)
		{
			elog("Error occurs when receiving NTP packet from index ${index} server ${server}",
				("index", index)("server", ntp_server_ip));
			return;
		}

		int64_t client_recv_time = fc::time_point::now().time_since_epoch().count();

		sock.close();

		recv_data.root_delay = endian_swap(recv_data.root_delay);
		recv_data.root_dispersion = endian_swap(recv_data.root_dispersion);
		recv_data.ref_ts_sec = endian_swap(recv_data.ref_ts_sec);
		recv_data.ref_ts_frac = endian_swap(recv_data.ref_ts_frac);
		recv_data.origin_ts_sec = endian_swap(recv_data.origin_ts_sec);
		recv_data.origin_ts_frac = endian_swap(recv_data.origin_ts_frac);
		recv_data.recv_ts_sec = endian_swap(recv_data.recv_ts_sec);
		recv_data.recv_ts_frac = endian_swap(recv_data.recv_ts_frac);
		recv_data.trans_ts_sec = endian_swap(recv_data.trans_ts_sec);
		recv_data.trans_ts_frac = endian_swap(recv_data.trans_ts_frac);

		int64_t server_recv_sec  = recv_data.recv_ts_sec;
		int64_t server_trans_sec = recv_data.trans_ts_sec;
		int64_t server_recv_frac  = ((unsigned long long)recv_data.recv_ts_frac * 1000000 + (1LL << 31)) >> 32;
		int64_t server_trans_frac = ((unsigned long long)recv_data.trans_ts_frac * 1000000 + (1LL << 31)) >> 32;
		// server_delta uses microsecond
		int64_t server_delta = (server_trans_sec - server_recv_sec) * 1000000 + server_trans_frac - server_recv_frac;

		int64_t offset = (client_recv_time - client_send_time - server_delta) / 2;

		int64_t carry_bit = server_trans_frac + offset >= 1000000 ? 1 : 0;
		int64_t ntp_frac  = server_trans_frac + offset >= 1000000 ?
			                server_trans_frac + offset - 1000000 :
						    server_trans_frac + offset;
		int64_t ntp_sec = server_trans_sec + carry_bit - 2208988800L;

		//time_t recv_secs = recv_data.recv_ts_sec - 2208988800L;
		//tm* ntp_time = localtime(&recv_secs);

		tm* ntp_time = localtime(&ntp_sec);

		std::stringstream ss;
		ss << std::setfill('0') << std::right;
		ss << std::setw(4) << ntp_time->tm_year + 1900 << "-";
		ss << std::setw(2) << ntp_time->tm_mon + 1 << "-";
		ss << std::setw(2) << ntp_time->tm_mday << "T";
		ss << std::setw(2) << ntp_time->tm_hour << ":";
		ss << std::setw(2) << ntp_time->tm_min << ":";
		ss << std::setw(2) << ntp_time->tm_sec;

		ilog("Get NTP time from index ${index} server ${server} successfully: ${ntp_time} ${offset} ${local_time}",
			("index", index)("server", ntp_server_ip)("ntp_time", recv_data.trans_ts_sec)
			("offset", offset)
			("local_time", ss.str()));
		
		// update system time
#ifdef linux
		//tm _tm;
		//_tm.tm_year = ntp_time->tm_year;
		//_tm.tm_mon = ntp_time->tm_mon;
		//_tm.tm_mday = ntp_time->tm_mday;
		//_tm.tm_hour = ntp_time->tm_hour;
		//_tm.tm_min = ntp_time->tm_min;
		//_tm.tm_sec = ntp_time->tm_sec;

		timeval tv;
		//tv.tv_sec = mktime(ntp_time);
		tv.tv_sec  = ntp_sec;
		tv.tv_usec = ntp_frac;

		int error_code = settimeofday(&tv, (struct timezone*)0);
		switch (error_code)
		{
		case EFAULT:
			elog("One of tv or tz pointed outside the accessible address space");
			break;
		case EINVAL:
			elog("Timezone (or something else) is invalid");
			break;
		case EPERM:
			elog("The process has insufficient privilege to set system time");
			break;
		default:
			ilog("Update system time successfully");
			break;
		}
#endif // linux
	}

	uint32_t sync_ntp_manager::endian_swap(uint32_t data)
	{
		return ((data >> 24) |
			((data & 0x00ff0000) >> 8) |
			((data & 0x0000ff00) << 8) |
			((data & 0x000000ff) << 24));
	}

	size_t sync_ntp_manager::get_random_index()
	{
		boost::random::mt19937 random_num_gen(fc::time_point::now().sec_since_epoch());
		boost::random::uniform_int_distribution<> index_range(0, (int)ntp_server_vec.size()-1);
		return index_range(random_num_gen);
	}

} }
