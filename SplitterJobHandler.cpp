#include <vector>
#include "SplitterJobHandler.h"

void SplitterJobHandler::work(std::vector <SplitterOpts> &jobs) {
    int completedCount = 0;
    for (auto& job : jobs) {
        bool inPathOK = ssio.setInPath(job.inDirectory, job.isPNGInDirectory, job.recursive);
        bool outPathOK = ssio.setOutPath(job.outDirectory);
        // only work the job if the input is OK. The only input error that can happen from now on is a png decode error.
        if (! (inPathOK && outPathOK)) continue;

        for (int i = 0; i < job.workAmount; ++i) {

        }
    }



}
