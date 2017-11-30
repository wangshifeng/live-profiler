#include "./Cases/Profiler/TestProfiler.hpp"
#include "./Cases/Collectors/TestCpuSampleLinuxCollector.hpp"
#include "./Cases/Utils/TestLinuxProcessUtils.hpp"
#include "./Cases/Utils/TestLinuxPerfUtils.hpp"

namespace LiveProfilerTests {
	void testAll() {
		testProfiler();
		testCpuSampleLinuxCollector();
		testLinuxProcessUtils();
		testLinuxPerfUtils();
	}
}

int main() {
	LiveProfilerTests::testAll();
	return 0;
}
