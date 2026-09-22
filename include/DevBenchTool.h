#pragma once

// DevBench tool "bpfc.control" (rules 31 and 64): the live state of the hold, and ops that DRIVE it -
// a Y tap or a Y hold queued into the input stream so it passes through this patch's own filter.
namespace DevBenchTool
{
	// Registers the tool. a_lastAttempt = true on the final retry (kDataLoaded), which logs a miss at info.
	void Init(bool a_lastAttempt);
}
