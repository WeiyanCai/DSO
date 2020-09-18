//
// Created by caiweiyan on 18-11-20.
//

#pragma once

#include <chrono>
#include <string>
#include <cinttypes>

namespace dso {
	class PerfMonitor {
	public:
		explicit PerfMonitor(std::string&& hint)
				: hint_(hint),
				  begin_(std::chrono::steady_clock::now()),
				  last_(begin_),
				  stopped_ {false} {}

		void clock() {
			auto now = std::chrono::steady_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last_);

			printf("[%s] : %" PRId64 " us\n", hint_.c_str(), duration.count());

			last_ = now;
		}

		void clock(std::string const& message) {
			auto now = std::chrono::steady_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last_);

			printf("[%s] %s : %" PRId64 " us\n", hint_.c_str(), message.c_str(), duration.count());

			last_ = now;
		}

		void stopwatch() {
			if(stopped_)
				return;

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::steady_clock::now() - begin_);

			printf("[%s] : %" PRId64 " us\n", hint_.c_str(), duration.count());

			stopped_ = true;
		}

		~PerfMonitor() {
			stopwatch();
		}

	private:
		std::string                           hint_;
		std::chrono::steady_clock::time_point begin_;
		std::chrono::steady_clock::time_point last_;
		bool                                  stopped_;
	};
}