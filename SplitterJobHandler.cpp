#include <vector>
#include "SplitterJobHandler.h"

void SplitterJobHandler::Work(std::vector <SplitterOpts> &jobs) {
    for (auto& job : jobs) {
        std::cout << "indir: " << job.inDirectory << "\n";
        loader.setNewPath(job.inDirectory);
    }



}
