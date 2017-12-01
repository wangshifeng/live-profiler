#pragma once
#include <linux/perf_event.h>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include "BaseCollector.hpp"
#include "../Models/CpuSampleModel.hpp"
#include "../Utils/LinuxProcessUtils.hpp"
#include "../Utils/LinuxPerfEntryAllocator.hpp"
#include "../Utils/LinuxPerfUtils.hpp"

namespace LiveProfiler {
	/**
	 * Collector for collecting cpu samples on linux, based on perf_events
	 */
	class CpuSampleLinuxCollector : public BaseCollector<CpuSampleModel> {
	public:
		/**
		 * The default value pass to LinuxPerfEntryAllocator.
		 * The page 0 is metadata page(perf_event_mmap_page).
		 * The remaining pages are ring buffer.
		 */
		static const std::size_t DefaultMmapPageCount = 32;
		/** The default value set to perf_event_attr::sample_period */
		static const std::size_t DefaultSamplePeriod = 100000;

		/** Reset the state to it's initial state */
		void reset() override {
			// TODO
		}

		/** Enable performance data collection */
		void enable() override {
			// TODO
		}

		/** Collect performance data for the specified timeout period */
		const std::vector<CpuSampleModel>& collect(
			std::chrono::high_resolution_clock::duration) & override {
			// update the processes to monitor every specified interval
			auto now = std::chrono::high_resolution_clock::now();
			if (now - processesUpdated_ > processesUpdateInterval_) {
				LinuxProcessUtils::listProcesses(processes_, filter_);
				updatePerfEvents();
				processesUpdated_ = now;
			}
			// poll events
			// - read from mmap
			// - convert to model
			// TODO
			return result_;
		}

		/** Disable performance data collection */
		void disable() override {
			// TODO
		}

		/** Set how many pages for mmap ring buffer, the count contains metadata page */
		void setMmapPageCount(std::size_t mmapPageCount) {
			perfEntryAllocator_.setMmapPageCount(mmapPageCount);
		}

		/** Set how often to take a sample, the unit is cpu clock */
		void setSamplePeriod(std::size_t samplePeriod) {
			samplePeriod_ = samplePeriod;
		}

		/** Set how often to update the list of processes */
		template <class Rep, class Period>
		void setProcessesUpdateInterval(std::chrono::duration<Rep, Period> interval) {
			processesUpdateInterval_ = std::chrono::duration_cast<
				std::decay_t<decltype(processesUpdateInterval_)>>(interval);
		}

		/** Use the specified function to decide which processes to monitor */
		void filterProcessBy(const std::function<bool(pid_t)>& filter) {
			filter_ = filter;
		}

		/** Use the specified process name to decide which processes to monitor */
		void filterProcessByName(const std::string& name) {
			filterProcessBy(LinuxProcessUtils::getProcessFilterByName(name));
		}

		/** Constructor */
		CpuSampleLinuxCollector() :
			result_(),
			filter_(),
			processes_(),
			processesUpdated_(),
			processesUpdateInterval_(std::chrono::milliseconds(100)),
			pidToPerfEntry_(),
			perfEntryAllocator_(DefaultMmapPageCount),
			samplePeriod_(DefaultSamplePeriod) { }

	protected:
		/** Update the processes to monitor based on the latest list */
		void updatePerfEvents() {
			std::sort(processes_.begin(), processes_.end());
			// find out which processes newly created
			for (pid_t pid : processes_) {
				if (pidToPerfEntry_.find(pid) != pidToPerfEntry_.end()) {
					continue;
				}
				// monitor this process
				auto entry = perfEntryAllocator_.allocate();
				entry->setPid(pid);
				LinuxPerfUtils::monitorCpuSample(entry, samplePeriod_);
				pidToPerfEntry_.emplace(pid, std::move(entry));
			}
			// find out which processes no longer exist
		}

	protected:
		std::vector<CpuSampleModel> result_;
		std::function<bool(pid_t)> filter_;
		std::vector<pid_t> processes_;
		std::chrono::high_resolution_clock::time_point processesUpdated_;
		std::chrono::high_resolution_clock::duration processesUpdateInterval_;
		std::unordered_map<pid_t, std::unique_ptr<LinuxPerfEntry>> pidToPerfEntry_;
		LinuxPerfEntryAllocator perfEntryAllocator_;
		std::size_t samplePeriod_;
	};
}

