//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.

#pragma once

#include <array>
#include <iostream>
#include <utility>
#include <vector>
#include <future>
#include <chrono>
#include <memory>


#ifndef THREAD_SLEEP
#define THREAD_SLEEP(s) std::this_thread::sleep_for(std::chrono::seconds(s));
#endif

namespace rtpsr {
	using sample_t = int16_t;

	inline double convertSampleToDouble(sample_t s) {
		return ((double)s) / INT16_MAX;
	}
	inline sample_t convertDoubleToSample(double d) {
		return (int16_t)(d * INT16_MAX);
	}

	struct buffer_data {
		sample_t* ptr;
		size_t    size;
	};

	using buffertype = std::vector<sample_t>;

	using readfn_type = int (*)(void*, uint8_t*, int);
	using seekfn_type = int64_t (*)(void*, int64_t, int);

	using duration_type = std::chrono::duration<double, std::ratio<1, 48000>>;

	enum class Codec { PCM_s16BE, OPUS, INVALID = -1 };


	template<class T>
	class AvSmartPtr {
	public:
		AvSmartPtr() {
			size_t s = sizeof(T);
			ptr      = av_malloc(s);
		}
		~AvSmartPtr() {
			av_free(ptr);
		}
		// do not copy
		T operator=(AvSmartPtr<T> i) = delete;

		T* get() {
			return reinterpret_cast<T*>(ptr);
		}
		T* operator->() {
			return get();
		}
		T& operator*() {
			return *get();
		}

	private:
		void* ptr;
	};


	inline std::string getCodecName(Codec c) {
		switch (c) {
			case Codec::PCM_s16BE:
				return "pcm_s16be";
				break;
			case Codec::OPUS:
				return "libopus";
				break;
			default:
				return "";
		}
	}
	inline size_t getCodecDataSize(Codec c) {
		if (c == Codec::PCM_s16BE) {
			return sizeof(int16_t);
		}
		return sizeof(double);
	}

	inline Codec getCodecByName(const std::string& name) {
		if (name == "pcm_s16be") {
			return Codec::PCM_s16BE;
		}
		if (name == "opus") {
			return Codec::OPUS;
		}
		return Codec::INVALID;
	}
}    // namespace rtpsr