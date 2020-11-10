#pragma once

#include <algorithm>
#include <atomic>

template<typename T>
class LockFreeRingbuf {
public:
	LockFreeRingbuf() = delete;
	explicit LockFreeRingbuf(int size)
	: buffer(size, 0) { }
	void resize(int size) {
		buffer.resize(size);
		std::fill(buffer.begin(), buffer.end(), 0);
	}
	bool read(T& res_ref) {
		if (read_index == write_index) {
			return false;
		}
		res_ref    = buffer[read_index];
		read_index = stepIndex(read_index, 1);
		return true;
	}
	bool write(T elem) {
		write_index = stepIndex(write_index, 1);
		if (read_index == write_index) {
			return false;
		}
		buffer[write_index] = elem;
		return true;
	}
	// read from read_index to specified size. note that size should be length of
	// array, not a byte size
	bool readRange(std::vector<T>& res_array, size_t size) {
		if (getReadMargin() <= size) {
			return false;
		}
		size_t localread           = read_index;
		size_t target_index        = localread + size;
		size_t target_index_ringed = (target_index % buffer.size());
		if (target_index > buffer.size()) {
			auto&& iter = std::copy(buffer.begin() + localread, buffer.end(), res_array.begin());
			std::copy(buffer.begin(), buffer.begin() + target_index_ringed, iter);
		}
		else {
			std::copy_n(buffer.begin() + localread, size, res_array.begin());
		}
		read_index = target_index_ringed;
		return true;
	}
	bool writeRange(const std::vector<T>& src_array, size_t size) {
		if (getWriteMargin() <= size) {
			return false;
		}
		size_t localwrite          = write_index;
		size_t target_index        = localwrite + size;
		size_t target_index_ringed = (target_index % buffer.size());
		if (target_index > buffer.size()) {
			size_t writetoend = (buffer.size() - localwrite);
			auto&& iter       = std::copy_n(src_array.begin(), buffer.size() - localwrite, buffer.begin() + localwrite);
			std::copy_n(src_array.begin() + writetoend, target_index_ringed, buffer.begin());
		}
		else {
			std::copy_n(src_array.begin(), size, buffer.begin() + localwrite);
		}
		write_index = target_index_ringed;
		return true;
	}
	int getReadMargin() {
		size_t localread  = read_index;
		size_t localwrite = write_index;

		auto margin = localwrite - localread;
		if (margin < 0) {
			margin = buffer.size() - (localread - localwrite);
		}
		return margin;
	}
	int getWriteMargin() {
		return buffer.size() - getReadMargin();
	}

private:
	std::atomic<size_t> write_index = 0;
	std::atomic<size_t> read_index  = 0;
	std::vector<T>      buffer;
	size_t              stepIndex(size_t i, int increment) {
        return (i + increment) % buffer.size();
	}
};