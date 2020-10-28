
#include <array>
#include <cmath>
#include <future>
#include <iostream>
#include <sstream>
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS
#include "lockfree_ringbuffer.hpp"
#include <catch.hpp>    // required unit test header
TEST_CASE("Lockfree Ringbuffer") {
	const int                size = 512;
	LockFreeRingbuf<int16_t> ringbuf(size);

	const int try_size = 2000;

	constexpr int        chunk_write = 30;
	constexpr int        chunk_read  = 30;
	std::vector<int16_t> reference;
	std::vector<int16_t> answer;
	std::thread          th1 {[&]() {
        std::vector<int16_t> src(chunk_write, 0);
        int                  counter = 0;
        while (true) {
            for (int i = 0; i < chunk_write; i++) {
                int16_t num = counter;
                reference.push_back(num);
                src.at(i) = num;
                counter++;
            }
            while (true) {
                bool writeres = ringbuf.writeRange(src, chunk_write);
                if (writeres) {
                    break;
                }
            }
            if (counter > try_size + chunk_read) {
                break;
            }
        }
        return true;
    }};
	std::thread          th2 {[&]() {
        std::vector<int16_t> src(chunk_read, 0);
        int                  counter = 0;
        while (true) {
            bool readres = ringbuf.readRange(src, chunk_read);
            if (readres) {
                answer.insert(answer.end(), src.begin(), src.end());
                counter += chunk_read;
                if (counter > try_size) {
                    break;
                }
            }
        }
        return true;
    }};
	th1.join();
	th2.join();
	reference.resize(try_size);
	answer.resize(try_size);
	std::stringstream strstr;
	strstr << "different element: ";
	for (int i = 0; i < try_size; i++) {
		if (reference[i] != answer[i]) {
			strstr << i << ", ";
		}
	}
	strstr << std::endl;
	std::cerr << strstr.str() << std::endl;
	bool res = reference == answer;
	REQUIRE(res);
}