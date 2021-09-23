#include <vector>
#include "SplitterJobHandler.h"

void SplitterJobHandler::Work(std::vector <SplitterOpts> &jobs) {
    for (auto& job : jobs) {
        ssio.setNewPath(job.inDirectory);
    }



}
