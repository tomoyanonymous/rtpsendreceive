#pragma once

#include <atomic>
#include <algorithm>

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
	// read from read_index to specified size. note that size should be length of array, not a byte size
	bool readRange(std::vector<T>& res_array, size_t size) {
		if (getReadMargin() < size) {
			return false;
		}
		size_t target_index        = read_index + size;
		size_t target_index_ringed = (target_index % buffer.size());
		if (target_index > buffer.size()) {
            auto&& iter = std::copy(buffer.begin()+read_index,buffer.end(),res_array.begin());
            std::copy(buffer.begin(),buffer.begin()+target_index_ringed,iter);
		}
		else {
            std::copy_n(buffer.begin()+read_index,size,res_array.begin());
		}
		read_index = target_index_ringed;
		return true;
	}
	bool writeRange(const std::vector<T>& src_array, size_t size) {
		if (getWriteMargin() < size) {
			return false;
		}
		size_t target_index        = write_index + size;
		size_t target_index_ringed = (target_index % buffer.size());
		if (target_index > buffer.size()) {
			size_t writetoend = (buffer.size() - write_index);
            auto&& iter = std::copy_n(src_array.begin(),buffer.size() - write_index,buffer.begin()+write_index);
            std::copy_n(src_array.begin()+writetoend,target_index_ringed,buffer.begin());
		}
		else {
            std::copy_n(src_array.begin(),size,buffer.begin()+write_index);
		}
		write_index = target_index_ringed;
		return true;
	}
	int getReadMargin() {
        
        int margin = write_index - read_index;
        if(margin<0){
            margin = buffer.size()-(read_index-write_index);
        }
		return margin;
	}
	int getWriteMargin() {

		return buffer.size()-getReadMargin();
	}

private:
	std::atomic<size_t> write_index = 0;
	std::atomic<size_t> read_index  = 0;
	std::vector<T>      buffer;
	size_t              stepIndex(size_t i, int increment) {
        return (i + increment) % buffer.size();
	}
};